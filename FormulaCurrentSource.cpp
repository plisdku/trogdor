/*
 *  FormulaCurrentSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "FormulaCurrentSource.h"

#include "VoxelizedPartition.h"
#include "CalculationPartition.h"

#include <iostream>
using namespace std;

SetupFormulaCurrentSource::
SetupFormulaCurrentSource(const CurrentSourceDescPtr & description) :
    SetupCurrentSource(description)
{
//    LOG << "Doing my thing, just doing my thing!\n";
}

Pointer<CurrentSource> SetupFormulaCurrentSource::
makeCurrentSource(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    return Pointer<CurrentSource>(new FormulaCurrentSource(getDescription(),
        vp, cp));
}

FormulaCurrentSource::
FormulaCurrentSource(const CurrentSourceDescPtr & description,
    const VoxelizedPartition & vp, const CalculationPartition & cp) :
    mFormula(description->getFormula()),
    mCurrents(description->getSourceCurrents()),
    mDt(cp.getDt()),
    mDurations(description->getDurations()),
    mCurrentDuration(0)
{   
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].getLast() > (cp.getDuration()-1))
        mDurations[dd].setLast(cp.getDuration()-1);
    
	// The calculator will eventually update "n" and "t" to be the current
	// timestep and current time; we can set them here to test the formula.
	mCalculator.set("n", 0);
	mCalculator.set("t", 0);
	
	LOGF << "Formula is " << mFormula << endl;
	
	bool err = mCalculator.parse(mFormula);
	if (err)
	{
		LOG << "Error found.";
		cerr << "Calculator cannot parse\n"
			<< mFormula << "\n"
			<< "Error message:\n";
		mCalculator.report_error(cerr);
		cerr << "Quitting.\n";
		assert(!"Assert death.");
		exit(1);
	}
}


void FormulaCurrentSource::
prepareJ(long timestep)
{
    mJ = Vector3f(0.0f, 0.0f, 0.0f);
    if (norm2(mCurrents.getWhichJ()) == 0)
        return;
    
    if (mCurrentDuration >= mDurations.size())
    {
        return;
    }
    while (timestep > mDurations[mCurrentDuration].getLast())
    {
        mCurrentDuration++;
        if (mCurrentDuration >= mDurations.size())
            return;
    }
    
    if (timestep >= mDurations[mCurrentDuration].getFirst())
    {
        updateJ(timestep);
    }
}

void FormulaCurrentSource::
prepareK(long timestep)
{
    mK = Vector3f(0.0f, 0.0f, 0.0f);
    if (norm2(mCurrents.getWhichK()) == 0)
        return;
    
    if (mCurrentDuration >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentDuration].getLast())
    {
        mCurrentDuration++;
        if (mCurrentDuration >= mDurations.size())
            return;
    }
    
    if (timestep >= mDurations[mCurrentDuration].getFirst())
    {
        updateK(timestep);
    }
}

float FormulaCurrentSource::
getJ(int direction) const
{
    assert(direction >= 0 && direction < 3);
    return mJ[direction];
}

float FormulaCurrentSource::
getK(int direction) const
{
    assert(direction >= 0 && direction < 3);
    return mK[direction];
}


void FormulaCurrentSource::
updateJ(long timestep)
{
    float val;
    
	mCalculator.set("n", timestep);
	mCalculator.set("t", mDt*timestep);
	mCalculator.parse(mFormula);
    
	val = mCalculator.get_value();
    
//    LOG << "Value: " << val << "\n";
    
    if (mCurrents.usesPolarization())
        mJ = val*mCurrents.getPolarization();
    else
        mJ = Vector3f(val,val,val);
}

void FormulaCurrentSource::
updateK(long timestep)
{
    float val;
    
	mCalculator.set("n", timestep);
	mCalculator.set("t", mDt*timestep);
	mCalculator.parse(mFormula);
    
	val = mCalculator.get_value();
//    LOG << "Value: " << val << "\n";
    
    if (mCurrents.usesPolarization())
        mK = val*mCurrents.getPolarization();
    else
        mK = Vector3f(val,val,val);
}

