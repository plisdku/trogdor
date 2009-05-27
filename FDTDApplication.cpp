/*
 *  FDTDApplication.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/10/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "XMLParameterFile.h"
#include "SimulationDescription.h"
#include "VoxelizedPartition.h"
#include "CalculationPartition.h"
#include "YeeUtilities.h"

#include "FDTDApplication.h"

#include "Pointer.h"
#include "Map.h"
#include "StreamFromString.h"
#include "TimeWrapper.h"
#include "Version.h"
#include "STLOutput.h"

#include <Magick++.h>

#include <set>
#include <vector>
#include <fstream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <sstream>

using namespace std;
using namespace YeeUtilities;

FDTDApplication FDTDApplication::sInstance;

void FDTDApplication::
runNew(string parameterFile)
{
	Map<GridDescPtr, VoxelizedPartitionPtr> voxelizedGrids;
    Map<string, CalculationPartitionPtr> calculationGrids;
	
	SimulationDescPtr sim = loadSimulation(parameterFile);
	//Mat3i orientation = guessFastestOrientation(*sim);
	
	//sim->cycleCoordinates();
	//sim->cycleCoordinates();
	voxelizeGrids(sim, voxelizedGrids); // includes setup runlines
	
	// in here: do any setup that requires the voxelized grids
	// extract all information that will be needed after the setup grid is gone
	
    trimVoxelizedGrids(voxelizedGrids); // ditch VoxelGrid & PartitionCellCount
    makeCalculationGrids(sim, calculationGrids, voxelizedGrids);
	voxelizedGrids.clear();
    allocateAuxBuffers(calculationGrids);
	
	// allocate memory that can be postponed
    
    set<MemoryBuffer*>::const_iterator itr;
    LOG << "All buffers:\n";
    for (itr = MemoryBuffer::getAllBuffers().begin();
        itr != MemoryBuffer::getAllBuffers().end(); itr++)
        LOGMORE << *(*itr) << endl;
    
	// RUN THE SIMULATION hoorah.
    LOG << "Beginning calculation.\n";
    
    long numT = sim->getDuration();
    for (long tt = 0; tt < numT; tt++)
    {
        //outputE
        //outputH
        calcE(calculationGrids);
        calcAfterE(calculationGrids);
        calcH(calculationGrids);
        calcAfterH(calculationGrids);
    }
    
    LOG << "Done with simulation.\n";
	Paint::clearPalette(); // avert embarassing segfaults at the end
}

SimulationDescPtr FDTDApplication::
loadSimulation(string parameterFile)
{
	Pointer<XMLParameterFile> file;
	SimulationDescPtr sim;
	try {
		file = Pointer<XMLParameterFile>(new XMLParameterFile(parameterFile));
		sim = SimulationDescPtr(new SimulationDescription(*file));
	} catch (Exception & e) {
		cerr << "Error trying to read " << parameterFile << ".  Reason:\n";
		cerr << e.what() << endl;
		exit(1);
	}
	
	return sim;
}

Mat3i FDTDApplication::
guessFastestOrientation(const SimulationDescription & grid) const
{
	LOG << "Defaulting to standard orientation.\n";
	return Mat3i::eye(); // identity matrix
}

void FDTDApplication::
voxelizeGrids(const SimulationDescPtr sim,
	Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids)
{
	static const int USE_MPI = 0;
	Rect3i myPartition, myCalcRegion;
	
	if (USE_MPI)
	{
		LOG << "Voxelizing the main grid for MPI.\n";
		LOG << "Determining partitions for each node.\n";
		assert(sim->getGrids().size() == 1);
		LOG << "Good, only one main grid present per MPI Trog. limitations.\n";
		LOG << "Voxelizing per current grid's partition size.\n";
		
		myPartition = Rect3i(0,0,0,0,0,0); 
		myCalcRegion = inset(myPartition, 1,1,1,1,1,1);
		assert(!"Can't go further.");
		//voxelizeGridRecursor(voxelizedGrids, sim->getGrids()[0], myPartition,
		//	myCalcRegion);
	}
	
	LOG << "Voxelizing the grids.\n";
	for (unsigned int ii = 0; ii < sim->getGrids().size(); ii++)
	{
		GridDescPtr g = sim->getGrids()[ii];
		
		// the recursor paints the setup grid and creates new grids as needed
		// to implement all TFSF sources.
		
		LOG << "Determining non-MPI partition size.\n";
		
		Vector3i numNodes(1,1,1);
		Vector3i thisNode(0,0,0);
		
		Rect3i partitionWalls = Rect3i(-100000000, -100000000, -100000000, 
			100000000, 100000000, 100000000);
		
		voxelizeGridRecursor(voxelizedGrids, g, numNodes, thisNode,
			partitionWalls);
	}
}

void FDTDApplication::
voxelizeGridRecursor(Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
	GridDescPtr currentGrid, Vector3i numNodes, Vector3i thisNode, 
	Rect3i partitionWalls)
{
	Rect3i myPartition = clip(partitionWalls, currentGrid->getHalfCellBounds());
	Rect3i myCalcRegion(myPartition);
	Rect3i allocRegion(myPartition);
	
	for (int mm = 0; mm < 3; mm++)
	if (numNodes[mm] != 1 && currentGrid->getNumYeeCells()[mm] != 1)
	{
		allocRegion.p1[mm] -= 1;
		allocRegion.p2[mm] += 1;
	}
	
    LOG << "Recursing:\n";
	LOGMORE << "Recursing for grid " << currentGrid->getName() << ".\n";
	LOGMORE << "I am partition " << thisNode << " of " << numNodes << ".\n";
	LOGMORE << "My node partition region is " << partitionWalls << "\n";
	LOGMORE << "My partition is " << myPartition << ".\n";
	LOGMORE << "I allocate fields over the region " << allocRegion << ".\n";
	LOGMORE << "My calc region is " << myCalcRegion << ".\n";
	
	static const int EXTRUDE_PML = 1;
	if (EXTRUDE_PML)
	{
		LOG << "Adding an Extrude command to implement PML.\n";
		
		InstructionPtr extendThisRegion(new Extrude(
			currentGrid->getNonPMLRegion(),
			currentGrid->getHalfCellBounds() ));
		vector<InstructionPtr> assemblyStuff(
			currentGrid->getAssembly()->getInstructions());
		assemblyStuff.push_back(extendThisRegion);
		AssemblyDescPtr assembly(new AssemblyDescription(assemblyStuff));
		currentGrid->setAssembly(assembly);
	}
	
	VoxelizedPartitionPtr partition(new VoxelizedPartition(
		*currentGrid, voxelizedGrids, allocRegion, myCalcRegion));
	voxelizedGrids[currentGrid] = partition;
    
    //cout << *partition << endl;
	
	// Turn TFSF sources into auxiliary grids and TFSF links
    
	const std::vector<HuygensSurfaceDescPtr> surfs = currentGrid->
		getHuygensSurfaces();
	const std::vector<Vector3i> & gridSymmetries = partition->
		getHuygensRegionSymmetries();
	assert(surfs.size() == gridSymmetries.size());
	
	for (unsigned int nn = 0; nn < surfs.size(); nn++)
	if (surfs[nn]->getType() == kTFSFSource) // only TFSFSources turn into links
	{
		Vector3i sourceSymm = surfs[nn]->getSymmetries();
		Vector3i gridSymm = gridSymmetries[nn];
		Vector3i gridSize = currentGrid->getNumYeeCells();
		Vector3i collapsible(
			sourceSymm[0]*gridSymm[0] != 0 && gridSize[0] > 1,
			sourceSymm[1]*gridSymm[1] != 0 && gridSize[1] > 1,
			sourceSymm[2]*gridSymm[2] != 0 && gridSize[2] > 1);
		
		LOG << "collapsibleDimensions = " << collapsible << endl;
		
		if (norm2(collapsible) > 0)
		{
			LOG << "Collapsing the grid.\n";
			
			ostringstream auxGridName;
			auxGridName << currentGrid->getName() << "_autoaux_" << nn;
			GridDescPtr gPtr = makeAuxGridDescription(collapsible,
				currentGrid, surfs[nn], auxGridName.str());
			
			Rect3i auxPartitionWalls(partitionWalls);
			
			for (int ll = 0; ll < 3; ll++)
			if (collapsible[ll] != 0)
			{
				auxPartitionWalls.p1[ll] = 0;
				auxPartitionWalls.p2[ll] = 1;
			}
			voxelizeGridRecursor(voxelizedGrids, gPtr, numNodes, thisNode,
				auxPartitionWalls);
		}
		else if (currentGrid->getNumDimensions() == 1)
		{
			LOG << "Need to create last aux grid.\n";
			ostringstream srcGridName;
			srcGridName << currentGrid->getName() << "_autosrc";
			
			// makeSourceGridDescription will decide based on the
			// number of omitted sides whether to again make a TFSF
			// source or to use a hard source.
			GridDescPtr gPtr = makeSourceGridDescription(
				currentGrid, surfs[nn], srcGridName.str());
			voxelizeGridRecursor(voxelizedGrids, gPtr, numNodes, thisNode,
				partitionWalls);
		}
		else
		{
			cerr << "Error: we need a TFSF source, but the grid is not "
				"collapsible and the current grid is not 1D yet.  Dying.\n";
			exit(1);
		}
	}
	else if (surfs[nn]->getType() == kCustomTFSFSource)
	{
		// Write data request
		LOG << "Need to write data request.\n";
	}
}


GridDescPtr FDTDApplication::
makeAuxGridDescription(Vector3i collapsible, GridDescPtr parentGrid,
	HuygensSurfaceDescPtr huygensSurface, string auxGridName)
{
	int nn;
	Mat3i collapser(Mat3i::eye());
	collapser(0,0) = !collapsible[0];
	collapser(1,1) = !collapsible[1];
	collapser(2,2) = !collapsible[2];
	Vector3i origin(collapser*parentGrid->getOriginYee());
	
	const set<Vector3i> & omittedSides = huygensSurface->getOmittedSides();
	
	// What does the aux grid look like?
	// It's the size of the original total field region, collapsed, and all
	// non-1D dimensions get 10 cells of PML.
	
	Rect3i tfRect(collapser*huygensSurface->getHalfCells());
	tfRect.p2 = vec_max(tfRect.p2, Vector3i(1,1,1));
	
	Vector3i bigDimensions(!collapsible[0], !collapsible[1], !collapsible[2]);
    for (nn = 0; nn < 3; nn++)
	if (tfRect.size(nn) == 1)
		bigDimensions[nn] = 0;
	
	Rect3i copyTo(tfRect); // needs to be the same size as original TF rect
	tfRect.p1 -= bigDimensions*2; // expand TF rect by one cell
	tfRect.p2 += bigDimensions*2;
	
	Rect3i nonPMLRect(rectYeeToHalf(rectHalfToYee(tfRect))); // full Yee cells
	nonPMLRect.p1 -= bigDimensions*4;
	nonPMLRect.p2 += bigDimensions*4; // two Yee cells of spacing
	
	Rect3i gridBounds(nonPMLRect);
	gridBounds.p1 -= bigDimensions*20; // 10 cells of PML
	gridBounds.p2 += bigDimensions*20; // 10 cells of PML
	
	// We need to copy the structure from the parent grid to the child grid.
	// Determine the copyFrom rect; the copyTo rect is tfRect.
	// In directions which do NOT collapse, the copyFrom rect is the same size
	// as the copyTo rect.  However, if one side was omitted deliberately from
	// the Huygens surface, we are required to match only the other side.
	// If both sides are omitted, then the behavior is really quite arbitrary.
	// This is probably a rare case; still, for this reason, I'll print a
	// warning to the user here...
	for (nn = 0; nn < 3; nn++)
	if (bigDimensions[nn] != 0) // only applicable to nontrivial dimensions
	if (omittedSides.count(cardinalDirection(nn*2)) &&
		omittedSides.count(cardinalDirection(nn*2+1)))
		cerr << "Warning: Huygens surface facing sides (e.g. +x and -x) are "
			"both omitted.  This results in arbitrary and possibly incorrect "
			"behavior.\n";
    LOG << "What is omitted?\n";
    LOGMORE << omittedSides << endl;
	
	Rect3i copyFrom(huygensSurface->getHalfCells());
	for (nn = 0; nn < 3; nn++)
	if (omittedSides.count(cardinalDirection(nn*2)))
		copyFrom.p1[nn] = copyFrom.p2[nn];
	else if (omittedSides.count(cardinalDirection(nn*2+1)))
		copyFrom.p2[nn] = copyFrom.p1[nn];
	else if (collapsible[nn] != 0)
		copyFrom.p1[nn] = copyFrom.p2[nn]; // symmetry: either side is ok here!
	
	// Now let's shift all the rects to have the same origin (namely, zero).
	// copyFrom is not shifted because it is in the parent grid.
	nonPMLRect = nonPMLRect - gridBounds.p1;
	tfRect = tfRect - gridBounds.p1;
	copyTo = copyTo - gridBounds.p1;
	origin = origin - vecHalfToYee(gridBounds.p1);
	gridBounds = gridBounds - gridBounds.p1;

	GridDescPtr gPtr(new GridDescription(auxGridName,
		(gridBounds.size() + Vector3i(1,1,1))/2, // num Yee cells
		gridBounds, // calc region
		nonPMLRect,
		origin));
	
	HuygensSurfaceDescPtr hPtr;
	if (huygensSurface->getType() == kTFSFSource)
	{
		//	Omitting the "back side" is harmless in aux grids, and desirable
		//  for 1D aux grids in the terminal recursion.
		set<Vector3i> newSourceOmittedSides(omittedSides);
		newSourceOmittedSides.insert(huygensSurface->getDirection());
        if (huygensSurface->getFormula() != "")
            hPtr = HuygensSurfaceDescPtr(HuygensSurfaceDescription::
                newTFSFFormulaSource(huygensSurface->getSourceFields(),
                    huygensSurface->getFormula(),
                    huygensSurface->getDirection(),
                    tfRect,
                    ///huygensSurface->getHalfCells(),
                    newSourceOmittedSides,
                    huygensSurface->isTotalField()));
        else
            hPtr = HuygensSurfaceDescPtr(HuygensSurfaceDescription::
                newTFSFTimeSource(huygensSurface->getSourceFields(),
                    huygensSurface->getTimeFile(),
                    huygensSurface->getDirection(),
                    tfRect,
                    //huygensSurface->getHalfCells(),
                    newSourceOmittedSides,
                    huygensSurface->isTotalField()));
    }
	else if (huygensSurface->getType() == kCustomTFSFSource)
	{
		hPtr = HuygensSurfaceDescPtr(HuygensSurfaceDescription::
            newCustomTFSFSource(
			huygensSurface->getFile(),
			huygensSurface->getSymmetries(),
            tfRect,
            huygensSurface->getDuration(),
			huygensSurface->getOmittedSides(),
            huygensSurface->isTotalField()));
	}
	
	InstructionPtr copyThisRegion(new CopyFrom(copyFrom, copyTo, parentGrid));
	InstructionPtr extendThisRegion(new Extrude(copyTo, gridBounds));
	vector<InstructionPtr> assemblyStuff;
	assemblyStuff.push_back(copyThisRegion);
	assemblyStuff.push_back(extendThisRegion);
	AssemblyDescPtr assembly(new AssemblyDescription(assemblyStuff));
	
	gPtr->setHuygensSurfaces(vector<HuygensSurfaceDescPtr>(1,hPtr));
	gPtr->setAssembly(assembly);
	
	return gPtr;
}

GridDescPtr FDTDApplication::
makeSourceGridDescription(GridDescPtr parentGrid,
	HuygensSurfaceDescPtr huygensSurface, string srcGridName)
{
	assert(huygensSurface->getType() == kTFSFSource);
	int nn;
	
	const set<Vector3i> & omittedSides = huygensSurface->getOmittedSides();
	Vector3i srcDir = huygensSurface->getDirection();
	Vector3i origin = parentGrid->getOriginYee();
	
	// What does the source grid look like?
	// Two cases:
	//   1.  Back side of source region is omitted.  Then the source is tiny,
	//   just large enough to provide the fields to handle the front boundary.
	//   2.  Back side of source region is not omitted.  Then we clone the
	//   whole space and make a source with omitted back side.
	
	Rect3i tfRect(huygensSurface->getHalfCells());
	tfRect.p2 = vec_max(tfRect.p2, Vector3i(1,1,1));
	
	Vector3i bigDimensions(1,1,1);
	for (nn = 0; nn < 3; nn++)
	if (tfRect.size(nn) == 1)
		bigDimensions[nn] = 0;
	
	// Collapse the total-field rect if the TFSF boundary has an omitted side
	// along the direction of propagation.
	if (omittedSides.count(srcDir))
	{
		// Figure out which side this is... there should be a better way than
		// a crummy for-loop, huh.  Another opaque lookup table maybe?  (-:
		for (nn = 0; nn < 3; nn++)
		if (srcDir[nn] > 0)
			tfRect.p2[nn] = tfRect.p1[nn];
		else if (srcDir[nn] < 0)
			tfRect.p1[nn] = tfRect.p2[nn];
	}
	tfRect.p1 -= bigDimensions*2; // expand TF rect by one cell
	tfRect.p2 += bigDimensions*2;
	
	Rect3i nonPMLRect(YeeUtilities::rectYeeToHalf(
		YeeUtilities::rectHalfToYee(tfRect)));   // now it lies on Yee bounds
	nonPMLRect.p1 -= bigDimensions*4;
	nonPMLRect.p2 += bigDimensions*4; // two yee cells of spacing
	
	Rect3i gridBounds(nonPMLRect);
	gridBounds.p1 -= bigDimensions*20; // 10 cells of PML
	gridBounds.p2 += bigDimensions*20; // 10 cells of PML
	
	// We need to copy the structure from the parent grid to the child grid.
	// Determine the copyFrom rect; the copyTo rect is done above.
	// In directions which do NOT collapse, the copyFrom rect is the same size
	// as the copyTo rect.  However, if one side was omitted deliberately from
	// the Huygens surface, we are required to match only the other side.
	// If both sides are omitted, then the behavior is really quite arbitrary.
	// This is probably a rare case; still, for this reason, I'll print a
	// warning to the user here...
	for (nn = 0; nn < 3; nn++)
	if (bigDimensions[nn] == 0) // only applicable to nontrivial dimensions
	if (omittedSides.count(YeeUtilities::cardinalDirection(nn*2)) &&
		omittedSides.count(YeeUtilities::cardinalDirection(nn*2+1)))
		cerr << "Warning: Huygens surface facing sides (e.g. +x and -x) are "
			"both omitted.  This results in arbitrary and possibly incorrect "
			"behavior.\n";
	
	Rect3i copyFrom(huygensSurface->getHalfCells());
	for (nn = 0; nn < 3; nn++)
	if (omittedSides.count(YeeUtilities::cardinalDirection(nn*2)))
		copyFrom.p1[nn] = copyFrom.p2[nn];
	else if (omittedSides.count(YeeUtilities::cardinalDirection(nn*2+1)))
		copyFrom.p2[nn] = copyFrom.p1[nn];
	Rect3i copyTo(copyFrom);
	
	// Now let's shift all the rects to have the same origin (namely, zero).
	// copyFrom is not shifted because it is in the parent grid.
	nonPMLRect = nonPMLRect - gridBounds.p1;
	tfRect = tfRect - gridBounds.p1;
	copyTo = copyTo - gridBounds.p1;
	origin = origin - YeeUtilities::vecHalfToYee(gridBounds.p1);
	gridBounds = gridBounds - gridBounds.p1;

	GridDescPtr gPtr(new GridDescription(srcGridName,
		(gridBounds.size() + Vector3i(1,1,1))/2, // num Yee cells
		gridBounds, // calc region
		nonPMLRect,
		origin));
	
	vector<HuygensSurfaceDescPtr> huygensSurfaces; // might end up empty!
	vector<SourceDescPtr> hardSources; // or this will be empty.
	if (omittedSides.count(srcDir))
	{
		// use a hard source
		LOG << "Using a hard source.\n";
        const int SIGNIFIESHARDSOURCE = 0;
				
		Rect3i yeeTFRect = YeeUtilities::rectHalfToYee(tfRect);
		Vector3i srcYeeCell = clip(yeeTFRect, Vector3i(1000000*srcDir));
		srcYeeCell -= srcDir;
		
        vector<Region> regions(1, Region(Rect3i(srcYeeCell,srcYeeCell)));
        
		SourceDescPtr sPtr(new SourceDescription(
            huygensSurface->getSourceFields(),
			huygensSurface->getFormula(),
			huygensSurface->getTimeFile(),
			"", // HuygensSurface has no spaceTimeFile (TFSFSource anyway)
            SIGNIFIESHARDSOURCE,
            regions,
            vector<Duration>(1,huygensSurface->getDuration())));
		hardSources.push_back(sPtr);
	}
	else
	{
		// use a soft source
		LOG << "Using a soft source.\n";
        const int SIGNIFIESSOFTSOURCE = 1;
        
		HuygensSurfaceDescPtr hPtr(
            new HuygensSurfaceDescription(*huygensSurface, tfRect));
		huygensSurfaces.push_back(hPtr);
	}
	
	InstructionPtr copyThisRegion(new CopyFrom(copyFrom, copyTo, parentGrid));
	InstructionPtr extendThisRegion(new Extrude(copyTo, gridBounds));
	vector<InstructionPtr> assemblyStuff;
	assemblyStuff.push_back(copyThisRegion);
	assemblyStuff.push_back(extendThisRegion);
	AssemblyDescPtr assembly(new AssemblyDescription(assemblyStuff));
	
	gPtr->setSources(hardSources);
	gPtr->setHuygensSurfaces(huygensSurfaces);
	gPtr->setAssembly(assembly);
	
	return gPtr;
}


void FDTDApplication::
trimVoxelizedGrids(Map<GridDescPtr, VoxelizedPartitionPtr> & vgs)
{
    LOG << "Trimming the fat from " << vgs.size() << " grids.\n";
    map<GridDescPtr, VoxelizedPartitionPtr>::iterator itr;
    for (itr = vgs.begin(); itr != vgs.end(); itr++)
    {
        itr->second->clearVoxelGrid();
        itr->second->clearCellCountGrid();
    }
}

void FDTDApplication::
makeCalculationGrids(const SimulationDescPtr sim,
    Map<string, CalculationPartitionPtr> & calcs,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & voxParts)
{
    LOG << "Making calc grids for " << voxParts.size() << " grids.\n";
    map<GridDescPtr, VoxelizedPartitionPtr>::const_iterator itr;
    for (itr = voxParts.begin(); itr != voxParts.end(); itr++)
    {
        CalculationPartitionPtr calcPart(
            new CalculationPartition(*itr->second,
                sim->getDxyz(), sim->getDt(), sim->getDuration()));
        calcs[itr->first->getName()] = calcPart;
    }
}

void FDTDApplication::
allocateAuxBuffers(Map<string, CalculationPartitionPtr> & calcs)
{
    LOG << "Completing allocation.\n";
    
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcs.begin(); itr != calcs.end(); itr++)
        itr->second->allocateAuxBuffers();
}
    
void FDTDApplication::
calcE(Map<string, CalculationPartitionPtr> & calcGrids)
{
    //cout << "\tUpdating E fields\n";
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->calcE();
}

void FDTDApplication::
calcAfterE(Map<string, CalculationPartitionPtr> & calcGrids)
{
    //cout << "\tMPI E exchange\n\tOutput E\n";
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->calcAfterE();
}

void FDTDApplication::
calcH(Map<string, CalculationPartitionPtr> & calcGrids)
{
    //cout << "\tUpdating H fields\n";
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->calcH();
}

void FDTDApplication::
calcAfterH(Map<string, CalculationPartitionPtr> & calcGrids)
{
    //cout << "\tMPI H exchange\n\tOutput H\n";
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->calcAfterH();
}


/*
#pragma mark *** Run the simulation ***

void FDTDApplication::
runSimulation(Map<string, SimulationGridPtr> & simulationGrids)
{
	double t0, t1;
    LOGF << "runSimulation()..." << endl;
	
    map<string, SimulationGridPtr>::iterator gridItr;
    
    for (int n = 0; n < mNumT; n++)
    {
		mTimestepStartTimes[n] = getTimeInMicroseconds();
		
		t0 = getTimeInMicroseconds();
        cout << "\r                                                          "
            << flush;
        cout << "\rTimestep " << n+1 << " of " << mNumT << flush;
		t1 = getTimeInMicroseconds();
		mPrintTimestepTime += t1 - t0;
		
        //  1.  E fields
		t0 = getTimeInMicroseconds();
        for (gridItr = simulationGrids.begin();
            gridItr != simulationGrids.end(); gridItr++)
        {
            SimulationGridPtr & grid = (*gridItr).second;
            const string & nom = (*gridItr).first;            
            //grid->sanityCheck();
            //cout << nom << ": update E fields... " << flush;
            grid->calculateE(1/m_dx, 1/m_dy, 1/m_dz, m_dt);
            //grid->sanityCheck();
            //cout << "source E fields... " << flush;
			
            grid->sourceE(1/m_dx, 1/m_dy, 1/m_dz, m_dt, n);
            
            grid->inputE();
			
			//grid->writeCoordinatesToFields();
        }
		t1 = getTimeInMicroseconds();
		mUpdateETime += t1 - t0;
		
		t0 = getTimeInMicroseconds();
        for (gridItr = simulationGrids.begin();
            gridItr != simulationGrids.end(); gridItr++)
        {
            SimulationGridPtr & grid = (*gridItr).second;
            const string & nom = (*gridItr).first;      
			
			//grid->assertCoordinatesInFields();
			//LOG << "Skipping E buffers.\n";
			grid->updateBuffersE();
			//grid->assertCoordinatesInFields();
        }
		t1 = getTimeInMicroseconds();
		mBufferETime += t1 - t0;
		
        
        //  2.  H fields
		t0 = getTimeInMicroseconds();
        for (gridItr = simulationGrids.begin();
            gridItr != simulationGrids.end(); gridItr++)
        {
            SimulationGridPtr & grid = (*gridItr).second;
            const string & nom = (*gridItr).first;
            
            //grid->sanityCheck();
            //cout << "update H fields... " << flush;
            grid->calculateH(1/m_dx, 1/m_dy, 1/m_dz, m_dt);
                
            //grid->sanityCheck();
            //cout << "source H fields... " << flush;
            grid->sourceH(1/m_dx, 1/m_dy, 1/m_dz, m_dt, n);
            
            grid->inputH();
			
			//grid->writeCoordinatesToFields();
        }
		t1 = getTimeInMicroseconds();
		mUpdateHTime += t1 - t0;
		
		t0 = getTimeInMicroseconds();
        for (gridItr = simulationGrids.begin();
            gridItr != simulationGrids.end(); gridItr++)
        {
            SimulationGridPtr & grid = (*gridItr).second;
            const string & nom = (*gridItr).first;      
			
			//grid->assertCoordinatesInFields();
			//LOG << "Skipping H buffers.\n";
			grid->updateBuffersH();
			//grid->assertCoordinatesInFields();
        }
		t1 = getTimeInMicroseconds();
		mBufferHTime += t1 - t0;
        
        //  3.  Outputs
		t0 = getTimeInMicroseconds();
        for (gridItr = simulationGrids.begin();
            gridItr != simulationGrids.end(); gridItr++)
        {
            SimulationGridPtr & grid = (*gridItr).second;
            const string & nom = (*gridItr).first;
            
            //cout << "write output... " << flush;
            grid->output(m_dx, m_dy, m_dz, m_dt, n);
        }
		t1 = getTimeInMicroseconds();
		mOutputTime += t1 - t0;
	}
	
}
*/


/*
void FDTDApplication::
runAll(string parameterFile, int numThreads, bool runSim,
	bool output3D, bool dumpGrid, bool output2D,
	int numTimestepsOverride)
{
    //  The grid encompasses all the structures, material models, links, etc.
    Map<string, SetupGridPtr> setupGrids;
    Map<string, SimulationGridPtr> simulationGrids;
    Map<string, string> simulationParams;
    
	LOGF << "Run parameters:\n";
	LOGFMORE << "   parameter file = " << parameterFile << "\n";
	LOGFMORE << "   threads =        " << numThreads << "\n";
	LOGFMORE << "   output3D =       " << output3D << "\n";
	LOGFMORE << "   dumpgrid =       " << dumpGrid << "\n";
	LOGFMORE << "   output2D =       " << output2D << "\n";
	LOGFMORE << "   time override =  " << numTimestepsOverride << "\n";
	LOGFMORE << "   run simulation = " << runSim << endl;
	
	if (numThreads != 1)
	{
		LOG << "Error: multithreading disabled in this version.\n";
		exit(1);
	}
	
    //  this step is the ONLY one that uses tinyxml.
    cout << "Interpreting parameter file." << endl;
    readParameterFile(parameterFile, setupGrids,
        simulationParams, output3D, dumpGrid, output2D);
	
    simulationParams["dx"] >> m_dx;
    simulationParams["dy"] >> m_dy;
    simulationParams["dz"] >> m_dz;
    simulationParams["dt"] >> m_dt;
	if (numTimestepsOverride == -1)
		simulationParams["numT"] >> mNumT;
    else
		mNumT = numTimestepsOverride;
	mTimestepStartTimes.resize(mNumT);
	
	//	here's the recursion to create auxiliary grids.
	setupTFSFBuffers(setupGrids);
	
	//  Set up AFP source request files
	setupAFPRequests(setupGrids);
	
	if (runSim)
	{
		double t0, t1;
		map<string, SimulationGridPtr>::iterator gridItr;
		
		ofstream runlog("runlog.m");
		
		runlog << "% Auto-generated performance information\n"
			"% Simulation parameters (saved before simulation runs)\n";
		
		runlog << "trogdor.versionName = '" << TROGDOR_VERSION_TEXT << "';\n";
		
		runlog << "trogdor.dx = " << m_dx << ";\n"
			<< "trogdor.dy = " << m_dy << ";\n"
			<< "trogdor.dz = " << m_dz << ";\n"
			<< "trogdor.dt = " << m_dt << ";\n"
			<< "trogdor.numT = " << mNumT << ";\n"
			<< "trogdor.threads = " << numThreads << ";\n";
		
		runlog << flush;
		
		//  in this step, all structureGrids are destroyed.
		cout << "Setting up runtime (allocating fields, etc.)." << endl;
		t0 = getTimeInMicroseconds();
		initializeRuntime(setupGrids, simulationGrids, 
			numThreads, m_dx, m_dy, m_dz, m_dt);
		t1 = getTimeInMicroseconds();
		
		//runlog << "\nSetup time: " << (t1-t0)*1e-6 << " seconds" << endl;
		runlog << "trogdor.setupTime = " << (t1-t0)*1e-6 << ";" << endl;
		
		cout << "Running simulation." << endl;
		t0 = getTimeInMicroseconds();
		runSimulation(simulationGrids);
		t1 = getTimeInMicroseconds();
		cout << "\nSimulation ending.\n";
		
		runlog << "% Post-simulation analysis\n"
			"% Runtime is the total after the setup is complete.\n"
			"% Work time is the time not including printing timesteps to "
				"standard output.\n";
		runlog << "trogdor.runtime = " << (t1-t0)*1e-6 << ";\n";
		runlog << "trogdor.printTimestepTime = " << mPrintTimestepTime*1e-6
			<< ";\n";
		runlog << "trogdor.workTime = " << (t1-t0-mPrintTimestepTime)*1e-6
			<< ";\n";
		
		runlog << "% Per-grid analysis\n";
		
		double totalTotalMat_us = 0;
		double totalTotalOut_us = 0;
		double totalTotalIn_us = 0;
		double totalTotalSrc_us = 0;
		double totalTotalBuf_us = 0;
		
		int gridN = 1;
		for (gridItr = simulationGrids.begin();
                gridItr != simulationGrids.end(); gridItr++)
		{
			SimulationGridPtr & grid = (*gridItr).second;
			const string & nom = (*gridItr).first;
			
			ostringstream logPrefix;
			logPrefix << "trogdor.grid{" << gridN << "}.";
			string pre(logPrefix.str());
			
			runlog << pre << "name = '" << nom << "';\n";
			
			const vector<double> & materialTimes =
				grid->getMaterialMicroseconds();
			const vector<double> & outputTimes =
				grid->getOutputMicroseconds();
			const vector<double> & inputTimes =
				grid->getInputMicroseconds();
			const vector<double> & sourceTimes =
				grid->getSourceMicroseconds();
			const vector<double> & bufferTimes =
				grid->getBufferMicroseconds();
			
			const vector<MaterialPtr> & materials =
				grid->getMaterials();
						
			double totalMat_us = 0;
			double totalOut_us = 0;
			double totalIn_us = 0;
			double totalSrc_us = 0;
			double totalBuf_us = 0;
			
			//LOGF << "\t" << nom << ":\n";
			
			int cellTimesteps;
			int nn;
			
			for (nn = 0; nn < materialTimes.size(); nn++)
			{
				cellTimesteps = materials[nn]->numCells()*mNumT;
				
				runlog << pre << "material{" << nn+1 << "}.name = '"
					<< materials[nn]->getMaterialName() << "';\n";
				runlog << pre << "material{" << nn+1 << "}.numRunlines = "
					<< materials[nn]->numRunlines() << ";\n";
				runlog << pre << "material{" << nn+1 << "}.numHalfCells = "
					<< materials[nn]->numCells() << ";\n";
				runlog << pre << "material{" << nn+1 << "}.numYeeCells = "
					<< materials[nn]->numCells()/6.0 << ";\n";
				runlog << pre << "material{" << nn+1 << "}.halfCellTime = "
					<< materialTimes[nn]*1e-6/cellTimesteps << ";\n";
				
				totalMat_us += materialTimes[nn];
			}
			
			totalOut_us = accumulate(outputTimes.begin(), outputTimes.end(), 0.0);
			totalIn_us = accumulate(inputTimes.begin(), inputTimes.end(), 0.0);
			totalSrc_us = accumulate(sourceTimes.begin(), sourceTimes.end(), 0.0);
			totalBuf_us = accumulate(bufferTimes.begin(), bufferTimes.end(), 0.0);
			
			totalTotalMat_us += totalMat_us;
			totalTotalOut_us += totalOut_us;
			totalTotalIn_us += totalIn_us;
			totalTotalSrc_us += totalSrc_us;
			totalTotalBuf_us += totalBuf_us;
			
			runlog << pre << "calcTime = " << totalMat_us*1e-6 << ";\n";
			runlog << pre << "outputTime = " << totalOut_us*1e-6 << ";\n";
			runlog << pre << "inputTime = " << totalIn_us*1e-6 << ";\n";
			runlog << pre << "sourceTime = " << totalSrc_us*1e-6 << ";\n";
			runlog << pre << "bufferTime = " << totalBuf_us*1e-6 << ";\n";
			
			gridN++;
		}
		
		runlog << "% Cumulative performance information\n"
			"% The total___Time variables are summed inside loops\n"
			"% The loops___Time variables are summed outside loops\n"
			"% Subtract them to estimate loop overhead time.\n";
		
		runlog << "trogdor.totalCalcTime = " << totalTotalMat_us*1e-6
			<< ";\n";
		runlog << "trogdor.totalInputTime = " << totalTotalIn_us*1e-6
			<< ";\n";
		runlog << "trogdor.totalSourceTime = " << totalTotalSrc_us*1e-6
			<< ";\n";
		runlog << "trogdor.totalOutputTime = " << totalTotalOut_us*1e-6
			<< ";\n";
		runlog << "trogdor.totalBuffertime = " << totalTotalBuf_us*1e-6
			<< ";\n";
		
		runlog << "trogdor.loopsETime = " << mUpdateETime*1e-6 << ";\n";
		runlog << "trogdor.loopsHTime = " << mUpdateHTime*1e-6 << ";\n";
		runlog << "trogdor.loopsOutTime = " << mOutputTime*1e-6 << ";\n";
		runlog << "trogdor.loopsBufferETime = " << mBufferETime*1e-6
			<< ";\n";
		runlog << "trogdor.loopsBufferHTime = " << mBufferHTime*1e-6
			<< ";\n";
		
		//runlog << "\n\nTimestep trace (s):\n";
		
		runlog << "% Timestep trace\n";
		runlog << "trogdor.stepTimes = [...\n";
		
		double initTime = mTimestepStartTimes[0];
		for (int tt = 0; tt < mNumT; tt++)
		{
			runlog << "\t" << 1e-6*(mTimestepStartTimes[tt]-initTime)
				<< " ...\n";
			//runlog << "\t" << 1e-6*(mTimestepStartTimes[tt] - initTime) << "\n";
		}
		runlog << "];\n";
		runlog << "% End auto-generated performance information\n";
		
		runlog.close();

	}
}
*/

#pragma mark *** Singleton stuff ***

FDTDApplication & FDTDApplication::
getInstance()
{
    return sInstance;
}


FDTDApplication::
FDTDApplication() :
	m_dx(0.0),
	m_dy(0.0),
	m_dz(0.0),
	m_dt(0.0),
	mNumT(0),
	mTimestepStartTimes(),
	mUpdateETime(0.0),
	mUpdateHTime(0.0),
	mBufferETime(0.0),
	mBufferHTime(0.0),
	mOutputTime(0.0),
	mPrintTimestepTime(0.0)
{
}

FDTDApplication::
~FDTDApplication()
{
}
