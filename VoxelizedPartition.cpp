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
#include "InterleavedLattice.h"
#include "HuygensSurface.h"
#include "PartitionCellCount.h"

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
	//LOG << "VoxelizedPartition()\n";
	
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
	
    mLattice = InterleavedLatticePtr(new InterleavedLattice(gridDesc.getName(),
        mFieldAllocHalfCells));
    
	mNonPMLHalfCells = gridDesc.getNonPMLHalfCells();
	mOriginYee = gridDesc.getOriginYee();
	
    /*
    LOG << "Partition geometry:\n";
	LOGMORE << "nonPML " << mNonPMLHalfCells << "\n";
	LOGMORE << "alloc region " << mAuxAllocRegion << "\n";
	LOGMORE << "full field alloc region (integer num Yee cells) "
		<< mFieldAllocHalfCells << "\n";
	LOGMORE << "calc " << mCalcHalfCells << "\n";
	LOGMORE << "origin " << mOriginYee << "\n";
	*/
    
	paintFromAssembly(gridDesc, voxelizedGrids);
	calculateHuygensSymmetries(gridDesc); // * NOT MPI FRIENDLY YET
	
    paintFromCurrentSources(gridDesc);
	
	//cout << mVoxels << endl;
	
	mVoxels.overlayPML(); // * grid-scale wraparound
	
	cout << mVoxels << endl;
	
	calculateMaterialIndices();
	createSetupMaterials(gridDesc);
	loadSpaceVaryingData(); // * grid-scale wraparound
	
    createSetupOutputs(gridDesc.getOutputs());
    createSetupSources(gridDesc.getSources());
}

Rect3i VoxelizedPartition::
getGridYeeCells() const
{
    return halfToYee(mGridHalfCells);
}

Rect3i VoxelizedPartition::
getAllocYeeCells() const
{
    return halfToYee(mFieldAllocHalfCells);
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
createHuygensSurfaces(const GridDescPtr & gridDescription,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & grids)
{   
    const vector<HuygensSurfaceDescPtr> & surfaces = 
        gridDescription->getHuygensSurfaces();
    
    LOG << "I am making " << surfaces.size() << " setup Huygens surfaces.\n";
    LOGMORE << "I am " << mGridHalfCells << " half cells across.\n";
    for (unsigned int nn = 0; nn < surfaces.size(); nn++)
    {
        ostringstream huygensSurfaceName;
        huygensSurfaceName << gridDescription->getName() << " HS " << nn;
        mHuygensSurfaces.push_back(
            HuygensSurfaceFactory::newHuygensSurface(
                huygensSurfaceName.str(), *this, grids,
                surfaces[nn]));
    }
    
    paintFromHuygensSurfaces(*gridDescription);
    
    cout << mVoxels << "\n";
    
}


void VoxelizedPartition::
calculateRunlines()
{
    generateRunlines(); // * partition wraparound
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
	
	for (unsigned int nn = 0; nn < mHuygensSurfaces.size(); nn++)
	{
		mVoxels.overlayHuygensSurface(*mHuygensSurfaces[nn]);
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
		Vector3i e1 = -cardinal(2*side_i);
		Vector3i e2 = -cardinal( (2*side_i+2)%6 );
		Vector3i e3 = -cardinal( (2*side_i+4)%6 );
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
    
	//LOG << "Iterating over paints...\n";
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
            cells = mCentralIndices->getNumCells(p, octantE(fieldDir));
            mat.setNumCellsE(fieldDir, cells);            
            cells = mCentralIndices->getNumCells(p, octantH(fieldDir));
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
	
    LOG << "Warning: about to use some magic numbers (0 and 7 are unused"
        " octants.\n";
    for (int octantNum = 1; octantNum <= 6; octantNum++)
    {
        genRunlinesInOctant(octantNum);
    }
    /*
	for (int fieldNum = 0; fieldNum < 6; fieldNum++)
	{
		// Remember to set up the buffers here!
		//LOG << "Runlines for offset " << halfCellFieldOffset(fieldNum) << "\n";
        
		genRunlinesInOctant(octant(halfCellFieldOffset(fieldNum)));
	}
    */
	
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


