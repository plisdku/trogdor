/*
 *  PerfectConductor.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "PerfectConductor.h"
#include "CalculationPartition.h"
#include "Paint.h"
#include "Log.h"

#include <sstream>

using namespace std;



UpdateEquationPtr SetupPerfectConductor::
makeUpdateEquation(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    int numRunlinesE = runlinesE(0).size() + runlinesE(1).size()
        + runlinesE(2).size();
    int numRunlinesH = runlinesH(0).size() + runlinesH(1).size()
        + runlinesH(2).size();
    
    int numHalfCellsE = 0;
    int numHalfCellsH = 0;
    
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < runlinesE(direction).size(); nn++)
        numHalfCellsE += runlinesE(direction)[nn]->length;
    
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < runlinesH(direction).size(); nn++)
        numHalfCellsH += runlinesH(direction)[nn]->length;
    
    return UpdateEquationPtr(new PerfectConductor(numRunlinesE, numRunlinesH,
        numHalfCellsE, numHalfCellsH));
}

PerfectConductor::
PerfectConductor(int numRunlinesE, int numRunlinesH, int numHalfCellsE,
    int numHalfCellsH) :
    UpdateEquation(),
    mNumRunlinesE(numRunlinesE),
    mNumRunlinesH(numRunlinesH),
    mNumHalfCellsE(numHalfCellsE),
    mNumHalfCellsH(numHalfCellsH)
{
}

string PerfectConductor::
modelName() const
{
    return string("PerfectConductor");
}

void PerfectConductor::
calcEPhase(int direction)
{
    //LOG << "Calculating E.\n";
}

void PerfectConductor::
calcHPhase(int direction)
{
    //LOG << "Calculating H.\n";
}
