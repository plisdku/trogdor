/*
 *  MaterialFactory.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/30/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "MaterialFactory.h"
#include "Map.h"

#include "PECModel.h"
#include "StaticDielectricModel.h"
#include "StaticLossyDielectricModel.h"
#include "DrudeMetalModel.h"

//  C++ headers
#include <string>
#include <cstdlib>
using namespace std;


MaterialFactory::
MaterialFactory()
{
    LOG << "How did you call this constructor?\n";
}


MaterialModel* MaterialFactory::
createMaterial(const SetupMaterialModel & inMaterial)
{
    MaterialModel* material = NULL;
    
    const string& matClass = inMaterial.getClass();
    const Map<string, string> & matParams = inMaterial.getParameters();
    
    if (matClass == "StaticDielectricModel")
        material = new StaticDielectricModel(matParams);
    else if (matClass == "PECModel")
        material = new PECModel();
    else if (matClass == "DrudeMetalModel")
        material = new DrudeMetalModel(matParams);
    else if (matClass == "StaticLossyDielectricModel")
        material = new StaticLossyDielectricModel(matParams);
    else
    {
        cerr << "Unimplemented material class "
            << matClass << ".  Look at MaterialFactory.cpp.\n";
        exit(1);
    }
        
    return material;
}


