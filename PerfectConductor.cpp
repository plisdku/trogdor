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

PerfectConductorDelegate::
PerfectConductorDelegate() :
	SimpleBulkMaterialDelegate()
{
    
}


MaterialPtr PerfectConductorDelegate::
makeCalcMaterial(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    return MaterialPtr(new PerfectConductor);
}


PerfectConductor::
PerfectConductor()
{
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
