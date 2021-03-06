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

#include "Performance.h"
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

struct SimulationPreferences
{
public:
    SimulationPreferences();
    
    int numThreads;
    long numTimestepsOverride;
    bool output3D;
    bool output2D;
    bool dumpGrid;
    bool runSim;
    char runlineDirection;
    bool savePerformanceInfo;
};


/**
 *  Core application singleton
 *
 *  This class encompasses the broad strokes of the FDTD simulation.  The
 *  method run() is called in main(), and splits the lifetime of the simulation
 *  into three stages (subroutines):  execution of the parameter file
 *  instructions, creation of the runtime structures, and running the sim.
 *
 *  This is a singleton class; access is through instance().
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
	void runNew(std::string parameterFile, const SimulationPreferences & prefs);

private:
	SimulationDescPtr loadSimulation(std::string parameterFile);
	
	void voxelizeGrids(const SimulationDescPtr sim,
		Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
        int runlineDirection);
	
	void voxelizeGridRecursor(
		Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
        SimulationDescPtr simulationDescription,
		GridDescPtr currentGrid, Vector3i numNodes, Vector3i thisNode,
		Rect3i partitionWallsHalf, int runlineDirection);
	
	GridDescPtr makeAuxGridDescription(Vector3i collapsableDimensions,
		GridDescPtr parentGrid, HuygensSurfaceDescPtr huygensSurface,
		std::string auxGridName);
	
	GridDescPtr makeAux1DGridDescription(GridDescPtr parentGrid,
		HuygensSurfaceDescPtr huygensSurface, std::string auxGridName);
	
	GridDescPtr makeSourceGridDescription(GridDescPtr parentGrid,
		HuygensSurfaceDescPtr huygensSurface, std::string srcGridName);
    
    /**
     *  Write all output that depends on the grid contents but not on actual
     *  fields and other calculation results; this includes cross-sections of
     *  the structure (--xsections), OBJ dumps of surfaces of materials
     *  (--geometry), ASCII dumps of all grids (--dumpgrids), and maybe more.
     *  All of this information can be obtained without beginning to allocate
     *  space for E & H fields etc; writing these reports happens right before
     *  checking for the --nosim flag.
     */
    void writeReports(Map<GridDescPtr, VoxelizedPartitionPtr> & vgs,
        const SimulationPreferences & prefs);
    
    /**
     *  Write Matlab m-files with descriptions of the ordering of data for
     *  current sources and custom TFSF sources.  This happens before checking
     *  for the --nosim flag.
     */
    void writeDataRequests(Map<GridDescPtr, VoxelizedPartitionPtr> & vgs,
        const SimulationPreferences & prefs);
    
    void trimVoxelizedGrids(Map<GridDescPtr, VoxelizedPartitionPtr> & vgs);
            
    void makeCalculationGrids(const SimulationDescPtr sim, 
        Map<std::string, CalculationPartitionPtr> & calcs,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & voxParts);
    
    void allocateAuxBuffers(Map<std::string, CalculationPartitionPtr>
        & calcs);
    
    void updateE(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void sourceE(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void outputE(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void updateH(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void sourceH(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void outputH(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    
    void updateETimed(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void sourceETimed(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void outputETimed(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void updateHTimed(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void sourceHTimed(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
    void outputHTimed(Map<std::string, CalculationPartitionPtr> & calcGrids,
        long timestep);
        
    void runUntimed(
        Map<std::string, CalculationPartitionPtr> & calculationGrids);
    void runTimed(Map<std::string, CalculationPartitionPtr> & calculationGrids);
    
    void reportPerformance(
        Map<std::string, CalculationPartitionPtr> & calculationGrids);
    
private:
    float m_dx;
    float m_dy;
    float m_dz;
    float m_dt;
    int mNumT;
    
    GlobalStatistics mPerformance;
    
#pragma mark *** Singleton Stuff ***
public:
    static FDTDApplication & instance();
private:
    FDTDApplication();
    virtual ~FDTDApplication();
    static FDTDApplication sInstance;
};


#endif

