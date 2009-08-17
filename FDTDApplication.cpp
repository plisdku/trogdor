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
#include "StructuralReports.h"

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


SimulationPreferences::
SimulationPreferences()
{
    numThreads = 1;
    numTimestepsOverride = -1;
    output3D = 0;
    output2D = 0;
    dumpGrid = 0;
    runSim = 1;
    runlineDirection = 'x';
}



void FDTDApplication::
runNew(string parameterFile, const SimulationPreferences & prefs)
{
    double t0, t1;
	Map<GridDescPtr, VoxelizedPartitionPtr> voxelizedGrids;
    Map<string, CalculationPartitionPtr> calculationGrids;
	
    t0 = getTimeInMicroseconds();
	SimulationDescPtr sim = loadSimulation(parameterFile);
    mNumT = sim->getDuration();
    t1 = getTimeInMicroseconds();
    mPerformance.setReadDescriptionMicroseconds(t1-t0);
	
    t0 = getTimeInMicroseconds();
    
    int runlineDirection = 0;
    if (prefs.runlineDirection == 'x')
        LOGF << "Not rotating.\n";
    else if (prefs.runlineDirection == 'y')
    {
        LOGF << "Rotating once.\n";
        runlineDirection = 1;
    }
    else if (prefs.runlineDirection == 'z')
    {
        LOGF << "Rotating twice.\n";
        runlineDirection = 2;
    }
    else
        throw(Exception("Bad fastaxis direction (should be x, y or z)."));
    
    // this step includes making setup runlines
	voxelizeGrids(sim, voxelizedGrids, runlineDirection);
	
	// in here: do any setup that requires the voxelized grids
	// extract all information that will be needed after the setup grid is gone
	t1 = getTimeInMicroseconds();
    mPerformance.setVoxelizeMicroseconds(t1-t0);
    
    map<GridDescPtr, VoxelizedPartitionPtr>::const_iterator itr;
    if (prefs.output2D)
    {
        for (itr = voxelizedGrids.begin(); itr != voxelizedGrids.end(); itr++)
        {
            StructuralReports::saveOutputCrossSections(*itr->first,
                *itr->second);
        }
    }
    if (prefs.output3D)
    {
        for (itr = voxelizedGrids.begin(); itr != voxelizedGrids.end(); itr++)
        {
            StructuralReports::saveMaterialBoundariesBeta(*itr->first,
                *itr->second);
        }
    }
    
    if (prefs.runSim == 0)
    {
        cout << "Not running simulation.\n";
        return;
    }
    
    t0 = getTimeInMicroseconds();
    trimVoxelizedGrids(voxelizedGrids); // delete VoxelGrid & PartitionCellCount
    makeCalculationGrids(sim, calculationGrids, voxelizedGrids);
	voxelizedGrids.clear();  // this will delete the setup objects
    allocateAuxBuffers(calculationGrids);
    t1 = getTimeInMicroseconds();
    mPerformance.setSetupCalculationMicroseconds(t1-t0);
	
	// allocate memory that can be postponed
    
    /*
    set<MemoryBuffer*>::const_iterator itr;
    LOG << "All buffers:\n";
    for (itr = MemoryBuffer::getAllBuffers().begin();
        itr != MemoryBuffer::getAllBuffers().end(); itr++)
        LOGMORE << *(*itr) << endl;
    */
    
	// RUN THE SIMULATION hoorah.
//    LOG << "Beginning calculation.\n";
    
    // The execution order here is a wee bit weird, with the priming-the-pump
    // step.  Timestep 0 is considered to be the initial condition, and is
    // settable through hard or soft sources.  Also timestep zero needs to be
    // written to the output files.  So there are N-1 updates in an N step
    // simulation.
    
    const bool BENCHMARK_ALL = 1;
    
    t0 = getTimeInMicroseconds();
    if (BENCHMARK_ALL)
        runTimed(calculationGrids);
    else
        runUntimed(calculationGrids);
    t1 = getTimeInMicroseconds();
    mPerformance.setRunCalculationMicroseconds(t1-t0);
    
    if (BENCHMARK_ALL)
    {
        reportPerformance(calculationGrids);
    }
    
    LOGF << "Done with simulation.\n";
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


void FDTDApplication::
voxelizeGrids(const SimulationDescPtr sim,
	Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
    int runlineDirection)
{
	static const int USE_MPI = 0;
	Rect3i myPartition, myCalcRegion;
    
    assert(runlineDirection >= 0 && runlineDirection < 3);
	
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
	GridDescPtr g;
    unsigned int ii;
    
	LOGF << "Stage 1: paint in the structure and recurse for aux grids.\n";
	for (ii = 0; ii < sim->getGrids().size(); ii++)
	{
		g = sim->getGrids()[ii];
		
		// the recursor paints the setup grid and creates new grids as needed
		// to implement all TFSF sources.
		
//		LOG << "Determining non-MPI partition size.\n";
		
		Vector3i numNodes(1,1,1);
		Vector3i thisNode(0,0,0);
		
		Rect3i partitionWallsHalf = Rect3i(-100000000, -100000000, -100000000, 
			100000000, 100000000, 100000000);
		
		voxelizeGridRecursor(voxelizedGrids, g, numNodes, thisNode,
			partitionWallsHalf, runlineDirection);
	}
    
    LOGF << "Stage 2: Create setup Huygens surfaces and paint them in.\n";
    map<GridDescPtr, VoxelizedPartitionPtr>::iterator itr;
    for (itr = voxelizedGrids.begin(); itr != voxelizedGrids.end(); itr++)
    {
        itr->second->createHuygensSurfaces(itr->first,
            voxelizedGrids);
    }
    
    LOGF << "Stage 3: make the runlines.\n";
    for (itr = voxelizedGrids.begin(); itr != voxelizedGrids.end(); itr++)
    {
        itr->second->calculateRunlines();
    }
}

void FDTDApplication::
voxelizeGridRecursor(Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
	GridDescPtr currentGrid, Vector3i numNodes, Vector3i thisNode, 
	Rect3i partitionWallsHalf, int runlineDirection )
{
	Rect3i myPartitionHalfCells = clip(partitionWallsHalf,
        currentGrid->getHalfCellBounds());
	Rect3i myCalcHalfCells(myPartitionHalfCells);
	Rect3i myAllocatedHalfCells(myPartitionHalfCells);
    
	for (int mm = 0; mm < 3; mm++)
	if (numNodes[mm] != 1 && currentGrid->getNumYeeCells()[mm] != 1)
	{
		myAllocatedHalfCells.p1[mm] -= 2;
		myAllocatedHalfCells.p2[mm] += 2;
	}
	
    /*
    LOG << "Recursing:\n";
	LOGMORE << "Recursing for grid " << currentGrid->getName() << ".\n";
	LOGMORE << "I am partition " << thisNode << " of " << numNodes << ".\n";
	LOGMORE << "My node partition region is " << partitionWallsHalf << "\n";
	LOGMORE << "My partition is " << myPartitionHalfCells << ".\n";
	LOGMORE << "I allocate fields in " << myAllocatedHalfCells << ".\n";
	LOGMORE << "My calc region is " << myCalcHalfCells << ".\n";
	*/
    
	static const int EXTRUDE_PML = 1;
	if (EXTRUDE_PML)
	{
		LOGF << "Adding an Extrude command to implement PML.\n";
		
		InstructionPtr extendThisRegion(new Extrude(
			currentGrid->getNonPMLHalfCells(),
			currentGrid->getHalfCellBounds() ));
		vector<InstructionPtr> assemblyStuff(
			currentGrid->getAssembly()->getInstructions());
		assemblyStuff.push_back(extendThisRegion);
		AssemblyDescPtr assembly(new AssemblyDescription(assemblyStuff));
		currentGrid->setAssembly(assembly);
	}
	
	VoxelizedPartitionPtr partition(new VoxelizedPartition(
		*currentGrid, voxelizedGrids, myAllocatedHalfCells, myCalcHalfCells,
        runlineDirection));
	voxelizedGrids[currentGrid] = partition;
    
    //cout << *partition << endl;
	
	// Turn TFSF sources into auxiliary grids and TFSF links
    
	const std::vector<HuygensSurfaceDescPtr> surfs = currentGrid->
		getHuygensSurfaces();
	const std::vector<Vector3i> & gridSymmetries = partition->
		getHuygensRegionSymmetries();
	assert(surfs.size() == gridSymmetries.size());
	
	for (unsigned int nn = 0; nn < surfs.size(); nn++)
    if (surfs[nn]->getType() == kLink)
    {
//        LOG << "Links don't recurse.\n";
    }
    else
	{
		Vector3i sourceSymm = surfs[nn]->getSymmetries();
		Vector3i gridSymm = gridSymmetries[nn];
		Vector3i gridYeeCells = currentGrid->getNumYeeCells();
		Vector3i collapsible(
			sourceSymm[0]*gridSymm[0] != 0 && gridYeeCells[0] > 1,
			sourceSymm[1]*gridSymm[1] != 0 && gridYeeCells[1] > 1,
			sourceSymm[2]*gridSymm[2] != 0 && gridYeeCells[2] > 1);
		
		//LOG << "collapsibleDimensions = " << collapsible << endl;
		
		if (norm2(collapsible) > 0)
		{
			//LOG << "Collapsing the grid.\n";		
			ostringstream auxGridName;
			auxGridName << currentGrid->getName() << "_autoaux_" << nn;
			GridDescPtr gPtr = makeAuxGridDescription(collapsible,
				currentGrid, surfs[nn], auxGridName.str());
			
			Rect3i auxPartitionWallsHalf(partitionWallsHalf);
			
			for (int ll = 0; ll < 3; ll++)
			if (collapsible[ll] != 0)
			{
				auxPartitionWallsHalf.p1[ll] = 0;
				auxPartitionWallsHalf.p2[ll] = 1;
			}
			voxelizeGridRecursor(voxelizedGrids, gPtr, numNodes, thisNode,
				auxPartitionWallsHalf, runlineDirection);
		}
		else if (currentGrid->getNumDimensions() == 1)
		{
            assert(surfs[nn]->getType() != kLink); // how could this happen?
            
			//LOG << "Need to create last aux grid.\n";
			ostringstream srcGridName;
			srcGridName << currentGrid->getName() << "_autosrc";
			
			// makeSourceGridDescription will decide based on the
			// number of omitted sides whether to again make a TFSF
			// source or to use a hard source.
			GridDescPtr gPtr = makeSourceGridDescription(
				currentGrid, surfs[nn], srcGridName.str());
			voxelizeGridRecursor(voxelizedGrids, gPtr, numNodes, thisNode,
				partitionWallsHalf, runlineDirection);
		}
		else if (surfs[nn]->getType() != kCustomTFSFSource)
		{
			cerr << "Error: we need a TFSF source, but the grid is not "
				"collapsible and the current grid is not 1D yet.  Dying.\n";
			exit(1);
		}
        else
        {
            assert(surfs[nn]->getType() == kCustomTFSFSource);
            voxelizedGrids[currentGrid]->writeDataRequest(surfs[nn],
                currentGrid);
        }
	}
}


GridDescPtr FDTDApplication::
makeAuxGridDescription(Vector3i collapsible, GridDescPtr parentGrid,
	HuygensSurfaceDescPtr huygensSurface, string auxGridName)
{
	int nn;
    //Mat3i collapser(Mat3i::diagonal(1-collapsible));
    Mat3i collapser(Mat3i::diagonal(!collapsible));
    Vector3i originYee(collapser*parentGrid->getOriginYee());
	
	const set<Vector3i> & omittedSides = huygensSurface->getOmittedSides();
	
	// What does the aux grid look like?
	// It's the size of the original total field region, collapsed, and all
	// non-1D dimensions get 10 cells of PML.
	
	Rect3i tfHalfCells(collapser*huygensSurface->getHalfCells());
	tfHalfCells.p2 = vec_max(tfHalfCells.p2, Vector3i(1,1,1));
    Rect3i linkSourceHalfCells(tfHalfCells);
	
	//Vector3i bigDimensions(!collapsible[0], !collapsible[1], !collapsible[2]);
    Vector3i bigDimensions(!collapsible);
    for (nn = 0; nn < 3; nn++)
	if (tfHalfCells.size(nn) == 1)
		bigDimensions[nn] = 0;
	
	Rect3i copyTo(tfHalfCells); // needs to be the same size as original TF rect
	tfHalfCells.p1 -= bigDimensions*2; // expand TF rect by one cell
	tfHalfCells.p2 += bigDimensions*2;
	
    // make nonPMLHalfCells be full Yee cells in size.
    // This line does something!
	Rect3i nonPMLHalfCells(yeeToHalf(halfToYee(tfHalfCells)));
	nonPMLHalfCells.p1 -= bigDimensions*4;
	nonPMLHalfCells.p2 += bigDimensions*4; // two Yee cells of spacing
	
	Rect3i gridHalfCells(nonPMLHalfCells);
	gridHalfCells.p1 -= bigDimensions*20; // 10 Yee cells of PML
	gridHalfCells.p2 += bigDimensions*20; // 10 Yee cells of PML
	
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
	if (omittedSides.count(cardinal(nn*2)) &&
		omittedSides.count(cardinal(nn*2+1)))
		cerr << "Warning: Huygens surface facing sides (e.g. +x and -x) are "
			"both omitted.  This results in arbitrary and possibly incorrect "
			"behavior.\n";
    //LOG << "What is omitted?\n";
    //LOGMORE << omittedSides << endl;
	
	Rect3i copyFrom(huygensSurface->getHalfCells());
	for (nn = 0; nn < 3; nn++)
	if (omittedSides.count(cardinal(nn*2)))
		copyFrom.p1[nn] = copyFrom.p2[nn];
	else if (omittedSides.count(cardinal(nn*2+1)))
		copyFrom.p2[nn] = copyFrom.p1[nn];
	else if (collapsible[nn] != 0)
		copyFrom.p1[nn] = copyFrom.p2[nn]; // symmetry: either side is ok here!
	
	// Now let's shift all the rects to have the same origin (namely, zero).
	// copyFrom is not shifted because it is in the parent grid.
	nonPMLHalfCells = nonPMLHalfCells - gridHalfCells.p1;
	tfHalfCells = tfHalfCells - gridHalfCells.p1;
	copyTo = copyTo - gridHalfCells.p1;
	originYee = originYee - halfToYee(gridHalfCells.p1);
    linkSourceHalfCells = linkSourceHalfCells - gridHalfCells.p1;
	gridHalfCells = gridHalfCells - gridHalfCells.p1;

	GridDescPtr childGrid(new GridDescription(auxGridName,
		(gridHalfCells.size() + Vector3i(1,1,1))/2, // num Yee cells
		gridHalfCells, // calc region
		nonPMLHalfCells,
		originYee,
        parentGrid->getDxyz(),
        parentGrid->getDt()));
	
	HuygensSurfaceDescPtr childSource;
	if (huygensSurface->getType() == kTFSFSource)
	{
		//	Omitting the "back side" is harmless in aux grids, and desirable
		//  for 1D aux grids in the terminal recursion.
		set<Vector3i> newSourceOmittedSides(omittedSides);
		newSourceOmittedSides.insert(huygensSurface->getDirection());
        if (huygensSurface->getFormula() != "")
            childSource = HuygensSurfaceDescPtr(HuygensSurfaceDescription::
                newTFSFFormulaSource(huygensSurface->getSourceFields(),
                    huygensSurface->getFormula(),
                    huygensSurface->getDirection(),
                    tfHalfCells,
                    ///huygensSurface->getHalfCells(),
                    newSourceOmittedSides,
                    huygensSurface->isTotalField()));
        else
            childSource = HuygensSurfaceDescPtr(HuygensSurfaceDescription::
                newTFSFTimeSource(huygensSurface->getSourceFields(),
                    huygensSurface->getTimeFile(),
                    huygensSurface->getDirection(),
                    tfHalfCells,
                    //huygensSurface->getHalfCells(),
                    newSourceOmittedSides,
                    huygensSurface->isTotalField()));
    }
	else if (huygensSurface->getType() == kCustomTFSFSource)
	{
		childSource = HuygensSurfaceDescPtr(HuygensSurfaceDescription::
            newCustomTFSFSource(
			huygensSurface->getFile(),
			huygensSurface->getSymmetries(),
            tfHalfCells,
            huygensSurface->getDuration(),
			huygensSurface->getOmittedSides(),
            huygensSurface->isTotalField()));
	}
	
	InstructionPtr copyThisRegion(new CopyFrom(copyFrom, copyTo, parentGrid));
	InstructionPtr extendThisRegion(new Extrude(copyTo, gridHalfCells));
	vector<InstructionPtr> assemblyStuff;
	assemblyStuff.push_back(copyThisRegion);
	assemblyStuff.push_back(extendThisRegion);
	AssemblyDescPtr assembly(new AssemblyDescription(assemblyStuff));
	
	childGrid->setHuygensSurfaces(vector<HuygensSurfaceDescPtr>(1,childSource));
	childGrid->setAssembly(assembly);
    
    // And last of all, turn the parent source into a link to the child grid.
    huygensSurface->becomeLink(childGrid, linkSourceHalfCells);
	
	return childGrid;
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
    
    //LOG << omittedSides << "\n";
	
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
    Rect3i linkSourceHalfCells(tfRect);
	
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
	
	Rect3i nonPMLRect(YeeUtilities::yeeToHalf(
		YeeUtilities::halfToYee(tfRect)));   // now it lies on Yee bounds
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
	if (omittedSides.count(YeeUtilities::cardinal(nn*2)) &&
		omittedSides.count(YeeUtilities::cardinal(nn*2+1)))
		cerr << "Warning: Huygens surface facing sides (e.g. +x and -x) are "
			"both omitted.  This results in arbitrary and possibly incorrect "
			"behavior.\n";
	
	Rect3i copyFrom(huygensSurface->getHalfCells());
	for (nn = 0; nn < 3; nn++)
	if (omittedSides.count(YeeUtilities::cardinal(nn*2)))
		copyFrom.p1[nn] = copyFrom.p2[nn];
	else if (omittedSides.count(YeeUtilities::cardinal(nn*2+1)))
		copyFrom.p2[nn] = copyFrom.p1[nn];
	Rect3i copyTo(copyFrom);
	
	// Now let's shift all the rects to have the same origin (namely, zero).
	// copyFrom is not shifted because it is in the parent grid.
	nonPMLRect = nonPMLRect - gridBounds.p1;
	tfRect = tfRect - gridBounds.p1;
	copyTo = copyTo - gridBounds.p1;
	origin = origin - YeeUtilities::halfToYee(gridBounds.p1);
    linkSourceHalfCells = linkSourceHalfCells - gridBounds.p1;
	gridBounds = gridBounds - gridBounds.p1;

	GridDescPtr childGrid(new GridDescription(srcGridName,
		(gridBounds.size() + Vector3i(1,1,1))/2, // num Yee cells
		gridBounds, // calc region
		nonPMLRect,
		origin,
        parentGrid->getDxyz(),
        parentGrid->getDt()));
	
	vector<HuygensSurfaceDescPtr> huygensSurfaces; // might end up empty!
	vector<SourceDescPtr> hardSources; // or this will be empty.
	if (omittedSides.count(srcDir))
	{
		// use a hard source
		//LOG << "Using a hard source.\n";
        const int SIGNIFIESHARDSOURCE = 0;
        //const int SIGNIFIESSOFTSOURCE = 1;
				
		Rect3i yeeTFRect = YeeUtilities::halfToYee(tfRect);
		Vector3i srcYeeCell = clip(yeeTFRect, Vector3i(1000000*srcDir));
		srcYeeCell -= 2*srcDir;
		
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
		// use a soft source, and omit the back side
//		LOG << "Using a soft source.\n";
        
		HuygensSurfaceDescPtr hPtr(
            new HuygensSurfaceDescription(*huygensSurface, tfRect));
        hPtr->omitSide(srcDir);
		huygensSurfaces.push_back(hPtr);
	}
	
	InstructionPtr copyThisRegion(new CopyFrom(copyFrom, copyTo, parentGrid));
	InstructionPtr extendThisRegion(new Extrude(copyTo, gridBounds));
	vector<InstructionPtr> assemblyStuff;
	assemblyStuff.push_back(copyThisRegion);
	assemblyStuff.push_back(extendThisRegion);
	AssemblyDescPtr assembly(new AssemblyDescription(assemblyStuff));
	
	childGrid->setSources(hardSources);
	childGrid->setHuygensSurfaces(huygensSurfaces);
	childGrid->setAssembly(assembly);
    
    // And last of all, turn the parent source into a link to the child grid.
    huygensSurface->becomeLink(childGrid, linkSourceHalfCells);
	
	return childGrid;
}


void FDTDApplication::
trimVoxelizedGrids(Map<GridDescPtr, VoxelizedPartitionPtr> & vgs)
{
    LOGF << "Trimming the fat from " << vgs.size() << " grids.\n";
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
    LOGF << "Making calc grids for " << voxParts.size() << " grids.\n";
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
//    LOG << "Completing allocation.\n";
    
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcs.begin(); itr != calcs.end(); itr++)
        itr->second->allocateAuxBuffers();
}


void FDTDApplication::
updateE(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->updateE(timestep);
}

void FDTDApplication::
sourceE(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->sourceE(timestep);
}

void FDTDApplication::
outputE(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->outputE(timestep);
}

void FDTDApplication::
updateH(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->updateH(timestep);
}

void FDTDApplication::
sourceH(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->sourceH(timestep);
}

void FDTDApplication::
outputH(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->outputH(timestep);
}

void FDTDApplication::
updateETimed(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->timedUpdateE(timestep);
//    if (calcGrids.count("Main Grid"))
//        calcGrids["Main Grid"]->printFields(cout, octantE(1), 1.0);
}

void FDTDApplication::
sourceETimed(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->timedSourceE(timestep);
}

void FDTDApplication::
outputETimed(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->timedOutputE(timestep);
}

void FDTDApplication::
updateHTimed(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->timedUpdateH(timestep);
}

void FDTDApplication::
sourceHTimed(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->timedSourceH(timestep);
}

void FDTDApplication::
outputHTimed(Map<string, CalculationPartitionPtr> & calcGrids, int timestep)
{
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calcGrids.begin(); itr != calcGrids.end(); itr++)
        itr->second->timedOutputH(timestep);
}

void FDTDApplication::
runUntimed(Map<string, CalculationPartitionPtr> & calculationGrids)
{
    sourceE(calculationGrids, 0);
    outputE(calculationGrids, 0);
    sourceH(calculationGrids, 0);
    outputH(calculationGrids, 0);
    for (long tt = 1; tt < mNumT; tt++)
    {
        cout << "\r                                                          "
            << flush;
        cout << "\rTimestep " << tt << " of " << mNumT << flush;
        updateE(calculationGrids, tt);
        sourceE(calculationGrids, tt);
        outputE(calculationGrids, tt);
        updateH(calculationGrids, tt);
        sourceH(calculationGrids, tt);
        outputH(calculationGrids, tt);
    }
}

void FDTDApplication::
runTimed(Map<string, CalculationPartitionPtr> & calculationGrids)
{
    double t0, t1, printTimestepTotalTime;
    printTimestepTotalTime = 0.0;
    
    sourceETimed(calculationGrids, 0);
    outputETimed(calculationGrids, 0);
    sourceHTimed(calculationGrids, 0);
    outputHTimed(calculationGrids, 0);
    for (long tt = 1; tt < mNumT; tt++)
    {
		t0 = getTimeInMicroseconds();
        cout << "\r                                                          "
            << flush;
        cout << "\rTimestep " << tt << " of " << mNumT << flush;
		t1 = getTimeInMicroseconds();
        printTimestepTotalTime += (t1-t0);
        updateETimed(calculationGrids, tt);
        sourceETimed(calculationGrids, tt);
        outputETimed(calculationGrids, tt);
        updateHTimed(calculationGrids, tt);
        sourceHTimed(calculationGrids, tt);
        outputHTimed(calculationGrids, tt);
    }
    mPerformance.setPrintTimestepMicroseconds(printTimestepTotalTime);
}

void FDTDApplication::
reportPerformance(Map<string, CalculationPartitionPtr> & calculationGrids)
{
    ofstream runlog("runlog.m");
    mPerformance.printForMatlab(runlog);
    int gridN = 1;
    map<string, CalculationPartitionPtr>::iterator itr;
    for (itr = calculationGrids.begin(); itr != calculationGrids.end(); itr++)
    {
        ostringstream prefix;
        prefix << "trogdor.grid{" << gridN << "}.";
        runlog << prefix.str() << "name = '" << itr->first << "';\n";
        itr->second->printPerformanceForMatlab(runlog, prefix.str());
        gridN++;
    }
    
    
    
    runlog.close();
}



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
	mNumT(0)
{
}

FDTDApplication::
~FDTDApplication()
{
}
