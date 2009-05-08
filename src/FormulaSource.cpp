/*
 *  FormulaSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/7/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "FormulaSource.h"

#include "SimulationDescription.h"
#include <cstdlib>
#include <iostream>

using namespace std;

#pragma mark *** Delegate ***

FormulaSourceDelegate::
FormulaSourceDelegate(const SourceDescPtr & desc) :
    SourceDelegate(),
    mDesc(desc)
{
}

OutputPtr FormulaSourceDelegate::
makeSource(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    return OutputPtr(new FormulaSource(*mDesc));
}


FormulaSource::
FormulaSource(const SourceDescription & desc) :
    Source(),
    mFormula(desc.getFormula()),
    mPolarization(desc.getPolarization()),
    mWhichE(desc.getWhichE()),
    mWhichH(desc.getWhichH()),
    mIsSpaceVarying(desc.isSpaceVarying()),
    mIsSoft(desc.isSoft()),
    mParams(desc.getParams())
{
}



#pragma mark *** Source ***





