/*
 *  SourceEH.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "SourceEH.h"

#include "VoxelizedPartition.h"
#include "CalculationPartition.h"

SetupSourceEH::
SetupSourceEH(const SourceDescPtr sourceDescription) :
    SetupSource(),
    mDescription(sourceDescription)
{
}

SetupSourceEH::
~SetupSourceEH()
{
}

SourcePtr SetupSourceEH::
makeSource(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    return SourcePtr(new SourceEH(mDescription, vp, cp));
}


SourceEH::
SourceEH(const SourceDescPtr sourceDescription,
    const VoxelizedPartition & vp,
    const CalculationPartition & cp) :
    Source(),
    mFieldInput(sourceDescription, cp.dt()),
    mCurrentDuration(0),
    mFields(sourceDescription->sourceFields()),
    mIsSoft(sourceDescription->isSoftSource()),
    mRegions(sourceDescription->regions()),
    mDurations(sourceDescription->durations())
{
    for (int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect(clip(vp.gridYeeCells(), mRegions[rr].yeeCells()));
        mRegions[rr].setYeeCells(rect);
    }
    
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].last() > (cp.duration()-1))
        mDurations[dd].setLast(cp.duration()-1);
}

SourceEH::
~SourceEH()
{
}


void SourceEH::
sourceEPhase(CalculationPartition & cp, int timestep)
{
    if (norm2(mFields.whichE()) == 0)
        return;
    
    if (mCurrentDuration >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentDuration].last())
    {
        mCurrentDuration++;
        if (mCurrentDuration >= mDurations.size())
            return;
    }
    
    if (timestep >= mDurations[mCurrentDuration].first())
    {
        doSourceE(cp, timestep);
    }
}

void SourceEH::
sourceHPhase(CalculationPartition & cp, int timestep)
{
    if (norm2(mFields.whichH()) == 0)
        return;
    
    if (mCurrentDuration >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentDuration].last())
    {
        mCurrentDuration++;
        if (mCurrentDuration >= mDurations.size())
            return;
    }
    
    if (timestep >= mDurations[mCurrentDuration].first())
    {
        doSourceH(cp, timestep);
    }
}


void SourceEH::
doSourceE(CalculationPartition & cp, int timestep)
{
    float val;
    InterleavedLattice& lattice(cp.lattice());
    
    mFieldInput.startHalfTimestep(timestep);
    
    LOG << "Sourcing " << mFieldInput.getField(0) << "\n";
    
    for (int dir = 0; dir < 3; dir++)
    {
        mFieldInput.stepToNextFieldDirection(dir);
        if (mFields.whichE()[dir] != 0)
        for (unsigned int rr = 0; rr < mRegions.size(); rr++)
        {
            Rect3i rect = mRegions[rr].yeeCells();
            {
                Vector3i xx;
                if (!mIsSoft)
                {
                    for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                    for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                    for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    {
                        val = mFieldInput.getField(dir);
                        lattice.setE(dir, xx, val);
                        mFieldInput.stepToNextValue();
                    }
                }
                else
                {
                    for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                    for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                    for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    {
                        val = mFieldInput.getField(dir);
                        lattice.setE(dir, xx, lattice.getE(dir, xx) + val);
                        mFieldInput.stepToNextValue();
                    }
                }
            }
        }
    }
}

void SourceEH::
doSourceH(CalculationPartition & cp, int timestep)
{
    float val;
    InterleavedLattice& lattice(cp.lattice());
    
    mFieldInput.startHalfTimestep(timestep);
    
    for (int dir = 0; dir < 3; dir++)
    {
        mFieldInput.stepToNextFieldDirection(dir);
        if (mFields.whichH()[dir] != 0)
        for (unsigned int rr = 0; rr < mRegions.size(); rr++)
        {
            Rect3i rect = mRegions[rr].yeeCells();
            {
                Vector3i xx;
                if (!mIsSoft)
                {
                    for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                    for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                    for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    {
                        val = mFieldInput.getField(dir);
                        LOG << "val " << val << " at " << xx << " dir "
                            << dir << "\n";
                        lattice.setH(dir, xx, val);
                        mFieldInput.stepToNextValue();
                    }
                }
                else
                {
                    for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                    for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                    for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    {
                        val = mFieldInput.getField(dir);
                        LOG << "val " << val << " at " << xx << " dir "
                            << dir << "\n";
                        lattice.setH(dir, xx, lattice.getH(dir, xx) + val);
                        mFieldInput.stepToNextValue();
                    }
                }
            }
        }
    }
}

