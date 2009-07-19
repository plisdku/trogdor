/*
 *  PerfectConductor.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _PERFECTCONDUCTOR_
#define _PERFECTCONDUCTOR_

#include "SimulationDescription.h"
#include "SimpleSetupMaterial.h"
#include "SimpleMaterialTemplates.h"

class PerfectConductor : public SimpleMaterial<SimpleRunline>
{
public:
    PerfectConductor(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt);
    
    virtual std::string getModelName() const;
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
};


#endif