/*
 *  Source.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "Source.h"

#include "VoxelizedPartition.h"
#include "CalculationPartition.h"

SetupSource::
SetupSource(const SourceDescPtr sourceDescription) :
    mDescription(sourceDescription)
{
//    LOG << "HEY LOOK This does nothing, nothing at all, and can wait until"
//        " the CalculationPartition is constructed.\n";
}

SetupSource::
~SetupSource()
{
}

SourcePtr SetupSource::
makeSource(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    return SourcePtr(new Source(mDescription, vp, cp));
}


Source::
Source(const SourceDescPtr sourceDescription,
    const VoxelizedPartition & vp,
    const CalculationPartition & cp) :
    mDescription(sourceDescription),
    mFieldInput(sourceDescription),
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

Source::
~Source()
{
}


void Source::
sourceEPhase(CalculationPartition & cp, long timestep)
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

void Source::
sourceHPhase(CalculationPartition & cp, long timestep)
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


void Source::
doSourceE(CalculationPartition & cp, long timestep)
{
    float val;
    InterleavedLattice& lattice(cp.lattice());
    
    mFieldInput.startHalfTimestepE(timestep, cp.dt()*timestep);
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        mFieldInput.restartMaskPointer(xyz);
        
        if (mFields.whichE()[xyz] != 0)
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
                        val = mFieldInput.getFieldE(xyz);
                        lattice.setE(xyz, xx, val);
                    }
                }
                else
                {
                    for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                    for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                    for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    {
                        val = mFieldInput.getFieldE(xyz);
                        lattice.setE(xyz, xx, lattice.getE(xyz, xx) + val);
                    }
                }
            }
        }
    }
}

void Source::
doSourceH(CalculationPartition & cp, long timestep)
{
    float val;
    InterleavedLattice& lattice(cp.lattice());
    
    mFieldInput.startHalfTimestepH(timestep, cp.dt()*(timestep+0.5));
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        mFieldInput.restartMaskPointer(xyz);
        
        if (mFields.whichE()[xyz] != 0)
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
                        val = mFieldInput.getFieldH(xyz);
                        lattice.setH(xyz, xx, val);
                    }
                }
                else
                {
                    for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                    for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                    for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    {
                        val = mFieldInput.getFieldH(xyz);
                        lattice.setH(xyz, xx, lattice.getH(xyz, xx) + val);
                    }
                }
            }
        }
    }
}

