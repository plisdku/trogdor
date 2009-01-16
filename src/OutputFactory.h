/*
 *  OutputFactory.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/2/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
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


