/*
 *  MaterialFactory.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "MaterialFactory.h"

#include "VoxelGrid.h"
#include "VoxelizedPartition.h"
#include "PartitionCellCount.h"
#include "SimulationDescription.h"

#include "STLOutput.h"
#include "geometry.h"

// Headers for the materials we'll make
#include "StaticDielectric.h"
#include "StaticLossyDielectric.h"
#include "DrudeModel1.h"
#include "PerfectConductor.h"

// Headers for the available PML types
#include "CFSRIPML.h"


template<class MaterialT, class RunlineT>
static RunlineEncoderPtr newCFSRIPML(Paint* parentPaint,
    vector<int> numCellsE, vector<int> numCellsH, vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<string, string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );

#pragma mark *** Material Factory ***

RunlineEncoderPtr MaterialFactory::
newRunlineEncoder(const VoxelGrid & vg, const PartitionCellCountPtr cg, 
    const GridDescription & gridDesc,
	Paint* parentPaint,
    std::vector<int> numCellsE,
    std::vector<int> numCellsH,
    std::vector<Rect3i> pmlRects,
    int runlineDirection )
{
	assert(parentPaint != 0L);
    
    RunlineEncoderPtr setupMat;
	const MaterialDescription* bulkMaterial = parentPaint->getBulkMaterial();
    const Map<Vector3i, Map<string, string> > & gridPMLParams(
        gridDesc.getPMLParams());
    Map<Vector3i, Map<string, string> > pmlParams;
    
    //LOG << "Hey, grid pml: \n";
    //LOGMORE << gridPMLParams << endl;
    
    //LOG << "PML rects " << pmlRects << "\n";
    
    // This creates the map of PML parameters, first consulting the material's
    // parameters and secondarily the grid's default parameters and the global
    // default parameters.  In the usual case neither map will have anything,
    // because I will be confident in the parameters that I build in to the
    // program.  (-: (-: (-:
    if (parentPaint->isPML())
    {
        pmlParams = defaultPMLParams();
        const Map<Vector3i, Map<string, string> > & matParams =
            bulkMaterial->getPMLParams();
        
        for (int sideNum = 0; sideNum < 6; sideNum++)
        {
            Vector3i dir = cardinal(sideNum);
            
            map<string, string>::const_iterator itr;
            for (itr = pmlParams[dir].begin(); itr != pmlParams[dir].end();
                itr++)
            {
                if (matParams.count(dir) && matParams[dir].count(itr->first))
                    pmlParams[dir][itr->first] =
                        matParams[dir][itr->first];
                else if (gridPMLParams.count(dir) &&
                    gridPMLParams[dir].count(itr->first))
                    pmlParams[dir][itr->first] =
                        gridPMLParams[dir][itr->first];
            }
        }
        //LOG << "PML is " << pmlParams << "\n";
    }
    
	//LOG << "Getting delegate for " << *parentPaint << ".\n"; 
    
    if (0 == parentPaint->isPML())
    {
        
        if (bulkMaterial->getModelName() == "StaticDielectric")
        {
            setupMat = RunlineEncoderPtr(
                new SimpleSetupMaterial<StaticDielectric, SimpleRunline>(
                    parentPaint, numCellsE, numCellsH, gridDesc.getDxyz(),
                    gridDesc.getDt()));
        }
        else if (bulkMaterial->getModelName() == "StaticLossyDielectric")
        {
            setupMat = RunlineEncoderPtr(
                new SimpleSetupMaterial<StaticLossyDielectric, SimpleRunline>(
                    parentPaint, numCellsE, numCellsH, gridDesc.getDxyz(),
                    gridDesc.getDt()));
        }
        else if (bulkMaterial->getModelName() == "DrudeMetal1")
        {
            setupMat = RunlineEncoderPtr(
                new SimpleSetupMaterial<DrudeModel1, SimpleAuxRunline>(
                    parentPaint, numCellsE, numCellsH, gridDesc.getDxyz(),
                    gridDesc.getDt()));
        }
        else if (bulkMaterial->getModelName() == "PerfectConductor")
        {
            setupMat = RunlineEncoderPtr(new SetupPerfectConductor);
        }
        else
        {
            throw(Exception(string("Unsupported material model: ") + 
                bulkMaterial->getModelName()));
        }
    }
    else
    {
        if (bulkMaterial->getModelName() == "StaticDielectric")
        {
            setupMat = newCFSRIPML<StaticDielectric, SimpleAuxPMLRunline>(
                parentPaint, numCellsE, numCellsH, pmlRects, pmlParams,
                gridDesc.getDxyz(), gridDesc.getDt(), runlineDirection);
        }
        else if (bulkMaterial->getModelName() == "StaticLossyDielectric")
        {
            setupMat = newCFSRIPML<StaticLossyDielectric, SimpleAuxPMLRunline>(
                parentPaint, numCellsE, numCellsH, pmlRects, pmlParams,
                gridDesc.getDxyz(), gridDesc.getDt(), runlineDirection);
        }
        else if (bulkMaterial->getModelName() == "DrudeMetal1")
        {
            setupMat = newCFSRIPML<DrudeModel1, SimpleAuxPMLRunline>(
                parentPaint, numCellsE, numCellsH, pmlRects, pmlParams,
                gridDesc.getDxyz(), gridDesc.getDt(), runlineDirection);
        }
        else if (bulkMaterial->getModelName() == "PerfectConductor")
        {
            setupMat = RunlineEncoderPtr(new SetupPerfectConductor);
        }
        else
        {
            throw(Exception(string("Unsupported PML material model: ") + 
                bulkMaterial->getModelName()));
        }
    }
    
    
    return setupMat;
}


Map<Vector3i, Map<string, string> > MaterialFactory::
defaultPMLParams()
{
    Map<string, string> allDirectionsDefault;
    Map<Vector3i, Map<string, string> > params;
    
    allDirectionsDefault["sigma"] =
        "(d^3)*0.8*4/(((mu0/eps0)^0.5)*dx)";
    allDirectionsDefault["alpha"] =
        "d*3e8*eps0/(50*dx)";
    allDirectionsDefault["kappa"] =
        "1 + (5-1)*(d^3)";
    /*
    allDirectionsDefault["kappa"] = "d";
    allDirectionsDefault["alpha"] = "d";
    allDirectionsDefault["sigma"] = "d"; // I'll take L to be the PML thickness
    */
    
    for (int sideNum = 0; sideNum < 6; sideNum++)
        params[cardinal(sideNum)] = allDirectionsDefault;
    
    return params;
}


#pragma mark *** Local templated functions ***

template<class MaterialT, class RunlineT>
static RunlineEncoderPtr newCFSRIPML(Paint* parentPaint,
    vector<int> numCellsE, vector<int> numCellsH, vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<string, string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection )
{
    Vector3i pmlDirs = parentPaint->getPMLDirections();
    Vector3i rotatedPMLDirs = cyclicPermute(pmlDirs, (3-runlineDirection)%3);
    
    LOGF << "This PML absorbs along " << pmlDirs
        << " and the memory direction"
        " is " << runlineDirection << ", so the rotated PML direction is "
        << rotatedPMLDirs << ".\n";
    
    RunlineEncoder* m;
    if (rotatedPMLDirs[0] != 0)
    {
        if (rotatedPMLDirs[1] != 0)
        {
            if (rotatedPMLDirs[2] != 0)
            {
                m = new SimpleSetupPML<MaterialT, RunlineT, CFSRIPML<1,1,1> >(
                    parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                    dxyz, dt);
            }
            else
            {
                m = new SimpleSetupPML<MaterialT, RunlineT, CFSRIPML<1,1,0> >(
                    parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                    dxyz, dt);
            }
        }
        else
        {
            if (rotatedPMLDirs[2] != 0)
            {
                m = new SimpleSetupPML<MaterialT, RunlineT, CFSRIPML<1,0,1> >(
                    parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                    dxyz, dt);
            }
            else
            {
                m = new SimpleSetupPML<MaterialT, RunlineT, CFSRIPML<1,0,0> >(
                    parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                    dxyz, dt);
            }
        }
    }
    else
    {
        if (rotatedPMLDirs[1] != 0)
        {
            if (rotatedPMLDirs[2] != 0)
            {
                m = new SimpleSetupPML<MaterialT, RunlineT, CFSRIPML<0,1,1> >(
                    parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                    dxyz, dt);
            }
            else
            {
                m = new SimpleSetupPML<MaterialT, RunlineT, CFSRIPML<0,1,0> >(
                    parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                    dxyz, dt);
            }
        }
        else
        {
            if (rotatedPMLDirs[2] != 0)
            {
                m = new SimpleSetupPML<MaterialT, RunlineT, CFSRIPML<0,0,1> >(
                    parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                    dxyz, dt);
            }
            else
            {
                m = 0L;
                assert(!"PML must have a direction.");
                cerr << "PML must have a direction.\n";
                exit(1);
            }
        }
    }
    return RunlineEncoderPtr(m);
}
