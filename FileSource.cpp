/*
 *  FileSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/24/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "FileSource.h"

#include "SimulationDescription.h"
#include "CalculationPartition.h"
#include "VoxelizedPartition.h"
#include "InterleavedLattice.h"
#include <cstdlib>

using namespace std;

#pragma mark *** Setup ***

FileSetupSource::
FileSetupSource(const SourceDescPtr & desc) :
    SetupSource(),
    mDesc(desc)
{
}

SourcePtr FileSetupSource::
makeSource(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    return SourcePtr(new FileSource(*mDesc, vp, cp));
}


FileSource::
FileSource(const SourceDescription & desc, const VoxelizedPartition & vp,
    const CalculationPartition & cp) :
    Source(),
    mFileStream(),
    mCurrentDuration(0),
    mFields(desc.sourceFields()),
    mDt(cp.dt()),
    mIsSpaceVarying(desc.isSpaceVarying()),
    mIsSoft(desc.isSoftSource()),
    mRegions(desc.regions()),
    mDurations(desc.durations())
{
    if (desc.isSpaceVarying())
        throw(Exception("Space-varying file source not yet supported.\n"));
    
    mFileStream.open(desc.timeFile().c_str(), ios::binary);
    if (mFileStream.good())
        LOGF << "Opened binary file " << desc.timeFile() << ".\n";
    else
        throw(Exception(string("Could not open binary file")
            + desc.timeFile()));
    
    for (int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect(clip(vp.gridYeeCells(), mRegions[rr].yeeCells()));
        mRegions[rr].setYeeCells(rect);
    }
    
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].last() > (cp.duration()-1))
        mDurations[dd].setLast(cp.duration()-1);
}


void FileSource::
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
        if (mFields.usesPolarization())
            polarizedSourceE(cp, timestep);
        else
            uniformSourceE(cp, timestep);
    }
}

void FileSource::
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
        if (mFields.usesPolarization())
            polarizedSourceH(cp, timestep);
        else
            uniformSourceH(cp, timestep);
    }
}
/*
void FileSource::
newSourceE(CalculationPartition & cp, int timestep)
{
    float val;
    Vector3i x;
    
    // This is a chance for the input to perform a new calculation, init new
    // splines, or whatever.
    fieldInput.startTimestep(timestep);
    
    const int dir0 = fieldInput.runlineDirection();
    const int dir1 = (dir0+1)%3;
    const int dir2 = (dir1+1)%3;
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i yeeCells = mRegions[rr].yeeCells();
        
        if (fieldInput.order() == kPositionThenPolarization)
        {
            for (x[dir2] = rect.p1[dir2]; x[dir2] <= rect.p2[dir2]; x[dir2]++)
            for (x[dir1] = rect.p1[dir1]; x[dir1] <= rect.p2[dir1]; x[dir1]++)
            for (x[dir0] = rect.p1[dir0]; x[dir0] <= rect.p2[dir0]; x[dir0]++)
            {
                lattice.setE(0, xx, fieldInput.getField(0));
                lattice.setE(1, xx, fieldInput.getField(1));
                lattice.setE(2, xx, fieldInput.getField(2));
                fieldInput.stepToNext();
            }
        }
        else if (fieldInput.order() == kPolarizationThenPosition)
        {
            for (int fieldDir = 0; fieldDir < 3; fieldDir++)
            for (x[dir2] = rect.p1[dir2]; x[dir2] <= rect.p2[dir2]; x[dir2]++)
            for (x[dir1] = rect.p1[dir1]; x[dir1] <= rect.p2[dir1]; x[dir1]++)
            for (x[dir0] = rect.p1[dir0]; x[dir0] <= rect.p2[dir0]; x[dir0]++)
            {
                lattice.setE(fieldDir, xx, fieldInput.getField(fieldDir));
                fieldInput.stepToNext();
            }
        }
    }
}
*/

void FileSource::
uniformSourceE(CalculationPartition & cp, int timestep)
{
    //LOG << "Source E\n";
    float val;
    InterleavedLattice& lattice(cp.lattice());
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    else
        throw(Exception("Cannot read further from file."));
    
//    LOG << "Source E val " << val << "\n";
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].yeeCells();
        for (int dir = 0; dir < 3; dir++)
        if (mFields.whichE()[dir] != 0)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                {
                    lattice.setE(dir, xx, val);
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
                    lattice.setE(dir, xx, lattice.getE(dir, xx)+val);
            }
        }
    }
}

void FileSource::
uniformSourceH(CalculationPartition & cp, int timestep)
{
    //LOG << "Source H\n";
    float val;
    InterleavedLattice& lattice(cp.lattice());
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    else
        throw(Exception("Cannot read further from file."));
    
//    LOG << "Source H val " << val << "\n";
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].yeeCells();
        for (int dir = 0; dir < 3; dir++)
        if (mFields.whichH()[dir] != 0)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice.setH(dir, xx, val);
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice.setH(dir, xx, lattice.getH(dir, xx)+val);
            }
        }
    }
}

void FileSource::
polarizedSourceE(CalculationPartition & cp, int timestep)
{
    //assert(mIsSoft);
    
    ////LOG << "Source E\n";
    Vector3f polarization = mFields.polarization();
    float val;
    InterleavedLattice & lattice(cp.lattice());
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    else
        throw(Exception("Cannot read further from file."));
    
//    LOG << "Source E val " << val << "\n";
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].yeeCells();
        for (int dir = 0; dir < 3; dir++)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                {
                    lattice.setE(dir, xx, val*polarization[dir]);
                }
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                lattice.setE(dir, xx,
                    lattice.getE(dir, xx)+val*polarization[dir]);
            }
        }
    }
}

void FileSource::
polarizedSourceH(CalculationPartition & cp, int timestep)
{
    //assert(mIsSoft);
    
    //LOG << "Source H\n";
    Vector3f polarization = mFields.polarization();
    float val;
    InterleavedLattice & lattice(cp.lattice());
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    else
        throw(Exception("Cannot read further from file."));
    
//    LOG << "Source H val " << val << "\n";
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].yeeCells();
        for (int dir = 0; dir < 3; dir++)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice.setH(dir, xx, val*polarization[dir]);
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice.setH(dir, xx,
                        lattice.getH(dir, xx)+val*polarization[dir]);
            }
        }
    }
}

