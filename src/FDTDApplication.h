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

#include "SetupGrid.h"
#include "StructureGrid.h"
#include "SimulationGrid.h"

#include "TFSFBufferSet.h"

#include "tinyxml.h"

#include <string>


class Fields;
typedef Pointer<Fields> FieldsPtr;

class SimulationDescription;
typedef Pointer<SimulationDescription> SimulationDescPtr;

class GridDescription;
typedef Pointer<GridDescription> GridDescriptionPtr;

class VoxelizedGrid;
typedef Pointer<VoxelizedGrid> VoxelizedGridPtr;

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
    void runAll(std::string parameterFile, int numThreads, bool runSim,
		bool output3D, bool dumpGrid, bool output2D,
		int numTimestepsOverride = -1);
	
	void runNew(std::string parameterFile);

private: // heh, "new private" stuff
	
	SimulationDescPtr loadSimulation(std::string parameterFile);
	
	Mat3i guessFastestOrientation(const SimulationDescription & sim) const;
	
	void voxelizeGrids(const SimulationDescPtr sim,
		Map<std::string, GridDescriptionPtr> gridDescriptions,
		Map<std::string, VoxelizedGridPtr> voxelizedGrids,
		Mat3i orientation);
	
	void voxelizeGridRecursor(
		Map<std::string, GridDescriptionPtr> & gridDescriptions,
		Map<std::string, VoxelizedGridPtr> & voxelizedGrids,
		GridDescriptionPtr currentGrid,
		Mat3i orientation);
	
private:
    /**
     *  Load and execute the simulation description
     *
     *  The parameter file contains descriptions of materials, outputs, sources,
     *  and the structures of all grids.  The grid structure is loaded from
     *  any necessary other files (e.g. height maps) in this stage.
     *
     *  @param  paramFileName   the local xml instruction file, e.g. "foo.xml"
     *  @param  setupGrids      map of grid names to setup data, to be filled in
     *  @param  simulationParams dx, dy, dz, dt, numT
     */
    void
    readParameterFile(std::string paramFileName,
        Map<std::string, SetupGridPtr> & setupGrids,
        Map<std::string, std::string> & simulationParams,
		bool output3D, bool dumpGrid, bool output2D);
	
	/**
	 *	Set up field buffers and auxiliary grids for TFSF sources
	 */
	void
	setupTFSFBuffers(Map<std::string, SetupGridPtr> & setupGrids);
	
	void
	setupTFSFBuffersRecursor(SetupGridPtr setupGrid,
		std::vector<SetupGridPtr> & auxGrids);
	
	SetupGridPtr
	makeAndLinkAuxiliaryGrid(SetupGridPtr parentGrid, std::string auxName,
		SetupTFSFSourcePtr source, Vector3b combinedSymmetries,
		Vector3b periodicDimensions);
	
	void
	setupTFSFBufferBase1D(SetupGridPtr setupGrid,
		std::vector<SetupGridPtr> & auxGrids,
		SetupTFSFSourcePtr source);
	
	void
	setupAFPRequests(Map<std::string, SetupGridPtr> & setupGrids);
    
    
    /**
     *  Allocate runtime memory and deallocate temporary structures
     *
     *  This function performs a memory shuffle to unload the StructureGrids
     *  (which are large) and allocate the memory for fields and runlines.
     *  Setup methods for MaterialModel, Output and Source classes are called
     *  here.
     *
     *  @param  setupGrids      structures read from the XML file
     *  @param  simulationGrids runtime memory: fields, materials, etc.
     */
    void
    initializeRuntime(Map<std::string, SetupGridPtr> & setupGrids,
        Map<std::string, SimulationGridPtr> & simulationGrids,
		int numThreads, float dx, float dy, float dz, float dt);
    
    /**
     *  Run the FDTD simulation
     *
     *  This is the main simulation loop that calculates fields and outputs
     *  data.
     *
     *  @param  simulationGrids runtime memory: fields, materials, etc.
     */
    void
    runSimulation(Map<std::string, SimulationGridPtr> & simulationGrids);
    
    //  initializeRuntime helpers
    
	void
	makeRuntimeBuffers(SetupGridPtr grid, Map<std::string, FieldsPtr> & fields,
		Map<SetupTFSFBufferSetPtr, TFSFBufferSetPtr> & buffers);
	
    /**
     *  Helper function for initializeRuntime: create Sources
     *
     *  @param  grid        the setup grid for which to make Sources
     *  @param  inFields    memory addresses of electromagnetic fields
     */
    std::vector<SourcePtr>
    makeRuntimeSources(SetupGridPtr grid, FieldsPtr inFields);
    
    
    /**
     *  Helper function for initializeRuntime: create Outputs
     *
     *  @param  grid        the setup grid for which to make Outputs
     *  @param  inFields    memory addresses of electromagnetic fields
     *  @param  inOutputRunlines    run
     */
    std::vector<OutputPtr>
    makeRuntimeOutputs(SetupGridPtr grid, FieldsPtr inFields);
    
    /**
     *  Helper function for initializeRuntime: create Inputs
     *
     *  @param  grid        the setup grid for which to make Inputs
     *  @param  inFields    memory addresses of electromagnetic fields
     */
    std::vector<InputPtr>
    makeRuntimeInputs(SetupGridPtr grid, FieldsPtr inFields);
    
	std::vector<MaterialPtr>
    makeRuntimeMaterialModels(SetupGridPtr grid, FieldsPtr theFields,
        Map<std::string, FieldsPtr> & allFields,
        const std::vector<RunlineType> & inMaterialRunlines,
		Map<SetupTFSFBufferSetPtr, TFSFBufferSetPtr> & inBuffers,
		float dx, float dy, float dz, float dt);
	
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

