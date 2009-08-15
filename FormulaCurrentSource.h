/*
 *  FormulaCurrentSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _FORMULACURRENTSOURCE_
#define _FORMULACURRENTSOURCE_

#include "CurrentSource.h"
#include "calc.hh"

class SetupFormulaCurrentSource : public SetupCurrentSource
{
public:
    SetupFormulaCurrentSource(const CurrentSourceDescPtr & description);
    
private:
    calc_defs::Calculator<float> mCalculator;
};





#endif
