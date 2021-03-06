/*
 *  MaterialFactory.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "MaterialFactory.h"

#include "SimulationDescription.h"

#include "STLOutput.h"
#include "YeeUtilities.h"
#include "geometry.h"

// Headers for the materials we'll make
#include "StaticDielectric.h"
#include "StaticLossyDielectric.h"
#include "DrudeModel1.h"
#include "PerfectConductor.h"

// Headers for the available current types
#include "NullCurrent.h"
#include "BufferedCurrent.h"

// Headers for the available PML types
//#include "NullPML.h"    // not needed in this file.
#include "CFSRIPML.h"

using namespace std;
using namespace YeeUtilities;

// These three functions successively determine the material, current and
// PML for a SetupUpdateEquation.
static SetupUpdateEquationPtr newMaterialCurrentPML(Paint* parentPaint,
    vector<long> numCellsE, vector<long> numCellsH, vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<string, string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );

template<class MaterialT, class NonPMLRunlineT, class PMLRunlineT>
static SetupUpdateEquationPtr newCurrentPML(Paint* parentPaint,
    vector<long> numCellsE, vector<long> numCellsH, vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<string, string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );
        
template<class MaterialT, class NonPMLRunlineT, class PMLRunlineT,
    class CurrentT>
static SetupUpdateEquationPtr newPML(Paint* parentPaint,
    vector<long> numCellsE, vector<long> numCellsH, vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<string, string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );


#pragma mark *** Material Factory ***

SetupUpdateEquationPtr MaterialFactory::
newSetupUpdateEquation(
    const GridDescription & gridDesc,
	Paint* parentPaint,
    std::vector<long> numCellsE,
    std::vector<long> numCellsH,
    std::vector<Rect3i> pmlRects,
    int runlineDirection )
{
	assert(parentPaint != 0L);
    
    SetupUpdateEquationPtr setupMat;
	MaterialDescPtr bulkMaterial = parentPaint->bulkMaterial();
    const Map<Vector3i, Map<string, string> > & gridPMLParams(
        gridDesc.pmlParams());
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
            bulkMaterial->pmlParams();
        
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
        LOGF << "PML is " << pmlParams << "\n";
    }
    
    setupMat = newMaterialCurrentPML(parentPaint, numCellsE, numCellsH,
        pmlRects, pmlParams, gridDesc.dxyz(), gridDesc.dt(),
        runlineDirection);
        
    return setupMat;
}


Map<Vector3i, Map<string, string> > MaterialFactory::
defaultPMLParams()
{
    Map<string, string> allDirectionsDefault;
    Map<Vector3i, Map<string, string> > params;
    
    LOGF << "Setting default PML parameters." << endl;
    
    allDirectionsDefault["sigma"] =
        "(d^3)*0.8*4/(((mu0/eps0)^0.5)*dx)";
    allDirectionsDefault["alpha"] =
        "(1-d)*3e8*eps0/(50*dx)";
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


static SetupUpdateEquationPtr newMaterialCurrentPML(Paint* parentPaint,
    vector<long> numCellsE, vector<long> numCellsH, vector<Rect3i> pmlRects,
        Map<Vector3i, Map<string, string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection )
{
	MaterialDescPtr bulkMaterial = parentPaint->bulkMaterial();
    SetupUpdateEquationPtr setupMaterial;
    
    if (bulkMaterial->modelName() == "StaticDielectric")
    {
        setupMaterial = newCurrentPML<StaticDielectric, SimpleRunline,
            SimpleAuxPMLRunline>(
                parentPaint, numCellsE, numCellsH, pmlRects, pmlParams,
                dxyz, dt, runlineDirection);
    }
    else if (bulkMaterial->modelName() == "StaticLossyDielectric")
    {
        setupMaterial = newCurrentPML<StaticLossyDielectric, SimpleRunline,
            SimpleAuxPMLRunline>(
                parentPaint, numCellsE, numCellsH, pmlRects, pmlParams,
                dxyz, dt, runlineDirection);
    }
    else if (bulkMaterial->modelName() == "DrudeMetal1")
    {
        setupMaterial = newCurrentPML<DrudeModel1, SimpleAuxRunline,
            SimpleAuxPMLRunline>(
                parentPaint, numCellsE, numCellsH, pmlRects, pmlParams,
                dxyz, dt, runlineDirection);
    }
    else if (bulkMaterial->modelName() == "PerfectConductor")
    {
        setupMaterial = SetupUpdateEquationPtr(new SetupPerfectConductor(
            MaterialDescPtr(parentPaint->bulkMaterial())));
    }
    else
    {
        throw(Exception(string("Unsupported PML material model: ") + 
            bulkMaterial->modelName()));
    }
    
    return setupMaterial;
}

template<class MaterialT, class NonPMLRunlineT, class PMLRunlineT>
static SetupUpdateEquationPtr newCurrentPML(Paint* parentPaint,
    vector<long> numCellsE, vector<long> numCellsH, vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<string, string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection )
{
    //const MaterialDescription* bulkMaterial = parentPaint->bulkMaterial();
    SetupUpdateEquationPtr setupMaterial;
    
//    LOG << "Here, with paint " << *parentPaint << "\n";
    
    if (0 == parentPaint->hasCurrentSource())
    {
        setupMaterial = newPML<MaterialT, NonPMLRunlineT, PMLRunlineT,
            NullCurrent>(parentPaint, numCellsE, numCellsH, pmlHalfCells,
                pmlParams, dxyz, dt, runlineDirection);
    }
    else
    {
        setupMaterial = newPML<MaterialT, NonPMLRunlineT, PMLRunlineT,
            BufferedCurrent>(parentPaint, numCellsE, numCellsH, pmlHalfCells,
                pmlParams, dxyz, dt, runlineDirection);
    }
    
    
    return setupMaterial;
}
        
template<class MaterialT, class NonPMLRunlineT, class PMLRunlineT,
    class CurrentT>
static SetupUpdateEquationPtr newPML(Paint* parentPaint,
    vector<long> numCellsE, vector<long> numCellsH, vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<string, string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection )
{
    Vector3i pmlDirs = parentPaint->pmlDirections();
    Vector3i rotatedPMLDirs = cyclicPermute(pmlDirs, (3-runlineDirection)%3);
    SetupUpdateEquation* m;
    
    if (0 == parentPaint->isPML())
    {
        m = new SetupModularUpdateEquation<MaterialT, NonPMLRunlineT, CurrentT>(
            parentPaint, numCellsE, numCellsH, dxyz, dt);
    }
    else
    {
        LOGF << "This PML absorbs along " << pmlDirs
            << " and the memory direction"
            " is " << runlineDirection << ", so the rotated PML direction is "
            << rotatedPMLDirs << ".\n";
        
        bool hasX = (rotatedPMLDirs[0] != 0);
        bool hasY = (rotatedPMLDirs[1] != 0);
        bool hasZ = (rotatedPMLDirs[2] != 0);
        
        if (hasX && hasY && hasZ)
        {
            m = new SimpleSetupPML<MaterialT, PMLRunlineT, CurrentT,
                CFSRIPML<1,1,1> >(
                parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                dxyz, dt);
        }
        else if (hasX && hasY)
        {
            m = new SimpleSetupPML<MaterialT, PMLRunlineT, CurrentT,
                CFSRIPML<1,1,0> >(
                parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                dxyz, dt);
        }
        else if (hasX && hasZ)
        {
            m = new SimpleSetupPML<MaterialT, PMLRunlineT, CurrentT,
                CFSRIPML<1,0,1> >(
                parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                dxyz, dt);
        }
        else if (hasX)
        {
            m = new SimpleSetupPML<MaterialT, PMLRunlineT, CurrentT,
                CFSRIPML<1,0,0> >(
                parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                dxyz, dt);
        }
        else if (hasY && hasZ)
        {
            m = new SimpleSetupPML<MaterialT, PMLRunlineT, CurrentT,
                CFSRIPML<0,1,1> >(
                parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                dxyz, dt);
        }
        else if (hasY)
        {
            m = new SimpleSetupPML<MaterialT, PMLRunlineT, CurrentT,
                CFSRIPML<0,1,0> >(
                parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams,
                dxyz, dt);
        }
        else if (hasZ)
        {
            m = new SimpleSetupPML<MaterialT, PMLRunlineT, CurrentT,
                CFSRIPML<0,0,1> >(
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
    return SetupUpdateEquationPtr(m);
}

