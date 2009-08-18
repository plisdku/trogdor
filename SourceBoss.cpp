/*
 *  SourceBoss.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/7/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "SourceBoss.h"

#include "VoxelizedPartition.h"
#include "CalculationPartition.h"
#include "SimulationDescription.h"

//#include "SimpleEHSource.h"
#include "FormulaSource.h"
#include "FileSource.h"

#include <cstdlib>

using namespace std;

SetupSourcePtr SourceFactory::
newSetupSource(const VoxelizedPartition & vp, const SourceDescPtr & desc)
{
    
    if (desc->formula() != "")
        return SetupSourcePtr(new FormulaSetupSource(desc));
    else if (desc->timeFile() != "" || desc->spaceTimeFile() != "")
    {
        return SetupSourcePtr(new FileSetupSource(desc));
    }
    
    LOG << "Using default (null) source... and crashing.\n";
    exit(1);
    
    return SetupSourcePtr(0L);
}


Source::
Source()
{
}

Source::
~Source()
{
}

void Source::
sourceEPhase(CalculationPartition & cp, int timestep)
{
    LOG << "Source E timestep " << timestep << "\n";
}

void Source::
sourceHPhase(CalculationPartition & cp, int timestep)
{
    LOG << "Source H timestep " << timestep << "\n";
}

