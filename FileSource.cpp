/*
 *  FileSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/24/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "FileSource.h"

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
    mCurrentDuration(0),
    mFields(desc.getSourceFields()),
    mDt(cp.getDt()),
    mIsSpaceVarying(desc.isSpaceVarying()),
    mIsSoft(desc.isSoftSource()),
    mRegions(desc.getRegions()),
    mDurations(desc.getDurations())
{
    if (desc.isSpaceVarying())
        throw(Exception("Space-varying file source not yet supported.\n"));
    
    mFileStreamStream.open(desc.getTimeFile(), ios::binary);
    if (mFileStreamStream.good())
        LOGF << "Opened binary file " << desc.getTimeFile() << ".\n";
    else
        throw(Exception(string("Could not open binary file")
            + desc.getTimeFile()));
    
    for (int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect(clip(vp.getGridYeeCells(), mRegions[rr].getYeeCells()));
        mRegions[rr].setYeeCells(rect);
    }
    
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].getLast() > (cp.getDuration()-1))
        mDurations[dd].setLast(cp.getDuration()-1);
}


void FileSource::
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
    {
        if (mFields.usesPolarization())
            polarizedSourceH(cp, timestep);
        else
            uniformSourceH(cp, timestep);
    }
}

void FileSource::
uniformSourceE(CalculationPartition & cp, int timestep)
{
    //LOG << "Source E\n";
    float val;
    InterleavedLatticePtr lattice(cp.getLattice());
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    
//    LOG << "Source E val " << val << "\n";
    
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

void FileSource::
uniformSourceH(CalculationPartition & cp, int timestep)
{
    //LOG << "Source H\n";
    float val;
    InterleavedLatticePtr lattice(cp.getLattice());
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    
//    LOG << "Source H val " << val << "\n";
    
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

void FileSource::
polarizedSourceE(CalculationPartition & cp, int timestep)
{
    //assert(mIsSoft);
    
    ////LOG << "Source E\n";
    Vector3f polarization = mFields.getPolarization();
    float val;
    InterleavedLatticePtr lattice(cp.getLattice());
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    
//    LOG << "Source E val " << val << "\n";
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].getYeeCells();
        for (int dir = 0; dir < 3; dir++)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                {
                    lattice->setE(dir, xx, val*polarization[dir]);
                }
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                lattice->setE(dir, xx,
                    lattice->getE(dir, xx)+val*polarization[dir]);
            }
        }
    }
}

void FileSource::
polarizedSourceH(CalculationPartition & cp, int timestep)
{
    //assert(mIsSoft);
    
    //LOG << "Source H\n";
    Vector3f polarization = mFields.getPolarization();
    float val;
    InterleavedLatticePtr lattice(cp.getLattice());
    
	if (mFileStream.good())
		mFileStream.read((char*)&val, (std::streamsize)sizeof(float));
    
//    LOG << "Source H val " << val << "\n";
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].getYeeCells();
        for (int dir = 0; dir < 3; dir++)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setH(dir, xx, val*polarization[dir]);
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setH(dir, xx,
                        lattice->getH(dir, xx)+val*polarization[dir]);
            }
        }
    }
}

/*
void FormulaSource::
sourceE(Vector3f value)
{
    InterleavedLatticePtr lattice(cp.getLattice());
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].getYeeCells();
        for (int dir = 0; dir < 3; dir++)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setE(dir, xx, value);
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setE(dir, xx,
                        lattice->getE(dir, xx)+value);
            }
        }
    }
}

void FormulaSource::
sourceH(Vector3f value)
{
    InterleavedLatticePtr lattice(cp.getLattice());
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        Rect3i rect = mRegions[rr].getYeeCells();
        for (int dir = 0; dir < 3; dir++)
        {
            Vector3i xx;
            if (!mIsSoft)
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setH(dir, xx, value);
            }
            else
            {
                for (xx[2] = rect.p1[2]; xx[2] <= rect.p2[2]; xx[2]++)
                for (xx[1] = rect.p1[1]; xx[1] <= rect.p2[1]; xx[1]++)
                for (xx[0] = rect.p1[0]; xx[0] <= rect.p2[0]; xx[0]++)
                    lattice->setH(dir, xx,
                        lattice->getH(dir, xx)+value);
            }
        }
    }
}
*/