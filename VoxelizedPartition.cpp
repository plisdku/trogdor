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
#include "BulkSetupMaterials.h"
#include "RunlineEncoder.h"
#include "MaterialFactory.h"
#include "STLOutput.h"
#include "InterleavedLattice.h"
#include "HuygensSurface.h"
#include "PartitionCellCount.h"
#include "IODescriptionFile.h"
#include "Source.h"

#include <algorithm>
#include <sstream>

using namespace std;
using namespace YeeUtilities;

#pragma mark *** VoxelizedPartition ***

VoxelizedPartition::
VoxelizedPartition(SimulationDescPtr simDesc, GridDescPtr gridDesc, 
	const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
	Rect3i allocRegion, Rect3i calcRegion, int runlineDirection) :
    mSimulationDescription(simDesc),
    mGridDescription(gridDesc),
	mVoxels(gridDesc, allocRegion),
	mGridHalfCells(gridDesc->halfCellBounds()),
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
    
    mNumAllocHalfCells = mFieldAllocHalfCells.size()+1;
	
    mLattice = InterleavedLatticePtr(new InterleavedLattice(gridDesc->name(),
        mFieldAllocHalfCells, runlineDirection));
    
	mNonPMLHalfCells = gridDesc->nonPMLHalfCells();
	mOriginYee = gridDesc->originYee();
	
    /*
    LOG << "Partition geometry:\n";
	LOGMORE << "nonPML " << mNonPMLHalfCells << "\n";
	LOGMORE << "alloc region " << mAuxAllocRegion << "\n";
	LOGMORE << "full field alloc region (integer num Yee cells) "
		<< mFieldAllocHalfCells << "\n";
	LOGMORE << "calc " << mCalcHalfCells << "\n";
	LOGMORE << "origin " << mOriginYee << "\n";
	*/
    
	paintFromAssembly(*gridDesc, voxelizedGrids);
	calculateHuygensSurfaceSymmetries(*gridDesc); // * NOT MPI FRIENDLY YET
    paintFromCurrentSources(*gridDesc);
	mVoxels.overlayPML(); // * grid-scale wraparound
	
	//cout << mVoxels << endl;
	
	calculateMaterialIndices();
	createSetupUpdateEquations(*gridDesc);
	loadSpaceVaryingData(*gridDesc); // * grid-scale wraparound
	
    createSetupOutputs(gridDesc->outputs());
    createSetupSources(gridDesc->sources());
    createSetupCurrentSources(gridDesc->currentSources());
}

Rect3i VoxelizedPartition::
gridYeeCells() const
{
    return halfToYee(mGridHalfCells);
}

Rect3i VoxelizedPartition::
allocYeeCells() const
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
pmlHalfCellsOnFace(int faceNum) const
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
partitionPMLHalfCellsOnFace(int faceNum) const
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
pmlHalfCells(Vector3i pmlDir) const
{
	Rect3i pml(mGridHalfCells);
	
	for (int xyz = 0; xyz < 3; xyz++)
	{
		if (pmlDir[xyz] < 0)
			pml.p2[xyz] = mNonPMLHalfCells.p1[xyz]-1;
		else if (pmlDir[xyz] > 0)
			pml.p1[xyz] = mNonPMLHalfCells.p2[xyz]+1;
		else
		{
			pml.p1[xyz] = mNonPMLHalfCells.p1[xyz];
			pml.p2[xyz] = mNonPMLHalfCells.p2[xyz];
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
        gridDescription->huygensSurfaces();
    
    for (unsigned int nn = 0; nn < surfaces.size(); nn++)
    {
        ostringstream huygensSurfaceName;
        huygensSurfaceName << gridDescription->name() << " HS " << nn;
        mHuygensSurfaces.push_back(
            HuygensSurfaceFactory::newHuygensSurface(
                huygensSurfaceName.str(), *this, grids,
                surfaces[nn]));
    }
    
    paintFromHuygensSurfaces(*gridDescription);
}

void VoxelizedPartition::
calculateRunlines()
{
    // Make a table of runline encoders
    
    Map<Paint*, RunlineEncoder*> encoders;
    
    //cout << mVoxels << "\n";
    
    map<Paint*, Pointer<SetupUpdateEquation> >::iterator itr;
    for (itr = mSetupUpdateEquations.begin(); itr != mSetupUpdateEquations.end(); itr++)
        encoders[itr->first] = &itr->second->encoder();
    
    for (int direction = 0; direction < 3; direction++)
    {
        runLengthEncode(encoders, halfToYee(mCalcHalfCells, octantE(direction)),
            octantE(direction));
        runLengthEncode(encoders, halfToYee(mCalcHalfCells, octantH(direction)),
            octantH(direction));
    }
    
    /*
    {
        map<Paint*, Pointer<SetupUpdateEquation> >::const_iterator itr;
        for (itr = mSetupUpdateEquations.begin(); itr != mSetupUpdateEquations.end(); itr++)
            itr->second->printRunlines(cout);
    }
    */
}

void VoxelizedPartition::
writeDataRequests() const
{
    // 1.  Current sources
    // 2.  Custom TFSF sources (HuygensSurfaces)
    
    //LOG << "Writing data requests." << endl;
    
    for (int nn = 0; nn < mSetupCurrentSources.size(); nn++)
    if (mSetupCurrentSources[nn]->description()->hasMask() ||
        mSetupCurrentSources[nn]->description()->isSpaceVarying())
    {
        // if this current source will require scheduled data from a file
        
        ostringstream str;
        str << "currentreq_" << nn << ".m";
        IODescriptionFile::write(
            str.str(),
            mSetupCurrentSources[nn]->description(),
            *this,
            mSetupCurrentSources[nn]->getRectsJ(),
            mSetupCurrentSources[nn]->getRectsK());
    }
    
    for (int nn = 0; nn < mHuygensSurfaces.size(); nn++)
    if (mHuygensSurfaces[nn]->description()->type() == kCustomTFSFSource)
    {
        ostringstream str;
        str << "tfsfreq_" << nn << ".m";
        IODescriptionFile::write(
            str.str(),
            *mHuygensSurfaces[nn],
            *this,
            mHuygensRegionSymmetries[nn]);
    }
}
/*
void VoxelizedPartition::
writeDataRequest(const HuygensSurfaceDescPtr surf,
    const GridDescPtr gridDescription) const
{
	// 1.  Simulation parameters
	// 2.  Source information (params, bounds)
	// 3.  Materials and parameters
	// 4.  Material sample (1D or 2D)
	// 5.  Required sample locations: Ex, Ey, Ez, Hx, Hy, Hz
    
    ostringstream fileNameStream;
    fileNameStream << surf->file() << ".m";
    
    //IODescriptionFile::write(fileNameStream.str(), surf, *this);
    
    ofstream file;
    file.open(fileNameStream.str().c_str());
    
    // I think the simplest thing will be for the source request to use user
    // coordinates in every way, but to request values at coordinates in the
    // appropriate direction.
    
    LOG << "WARNING: Data requests are not coordinate-rotated.\n";
    
    file << "clear afp;\n";
    file << "afp.dxdydzdt = [" << gridDescription->dxyz()[0] << " "
        << gridDescription->dxyz()[1] << " " << gridDescription->dxyz()[2]
        << "];\n";
    
    int numT = surf->duration().last() - surf->duration().first()
        + 1;
    file << "afp.numT = [" << numT << "];\n";
    
    file << "afp.halfCells = " << surf->halfCells() << ";\n";
    file << "afp.tfRect = afp.halfCells;\n";
    file << "afp.inputFile = '" << surf->file() << "';\n";
    
    // Now write the materials and their parameters.
    int ii, jj, kk;
    int numMaterials = 0;
    Rect3i sampleRect = ss->tiFRect();
    const vmlib::SMat<3,bool> & gridSymmetries =
        ss->cachedGridSymmetries();
    Vector3b sourceSymmetries = ss->symmetries();
    Vector3b periodicDimensions = periodicDimensions(sampleRect);
    Vector3b combinedSymmetries(0,0,0);
    
    LOGF << "Grid symmetries are " << gridSymmetries << endl;
    LOGF << "Source symmetries are " << sourceSymmetries << endl;
    LOGF << "Periodic dimensions are " << periodicDimensions << endl;
    
    for (int nSym = 0; nSym < 3; nSym++)
    {
        // There are two conditions under which a dimension can be dropped.
        //	 1.  If the sourceRect has full X symmetry (which means that
        //		gridSymmetries(nSym,:) == 1) then the X direction collapses.
        //	 2.  If the sourceRect has X symmetry on some faces, but
        //      the remaining faces don't need TFSF correction, then that
        //		is good enough.  That condition is usually the same as
        //		having a periodic dimension.  (Warning: this is a kludge.)
        
        Vector3b symmetryOrPeriodicity(
            periodicDimensions[0] || gridSymmetries(nSym,0),
            periodicDimensions[1] || gridSymmetries(nSym,1),
            periodicDimensions[2] || gridSymmetries(nSym,2) );
        
        if (minval(symmetryOrPeriodicity) == 1) // if vector == (1 1 1)
            combinedSymmetries[nSym] = 1;
    }
    
    LOGF << "Combined symmetries are " << combinedSymmetries << endl;
    
    for (int xyz = 0; xyz < 3; xyz++)
    if (combinedSymmetries[nn])
        sampleRect.p2[nn] = sampleRect.p1[nn];
    
    // Determine all the materials in this chunk of grid.
    Map<int, int> tagToParent;
    Map<int, int> parentToIndex;
    vector<int> materials;
    for (kk = sampleRect.p1[2]; kk <= sampleRect.p2[2]; kk++)
    for (jj = sampleRect.p1[1]; jj <= sampleRect.p2[1]; jj++)
    for (ii = sampleRect.p1[0]; ii <= sampleRect.p2[0]; ii++)
    {
        Paint* mat = mVoxels(ii,jj,kk);
        int parentTag;
        //cout << mat << "\n";
        if (tagToParent.count(mat) == 0)
        {
            const MaterialType & matType =
                mStructureGrid->materialType(mat);
            if (matType.isTFSF())
                parentTag = mStructureGrid->materialIndex(
                    matType.name());
            else
                parentTag = mat;
            
            tagToParent[mat] = parentTag;
        }
        
        parentTag = tagToParent[mat];
        if (parentToIndex.count(parentTag) == 0)
        {
            parentToIndex[parentTag] = numMaterials;
            materials.push_back(parentTag);
            numMaterials++;
        }
    }
    // Write the material descriptions
    for (int mm = 0; mm < numMaterials; mm++)
    {
        const MaterialType & matType = mStructureGrid->materialType(
            materials[mm]);
        const SetupUpdateEquationPtr setupMat = mMaterials[matType.name()];
        file << "afp.materials{" << mm+1 << "}.class = '" << 
            setupMat->getClass() << "';\n";
        file << "afp.materials{" << mm+1 << "}.name = '" <<
            setupMat->name() << "';\n";
        
        const Map<string, string> & params = setupMat->parameters();
        Map<string, string>::const_iterator itr;
        for (itr = params.begin(); itr != params.end(); itr++)
        {
            file << "afp.materials{" << mm+1 << "}." << itr->first
                << " = " << itr->second << ";\n";
        }
    }
    // Write the grid
    file << "afp.sampleBounds = " << sampleRect << ";\n";
    file << "afp.grid = [";
    for (kk = sampleRect.p1[2]; kk <= sampleRect.p2[2]; kk++)
    {
        file << "...\n";
        for (jj = sampleRect.p1[1]; jj <= sampleRect.p2[1]; jj++)
        {
            file << "\t";
            for (ii = sampleRect.p1[0]; ii <= sampleRect.p2[0]; ii++)
            {
                Paint* mat = mVoxels(ii,jj,kk);
                //int matInd = parentToIndex[tagToParent[mat]];
                //file << matInd+1 << " ";
            }
            file << "...\n";
        }
    }
    file << "];\n";
    
    // Write the buffer points required, first in half cells, then in meters
    SetupTFSFBufferSet temporaryBuffer(*ss, mActiveRegion);
    
    // Make a list of parities of Ex, Ey, Ez, Hx, Hy and Hz
    vector<Vector3i> fieldParities;
    fieldParities.push_back(Vector3i(1,0,0));
    fieldParities.push_back(Vector3i(0,1,0));
    fieldParities.push_back(Vector3i(0,0,1));
    fieldParities.push_back(Vector3i(0,1,1));
    fieldParities.push_back(Vector3i(1,0,1));
    fieldParities.push_back(Vector3i(1,1,0));
    vector<string> fieldNames;
    fieldNames.push_back("Ex");
    fieldNames.push_back("Ey");
    fieldNames.push_back("Ez");
    fieldNames.push_back("Hx");
    fieldNames.push_back("Hy");
    fieldNames.push_back("Hz");
    
    int ff, gg;
    for (ff = 0; ff < 6; ff++)
    {
        file << "afp.yee"+fieldNames[ff] << " = [ ...\n";
        
        for (gg = 0; gg < 6; gg++)  // iterate over sides (-x, +x, etc)
        if (!temporaryBuffer.omits(gg))
        {
            Rect3i yeeBounds = temporaryBuffer.yeeBufferRect(gg,
                fieldParities[ff]);
            for (kk = yeeBounds.p1[2]; kk <= yeeBounds.p2[2]; kk++)
            for (jj = yeeBounds.p1[1]; jj <= yeeBounds.p2[1]; jj++)
            for (ii = yeeBounds.p1[0]; ii <= yeeBounds.p2[0]; ii++)
                file << "\t[" << ii << ", " << jj << ", " << kk << "]; ...\n";
        }
        file << "];\n";
    }
    
    for (ff = 0; ff < 6; ff++)
    {
        file << "afp.pos"+fieldNames[ff] << " = [ ...\n";
        
        for (gg = 0; gg < 6; gg++)  // iterate over sides (-x, +x, etc)
        if (!temporaryBuffer.omits(gg))
        {
            Rect3i yeeBounds = temporaryBuffer.yeeBufferRect(gg,
                fieldParities[ff]);
            for (kk = yeeBounds.p1[2]; kk <= yeeBounds.p2[2]; kk++)
            for (jj = yeeBounds.p1[1]; jj <= yeeBounds.p2[1]; jj++)
            for (ii = yeeBounds.p1[0]; ii <= yeeBounds.p2[0]; ii++)
            {
                file << "\t[" << dx*(ii+0.5*fieldParities[ff][0]) << ", "
                    << dy*(jj+0.5*fieldParities[ff][1]) << ", "
                    << dz*(kk+0.5*fieldParities[ff][2]) << "]; ...\n";
            }
        }
        file << "];\n";
    }
    file << "% end auto-generated file \n";
    
    file.close();
}
*/


void VoxelizedPartition::
runLengthEncode(RunlineEncoder & encoder, Rect3i yeeCells, int octant)
    const
{
	// First task: generate a starting half-cell in the correct octant.
	// The loops may still end by not exceeding mCalcHalfCells.p2—this works fine.
	Vector3i offset = halfCellOffset(octant);
    Vector3i p1 = yeeToHalf(yeeCells.p1, octant);
    Vector3i p2 = yeeToHalf(yeeCells.p2, octant);
    
    // d0 is the direction of memory allocation.  In the for-loops, this is
    // the innermost of the three Cartesian directions.
    const int d0 = mLattice->runlineDirection();
    const int d1 = (d0+1)%3;
    const int d2 = (d0+2)%3;
    
	bool needNewRunline = 1;
	Vector3i x(p1), previous_x;
	Paint *thisPaint, *thisUpdateType, *firstUpdateType = 0L;
	for (x[d2] = p1[d2]; x[d2] <= p2[d2]; x[d2] += 2)
	for (x[d1] = p1[d1]; x[d1] <= p2[d1]; x[d1] += 2)
	for (x[d0] = p1[d0]; x[d0] <= p2[d0]; x[d0] += 2)
	{
		thisPaint = mVoxels(x);
		thisUpdateType = thisPaint->withoutCurlBuffers();
		
		if (!needNewRunline)
		{
			if (thisUpdateType == firstUpdateType &&
				encoder.canContinueRunline(*this, x, thisPaint))
            {
				encoder.continueRunline();
            }
			else
			{
				encoder.endRunline(*this, previous_x);
				needNewRunline = 1;
			}
		}
		if (needNewRunline)
		{
			encoder.startRunline(*this, x);
            firstUpdateType = thisUpdateType;
			needNewRunline = 0;
		}
        previous_x = x;
	}
	encoder.endRunline(*this, previous_x);
}

void VoxelizedPartition::
runLengthEncode(map<Paint*, RunlineEncoder*> & encoders,
    Rect3i yeeCells, int octant) const
{
	// First task: generate a starting half-cell in the correct octant.
	// The loops may still end by not exceeding mCalcHalfCells.p2—this works fine.
	Vector3i offset = halfCellOffset(octant);
    Vector3i p1 = yeeToHalf(yeeCells.p1, octant);
    Vector3i p2 = yeeToHalf(yeeCells.p2, octant);
    
    // d0 is the direction of memory allocation.  In the for-loops, this is
    // the innermost of the three Cartesian directions.
    const int d0 = mLattice->runlineDirection();
    const int d1 = (d0+1)%3;
    const int d2 = (d0+2)%3;
	
    RunlineEncoder* currentEncoder; // non-smart pointer is faster?
    
	bool needNewRunline = 1;
	Vector3i x(p1), previous_x;
	Paint *thisPaint, *thisUpdateType, *firstUpdateType = 0L;
	for (x[d2] = p1[d2]; x[d2] <= p2[d2]; x[d2] += 2)
	for (x[d1] = p1[d1]; x[d1] <= p2[d1]; x[d1] += 2)
	for (x[d0] = p1[d0]; x[d0] <= p2[d0]; x[d0] += 2)
	{
		thisPaint = mVoxels(x);
		thisUpdateType = thisPaint->withoutCurlBuffers();
		
		if (!needNewRunline)
		{
			if (thisUpdateType == firstUpdateType &&
				currentEncoder->canContinueRunline(*this, x, thisPaint))
            {
				currentEncoder->continueRunline();
            }
			else
			{
				currentEncoder->endRunline(*this, previous_x);
				needNewRunline = 1;
			}
		}
		if (needNewRunline && encoders.count(thisUpdateType) != 0)
		{
            currentEncoder = encoders[thisUpdateType];
            currentEncoder->startRunline(*this, x);
            firstUpdateType = thisUpdateType;
            needNewRunline = 0;
		}
        previous_x = x;
	}
    if (!needNewRunline)
        currentEncoder->endRunline(*this, previous_x);
}




	
#pragma mark *** Private methods ***


void VoxelizedPartition::
paintFromAssembly(const GridDescription & gridDesc,
	const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids)
{
	//LOG << "Painting from assembly.\n";
	
	const vector<InstructionPtr> & instructions = gridDesc.assembly()->
		instructions();
	
	for (unsigned int nn = 0; nn < instructions.size(); nn++)
	{
		switch(instructions[nn]->type())
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
						.grid()]->mVoxels);
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
//	LOG << "Painting from current sources.  (Doing something?)\n";
//    LOG << "We have " << mSetupCurrentSources.size() << " sources.\n";
    
    for (int nn = 0; nn < gridDesc.currentSources().size(); nn++)
    {
        mVoxels.overlayCurrentSource(gridDesc.currentSources().at(nn));
    }
    
    /*
    for (int nn = 0; nn < mSetupCurrentSources.size(); nn++)
    {
        mVoxels.overlayCurrentSource(*mSetupCurrentSources[nn]);
    }*/
}


void VoxelizedPartition::
calculateMaterialIndices()
{
	mCentralIndices = PartitionCellCountPtr(new PartitionCellCount(mVoxels,
		mAuxAllocRegion, mLattice->runlineDirection()));
	
	//cout << *mCentralIndices << endl;
}

void VoxelizedPartition::
calculateHuygensSurfaceSymmetries(const GridDescription & gridDesc)
{
	//LOG << "Calculating Huygens surface symmetries.\n";
	
	const vector<HuygensSurfaceDescPtr> & surfaces =
		gridDesc.huygensSurfaces();
	
	mHuygensRegionSymmetries.resize(surfaces.size());
	
	for (unsigned int nn = 0; nn < surfaces.size(); nn++)
	{
		mHuygensRegionSymmetries[nn] = 
			huygensSymmetry(*surfaces[nn]);
		/*
		LOG << "Surface " << nn << " has bounds " <<
			surfaces[nn]->halfCells() << " and symmetries "
			<< mHuygensRegionSymmetries[nn] << "\n";
        */
	}
}

Vector3i VoxelizedPartition::
huygensSymmetry(const HuygensSurfaceDescription & surf)
{
	int ii, jj, kk;
	
	Vector3i o = surf.halfCells().p1; // origin
	Vector3i p = surf.halfCells().p2; // and opposite corner
	Vector3i dim = surf.halfCells().size();
	const set<Vector3i> & omittedSides = surf.omittedSides();
    
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

static bool compareByMaterialID(const Paint* lhs, const Paint* rhs)
{ return lhs->bulkMaterial()->id() < rhs->bulkMaterial()->id(); }

void VoxelizedPartition::
createSetupUpdateEquations(const GridDescription & gridDesc)
{
	set<Paint*> allPaints = mCentralIndices->curlBufferParentPaints();
	
    vector<Paint*> sortedPaints;
    copy(allPaints.begin(), allPaints.end(), back_inserter(sortedPaints));
    sort(sortedPaints.begin(), sortedPaints.end(), &compareByMaterialID);
    
	// Cache PML rects (this is really just to simplify notation further down).
	vector<Rect3i> pmlRects;
	for (int nn = 0; nn < 6; nn++)
		pmlRects.push_back(pmlHalfCellsOnFace(nn));
	
    //cout << *mCentralIndices << endl;
    
	//LOG << "Iterating over paints...\n";
    int updateID = 0;
	for (int nn = 0; nn < sortedPaints.size(); nn++)
	{
		Paint* p = sortedPaints[nn];
        vector<long> numCellsE(3), numCellsH(3);
        
        int fieldDir;
        for (fieldDir = 0; fieldDir < 3; fieldDir++)
        {
            numCellsE[fieldDir] = 
                mCentralIndices->numCells(p, octantE(fieldDir));
            numCellsH[fieldDir] = 
                mCentralIndices->numCells(p, octantH(fieldDir));
        }
        
//        LOG << "Not calling that PML cells on side function.  What's it for?\n";
        
		if (mSetupUpdateEquations.count(p) == 0)
		{
			mSetupUpdateEquations[p] = MaterialFactory::newSetupUpdateEquation(gridDesc,
                p, numCellsE, numCellsH, pmlRects,
                mLattice->runlineDirection());
            mSetupUpdateEquations[p]->setID(updateID);
            updateID++;
		}
	}
}

void VoxelizedPartition::
loadSpaceVaryingData(const GridDescription & gridDesc)
{
//	LOG << "Setup materials need to provide temporary space!\n";
//	LOGMORE << "Not loading anything yet.\n";
    
    // Plan: iterate over assembly structure, but instead of painting the usual
    // way, paint into setup materials.  So for instance:
    // -- graded index materials: paint the index into aux arrays
    // -- effective material constants at boundaries: do local calculation and
    //    put contants into aux arrays
    // Y'know.  All that.
    
	const vector<InstructionPtr> & instructions = gridDesc.assembly()->
		instructions();
    
    //  Plan for boundaries:
    //  The assembly will be handled already as a big bunch of polyhedra.
    //  Now I need to get the fill factors.
    //
    //  In the ideal world, I would have access to the fill factors for each
    //  material, in order along the x-axis (say), as a result of raytracing.
    //
    // WHAT TO DO:
    //  
    //  For all boundary cells, set them to the most-inclusive material (which
    //  will be my Drude model, most likely, although this won't handle the
    //  static conductors at present).
    //  
    //  
    
    /*
        If I were meep:
        
        - define permittivity by a vector of coefficients
        - maintain a grid of high-dimensional permittivity data
        - at boundaries, use fractional fillings in each cell to paint in the
          permittivity (easy enough)
        
        So why can't I do this?  Well shoot, maybe no reason at all.  Perhaps
        this is tailor-made for my RLE data structure.  I'll want to give it
        a speedy iterator for access to adjacent elements (grid traversal).
        
        Yeah, this would make a lot of things a lot easier.
        
        In terms of update equations, the world does get a bit simpler.
        Every update equation will carry a vector of update coefficients.
        Runlines will provide a stride, either 1 or 0, for the update
        coefficients.  It will be marginally slower, but pretty simple.
        There will be no such thing as "gold-air" boundary, but rather some
        cells of Drude model will take on intermediate values between gold and
        air.  Fine as well.
        
        I need to branch this project.
    */
	
	for (unsigned int nn = 0; nn < instructions.size(); nn++)
	{
		switch(instructions[nn]->type())
		{
			case kBlockType:
				//mVoxels.paintBlock(gridDesc, 
				//	(const Block&)*instructions[nn]);
				break;
			case kKeyImageType:
				//mVoxels.paintKeyImage(gridDesc,
				//	(const KeyImage&)*instructions[nn]);
				break;
			case kHeightMapType:
				//mVoxels.paintHeightMap(gridDesc,
				//	(const HeightMap&)*instructions[nn]);
				break;
			case kEllipsoidType:
				//mVoxels.paintEllipsoid(gridDesc,
				//	(const Ellipsoid&)*instructions[nn]);
				break;
			case kCopyFromType:
				//mVoxels.paintCopyFrom(gridDesc,
				//	(const CopyFrom&)*instructions[nn],
				//	voxelizedGrids[((const CopyFrom&)*instructions[nn])
				//		.grid()]->mVoxels);
				break;
			case kExtrudeType:
				//mVoxels.paintExtrude(gridDesc,
				//	(const Extrude&)*instructions[nn]);
				break;
			default:
				throw(Exception("Unknown instruction type."));
				break;
		}
	}
    
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
            mSoftSetupSources.push_back(SetupSourcePtr(
                new SetupSource(sources[nn])));
//                SourceFactory::newSetupSource(*this, sources[nn]));
        else
            mHardSetupSources.push_back(SetupSourcePtr(
                new SetupSource(sources[nn])));
//                SourceFactory::newSetupSource(*this, sources[nn]));
    }
}

void VoxelizedPartition::
createSetupCurrentSources(const std::vector<CurrentSourceDescPtr> & currents)
{
    for (unsigned int nn = 0; nn < currents.size(); nn++)
    {
        mSetupCurrentSources.push_back(SetupCurrentSourcePtr(
            new SetupCurrentSource(currents[nn], *this)));
    }
}

std::ostream &
operator<< (std::ostream & out, const VoxelizedPartition & grid)
{
	out << grid.mVoxels;
	return out;
}


