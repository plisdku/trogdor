/*
 *  SetupInput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/26/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SetupInput.h"

using namespace std;


SetupInput::
SetupInput(string inFilePrefix, string inClass, Map<string, string> inParams) :
    mFilePrefix(inFilePrefix),
    mClass(inClass),
    mParams(inParams)
{
    LOGF << "Constructor.\n";
}

SetupInput::
~SetupInput()
{
    LOGF << "Destructor.\n";
}

