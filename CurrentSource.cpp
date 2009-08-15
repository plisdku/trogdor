/*
 *  CurrentSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "CurrentSource.h"


SetupCurrentSource::
SetupCurrentSource(const CurrentSourceDescPtr & description)
{
}

SetupCurrentSource::
~SetupCurrentSource()
{
}

Pointer<CurrentSource> SetupCurrentSource::
makeCurrentSource(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    return Pointer<CurrentSource>(0L);
}





CurrentSource::
CurrentSource()
{
}

CurrentSource::
~CurrentSource()
{
}


void CurrentSource::
prepareJ(long timestep)
{
    LOG << "Somehow loading J into all the right BufferedCurrents.\n";
}

void CurrentSource::
prepareK(long timestep)
{
    LOG << "Somehow loading K into all the right BufferedCurrents.\n";
}

