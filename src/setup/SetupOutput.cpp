/*
 *  SetupOutput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/19/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SetupOutput.h"


using namespace std;


SetupOutput::
SetupOutput(string inFilePrefix, string inClass, int inPeriod,
            Map<string, string> inParams) :
    mPeriod(inPeriod),
    mFilePrefix(inFilePrefix),
    mClass(inClass),
    mParams(inParams)
{
    LOGF << "Constructor.\n";
}

SetupOutput::
~SetupOutput()
{
    LOGF << "Destructor.\n";
}


