/*
 *  InputFactory.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/26/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _INPUTFACTORY_
#define _INPUTFACTORY_

#include "Input.h"
#include "SetupGrid.h"
#include "Fields.h"

#include <string>

class InputFactory
{
public:
    static Input*
    createInput(const SetupGrid & inGrid, Fields & inFields,
        const SetupInput & inInput);
    

private:
    InputFactory();
};



#endif
