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

class SetupPerfectConductor : public SimpleBulkSetupMaterial
{
public:
	SetupPerfectConductor();
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    
};


class PerfectConductor : public Material
{
public:
    PerfectConductor();
        
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
};


#endif