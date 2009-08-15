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
    if (description->getSpaceTimeFile() != "")
        throw(Exception("Cannot yet create space-varying source currents."));
    
}