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
SetupCurrentSource(const CurrentSourceDescPtr & description) :
    mDescription(description)
{
//    LOG << "Constructor!\n";
}

SetupCurrentSource::
~SetupCurrentSource()
{
//    LOG << "Destructor!\n";
}

Pointer<CurrentSource> SetupCurrentSource::
makeCurrentSource(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
//    LOG << "Making one!\n";
    
    return Pointer<CurrentSource>(new CurrentSource);
}





CurrentSource::
CurrentSource()
{
//    LOG << "Constructor!\n";
}

CurrentSource::
~CurrentSource()
{
//    LOG << "Destructor!\n";
}


void CurrentSource::
prepareJ(long timestep)
{
//    LOG << "Somehow loading J into all the right BufferedCurrents.\n";
}

void CurrentSource::
prepareK(long timestep)
{
//    LOG << "Somehow loading K into all the right BufferedCurrents.\n";
}



float CurrentSource::
getJ(int direction) const
{
    assert(direction >= 0 && direction < 3);
    return 0.0f;
}

float CurrentSource::
getK(int direction) const
{
    assert(direction >= 0 && direction < 3);
    return 0.0f;
}
