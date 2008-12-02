/*
 *  SourceFactory.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/14/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef _SOURCEFACTORY_
#define _SOURCEFACTORY_

#include "Source.h"
#include "SetupSource.h"
#include "SetupGrid.h"
#include "Fields.h"

//  C++ headers
#include "Map.h"
#include <string>

class SourceFactory
{
public:
    static Source*
    createSource(const SetupGrid & inGrid, Fields & inFields,
        const SetupSource & inSource);
private:
    SourceFactory();
    
};


#endif

