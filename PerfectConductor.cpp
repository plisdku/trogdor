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



MaterialPtr SetupPerfectConductor::
makeCalcMaterial(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    int numRunlinesE = getRunlinesE(0).size() + getRunlinesE(1).size()
        + getRunlinesE(2).size();
    int numRunlinesH = getRunlinesH(0).size() + getRunlinesH(1).size()
        + getRunlinesH(2).size();
    
    int numHalfCellsE = 0;
    int numHalfCellsH = 0;
    
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < getRunlinesE(direction).size(); nn++)
        numHalfCellsE += getRunlinesE(direction)[nn]->length;
    
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < getRunlinesH(direction).size(); nn++)
        numHalfCellsH += getRunlinesH(direction)[nn]->length;
    
    return MaterialPtr(new PerfectConductor(numRunlinesE, numRunlinesH,
        numHalfCellsE, numHalfCellsH));
}

PerfectConductor::
PerfectConductor(int numRunlinesE, int numRunlinesH, int numHalfCellsE,
    int numHalfCellsH) :
    Material(),
    mNumRunlinesE(numRunlinesE),
    mNumRunlinesH(numRunlinesH),
    mNumHalfCellsE(numHalfCellsE),
    mNumHalfCellsH(numHalfCellsH)
{
}

string PerfectConductor::
getModelName() const
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
