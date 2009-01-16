/*
 *  SetupOutput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/19/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
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


