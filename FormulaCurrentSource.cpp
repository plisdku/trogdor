/*
 *  FormulaCurrentSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "FormulaCurrentSource.h"

SetupFormulaCurrentSource::
SetupFormulaCurrentSource(const CurrentSourceDescPtr & description) :
    SetupCurrentSource(description)
{
    LOG << "Doing my thing, just doing my thing!\n";
}