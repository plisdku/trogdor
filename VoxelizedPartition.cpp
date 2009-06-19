/*
 *  VoxelizedPartition.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "VoxelizedPartition.h"

#include "SimulationDescription.h"
#include "Log.h"
#include "YeeUtilities.h"
#include "MaterialBoss.h"
#include "STLOutput.h"

#include <sstream>

using namespace std;
using namespace YeeUtilities;

#pragma mark *** VoxelizedPartition ***

VoxelizedPartition::
VoxelizedPartition(const GridDescription & gridDesc, 
	const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
	Rect3i allocRegion, Rect3i calcRegion) :
	mVoxels(allocRegion, gridDesc.getHalfCellBounds(), 
		gridDesc.getNonPMLHalfCells()),
	mGridHalfCells(gridDesc.getHalfCellBounds()),
	mFieldAllocHalfCells(expandToYeeRect(allocRegion)),
	mAuxAllocRegion(allocRegion),
	mCalcHalfCells(calcRegion)
{
	LOG << "VoxelizedPartition()\n";
	
    m_nnx = mFieldAllocHalfCells.size(0)+1;
    m_nny = mFieldAllocHalfCells.size(1)+1;
    m_nnz = mFieldAllocHalfCells.size(2)+1;
    m_nnx0 = mFieldAllocHalfCells.p1[0];
    m_nny0 = mFieldAllocHalfCells.p1[1];
    m_nnz0 = mFieldAllocHalfCells.p1[2];
	m_nx = (mFieldAllocHalfCells.size(0)+1)/2;
	m_ny = (mFieldAllocHalfCells.size(1)+1)/2;
	m_nz = (mFieldAllocHalfCells.size(2)+1)/2;
	int bufSize = m_nx*m_ny*m_nz;
    
    mNumAllocHalfCells = mFieldAllocHalfCells.size()+1;
	
    initFieldBuffers(gridDesc.getName(), gridDesc.getHuygensSurfaces());
    
	mNonPMLHalfCells = gridDesc.getNonPMLHalfCells();
	mOriginYee = gridDesc.getOriginYee();
	
    LOG << "Partition geometry:\n";
	LOGMORE << "nonPML " << mNonPMLHalfCells << "\n";
	LOGMORE << "alloc region " << mAuxAllocRegion << "\n";
	LOGMORE << "full field alloc region (integer num Yee cells) "
		<< mFieldAllocHalfCells << "\n";
	LOGMORE << "calc " << mCalcHalfCells << "\n";
	LOGMORE << "origin " << mOriginYee << "\n";
	
	paintFromAssembly(gridDesc, voxelizedGrids);
	calculateHuygensSymmetries(gridDesc); // * NOT MPI FRIENDLY YET
	paintFromHuygensSurfaces(gridDesc);
	paintFromCurrentSources(gridDesc);
	
	//cout << mVoxels << endl;
	
	mVoxels.overlayPML(); // * grid-scale wraparound
	
	cout << mVoxels << endl;
	
	calculateMaterialIndices();
	createSetupMaterials(gridDesc);
	loadSpaceVaryingData(); // * grid-scale wraparound
	generateRunlines(); // * partition wraparound
    
    createSetupOutputs(gridDesc.getOutputs());
    createSetupSources(gridDesc.getSources());
}

void VoxelizedPartition::
initFieldBuffers(string bufferNamePrefix, 
    const vector<HuygensSurfaceDescPtr> & surfaces)
{
	int bufSize = m_nx*m_ny*m_nz;
    // Main E and H fields
    mEHBuffers.resize(6);
	mEHBuffers[0] =
        MemoryBufferPtr(new MemoryBuffer(bufferNamePrefix+" Ex", bufSize));
	mEHBuffers[1] =
        MemoryBufferPtr(new MemoryBuffer(bufferNamePrefix+" Ey", bufSize));
	mEHBuffers[2] =
        MemoryBufferPtr(new MemoryBuffer(bufferNamePrefix+" Hz", bufSize));
	mEHBuffers[3] =
        MemoryBufferPtr(new MemoryBuffer(bufferNamePrefix+" Ez", bufSize));
	mEHBuffers[4] =
         MemoryBufferPtr(new MemoryBuffer(bufferNamePrefix+" Hy", bufSize));
	mEHBuffers[5] =
        MemoryBufferPtr(new MemoryBuffer(bufferNamePrefix+" Hx", bufSize));
    
    // Neighbor buffers (E and H fields for TFSF sources, links, etc.)
	for (unsigned int nn = 0; nn < surfaces.size(); nn++)
	{
		const vector<NeighborBufferDescPtr> & nbs = surfaces[nn]->getBuffers();
		
        /*
        LOG << "Surface " << nn << " has omitted sides:\n";
        LOGMORE << surfaces[nn]->getOmittedSides() << "\n";
        LOGMORE << "Surface has " << nbs.size() << " neighbor buffers.\n";
        */
        assert(nbs.size() == 6);
		for (unsigned int mm = 0; mm < 6; mm++)
		if (nbs[mm] != 0L)
        if (!surfaces[nn]->getOmittedSides().count(cardinalDirection(mm)))
		{
			NeighborBufferDescPtr nb = nbs[mm];
            const Rect3i & bufferVolume = nb->getBufferHalfRect();
            Vector3i numYeeCells = rectHalfToYee(bufferVolume).size()+1;
            int bufSize = numYeeCells[0]*numYeeCells[1]*numYeeCells[2];
            
            //LOG << "NB " << mm << " volume " << bufferVolume << " size " <<
            //    bufSize << "\n";
			
            mNBBuffers[nb].resize(6);
			for (int ff = 0; ff < 6; ff++)
			{
				ostringstream bufferName;
                bufferName << bufferNamePrefix << " HS " << nn << " NB " <<
                    mm << " field " << ff;
				mNBBuffers[nb][ff] = MemoryBufferPtr(
                    new MemoryBuffer(bufferName.str(), bufSize));
			}
		}
	}
}


Rect3i VoxelizedPartition::
getGridYeeCells() const
{
    return rectHalfToYee(mGridHalfCells);
}

Rect3i VoxelizedPartition::
getAllocYeeCells() const
{
    return rectHalfToYee(mFieldAllocHalfCells);
}

bool VoxelizedPartition::
partitionHasPML(int faceNum) const
{
	//LOG << "Using non-parallel-friendly method.\n";
	assert(faceNum >= 0);
	assert(faceNum < 6);
	
	if (faceNum%2 == 0) // low side
	{
		if (mNonPMLHalfCells.p1[faceNum/2] <= mAuxAllocRegion.p1[faceNum/2])
			return 0;
	}
	else
	{
		if (mNonPMLHalfCells.p2[faceNum/2] >= mAuxAllocRegion.p2[faceNum/2])
			return 0;
	}
	return 1;
}

Rect3i VoxelizedPartition::
getPMLHalfCellsOnFace(int faceNum) const
{
	//LOG << "Using non-parallel-friendly method.\n";
	assert(faceNum >= 0);
	assert(faceNum < 6);
	
	Rect3i pmlRegion(mGridHalfCells);
	
	if (faceNum%2 == 0) // low side
		pmlRegion.p2[faceNum/2] = mNonPMLHalfCells.p1[faceNum/2]-1;
	else
		pmlRegion.p1[faceNum/2] = mNonPMLHalfCells.p2[faceNum/2]+1;
	return pmlRegion;
}

Rect3i VoxelizedPartition::
getPartitionPMLHalfCellsOnFace(int faceNum) const
{
	//LOG << "Using non-parallel-friendly method.\n";
	assert(faceNum >= 0);
	assert(faceNum < 6);
	
	Rect3i pmlRegion(mAuxAllocRegion);
	
	if (faceNum%2 == 0) // low side
		pmlRegion.p2[faceNum/2] = mNonPMLHalfCells.p1[faceNum/2]-1;
	else
		pmlRegion.p1[faceNum/2] = mNonPMLHalfCells.p2[faceNum/2]+1;
	return pmlRegion;
}

Rect3i VoxelizedPartition::
getPMLHalfCells(Vector3i pmlDir) const
{
	Rect3i pml(mGridHalfCells);
	
	for (int nn = 0; nn < 3; nn++)
	{
		if (pmlDir[nn] < 0)
			pml.p2[nn] = mNonPMLHalfCells.p1[nn]-1;
		else if (pmlDir[nn] > 0)
			pml.p1[nn] = mNonPMLHalfCells.p2[nn]+1;
		else
		{
			pml.p1[nn] = mNonPMLHalfCells.p1[nn];
			pml.p2[nn] = mNonPMLHalfCells.p2[nn];
		}
	}
	return pml;
}

Vector3i VoxelizedPartition::
wrap(Vector3i vv) const
{
    vv = vv - mFieldAllocHalfCells.p1;
    
    if (vv[0] >= 0)
        vv[0] = m_nnx0 + vv[0]%m_nnx;
    else
        vv[0] = m_nnx0 + (m_nnx-1)-(-vv[0]-1)%m_nnx;
    
    if (vv[1] >= 0)
        vv[1] = m_nny0 + vv[1]%m_nny;
    else
        vv[1] = m_nny0 + (m_nny-1)-(-vv[1]-1)%m_nny;
    
    if (vv[2] >= 0)
        vv[2] = m_nnz0 + vv[2]%m_nnz;
    else
        vv[2] = m_nnz0 + (m_nnz-1)-(-vv[2]-1)%m_nnz;
    
    assert(mFieldAllocHalfCells.encloses(vv));
    
    return vv;
}

Vector3i VoxelizedPartition::
wrap(const NeighborBufferDescPtr & nb, Vector3i vv) const
{
	const Rect3i & halfCellBounds(nb->getDestHalfRect());
    const Vector3i & p1 = halfCellBounds.p1;
    Vector3i halfCells = halfCellBounds.size() + 1;
    
    vv = vv - p1;
    
    if (vv[0] >= 0)
        vv[0] = p1[0] + vv[0]%halfCells[0];
    else
        vv[0] = p1[0] + (halfCells[0]-1)-(-vv[0]-1)%halfCells[0];
    
    if (vv[1] >= 0)
        vv[1] = p1[1] + vv[1]%halfCells[1];
    else
        vv[1] = p1[1] + (halfCells[1]-1)-(-vv[1]-1)%halfCells[1];
    
    if (vv[2] >= 0)
        vv[2] = p1[2] + vv[2]%halfCells[2];
    else
        vv[2] = p1[2] + (halfCells[2]-1)-(-vv[2]-1)%halfCells[2];
    
    assert(halfCellBounds.encloses(vv));
    
    return vv;
}


/*
long VoxelizedPartition::
linearYeeIndex(int ii, int jj, int kk) const
{
    assert(mFieldAllocHalfCells.encloses(ii,jj,kk));
    
    ii = ii-m_nnx0;
    jj = jj-m_nny0;
    kk = kk-m_nnz0;
        
    return ( (ii/2)%m_nx + m_nx*( (jj/2)%m_ny ) + m_nx*m_ny*( (kk/2)%m_nz ) );
}
*/

long VoxelizedPartition::
linearYeeIndex(const Vector3i & halfCell) const
{
    assert(mFieldAllocHalfCells.encloses(halfCell));
    
    int ii = halfCell[0] - m_nnx0;
    int jj = halfCell[1] - m_nny0;
    int kk = halfCell[2] - m_nnz0;
    
    return ( (ii/2) + m_nx*(jj/2) + m_nx*m_ny*(kk/2) );
}


long VoxelizedPartition::
linearYeeIndex(const NeighborBufferDescPtr & nb,
	const Vector3i & halfCell) const
{
	const Rect3i & halfCellBounds (nb->getDestHalfRect());
    Vector3i halfCells = halfCellBounds.size() + 1;
    
	Rect3i yeeBounds(rectHalfToYee(halfCellBounds, halfCell%2));
	int nx = yeeBounds.size(0)+1;
	int ny = yeeBounds.size(1)+1;
	int nz = yeeBounds.size(2)+1;
    
    assert(halfCellBounds.encloses(halfCell));
    
	int ii = halfCell[0] - halfCellBounds.p1[0];
	int jj = halfCell[1] - halfCellBounds.p1[1];
	int kk = halfCell[2] - halfCellBounds.p1[2];
    
    return ( (ii/2) + nx*(jj/2) + nx*ny*(kk/2) );
} 

BufferPointer VoxelizedPartition::
fieldPointer(Vector3i halfCell) const
{
    halfCell = wrap(halfCell);
	long index = linearYeeIndex(halfCell);
	int fieldNum = octantFieldNumber(halfCell);
	return BufferPointer(*mEHBuffers[fieldNum], index);
}

BufferPointer VoxelizedPartition::
fieldPointer(const NeighborBufferDescPtr & nb, Vector3i halfCell) const
{
    halfCell = wrap(nb, halfCell);
	long index = linearYeeIndex(nb, halfCell);
	int fieldNum = octantFieldNumber(halfCell);
	return BufferPointer(*mNBBuffers[nb][fieldNum], index);
}


BufferPointer VoxelizedPartition::
getE(int direction, Vector3i xx) const
{
    return fieldPointer(vecYeeToHalf(xx, eOctantNumber(direction)));
}
    
BufferPointer VoxelizedPartition::
getH(int direction, Vector3i xx) const
{
    return fieldPointer(vecYeeToHalf(xx, hOctantNumber(direction)));
}

BufferPointer VoxelizedPartition::
getE(const NeighborBufferDescPtr & nb, int direction, Vector3i xx) const
{
    return fieldPointer(nb, vecYeeToHalf(xx, eOctantNumber(direction)));
}
    
BufferPointer VoxelizedPartition::
getH(const NeighborBufferDescPtr & nb, int direction, Vector3i xx) const
{
    return fieldPointer(nb, vecYeeToHalf(xx, hOctantNumber(direction)));
}

Vector3i VoxelizedPartition::
getFieldStride() const
{
    int stride = mEHBuffers[0]->getStride(); // same for all E, H
    
    for (int nn = 0; nn < 6; nn++)
        assert(mEHBuffers[nn]->getStride() == stride); // but make sure.
    
    return Vector3i(stride, stride*m_nx, stride*m_nx*m_ny);
}

void VoxelizedPartition::
clearVoxelGrid()
{
    //LOG << "Clearing voxel grid.\n";
    mVoxels.clear();
}

void VoxelizedPartition::
clearCellCountGrid()
{
    //LOG << "Clearing cell count grid.\n";
    mCentralIndices = 0L;
}

void VoxelizedPartition::
createSetupHuygensSurfaces(const vector<HuygensSurfaceDescPtr> & surfaces,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & grids)
{   
    LOG << "I am making " << surfaces.size() << " setup Huygens surfaces.\n";
    LOGMORE << "I am " << mGridHalfCells << " half cells across.\n";
    for (unsigned int nn = 0; nn < surfaces.size(); nn++)
    {
        mSetupHuygensSurfaces.push_back(
            HuygensSurfaceFactory::newSetupHuygensSurface(*this, grids,
                surfaces[nn]));
    }
}

	
#pragma mark *** Private methods ***


void VoxelizedPartition::
paintFromAssembly(const GridDescription & gridDesc,
	const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids)
{
	//LOG << "Painting from assembly.\n";
	
	const vector<InstructionPtr> & instructions = gridDesc.getAssembly()->
		getInstructions();
	
	for (unsigned int nn = 0; nn < instructions.size(); nn++)
	{
		switch(instructions[nn]->getType())
		{
			case kBlockType:
				mVoxels.paintBlock(gridDesc, 
					(const Block&)*instructions[nn]);
				break;
			case kKeyImageType:
				mVoxels.paintKeyImage(gridDesc,
					(const KeyImage&)*instructions[nn]);
				break;
			case kHeightMapType:
				mVoxels.paintHeightMap(gridDesc,
					(const HeightMap&)*instructions[nn]);
				break;
			case kEllipsoidType:
				mVoxels.paintEllipsoid(gridDesc,
					(const Ellipsoid&)*instructions[nn]);
				break;
			case kCopyFromType:
				mVoxels.paintCopyFrom(gridDesc,
					(const CopyFrom&)*instructions[nn],
					voxelizedGrids[((const CopyFrom&)*instructions[nn])
						.getGrid()]->mVoxels);
				break;
			case kExtrudeType:
				mVoxels.paintExtrude(gridDesc,
					(const Extrude&)*instructions[nn]);
				break;
			default:
				throw(Exception("Unknown instruction type."));
				break;
		}
	}
}

void VoxelizedPartition::
paintFromHuygensSurfaces(const GridDescription & gridDesc)
{
	//LOG << "Painting from Huygens surfaces.\n";
	
	const vector<HuygensSurfaceDescPtr> & surfaces =
		gridDesc.getHuygensSurfaces();
	
	for (unsigned int nn = 0; nn < surfaces.size(); nn++)
	{
        //LOG << "Omitted sides now: \n";
        //LOGMORE << surfaces[nn]->getOmittedSides() << "\n";
		mVoxels.overlayHuygensSurface(*surfaces[nn]);
	}
}

void VoxelizedPartition::
paintFromCurrentSources(const GridDescription & gridDesc)
{
	LOG << "Painting from current sources.  (Doing nothing.)\n";
}


void VoxelizedPartition::
calculateMaterialIndices()
{
	// This must be done separately for each octant.
	
	mCentralIndices = PartitionCellCountPtr(new PartitionCellCount(mVoxels,
		mAuxAllocRegion));
	
	//cout << *mCentralIndices << endl;
}

void VoxelizedPartition::
calculateHuygensSymmetries(const GridDescription & gridDesc)
{
	//LOG << "Calculating Huygens surface symmetries.\n";
	
	const vector<HuygensSurfaceDescPtr> & surfaces =
		gridDesc.getHuygensSurfaces();
	
	mHuygensRegionSymmetries.resize(surfaces.size());
	
	for (unsigned int nn = 0; nn < surfaces.size(); nn++)
	{
		mHuygensRegionSymmetries[nn] = 
			huygensSymmetry(*surfaces[nn]);
		/*
		LOG << "Surface " << nn << " has bounds " <<
			surfaces[nn]->getHalfCells() << " and symmetries "
			<< mHuygensRegionSymmetries[nn] << "\n";
        */
	}
}

Vector3i VoxelizedPartition::
huygensSymmetry(const HuygensSurfaceDescription & surf)
{
	int ii, jj, kk;
	
	Vector3i o = surf.getHalfCells().p1; // origin
	Vector3i p = surf.getHalfCells().p2; // and opposite corner
	Vector3i dim = surf.getHalfCells().size();
	const set<Vector3i> & omittedSides = surf.getOmittedSides();
    
	Vector3i symmetry(1, 1, 1);
    
	for (int side_i = 0; side_i < 3; side_i++)
	{
		int side_j = (side_i+1)%3;
		int side_k = (side_i+2)%3;
		Vector3i e1 = -cardinalDirection(2*side_i);
		Vector3i e2 = -cardinalDirection( (2*side_i+2)%6 );
		Vector3i e3 = -cardinalDirection( (2*side_i+4)%6 );
		Mat3i m(Mat3i::withColumns(e1,e2,e3));
		
		// 1.  Check the "front" and "back" sides (the sides perpendicular
		// to vector e1).
		if (!omittedSides.count(e1) && !omittedSides.count(-e1))
		{
			for (jj = 0; jj < dim[side_j]; jj++)
			for (kk = 0; kk < dim[side_k]; kk++)
			{
				Vector3i frontSide( (o + jj*e2 + kk*e3) );
				Vector3i backSide( frontSide + (dim[side_i]*e1) );
				if ( mVoxels(frontSide) != mVoxels(backSide) )
					symmetry[side_i] = 0;
			}
		}
		
		// 2.  Check the "j" sides.  All materials in stripes in the i direction
		// should be the same unless the side is omitted.
		if (!omittedSides.count(e2))
		{
			Vector3i sideOrigin = o + dim[side_j]*e2;
			for (kk = 0; kk < dim[side_k]; kk++)
			{
				Paint* matchMe = mVoxels( (sideOrigin+kk*e3) );
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != mVoxels(
						(sideOrigin+ii*e1 + kk*e3) ))
						symmetry[side_i] = 0;
			}
		}
		if (!omittedSides.count(-e2))
		{
			Vector3i sideOrigin = o;
			for (kk = 0; kk < dim[side_k]; kk++)
			{
				Paint* matchMe = mVoxels((sideOrigin+kk*e3) );
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != mVoxels(
						(sideOrigin+ii*e1 + kk*e3) ))
						symmetry[side_i] = 0;
			}
		}

		// 3.  Check the "k" sides.  All materials in stripes in the i direction
		// should be the same unless the side is omitted.
		if (!omittedSides.count(e3))
		{
			Vector3i sideOrigin = o + dim[side_k]*e3;
			for (jj = 0; jj < dim[side_j]; jj++)
			{
				Paint* matchMe = mVoxels((sideOrigin+jj*e2));
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != mVoxels(
						(sideOrigin+ii*e1 + jj*e2) ))
						symmetry[side_i] = 0;
			}
		}
		if (!omittedSides.count(-e3))
		{
			Vector3i sideOrigin = o;
			for (jj = 0; jj < dim[side_j]; jj++)
			{
				Paint* matchMe = mVoxels((sideOrigin+jj*e2));
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != mVoxels(
						(sideOrigin+ii*e1 + jj*e2) ))
						symmetry[side_i] = 0;
			}
		}
	} // foreach sideNum
	
    return Vector3i(symmetry);
}

void VoxelizedPartition::
createSetupMaterials(const GridDescription & gridDesc)
{
	set<Paint*> allPaints = mCentralIndices->getCurlBufferParentPaints();
	
	// Cache PML rects (this is really just to simplify notation further down).
	vector<Rect3i> pmlRects;
	for (int nn = 0; nn < 6; nn++)
		pmlRects.push_back(getPMLHalfCellsOnFace(nn));
	
    //cout << *mCentralIndices << endl;
    
	LOG << "Iterating over paints...\n";
	for (set<Paint*>::iterator itr = allPaints.begin(); itr != allPaints.end();
		itr++)
	{
		Paint* p = *itr;
		if (mSetupMaterials.count(p) == 0)
		{
			mSetupMaterials[p] = MaterialFactory::newSetupMaterial(
				mVoxels, mCentralIndices, gridDesc, p);
		}
		SetupMaterial & mat = *mSetupMaterials[p];
		
        int fieldDir;
        long cells;
        for (fieldDir = 0; fieldDir < 3; fieldDir++)
        {
            cells = mCentralIndices->getNumCells(p, eOctantNumber(fieldDir));
            mat.setNumCellsE(fieldDir, cells);            
            cells = mCentralIndices->getNumCells(p, hOctantNumber(fieldDir));
            mat.setNumCellsH(fieldDir, cells);
        }
        
        LOG << "Not calling that PML cells on side function.  What's it for?\n";
        if (p->isPML())
        for (int faceNum = 0; faceNum < 6; faceNum++)
        if (partitionHasPML(faceNum))
        {
            mat.setPMLHalfCells(faceNum, pmlRects[faceNum], gridDesc);
        }
	}
}

void VoxelizedPartition::
loadSpaceVaryingData()
{
	LOG << "Setup materials need to provide temporary space!\n";
	LOGMORE << "Not loading anything yet.\n";
}

void VoxelizedPartition::
generateRunlines()
{
	// Provide a RunlineEncoder for each uniquely-updating Paint
	
	// Walk the grid (ONCE only would be splendid) and step the appropriate
	// encoders.  (Where do setup runlines go?)
	
	LOG << "Check it out, we're sticking to the calc region.  I'm not sure "
		"yet precisely how to use this in the MPI context—work it out later.\n";
	
	for (int fieldNum = 0; fieldNum < 6; fieldNum++)
	{
		// Remember to set up the buffers here!
		//LOG << "Runlines for offset " << halfCellFieldOffset(fieldNum) << "\n";
		genRunlinesInOctant(halfCellIndex(halfCellFieldOffset(fieldNum)));
	}
	
    /*
	LOG << "Printing runlines.\n";
	map<Paint*, SetupMaterialPtr>::iterator itr;
	for (itr = mSetupMaterials.begin(); itr != mSetupMaterials.end(); itr++)
	{
		cout << *(itr->first) << "\n";
		itr->second->printRunlines(cout);
	}
    */
}

void VoxelizedPartition::
genRunlinesInOctant(int octant)
{
	// First task: generate a starting half-cell in the correct octant.
	// The loops may still end by not exceeding mCalcHalfCells.p2—this works fine.
	Vector3i offset = halfCellOffset(octant);
	Vector3i p1 = mCalcHalfCells.p1;
	for (int nn = 0; nn < 3; nn++)
	if (p1[nn] % 2 != offset[nn])
		p1[nn]++;
	//LOG << "Calc region " << mCalcHalfCells << endl;
	//LOG << "Runlines in octant " << octant << " at start " << p1 << endl;
	
	SetupMaterial* material; // unsafe pointer for speed in this case.
	
	// If there is a current runline
	//	Ask the SetupMaterial whether the current cell belongs to it
	//	YES: keep on loopin'
	//	NO: end runline and begin new runline
	//
	// Therafter, if there is not a current runline
	//	Begin new runline
	
	bool needNewRunline = 1;
	Vector3i x(p1), lastX(p1);
	Paint *xPaint, *xParentPaint = 0L, *lastXParentPaint = 0L;
	for (x[2] = p1[2]; x[2] <= mCalcHalfCells.p2[2]; x[2] += 2)
	for (x[1] = p1[1]; x[1] <= mCalcHalfCells.p2[1]; x[1] += 2)
	for (x[0] = p1[0]; x[0] <= mCalcHalfCells.p2[0]; x[0] += 2)
	{
		xPaint = mVoxels(x);
		xParentPaint = xPaint->withoutCurlBuffers();
		
		if (!needNewRunline)
		{
			if (xParentPaint == lastXParentPaint &&
				material->canContinueRunline(*this, lastX, x, xPaint)) // kludge
				material->continueRunline(x);
			else
			{
				material->endRunline();
				needNewRunline = 1;
			}
		}
		if (needNewRunline)
		{
			material = mSetupMaterials[xParentPaint];
			material->startRunline(*this, x);
			needNewRunline = 0;
		}
		lastX = x;
		lastXParentPaint = xParentPaint;
	}
	material->endRunline();  // DO NOT FORGET THIS... oh wait, I didn't!
}


void VoxelizedPartition::
createSetupOutputs(const std::vector<OutputDescPtr> & outputs)
{
    for (unsigned int nn = 0; nn < outputs.size(); nn++)
        mSetupOutputs.push_back(
            OutputFactory::newSetupOutput(*this, outputs[nn]));
}

void VoxelizedPartition::
createSetupSources(const std::vector<SourceDescPtr> & sources)
{
    for (unsigned int nn = 0; nn < sources.size(); nn++)
    {
        if (sources[nn]->isSoftSource())
            mSoftSetupSources.push_back(
                SourceFactory::newSetupSource(*this, sources[nn]));
        else
            mHardSetupSources.push_back(
                SourceFactory::newSetupSource(*this, sources[nn]));
    }
}

std::ostream &
operator<< (std::ostream & out, const VoxelizedPartition & grid)
{
	out << grid.mVoxels;
	return out;
}


