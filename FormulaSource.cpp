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
#include "CalculationPartition.h"
#include "VoxelizedPartition.h"
#include "InterleavedLattice.h"
#include <cstdlib>
#include <iostream>

using namespace std;

#pragma mark *** Delegate ***

FormulaSetupSource::
FormulaSetupSource(const SourceDescPtr & desc) :
    SetupSource(),
    mDesc(desc)
{
}

SourcePtr FormulaSetupSource::
makeSource(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    return SourcePtr(new FormulaSource(*mDesc, vp, cp));
}


FormulaSource::
FormulaSource(const SourceDescription & desc, const VoxelizedPartition & vp,
    const CalculationPartition & cp) :
    Source(),
    mCurrentDuration(0),
    mFormula(desc.getFormula()),
    mCalculator(),
    mFields(desc.getSourceFields()),
    mDt(cp.getDt()),
    mIsSpaceVarying(desc.isSpaceVarying()),
    mIsSoft(desc.isSoftSource()),
    mRegions(desc.getRegions()),
    mDurations(desc.getDurations())
{
    for (int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect(clip(vp.getGridYeeCells(), mRegions[rr].getYeeCells()));
        mRegions[rr].setYeeCells(rect);
    }
    
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


void FormulaSource::
sourceEPhase(CalculationPartition & cp, int timestep)
{
    if (norm2(mFields.getWhichE()) == 0)
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
        uniformSourceE(cp, timestep);
}

void FormulaSource::
sourceHPhase(CalculationPartition & cp, int timestep)
{
    if (norm2(mFields.getWhichH()) == 0)
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
        uniformSourceH(cp, timestep);
}

void FormulaSource::
uniformSourceE(CalculationPartition & cp, int timestep)
{
    //LOG << "Source E\n";
    float val;
    InterleavedLatticePtr lattice(cp.getLattice());
    
	mCalculator.set("n", timestep);
	mCalculator.set("t", mDt*timestep);
	mCalculator.parse(mFormula);
    
	val = mCalculator.get_value();
    
    //LOG << "Writing " << val << "\n";
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].getYeeCells();
        for (int dir = 0; dir < 3; dir++)
        if (mFields.getWhichE()[dir] != 0)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                {
                    lattice->setE(dir, xx, val);
                    //LOG << "Writing at " << ii << " " << jj << " " << kk
                    //    << "\n";
                    //LOG << "E is now " << cp.getE(dir, ii, jj, kk) << "\n";
                }
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setE(dir, xx, lattice->getE(dir, xx)+val);
            }
        }
    }
}

void FormulaSource::
uniformSourceH(CalculationPartition & cp, int timestep)
{
    //LOG << "Source H\n";
    float val;
    InterleavedLatticePtr lattice(cp.getLattice());
    
	mCalculator.set("n", timestep);
	mCalculator.set("t", mDt*(0.5f + timestep));
	mCalculator.parse(mFormula);
	val = mCalculator.get_value();
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].getYeeCells();
        for (int dir = 0; dir < 3; dir++)
        if (mFields.getWhichH()[dir] != 0)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setH(dir, xx, val);
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setH(dir, xx, lattice->getH(dir, xx)+val);
            }
        }
    }
}

void FormulaSource::
polarizedSourceE(CalculationPartition & cp, int timestep)
{
    LOG << "Doing nothing.\n";
}

void FormulaSource::
polarizedSourceH(CalculationPartition & cp, int timestep)
{
    LOG << "Doing nothing.\n";
}

