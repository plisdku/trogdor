/*
 *  MaterialFactory.h
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

#ifndef _MATERIALFACTORY_
#define _MATERIALFACTORY_

#include "MaterialModel.h"
#include "SetupMaterialModel.h"

class MaterialFactory
{
public:
    static MaterialModel* createMaterial(const SetupMaterialModel & inType);
private:
    MaterialFactory();
    
};

#endif
