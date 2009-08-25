/*
 *  CurrentSourceFactory.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "CurrentSourceFactory.h"

Pointer<SetupCurrentSource> CurrentSourceFactory::
newCurrentSource(const CurrentSourceDescPtr & description)
{
    /*
    if (description->spaceTimeFile() != "")
        throw(Exception("Cannot yet create space-varying source currents."));
    
    if (description->formula() != "")
        return Pointer<SetupCurrentSource>(
            new SetupFormulaCurrentSource(description));
    else if (description->timeFile() != "" ||
        description->spaceTimeFile() != "")
    {
        return Pointer<SetupCurrentSource>(
            new SetupFileCurrentSource(description));
    }
    */
    
    throw(Exception("Can't make this source type."));
    return Pointer<SetupCurrentSource>(0L);
}