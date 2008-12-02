/*
 *  OutputFactory.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/2/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _OUTPUTFACTORY_
#define _OUTPUTFACTORY_

#include "Output.h"
#include "SetupGrid.h"
#include "SetupOutput.h"
#include "Fields.h"

//  C++ headers
#include <string>

class OutputFactory
{
public:
    static Output*
    createOutput(const SetupGrid & inGrid, Fields & inFields,
        const SetupOutput & inOutput);
private:
    OutputFactory();
    
};



#endif


