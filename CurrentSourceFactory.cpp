/*
 *  CurrentSourceFactory.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "CurrentSourceFactory.h"

#include "FormulaCurrentSource.h"
#include "FileCurrentSource.h"

Pointer<SetupCurrentSource> CurrentSourceFactory::
newCurrentSource(const CurrentSourceDescPtr & description)
{
    if (description->getSpaceTimeFile() != "")
        throw(Exception("Cannot yet create space-varying source currents."));
    
    if (description->getFormula() != "")
        return Pointer<SetupCurrentSource>(
            new SetupFormulaCurrentSource(description));
    else if (description->getTimeFile() != "" ||
        description->getSpaceTimeFile() != "")
    {
        return Pointer<SetupCurrentSource>(
            new SetupFileCurrentSource(description));
    }
    
    throw(Exception("Can't make this source type."));
    return Pointer<SetupCurrentSource>(0L);
}