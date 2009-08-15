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

class FormulaCurrentSource : public CurrentSource
{
public:
    FormulaCurrentSource();
    
private:
    calc_defs::Calculator<float> mCalculator;
};





#endif
