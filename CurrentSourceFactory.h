/*
 *  CurrentSourceFactory.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _CURRENTSOURCEFACTORY_
#define _CURRENTSOURCEFACTORY_

#include "SimulationDescription.h"
#include "CurrentSource.h"


class CurrentSourceFactory
{
public:
    static Pointer<SetupCurrentSource> newCurrentSource(
        const CurrentSourceDescPtr & description);
private:
    CurrentSourceFactory();
};





#endif
