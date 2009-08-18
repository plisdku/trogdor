/*
 *  FileCurrentSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/17/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "FileCurrentSource.h"


#include "VoxelizedPartition.h"
#include "CalculationPartition.h"

#include <iostream>
using namespace std;

SetupFileCurrentSource::
SetupFileCurrentSource(const CurrentSourceDescPtr & description) :
    SetupCurrentSource(description)
{
    LOG << "Doing my thing, just doing my thing!\n";
}

Pointer<CurrentSource> SetupFileCurrentSource::
makeCurrentSource(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    LOG << "Making one!\n";
    
    return Pointer<CurrentSource>(new FileCurrentSource(description(),
        vp, cp));
}

FileCurrentSource::
FileCurrentSource(const CurrentSourceDescPtr & description,
    const VoxelizedPartition & vp, const CalculationPartition & cp) :
    mFormula(description->formula()),
    mCurrents(description->sourceCurrents()),
    mDt(cp.dt()),
    mDurations(description->durations()),
    mCurrentDuration(0)
{
    if (description->isSpaceVarying())
        throw(Exception("Space-varying file current source not yet"
            " supported.\n"));
        
    mFileStream.open(description->timeFile().c_str(), ios::binary);
    if (mFileStream.good())
        LOGF << "Opened binary file " << description->timeFile() << ".\n";
    else
        throw(Exception(string("Could not open binary file")
            + description->timeFile()));
    
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].last() > (cp.duration()-1))
        mDurations[dd].setLast(cp.duration()-1);
}


void FileCurrentSource::
prepareJ(long timestep)
{
    mJ = Vector3f(0.0f, 0.0f, 0.0f);
    if (norm2(mCurrents.whichJ()) == 0)
        return;
    
    if (mCurrentDuration >= mDurations.size())
    {
        return;
    }
    while (timestep > mDurations[mCurrentDuration].last())
    {
        mCurrentDuration++;
        if (mCurrentDuration >= mDurations.size())
            return;
    }
    
    if (timestep >= mDurations[mCurrentDuration].first())
    {
        updateJ(timestep);
    }
}

void FileCurrentSource::
prepareK(long timestep)
{
    mK = Vector3f(0.0f, 0.0f, 0.0f);
    if (norm2(mCurrents.whichK()) == 0)
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
        updateK(timestep);
    }
}

float FileCurrentSource::
getJ(int direction) const
{
    assert(direction >= 0 && direction < 3);
    return mJ[direction];
}

float FileCurrentSource::
getK(int direction) const
{
    assert(direction >= 0 && direction < 3);
    return mK[direction];
}


void FileCurrentSource::
updateJ(long timestep)
{
    float val;
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    else
        throw(Exception("Cannot read further from file."));
    
    LOG << "Value: " << val << "\n";
    
    if (mCurrents.usesPolarization())
        mJ = val*mCurrents.polarization();
    else
        mJ = Vector3f(val,val,val);
}

void FileCurrentSource::
updateK(long timestep)
{
    float val;
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    else
        throw(Exception("Cannot read further from file."));
    
    LOG << "Value: " << val << "\n";
    
    if (mCurrents.usesPolarization())
        mK = val*mCurrents.polarization();
    else
        mK = Vector3f(val,val,val);
}
