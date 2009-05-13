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

#include <cstdlib>

using namespace std;

SourceDelegatePtr SourceFactory::
getDelegate(const VoxelizedPartition & vp, const SourceDescPtr & desc)
{
    /*
    if (desc->getFormula() != "")
        return SourceDelegatePtr(new FormulaSourceDelegate(desc));
    else if (desc->getFileName() != "")
    {
        LOG << "File source not supported.\n";
        exit(1);
    }
    
    LOG << "Using default (null) source... and crashing.\n";
    exit(1);
    */
    return SourceDelegatePtr(0L);
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
sourceEPhase(int timestep)
{
    LOG << "Source E timestep " << timestep << "\n";
}

void Source::
sourceHPhase(int timestep)
{
    LOG << "Source H timestep " << timestep << "\n";
}

