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
#include "BulkSetupMaterials.h"

class SetupPerfectConductor : public BulkSetupUpdateEquation
{
public:
    SetupPerfectConductor(MaterialDescPtr description) :
        BulkSetupUpdateEquation(description) {}
    
    virtual UpdateEquationPtr makeUpdateEquation(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
};

class PerfectConductor : public UpdateEquation
{
public:
    PerfectConductor(int numRunlinesE, int numRunlinesH, int numHalfCellsE,
        int numHalfCellsH);
    
    virtual std::string modelName() const;
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
    
    virtual long numRunlinesE() const { return mNumRunlinesE; }
    virtual long numRunlinesH() const { return mNumRunlinesH; }
    virtual long numHalfCellsE() const { return mNumHalfCellsE; }
    virtual long numHalfCellsH() const { return mNumHalfCellsH; }

private:
    int mNumRunlinesE;
    int mNumRunlinesH;
    int mNumHalfCellsE;
    int mNumHalfCellsH;
};


#endif