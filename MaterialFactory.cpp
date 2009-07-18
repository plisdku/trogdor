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
#include "StaticDielectricPML.h"
#include "StaticLossyDielectric.h"
#include "DrudeModel1.h"
#include "DrudeModel1PML.h"
#include "PerfectConductor.h"

// Headers for the available PML types
#include "CFSRIPML.h"

SetupMaterialPtr MaterialFactory::
newSetupMaterial(const VoxelGrid & vg, const PartitionCellCountPtr cg, 
    const GridDescription & gridDesc,
	Paint* parentPaint,
    std::vector<int> numCellsE,
    std::vector<int> numCellsH,
    std::vector<Rect3i> pmlRects)
{
	assert(parentPaint != 0L);
    
    SetupMaterialPtr matDel;
	const MaterialDescPtr bulkMaterial = parentPaint->getBulkMaterial();
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
    
	if (bulkMaterial->getModelName() == "StaticDielectric")
	{
        if (0 == parentPaint->isPML())
        {
            matDel = SetupMaterialPtr(
                new SimpleSetupMaterial<StaticDielectric>(
                    parentPaint, numCellsE, numCellsH, gridDesc.getDxyz(),
                    gridDesc.getDt()));
        }
        else
        {
            matDel = SetupMaterialPtr(
                new SimpleSetupPML<StaticDielectric,  CFSRIPMLFactory>(
                    parentPaint, numCellsE, numCellsH, pmlRects, pmlParams,
                    gridDesc.getDxyz(), gridDesc.getDt()));
            
        }
	}
    else
    {
        cerr << "Warning: returning null material.\n";
    }
    //matDel->setParentPaint(parentPaint);
    
    return matDel;
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
