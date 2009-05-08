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

#include <Magick++.h>

#include <set>
#include <vector>
#include <fstream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <sstream>

using namespace std;

FDTDApplication FDTDApplication::sInstance;

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
    completeFieldAllocation(calculationGrids);
	
	// allocate memory that can be postponed
	
	// RUN THE SIMULATION hoorah.
    LOG << "Beginning calculation.\n";
    
    long numT = sim->getDuration();
    for (long tt = 0; tt < numT; tt++)
    {
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
	return Mat3i(1); // identity matrix
}

void FDTDApplication::
voxelizeGrids(const SimulationDescPtr sim,
	Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids)
{
	// Make a copy of the grid descriptions.  We need the ability to modify
	// them, because TFSFSources may require the creation of additional grids.
	
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
	
	LOG << "Recursing for grid " << currentGrid->getName() << ".\n";
	LOG << "I am partition " << thisNode << " of " << numNodes << ".\n";
	LOG << "My node partition region is " << partitionWalls << "\n";
	LOG << "My partition is " << myPartition << ".\n";
	LOG << "I allocate fields over the region " << allocRegion << ".\n";
	LOG << "My calc region is " << myCalcRegion << ".\n";
	
	LOG << "HEY!!!!! Lookee here, this function currently does not have any"
		" sort of boundary region to store neighboring nodes' fields.\n";
	
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
	
	VoxelizedPartitionPtr setupGrid(new VoxelizedPartition(
		*currentGrid, voxelizedGrids, allocRegion, myCalcRegion));
	voxelizedGrids[currentGrid] = setupGrid;
	
	// Handle TFSF sources.
	const std::vector<HuygensSurfaceDescPtr> surfs = currentGrid->
		getHuygensSurfaces();
	const std::vector<Vector3i> & gridSymmetries = setupGrid->
		getHuygensRegionSymmetries();
	assert(surfs.size() == gridSymmetries.size());
	
	for (unsigned int nn = 0; nn < surfs.size(); nn++)
	if (surfs[nn]->getType() == kTFSFSource) // only TFSFSources turn into links
	{
		Vector3i sourceSymm = surfs[nn]->getTFSFSourceSymmetries();
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
	else if (surfs[nn]->getType() == kDataRequest)
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
	Mat3i collapser(1);
	collapser(0,0) = !collapsible[0];
	collapser(1,1) = !collapsible[1];
	collapser(2,2) = !collapsible[2];
	Vector3i origin(collapser*parentGrid->getOriginYee());
	
	const set<Vector3i> & omittedSides = huygensSurface->getOmittedSides();
	
	// What does the aux grid look like?
	// It's the size of the original total field region, collapsed, and all
	// non-1D dimensions get 10 cells of PML.
	
	Rect3i tfRect(collapser*huygensSurface->getDestHalfRect());
	tfRect.p2 = maxval(tfRect.p2, Vector3i(1,1,1));
	
	Vector3i bigDimensions = Vector3i(1,1,1) - collapsible; // == !collapsible
	for (nn = 0; nn < 3; nn++)
	if (tfRect.size(nn) == 1)
		bigDimensions[nn] = 0;
	
	Rect3i copyTo(tfRect); // needs to be the same size as original TF rect
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
	// Determine the copyFrom rect; the copyTo rect is tfRect.
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
	
	Rect3i copyFrom(huygensSurface->getDestHalfRect());
	for (nn = 0; nn < 3; nn++)
	if (omittedSides.count(YeeUtilities::cardinalDirection(nn*2)))
		copyFrom.p1[nn] = copyFrom.p2[nn];
	else if (omittedSides.count(YeeUtilities::cardinalDirection(nn*2+1)))
		copyFrom.p2[nn] = copyFrom.p1[nn];
	else if (collapsible[nn] != 0)
		copyFrom.p1[nn] = copyFrom.p2[nn]; // symmetry: either side is ok here!
	
	// Now let's shift all the rects to have the same origin (namely, zero).
	// copyFrom is not shifted because it is in the parent grid.
	nonPMLRect = nonPMLRect - gridBounds.p1;
	tfRect = tfRect - gridBounds.p1;
	copyTo = copyTo - gridBounds.p1;
	origin = origin - YeeUtilities::vecHalfToYee(gridBounds.p1);
	gridBounds = gridBounds - gridBounds.p1;

	
	GridDescPtr gPtr(new GridDescription(auxGridName,
		(gridBounds.size() + Vector3i(1,1,1))/2, // num Yee cells
		gridBounds.size() + Vector3i(1,1,1), // num half cells
		gridBounds, // calc region
		nonPMLRect,
		origin));
	
	HuygensSurfaceDescPtr hPtr;
	if (huygensSurface->getType() == kTFSFSource)
	{
		//	Omitting the "back side" is harmless in aux grids, and desirable
		//  for 1D aux grids in the terminal recursion.
		set<Vector3i> newSourceOmittedSides(omittedSides);
		newSourceOmittedSides.insert(huygensSurface->getTFSFSourceDirection());
		hPtr = HuygensSurfaceDescPtr(HuygensSurfaceDescription::newTFSFSource(
			tfRect,
			huygensSurface->getTFSFSourceDirection(),
			huygensSurface->getTFSFSourceFormula(),
			huygensSurface->getTFSFSourceFileName(),
			huygensSurface->getTFSFSourcePolarization(),
			huygensSurface->getTFSFSourceField(),
			huygensSurface->getTFSFType(),
			huygensSurface->getTFSFSourceParameters(),
			newSourceOmittedSides));
	}
	else if (huygensSurface->getType() == kDataRequest)
	{
		hPtr = HuygensSurfaceDescPtr(HuygensSurfaceDescription::newCustomSource(
			tfRect,
			huygensSurface->getName(),
			huygensSurface->getTFSFSourceSymmetries(),
			huygensSurface->getTFSFType(),
			huygensSurface->getTFSFSourceParameters(),
			huygensSurface->getOmittedSides()));
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
	Vector3i srcDir = huygensSurface->getTFSFSourceDirection();
	Vector3i origin = parentGrid->getOriginYee();
	
	// What does the source grid look like?
	// Two cases:
	//   1.  Back side of source region is omitted.  Then the source is tiny,
	//   just large enough to provide the fields to handle the front boundary.
	//   2.  Back side of source region is not omitted.  Then we clone the
	//   whole space and make a source with omitted back side.
	
	Rect3i tfRect(huygensSurface->getDestHalfRect());
	tfRect.p2 = maxval(tfRect.p2, Vector3i(1,1,1));
	
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
	
	Rect3i copyFrom(huygensSurface->getDestHalfRect());
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
		gridBounds.size() + Vector3i(1,1,1), // num half cells
		gridBounds, // calc region
		nonPMLRect,
		origin));
	
	vector<HuygensSurfaceDescPtr> huygensSurfaces; // might end up empty!
	vector<SourceDescPtr> hardSources; // or this will be empty.
	if (omittedSides.count(srcDir))
	{
		// use a hard source
		LOG << "Using a hard source.\n";
				
		Rect3i yeeTFRect = YeeUtilities::rectHalfToYee(tfRect);
		Vector3i srcYeeCell = clip(yeeTFRect, Vector3i(1000000*srcDir));
		srcYeeCell -= srcDir;
		
		LOG << "To-do: ditch TFSFSource Params.\n";
		SourceDescPtr sPtr(new SourceDescription(
			huygensSurface->getTFSFSourceFormula(),
			huygensSurface->getTFSFSourceFileName(),
			huygensSurface->getTFSFSourcePolarization(),
			Rect3i(srcYeeCell, srcYeeCell),
			huygensSurface->getTFSFSourceField(),
			huygensSurface->getTFSFSourceParameters()));
		hardSources.push_back(sPtr);
	}
	else
	{
		// use a soft source
		LOG << "Using a soft source.\n";
		HuygensSurfaceDescPtr hPtr(HuygensSurfaceDescription::newTFSFSource(
			tfRect,
			huygensSurface->getTFSFSourceDirection(),
			huygensSurface->getTFSFSourceFormula(),
			huygensSurface->getTFSFSourceFileName(),
			huygensSurface->getTFSFSourcePolarization(),
			huygensSurface->getTFSFSourceField(),
			huygensSurface->getTFSFType(),
			huygensSurface->getTFSFSourceParameters(),
			huygensSurface->getOmittedSides()));
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
completeFieldAllocation(Map<string, CalculationPartitionPtr> & calcs)
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




#if 0

#pragma mark *** Setup phase ***

void FDTDApplication::
readParameterFile(string paramFileName, Map<string, SetupGridPtr> & setupGrids,
    Map<string, string> & simulationParams,
	bool output3D, bool dumpGrid, bool output2D )
{
    string errorMessage;
    
    TiXmlBase::SetCondenseWhiteSpace(0); // for ascii material layers with \n
    TiXmlDocument paramDoc(paramFileName.c_str());
    if (!(paramDoc.LoadFile()))
    {
        cerr << "Could not load parameter file " << paramFileName << endl;
        cerr << "Reason: " << paramDoc.ErrorDesc() << endl;
        cerr << "(possible location: row " << paramDoc.ErrorRow() << " column "
             << paramDoc.ErrorCol() << " .)" << endl;
        exit(1);
    }
    
    TiXmlElement* elem = paramDoc.RootElement();
    if (!elem)
    {
        cerr << "Parameter file " << paramFileName << " does not seem to have"
             << " a root element.\n";
        exit(1);
    }
    
    simulationParams = getAttributes(elem);
    
    //  Load the grids one at a time.
    TiXmlElement* gridElem;
    gridElem = elem->FirstChildElement("Grid");
    while (gridElem) // FOR EACH GRID: Load data
    {
        int nnx;
        int nny;
        int nnz;
        Rect3i activeRegion;
        Rect3i regionOfInterest;
        string name;
        Map<string, string> gridParams = getAttributes(gridElem);
        
        if (!validateSetupGrid(gridParams, errorMessage, gridElem->Row()))
        {
            cerr << errorMessage;
            exit(1);
        }
        
        name = gridParams["name"];
        if (gridParams.count("nx"))
        {
            gridParams["nx"] >> nnx;
            nnx *= 2;
        }
        else
            gridParams["nnx"] >> nnx;
        
        if (gridParams.count("ny"))
        {
            gridParams["ny"] >> nny;
            nny *= 2;
        }
        else
            gridParams["nny"] >> nny;
        
        if (gridParams.count("nz"))
        {
            gridParams["nz"] >> nnz;
            nnz *= 2;
        }
        else
            gridParams["nnz"] >> nnz;
        
        if (gridParams.count("regionOfInterest"))
        {
            gridParams["regionOfInterest"] >> regionOfInterest;
            regionOfInterest *= 2;
            regionOfInterest.p2 += Vector3i(1, 1, 1);
        }
        else
            gridParams["roi"] >> regionOfInterest;
        
        if (gridParams.count("activeRegion") != 0)
            gridParams["activeRegion"] >> activeRegion;
        else
        {
            activeRegion = Rect3i(1, 1, 1, nnx-2, nny-2, nnz-2);
            if (regionOfInterest.p1[0] == 0)
                activeRegion.p1[0] = 0;
            if (regionOfInterest.p1[1] == 0)
                activeRegion.p1[1] = 0;
            if (regionOfInterest.p1[2] == 0)
                activeRegion.p1[2] = 0;
            if (regionOfInterest.p2[0] == (nnx-1))
                activeRegion.p2[0] = nnx-1;
            if (regionOfInterest.p2[1] == (nny-1))
                activeRegion.p2[1] = nny-1;
            if (regionOfInterest.p2[2] == (nnz-1))
                activeRegion.p2[2] = nnz-1;
        }
        
        //  How this works:
        //      - StructureGrid is initialized to the right size, etc.
        //      - SetupGrid is initialized and given custody of StructureGrid
        //        for setting up the geometry and the tags and all.
        SetupGridPtr setupGrid(new SetupGrid( gridElem,
            name, nnx, nny, nnz, activeRegion, regionOfInterest, output3D));
        
        //if (gridParams.count("dumpGrid") != 0)
        //if (gridParams["dumpGrid"] == "1")
        if (dumpGrid)
		{
            ofstream fout;
            
            fout.open( (setupGrid->getName()+".grid.txt").c_str() );
            
            setupGrid->print(fout);
            assert(fout.good());
            assert(!fout.bad());
            fout.flush();
            assert(fout.good());
            fout.close();
            
            LOGF << "Grid dumped.\n";
        }
        
        setupGrids[setupGrid->getName()] = setupGrid;
        
        LOGF << "Grid dimensions: " << setupGrid->getBounds() << endl;
        
		if (output2D)
		{
			LOGF << "Saving output overlays... " << endl;
			setupGrid->saveOutputCrossSections();
		}
        gridElem = gridElem->NextSiblingElement("Grid");
    }   // END FOR EACH GRID
    
}


void FDTDApplication::
setupTFSFBuffers(Map<string, SetupGridPtr> & setupGrids)
{
	LOGF << "Setting up TFSF buffers.\n";
	
	// 1.  Make setup buffers and auxiliary grids for auto-TFSF.
	//	This iterates through grids and their SetupTFSFSources and processes
	// the ones which do autoTFSF.  Auxiliary grids are created recursively
	// with the setupTFSFBuffersRecursor function.
	
	// - For grid links, the TFSFBufferSet is the same size as the Link
	// region.  Give it the cell-by-cell coordinate transform and pointers to
	// the grids.
	vector<SetupGridPtr> auxGrids;
	
	map<string, SetupGridPtr>::iterator gridItr;
	for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
	{
		const string & nom = (*gridItr).first;
		SetupGridPtr grid = (*gridItr).second;
		
		// Do symmetry checks and such to determine size, extent and omitted
		// sides for aux grid and buffers.  This should be recursive, one way
		// or another.
		
		LOGF << "Making auxiliary grids for " << nom << endl;
		//LOG << "Skipping TFSF buffers.\n";
		setupTFSFBuffersRecursor(grid, auxGrids);
	}
	// put the aux grids into the grid list.
	for (int ii = 0; ii < auxGrids.size(); ii++)
		setupGrids[auxGrids[ii]->getName()] = auxGrids[ii];
	
	// 2.  Make Link buffers
	for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
	{
		const string & nom = (*gridItr).first;
		SetupGridPtr grid = (*gridItr).second;
		
		LOGF << "Making link buffers for " << nom << endl;
		//LOG << "Skipping link buffers.\n";
		grid->makeLinkBuffers(setupGrids);
	}
	
	// 3.  Make AFP buffers
	// - For AFP buffers, Trogdor should spit out requirements for the input
	// fields, in rects: Ex here, Ex there, Ex there too, ..., Ey here, ...
	// Hx here, ... etc.  Also send out coords for a source position maybe?
	// Anyway, the buffer only needs coordinate transforms from the current
	// setup grid to the buffer and vice versa.
	for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
	{
		const string & nom = (*gridItr).first;
		SetupGridPtr grid = (*gridItr).second;
		
		LOGF << "Making AFP buffers for " << nom << endl;
		//LOG << "Skipping AFP buffer creation.\n";
		grid->makeAFPBuffers();
		// This may also involve looking at grid symmetries (and source
		// symmetries).  Don't forget.
	}
	
	// Each buffer is saved in its own SetupGrid.  The link buffers can have a
	// smart pointer to the next SetupGrid, or a string (its name).  Pick one.
	
	// At the end of this function, the SetupLinks are no longer needed, and
	// all the info necessary to set up TFSF-modified materials is ready.
	
	//	 Okay, put in the TFSF!
	for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
	{
		const string & nom = (*gridItr).first;
		SetupGridPtr grid = (*gridItr).second;
		
		// Do symmetry checks and such to determine size, extent and omitted
		// sides for aux grid and buffers.  This should be recursive, one way
		// or another.
		
		LOGF << "Inserting TFSF cells for " << nom << endl;
		grid->insertTFSF();
	}
}

void FDTDApplication::
setupTFSFBuffersRecursor(SetupGridPtr setupGrid,
	vector<SetupGridPtr> & auxGrids)
{
	LOGF << "Recursor.\n";
	
	// We should request this information from the SetupGrid somehow.
	
	const vector<SetupTFSFSourcePtr> & sources = setupGrid->getTFSFSources();
	Vector3b currentSingularDimensions = setupGrid->getSingularDimensions();
	
	//LOG << "nsources = " << sources.size() << endl;
	for (int nSource = 0; nSource < sources.size(); nSource++)
	{
		if (sources[nSource]->getType() == kSFType)
		{
			cerr << "Error: currently Trogdor cannot handle SF-type "
				"TFSFSources.\n";
			assert(!"Wenti.");
		}
		
		vmlib::SMat<3,bool> gridSymmetries = setupGrid->getSymmetries(
			sources[nSource]->getTFRect());
		Vector3b sourceSymmetries = sources[nSource]->getSymmetries();
		Vector3b periodicDimensions = setupGrid->getPeriodicDimensions(
			sources[nSource]->getTFRect());
		Vector3b combinedSymmetries(0,0,0);
		
		LOGF << "Grid symmetries are " << gridSymmetries << endl;
		LOGF << "Source symmetries are " << sourceSymmetries << endl;
		LOGF << "Periodic dimensions are " << periodicDimensions << endl;
		
		for (int nSym = 0; nSym < 3; nSym++)
		if (sourceSymmetries[nSym] == 1)
		{
			// There are two conditions under which a dimension can be dropped.
			// Both require that the source is symmetrical with respect to
			// translation in (say) the X direction.  Then:
			//	 1.  If the sourceRect has full X symmetry (which means that
			//		gridSymmetries(nSym,:) == 1) then the X direction collapses.
			//	 2.  If the sourceRect has X symmetry in some directions, but
			//      the source rect actually extends the full height of the grid
			//		in the other dimensions, then this is good enough.
			
			Vector3b symmetryOrPeriodicity(
				periodicDimensions[0] || gridSymmetries(nSym,0),
				periodicDimensions[1] || gridSymmetries(nSym,1),
				periodicDimensions[2] || gridSymmetries(nSym,2) );
			
			if (minval(symmetryOrPeriodicity) == 1) // if vector == (1 1 1)
				combinedSymmetries[nSym] = 1;
		}
		
		LOGF << "Combined symmetries are " << combinedSymmetries << endl;
		LOGF << "Current singular dimensions are " << currentSingularDimensions
			<< endl;
		
		// Here we determine if we've got a recursive base case or not.
		// If the symmetries permit using a lower-dimensional auxiliary grid
		// and linking to it with a TFSF boundary, we do that here.  Otherwise
		// we've hit the base case, which is either a simple source that
		// Trogdor can handle (like an axis-oriented plane wave), or a more
		// complicated case that we need to ask Matlab for help with.
		if (!( combinedSymmetries == currentSingularDimensions) )
		{
			LOGF << "Recursing to make new setup grid.\n";
			
			ostringstream auxName;
			auxName << setupGrid->getName() << "_autoaux_" << nSource;
			
			SetupGridPtr auxiliaryGrid = makeAndLinkAuxiliaryGrid(setupGrid,
				auxName.str(), sources[nSource], combinedSymmetries, 
				periodicDimensions);
			
			auxGrids.push_back(auxiliaryGrid);
			setupTFSFBuffersRecursor(auxiliaryGrid, auxGrids);
		}
		else
		{
			// BASE CASE
			
			LOGF << "Base case reached." << endl;
			
			LOGF << "Grid symmetry: " << gridSymmetries << endl;
			LOGF << "Source symmetry: " << sourceSymmetries << endl;
			LOGF << "Periodic dimensions: " << periodicDimensions << endl;
			LOGF << "Singular dimensions: " << currentSingularDimensions << endl;
			
			// if the grid is homogeneous, insert a hard source
			// if the grid is inhomogeneous, make an auxiliary grid again
			
			int nSingular = 0;
			for (int mm = 0; mm < 3; mm++)
				nSingular += currentSingularDimensions[mm];
			
			if (nSingular == 2) // handle it in Trogdor
			{
				setupTFSFBufferBase1D(setupGrid, auxGrids, sources[nSource]);
			}
			else
			{
				LOGF << "This needs to be handled with Matlab.\n";
				setupGrid->addAFPSource(sources[nSource]);
			}
		}
	}
}

SetupGridPtr FDTDApplication::
makeAndLinkAuxiliaryGrid(SetupGridPtr parentGrid, string auxName,
	SetupTFSFSourcePtr source, Vector3b combinedSymmetries, 
	Vector3b periodicDimensions)
{
	// This function must do three things.
	// 1.  Calculate all bounding rects and such
	// 2.  Create the aux grid, giving it all the dimensions of interest
	// 3.  Create the link and add it to the parent grid
	
	const int PML_HALF_CELLS = 20;
	
	LOGF << "Making and linking!\n";
	
	Vector3b sourceSymmetries = source->getSymmetries();
	Rect3i linkDestRect = source->getTFRect();
	Rect3i linkSourceRect;
	Rect3i nestedSourceRect;
	Rect3i regionOfInterest;
	Rect3i auxBoundingRect;
	Rect3i copyFromRect;			// read from the old StructureGrid here
	Rect3i copyToRect;				// write to the new StructureGrid here
	int nn;
	
	LOGF << "TF rect is " << source->getTFRect() << endl;
	
	// The copyToRect should be the size of the parent grid's TF region,
	// but with applicable dimensions collapsed.  Thus the copyToRect will
	// likely end up smaller than the copyFromRect.
	// Then find the directions along which the source is not symmetrical, and
	// pad them with PML and a little room for the nested source.
	copyToRect = Rect3i(Vector3i(0,0,0), linkDestRect.p2 - linkDestRect.p1);
	for (nn = 0; nn < 3; nn++)
	{
		if (combinedSymmetries[nn])
			copyToRect.p2[nn] = 1;
		else if (periodicDimensions[nn] == 0 || sourceSymmetries[nn] == 0)
		{
			copyToRect.p1[nn] += PML_HALF_CELLS + 4;
			copyToRect.p2[nn] += PML_HALF_CELLS + 4;
		}
		/*
		if (0 == sourceSymmetries[nn])
		{
			copyToRect.p1[nn] += PML_HALF_CELLS + 4;
			copyToRect.p2[nn] += PML_HALF_CELLS + 4;
		}*/
	}
	
	// Now figure out what part of the parent StructureGrid to copy the
	// materials from.  This includes taking note of any omitted sides in the 
	// original source.
	copyFromRect = linkDestRect;
	for (nn = 0; nn < 3; nn++)
	if (combinedSymmetries[nn])
	{
		int lowSide = 2*nn;
		int highSide = 2*nn+1;
		if (0 == source->omits(highSide))
			copyFromRect.p2[nn] = copyFromRect.p1[nn] + 1;
		else if (0 == source->omits(lowSide))
			copyFromRect.p1[nn] = copyFromRect.p2[nn]-1;
		assert(!(source->omits(lowSide) && source->omits(highSide)));
	}
	
	// Make the region of interest bigger than copyToRect by the extra PML
	// buffer size.
	regionOfInterest = copyToRect;
	for (nn = 0; nn < 3; nn++)
	//if (0 == sourceSymmetries[nn])
	if (combinedSymmetries[nn] == 0 && (periodicDimensions[nn] == 0 ||
		sourceSymmetries[nn] == 0) )
	{
		regionOfInterest.p1[nn] = PML_HALF_CELLS;
		regionOfInterest.p2[nn] = copyToRect.p2[nn] + 4;
	}
	
	// Make the nested source rect fit tightly into the region of interest.
	nestedSourceRect = regionOfInterest;
	for (nn = 0; nn < 3; nn++)
	if (combinedSymmetries[nn] == 0 && (periodicDimensions[nn] == 0 ||
		sourceSymmetries[nn] == 0) )
	{
		nestedSourceRect.p1[nn] += 2;
		nestedSourceRect.p2[nn] -= 2;
	}
	
	// Set the auxiliary grid's bounding rect outside the region of interest
	auxBoundingRect = regionOfInterest;
	for (nn = 0; nn < 3; nn++)
	if (combinedSymmetries[nn] == 0 && (periodicDimensions[nn] == 0 ||
		sourceSymmetries[nn] == 0) )
	{
		auxBoundingRect.p1[nn] = 0;
		auxBoundingRect.p2[nn] += PML_HALF_CELLS;
	}
	
	// The copyToRect is actually the same thing as the linkSourceRect: it's the
	// incident field region in the auxiliary grid that maps to the total
	// field region in the parent grid.
	linkSourceRect = copyToRect;
	
	
	LOGF << "Rectangles of note in " << auxName << ":\n";
	LOGFMORE << "Copy-to rect is " << copyToRect << endl;
	LOGFMORE << "Copy-from rect is " << copyFromRect << endl;
	LOGFMORE << "Source rect is " << linkSourceRect << endl;
	LOGFMORE << "Dest rect is " << linkDestRect << endl;
	LOGFMORE << "Auxiliary source rect is " << nestedSourceRect << endl;
	LOGFMORE << "Aux bounding rect is " << auxBoundingRect << endl;
	LOGFMORE << "region of interest is " << regionOfInterest << endl;
	
	
	// Create the new source.  It's mostly a copy of the parent source, but it
	// is marginally bigger to nest outside the parent.
	SetupTFSFSource* pSource = new SetupTFSFSource(source->getClass(),
		nestedSourceRect, source->getDirection(),
		source->getParameters());
	SetupTFSFSourcePtr newSource(pSource);
	
	// Create the new grid.
	SetupGrid* pGrid = new SetupGrid(*parentGrid, auxName, copyFromRect,
		copyToRect, auxBoundingRect, regionOfInterest, newSource);
	SetupGridPtr auxGrid(pGrid);
	
	// Create the link from the parent grid to the new auxiliary grid.
	//LOG << "Omitting the link.\n";
	
	SetupLink* pLink = new SetupLink(kTFType, auxName,
		linkSourceRect, linkDestRect, source->getOmitSideFlags());
	SetupLinkPtr newLink(pLink);
	parentGrid->addLink(newLink);
	
	
	// Put in a three-field output for E
	/*
	Map<string,string> outParams;
	ostringstream str;
	str << "0 0 0 " << pGrid->get_nx()-1 << " " << pGrid->get_ny()-1
		<< " " << pGrid->get_nz()-1;
	outParams["region"] = str.str();
	outParams["field"] = "electric";
	SetupOutput* pOut = new SetupOutput(auxName+"_outE", "ThreeFieldOutput",
		1, outParams);
	SetupOutputPtr auxOutput(pOut);
	auxGrid->addOutput(auxOutput);
	*/
	
	return auxGrid;
}


void FDTDApplication::
setupTFSFBufferBase1D(SetupGridPtr setupGrid,
	std::vector<SetupGridPtr> & auxGrids,
	SetupTFSFSourcePtr source)
{
	const int PML_HALF_CELLS = 20;
	
	Vector3i unitVectors[3] = { Vector3i(1,0,0), Vector3i(0,1,0),
		Vector3i(0,0,1) };
	
	vmlib::SMat<3,bool> gridSymmetries = setupGrid->getSymmetries(
		source->getTFRect());
	Vector3b sourceSymmetries = source->getSymmetries();
	Vector3b periodicDimensions = setupGrid->getPeriodicDimensions(
		source->getTFRect());
	Vector3b currentSingularDimensions = setupGrid->getSingularDimensions();
	
	LOGF << "Base case information:\n";
	LOGFMORE << "Base grid: " << setupGrid->getName() << endl;
	LOGFMORE << "Grid symmetry: " << gridSymmetries << endl;
	LOGFMORE << "Source symmetry: " << sourceSymmetries << endl;
	LOGFMORE << "Periodic dimensions: " << periodicDimensions << endl;
	LOGFMORE << "Singular dimensions: " << currentSingularDimensions << endl;
	
	// quick and dirty way to figure out which direction this source is
	// headed.  we know already that it's going along the direction of the
	// grid axis from function preconditions.
	int asymDim = -1;
	for (int nn = 0; nn < 3; nn++)
		if (sourceSymmetries[nn] == 0)
			asymDim = nn;
	assert(asymDim != -1);
	
	// Check the off-diagonal symmetry in the asymmetrical direction.  This
	// means, check that the grid is homogeneous along the source direction.
	if (gridSymmetries(asymDim, (asymDim+1)%3) &&
		gridSymmetries(asymDim, (asymDim+2)%3))
	{
		// Insert a hard source.
		LOGF << "Insert a hard source.\n";
		
		// 1.  If it's a formula source I can insert a hard formula source.
		// 2.  If it's a file input source I can insert an Input.
		
		assert(source->getFormula() != "" || source->getInputFile() != "");
		/*
		if (source->getFormula() != "")
		{
			LOG << "Hard source has input formula\n"
				<< source->getFormula() << endl;
		}
		else
		{
			LOG << "Hard source has input file " << source->getInputFile()
				<< endl;
		}
		*/
		
		Rect3i sourceRegion; // it's in Yee cells
		if (dot(source->getAxialDirection(), unitVectors[asymDim]) > 0)
		{
			sourceRegion.p1 = source->getTFRect().p1/2 -
				source->getAxialDirection();
		}
		else if (dot(source->getAxialDirection(), unitVectors[asymDim]) < 0)
		{
			sourceRegion.p1 = source->getTFRect().p2/2 -
				source->getAxialDirection();
		}
		else
		{
			LOG << "Source direction trubbles.\n";
			assert(!"Die... die.");
		}
		sourceRegion.p2 = sourceRegion.p1;
		
		//LOG << "Source region is " << sourceRegion << endl;
		
		Field field;
		if (source->getField() == 'e')
			field = kElectric;
		else
			field = kMagnetic;
		
		//LOG << "Don't forget to eventually ditch the params.\n";
		
		//LOG << "Omitting source.\n";
		
		SetupSource* pSource = new SetupSource(source->getFormula(),
			source->getInputFile(), field, source->getPolarization(),
			sourceRegion, source->getParameters());
		SetupSourcePtr newSource(pSource);
		setupGrid->addSource(newSource);
		
	}
	else
	{
		// Link a new auxiliary grid.
		LOGF << "Insert a new auxiliary grid.\n";
		
		Rect3i copyFromRect;
		if (dot(source->getAxialDirection(), unitVectors[asymDim]) > 0)
		{
			copyFromRect.p1 = source->getTFRect().p1;
			copyFromRect.p2 = copyFromRect.p1;
		}
		else if (dot(source->getAxialDirection(), unitVectors[asymDim]) < 0)
		{
			copyFromRect.p2 = source->getTFRect().p2;
			copyFromRect.p1 = copyFromRect.p2;
		}
		else
		{
			LOG << "Source direction trubbles.\n";
			assert(!"DIE DIE DIE");
		}
		
		Rect3i copyToRect(Vector3i(0,0,0), source->getTFRect().p2 -
			source->getTFRect().p1);
		copyToRect.p1[asymDim] += PML_HALF_CELLS + 4;
		copyToRect.p2[asymDim] += PML_HALF_CELLS + 4;
		
		Rect3i auxROI(copyToRect);
		auxROI.p1[asymDim] = PML_HALF_CELLS;
		auxROI.p2[asymDim] = copyToRect.p2[asymDim] + 4;
		
		Rect3i auxBoundingRect(auxROI);
		auxBoundingRect.p1[asymDim] = 0;
		auxBoundingRect.p2[asymDim] = auxROI.p2[asymDim] + PML_HALF_CELLS;
		
		Rect3i nestedSourceRect(auxROI);
		nestedSourceRect.p1[asymDim] += 2;
		nestedSourceRect.p2[asymDim] -= 2;
		
		LOGF << "Copy from rect is " << copyFromRect;
		
		/*
		Rect3i copyToRect = source->getTFRect();
		copyToRect.p1[asymDim] += 2;
		copyToRect.p2[asymDim] += 2;
		Rect3i auxBoundingRect = setupGrid->getBounds();
		auxBoundingRect.p2[asymDim] += 4;
		Rect3i auxROI = setupGrid->getRegionOfInterest();
		auxROI.p2[asymDim] += 4;
		Rect3i newSourceRect = copyToRect;
		*/
		
		ostringstream auxName;
		auxName << setupGrid->getName() << "_autosrc";
		
		// Create the new source.  It's mostly a copy of the parent source.
		SetupTFSFSource* pSource = new SetupTFSFSource(source->getClass(),
			nestedSourceRect, source->getDirection(),
			source->getParameters());
		SetupTFSFSourcePtr newSource(pSource);
		
		// this is from above, for reference
		
		// Create the new grid.
		SetupGrid* pGrid = new SetupGrid(*setupGrid, auxName.str(),
			copyFromRect, copyToRect, auxBoundingRect,
			auxROI, newSource);
		SetupGridPtr auxGrid(pGrid);
		auxGrids.push_back(auxGrid);
		
		// Create the link
		SetupLink* pLink = new SetupLink(kTFType, auxName.str(),
			source->getTFRect(), source->getTFRect());
		SetupLinkPtr newLink(pLink);
		newLink->omitSide(source->getAxialDirection());
		setupGrid->addLink(newLink);
		
		
		// Put in a three-field output for E
		/*
		Map<string,string> outParams;
		ostringstream str;
		str << "0 0 0 " << pGrid->get_nx()-1 << " " << pGrid->get_ny()-1
			<< " " << pGrid->get_nz()-1;
		outParams["region"] = str.str();
		outParams["field"] = "electric";
		SetupOutput* pOut = new SetupOutput(auxName.str()+"_outE",
			"ThreeFieldOutput", 1, outParams);
		SetupOutputPtr auxOutput(pOut);
		auxGrid->addOutput(auxOutput);
		*/
		
		//LOG << "Omitting the last buffer base.\n";
		setupTFSFBufferBase1D(auxGrid, auxGrids, newSource);
		
	}
}


void FDTDApplication::
setupAFPRequests(Map<std::string, SetupGridPtr> & setupGrids)
{
	ostringstream str;
	
	Map<string, SetupGridPtr>::iterator gridItr;
	for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
	{
		const string & nom = (*gridItr).first;
		SetupGridPtr grid = (*gridItr).second;
		
		LOGF << "Handling AFP requests for " << nom << endl;
		
		grid->writeAFPRequests(m_dx, m_dy, m_dz, m_dt, mNumT);
	}
}

#pragma mark *** Allocation phase ***

void FDTDApplication::
initializeRuntime(Map<string, SetupGridPtr> & setupGrids,
    Map<string, SimulationGridPtr> & simulationGrids, int numThreads,
	float dx, float dy, float dz, float dt)
{
    LOGF << "initializeRuntime()\n";
    
    //  TASKS:
    //      1.  Save the setup runlines
    //      2.  Destroy the StructureGrids
    //      3.  Allocate runtime memory: E/H fields, material models & runlines,
	//			and TFSF buffers
    
    Map<string, FieldsPtr> fields;
    Map<string, Pointer< vector<RunlineType> > > materialRunlines;
	
	Map<SetupTFSFBufferSetPtr, TFSFBufferSetPtr> buffers;
    
    //  1.  Create setup runlines
    LOGF << "Creating setup runlines..." << endl;
    map<string, SetupGridPtr>::iterator gridItr;
    for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
    {
        const string & nom = (*gridItr).first;
        SetupGridPtr grid = (*gridItr).second;
        LOGF << "Making runlines for " << nom << endl;
        
        materialRunlines[nom] = grid->makeMaterialRunlines();
    }
    
    //  2.  Destroy StructureGrids
    LOGF << "Clearing StructureGrids..." << endl;
    //structureGrids.clear();
    
    //  3a. Allocate E/H fields (class Fields)
    LOGF << "Allocating E/H fields..." << endl;
    for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
    {
        const string & nom = (*gridItr).first;
        SetupGridPtr grid = (*gridItr).second;
        
        FieldsPtr pFields( new Fields(grid->get_nnx(), grid->get_nny(),
            grid->get_nnz()) );
        
        fields[nom] = pFields;
		
		//pFields->writeCoordinates();
    }
	
	// 3b.  Allocate TFSF buffers
	LOGF << "Allocating TFSF buffers..." << endl;
	for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
	{
        const string & nom = (*gridItr).first;
        SetupGridPtr grid = (*gridItr).second;
		
		makeRuntimeBuffers(grid, fields, buffers);
	}
	
    //  3c. Create real runlines and assemble the SimulationGrids from their
    //  components.
    LOGF << "Creating runlines and assembling SimulationGrids..." << endl;
    for (gridItr = setupGrids.begin(); gridItr != setupGrids.end(); gridItr++)
    {
        const string & nom = (*gridItr).first;
        SetupGridPtr grid = (*gridItr).second;
        FieldsPtr & theFields = fields[nom];
        
		// Select all the TFSF buffers that belong to the current grid.
		const vector<SetupTFSFBufferSetPtr> & setupBuffers = grid->getBuffers();
		vector<TFSFBufferSetPtr> theTFSFBuffers;
		
		for (unsigned int nn = 0; nn < setupBuffers.size(); nn++)
		{
			assert(buffers.count(setupBuffers[nn]) == 1);
			theTFSFBuffers.push_back(buffers[setupBuffers[nn]]);
		}
		
        vector<SourcePtr> theSources = makeRuntimeSources(grid, theFields);
        vector<OutputPtr> theOutputs = makeRuntimeOutputs(grid, theFields);
        vector<InputPtr> theInputs = makeRuntimeInputs(grid, theFields);
        vector<MaterialPtr> theMaterials = makeRuntimeMaterialModels(grid,
            theFields, fields, *materialRunlines[nom], buffers, dx, dy, dz, dt);
		
        SimulationGridPtr simGrid( new SimulationGrid( theFields, theSources,
            theMaterials, theOutputs, theInputs, theTFSFBuffers, numThreads) );
        
        simulationGrids[nom] = simGrid;
    }
    
    //  Now the setup stuff can be destroyed too.
    LOGF << "Clearing setup grids..." << endl;
    setupGrids.clear();
}

void FDTDApplication::
makeRuntimeBuffers(SetupGridPtr grid, Map<string, FieldsPtr> & fields,
	Map<SetupTFSFBufferSetPtr, TFSFBufferSetPtr> & buffers)
{
	const vector<SetupTFSFBufferSetPtr> & setups = grid->getBuffers();
	
	for (unsigned int nn = 0; nn < setups.size(); nn++)
	{
		string mainGridName = grid->getName();
		
		if (setups[nn]->isLinkType())
		{
			string auxGridName = setups[nn]->getAuxGridName();
			
			TFSFBufferSet* pBuffer = new TFSFBufferSet(setups[nn],
				fields[mainGridName], fields[auxGridName]);
			TFSFBufferSetPtr newBuffer(pBuffer);
			buffers[setups[nn]] = newBuffer;
			
			//LOG << "PRINTING THE BUFFER\n";
			//newBuffer->print(cout);
			
			//LOG << "Initializing the buffer once (DEBUG).\n";
			//newBuffer->updateE();
			//newBuffer->updateH();
		}
		else if (setups[nn]->isFileType())
		{
			ostringstream fieldsFileName;
			fieldsFileName << "SOURCE_" << grid->getName() << "_" << nn
				<< ".dat";
			
			TFSFBufferSet* pBuffer = new TFSFBufferSet(setups[nn],
				fields[mainGridName], fieldsFileName.str());
			TFSFBufferSetPtr newBuffer(pBuffer);
			buffers[setups[nn]] = newBuffer;
		}
		else
		{
			assert(!"weird");
		}
		
	}
}



vector<SourcePtr> FDTDApplication::
makeRuntimeSources(SetupGridPtr grid, FieldsPtr inFields)
{
    vector<SourcePtr> outSources;
    
    const vector<SetupSourcePtr> & sources = grid->getSources();
    
    for (int n = 0; n < sources.size(); n++)
    {
        SourcePtr newSource( SourceFactory::createSource(*grid, *inFields,
            *sources[n]) );
        outSources.push_back(newSource);
    }
    
    return outSources;
}

vector<OutputPtr> FDTDApplication::
makeRuntimeOutputs(SetupGridPtr grid, FieldsPtr inFields)
{
    vector<OutputPtr> outOutputs;
    
    const vector<SetupOutputPtr> & outputs = grid->getOutputs();
    
    for (unsigned int n = 0; n < outputs.size(); n++)
    {
        OutputPtr newOutput( OutputFactory::createOutput(*grid, *inFields,
            *outputs[n]) );
        outOutputs.push_back(newOutput);
        
        newOutput->writeSpecsFile();
        
        //LOG << "Note that outputs do not receive output runlines yet.\n";
    }
    
    return outOutputs;
}

vector<InputPtr> FDTDApplication::
makeRuntimeInputs(SetupGridPtr grid, FieldsPtr inFields)
{
    vector<InputPtr> outInputs;
    
    const vector<SetupInputPtr> & inputs = grid->getInputs();
    
    for (unsigned int n = 0; n < inputs.size(); n++)
    {
        InputPtr newInput( InputFactory::createInput(*grid, *inFields,
            *inputs[n]) );
        outInputs.push_back(newInput);
        
        newInput->readSpecsFile();
    }
    
    return outInputs;
}

vector<MaterialPtr> FDTDApplication::
makeRuntimeMaterialModels(SetupGridPtr grid, FieldsPtr theFields,
    Map<std::string, FieldsPtr> & allFields,
    const std::vector<RunlineType> & inMaterialRunlines,
	Map<SetupTFSFBufferSetPtr, TFSFBufferSetPtr> & inBuffers,
	float dx, float dy, float dz, float dt)
{
    vector<MaterialPtr> outMaterials;
    
    //  1.  Make the unmodified materials from inMaterials
    //  2.  Iterate over inMaterialRunlines, and create modified materials
    //      as needed; convert to actual runlines.  For TF/SF materials,
    //      use bigger runline structures (figure this guy out!)
    
    const Map<string, SetupMaterialPtr> & setupMaterials = grid->getMaterials();
    Map<MaterialType, MaterialPtr> matSetupRuntimeMap;
	set<MaterialPtr> newMaterials;
    
    //  1.  Set up basic material types
    //LOG << "Setting up " << setupMaterials.size() << " basic material types."
    //    << endl;
    map<string, SetupMaterialPtr>::const_iterator matItr;
    for (matItr = setupMaterials.begin(); matItr != setupMaterials.end();
        matItr++)
    {
        const string& nom = (*matItr).first;
        const SetupMaterialPtr & setupMat = (*matItr).second;
        MaterialPtr newMaterial( MaterialFactory::createMaterial(
            *setupMat) );
        MaterialType newType(nom);
        
        //LOG << "New type: " << newType.getDescription();
        
        matSetupRuntimeMap[newType] = newMaterial;
		newMaterials.insert(newMaterial);
    }
    
    //  2.  Create derived material models: PML and TF/SF corrected
    LOGF << "Deriving materials for " << inMaterialRunlines.size()
        << " runlines." << endl;
    for (int n = 0; n < inMaterialRunlines.size(); n++)
    {
        const RunlineType & rl = inMaterialRunlines[n];
        const MaterialType & rlMatType = rl.getType();
        
        if (rl.getType().isTFSF())
        {
            //rl.print(cout);
            //cout << "Self index: " << rl.getSelfIndex() << "\n";
            //rl.getNeighbors().print(cout);
            //rl.getAuxNeighbors().print(cout);
        }
        
        if (matSetupRuntimeMap.count(rlMatType) == 0)
        {
            MaterialType parentType(rlMatType.getName());
            if (matSetupRuntimeMap.count(parentType) == 0)
            { 
                LOG << "Error: no parent type found!\n";
                cerr << "Error: I'm trying to make a TFSF-modified or PML-\n"
                        "modified material, but I can't find a parent\n"
                        "material.  The parent type is supposedly\n";
                cerr << "'" << parentType.getDescription() << "'" << endl;
                LOG << "(parent type " << parentType.getDescription() << ")\n";
                LOG << "Quitting.\n";
                exit(1);
            }
            MaterialPtr parentPtr = matSetupRuntimeMap[parentType];
            
            if (rlMatType.isPML())
            {
                MaterialModel* pmlNew = parentPtr->createPML(
                    rlMatType.getDirection(), grid->getActiveRegion(),
                    grid->getRegionOfInterest(),
                    m_dx, m_dy, m_dz, m_dt);
                if (pmlNew == 0L)
                {
                    LOG << "Error on PML creation." << endl;
                    cerr << "Error: parent material "
                        << parentPtr->getMaterialName() << " cannot create "
                        << "PML child.  This means you should not let this \n"
                        "material cross into the PML region until its PML\n"
                        "is implemented.\n";
                    LOG << "(parent type " << parentPtr->getMaterialName()
                        << ")\n";
                    LOG << "Quitting.\n";
                    exit(1);
                }
                MaterialPtr pmlPtr( pmlNew );
                matSetupRuntimeMap[rlMatType] = pmlPtr;
				newMaterials.insert(pmlPtr);
            }
			else if (rlMatType.isTFSF())  // now TFSF is just like the parent...
			{
				matSetupRuntimeMap[rlMatType] = parentPtr;
				newMaterials.insert(parentPtr);
			}
            else
            {
                LOG << "Error: unknown/unexpected material type.\n";
                cerr << "Highly unexpected error." << endl;
                exit(1);
            }
        }
        
        //  Now add the runline to the material.
        MaterialPtr & itsModel = matSetupRuntimeMap[rlMatType];
        if (!rl.getType().isTFSF())
        {
            Runline newRunline(rl, *theFields);
            itsModel->addRunline(newRunline, rl.get_ii0(), rl.get_jj0(),
                rl.get_kk0());
        }
        else
        {
			Runline newRunline(rl, *theFields,
				*inBuffers[rlMatType.getBuffer()]);
            itsModel->addRunline(newRunline, rl.get_ii0(), rl.get_jj0(),
                rl.get_kk0());
        }
    }
    
    //  Do additional allocations now, and dump the material map to a vector.
    
	/*
    map<MaterialType, MaterialPtr>::iterator newMatItr;
    for (newMatItr = newMaterials.begin(); newMatItr != newMaterials.end();
        newMatItr++)
	*/
	set<MaterialPtr>::iterator newMatItr;
	for (newMatItr = newMaterials.begin(); newMatItr != newMaterials.end();
		newMatItr++)
    {
		MaterialPtr p = *newMatItr;
        p->allocate(dx, dy, dz, dt);
		outMaterials.push_back( p );
    }
    
    return outMaterials;
}


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

#endif // 0

#pragma mark *** Helper methods ***


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
