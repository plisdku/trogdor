/*
 *  OutputBoss.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/6/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "OutputBoss.h"

#include "VoxelizedPartition.h"
#include "CalculationPartition.h"
#include "SimulationDescription.h"

#include "SimpleEHOutput.h"

#include "Version.h"

#include <cstdlib>

using namespace std;


OutputDelegatePtr OutputFactory::
getDelegate(const VoxelizedPartition & vp, const OutputDescPtr & desc)
{
    Vector3i threeFalses(0,0,0);
    
    if (desc->getWhichJ() == threeFalses && desc->getWhichP() == threeFalses &&
        desc->getWhichP() == threeFalses && desc->getWhichM() == threeFalses)
        return OutputDelegatePtr(new SimpleEHOutputDelegate(desc));
    
    LOG << "Using default (null) output... and crashing.\n";
    exit(1);
    
    return OutputDelegatePtr(0L);
}


Output::
Output()
{
}

Output::
~Output()
{
}

void Output::
outputEPhase(const CalculationPartition & cp, int timestep)
{
    //LOG << "Output E timestep " << timestep << "\n";
}

void Output::
outputHPhase(const CalculationPartition & cp, int timestep)
{
    //LOG << "Output H timestep " << timestep << "\n";
}

void Output::
allocateAuxBuffers()
{
    LOG << "Not allocating output buffer.  (What buffer?)\n";
}


