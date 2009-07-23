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
    SetupPerfectConductor() {}
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
};

class PerfectConductor : public Material
{
public:
    PerfectConductor(int numRunlinesE, int numRunlinesH, int numHalfCellsE,
        int numHalfCellsH);
    
    virtual std::string getModelName() const;
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
    
    virtual long getNumRunlinesE() const { return mNumRunlinesE; }
    virtual long getNumRunlinesH() const { return mNumRunlinesH; }
    virtual long getNumHalfCellsE() const { return mNumHalfCellsE; }
    virtual long getNumHalfCellsH() const { return mNumHalfCellsH; }

private:
    int mNumRunlinesE;
    int mNumRunlinesH;
    int mNumHalfCellsE;
    int mNumHalfCellsH;
};


#endif