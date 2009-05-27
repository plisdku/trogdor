/*
 *  FDTDApplication.h
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

#ifndef _FDTDAPPLICATION_
#define _FDTDAPPLICATION_

#include "geometry.h"
#include "tinyxml.h"
#include "Pointer.h"
#include <string>
#include <vector>

class SimulationDescription;
typedef Pointer<SimulationDescription> SimulationDescPtr;

class GridDescription;
typedef Pointer<GridDescription> GridDescPtr;

class VoxelizedPartition;
typedef Pointer<VoxelizedPartition> VoxelizedPartitionPtr;

class HuygensSurfaceDescription;
typedef Pointer<HuygensSurfaceDescription> HuygensSurfaceDescPtr;

class CalculationPartition;
typedef Pointer<CalculationPartition> CalculationPartitionPtr;

/**
 *  Core application singleton
 *
 *  This class encompasses the broad strokes of the FDTD simulation.  The
 *  method run() is called in main(), and splits the lifetime of the simulation
 *  into three stages (subroutines):  execution of the parameter file
 *  instructions, creation of the runtime structures, and running the sim.
 *
 *  This is a singleton class; access is through getInstance().
 */
class FDTDApplication
{
public:
    /**
     *  Run the application.
     *
     *  The main simulation objects are allocated, used and deallocated here.
     *  Functionality is delegated to executeParamFile(), initializeRuntime(),
     *  and runSimulation().
     */
    /*
    void runAll(std::string parameterFile, int numThreads, bool runSim,
		bool output3D, bool dumpGrid, bool output2D,
		int numTimestepsOverride = -1);
	*/
	void runNew(std::string parameterFile);

private: // heh, "new private" stuff
	
	SimulationDescPtr loadSimulation(std::string parameterFile);
	
	Mat3i guessFastestOrientation(const SimulationDescription & sim) const;
	
	void voxelizeGrids(const SimulationDescPtr sim,
		//Map<std::string, GridDescriptionPtr> gridDescriptions,
		Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids);
	
	void voxelizeGridRecursor(
		Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
		GridDescPtr currentGrid, Vector3i numNodes, Vector3i thisNode,
		Rect3i partitionWalls);
	
	GridDescPtr makeAuxGridDescription(Vector3i collapsableDimensions,
		GridDescPtr parentGrid, HuygensSurfaceDescPtr huygensSurface,
		std::string auxGridName);
	
	GridDescPtr makeAux1DGridDescription(GridDescPtr parentGrid,
		HuygensSurfaceDescPtr huygensSurface, std::string auxGridName);
	
	GridDescPtr makeSourceGridDescription(GridDescPtr parentGrid,
		HuygensSurfaceDescPtr huygensSurface, std::string srcGridName);
    
    void trimVoxelizedGrids(Map<GridDescPtr, VoxelizedPartitionPtr> & vgs);
    
    void makeCalculationGrids(const SimulationDescPtr sim, 
        Map<std::string, CalculationPartitionPtr> & calcs,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & voxParts);
    
    void allocateAuxBuffers(Map<std::string, CalculationPartitionPtr>
        & calcs);
    
    void calcE(Map<std::string, CalculationPartitionPtr> & calcGrids);
    void calcAfterE(Map<std::string, CalculationPartitionPtr> & calcGrids);
    void calcH(Map<std::string, CalculationPartitionPtr> & calcGrids);
    void calcAfterH(Map<std::string, CalculationPartitionPtr> & calcGrids);
    
private:
    float m_dx;
    float m_dy;
    float m_dz;
    float m_dt;
    int mNumT;
	
	std::vector<double> mTimestepStartTimes;
	
	double mUpdateETime;
	double mUpdateHTime;
	double mBufferETime;
	double mBufferHTime;
	double mOutputTime;
	double mPrintTimestepTime;
    
#pragma mark *** Singleton Stuff ***
public:
    static FDTDApplication & getInstance();
private:
    FDTDApplication();
    virtual ~FDTDApplication();
    static FDTDApplication sInstance;
};


#endif

