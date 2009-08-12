/*
 *  SimpleEHOutput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/7/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "SimpleEHOutput.h"
#include "SimulationDescription.h"
#include "CalculationPartition.h"
#include "VoxelizedPartition.h"
#include "YeeUtilities.h"
#include "InterleavedLattice.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Version.h"

using namespace std;
using namespace boost::posix_time;
using namespace YeeUtilities;

#pragma mark *** Delegate ***

SimpleEHSetupOutput::
SimpleEHSetupOutput(const OutputDescPtr & desc) :
    SetupOutput(),
    mDesc(desc)
{
}

OutputPtr SimpleEHSetupOutput::
makeOutput(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    return OutputPtr(new SimpleEHOutput(*mDesc, vp, cp));
}

#pragma mark *** Output ***

SimpleEHOutput::
SimpleEHOutput(const OutputDescription & desc,
    const VoxelizedPartition & vp,
    const CalculationPartition & cp) :
    mDatafile(),
    mCurrentSampleInterval(0),
    mIsInterpolated(desc.isInterpolated()),
    mInterpolationPoint(desc.getInterpolationPoint()),
    mWhichE(desc.getWhichE()),
    mWhichH(desc.getWhichH()),
    mDurations(desc.getDurations())
{
    // Clip the regions to the current partition bounds (calc bounds, not
    //     allocation bounds!)
    LOG << "Clipping output regions to partition bounds.  This is not in the"
        " right place; it should be performed earlier somehow.\n";
    assert(desc.getRegions().size() > 0);
    for (int rr = 0; rr < desc.getRegions().size(); rr++)
    {
        Rect3i outRect(clip(desc.getRegions()[rr].getYeeCells(),
            vp.getGridYeeCells()));        
        mRegions.push_back(Region(outRect, desc.getRegions()[rr].getStride()));
    }
    LOG << "Truncating durations to simulation duration.  This is in the "
        "wrong place; can't it be done earlier?\n";
    int numTimesteps = cp.getDuration();
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].getLast() > (numTimesteps-1))
        mDurations[dd].setLast(numTimesteps-1);
    
    string specfile(desc.getFile() + string(".txt"));
    string datafile(desc.getFile());
    string materialfile(desc.getFile() + string(".mat"));
    
    writeDescriptionFile(vp, cp, specfile, datafile, materialfile);
    
    mDatafile.open(datafile.c_str());
}

SimpleEHOutput::
~SimpleEHOutput()
{
    mDatafile.close();
}


void SimpleEHOutput::
outputEPhase(const CalculationPartition & cp, int timestep)
{
    if (norm2(mWhichE) == 0)
        return;
    if (mCurrentSampleInterval >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentSampleInterval].getLast())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDurations.size())
            return;
    }
    
    int firstT = mDurations[mCurrentSampleInterval].getFirst();
    int period = mDurations[mCurrentSampleInterval].getPeriod();
    
    if (timestep >= firstT && (timestep - firstT)%period == 0)
        writeE(cp);
}

void SimpleEHOutput::
outputHPhase(const CalculationPartition & cp, int timestep)
{
    if (norm2(mWhichH) == 0)
        return;
    if (mCurrentSampleInterval >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentSampleInterval].getLast())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDurations.size())
            return;
    }
    
    int firstT = mDurations[mCurrentSampleInterval].getFirst();
    int period = mDurations[mCurrentSampleInterval].getPeriod();
    
    if (timestep >= firstT && (timestep - firstT)%period == 0)
        writeH(cp);
}

void SimpleEHOutput::
writeE(const CalculationPartition & cp)
{   
    const InterleavedLattice & lattice(cp.getLattice());
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        for (int outDir = 0; outDir < 3; outDir++)
        {   
            // The regions have been counter-rotated
            Rect3i outRect = mRegions[rr].getYeeCells();
            Vector3i outStride = mRegions[rr].getStride();
            Vector3i p;
            
            if (mWhichE[outDir] != 0)
            {
                if (!mIsInterpolated)
                {
                    for (p[2] = outRect.p1[2]; p[2] <= outRect.p2[2];
                        p[2] += outStride[2])
                    for (p[1] = outRect.p1[1]; p[1] <= outRect.p2[1];
                        p[1] += outStride[1])
                    for (p[0] = outRect.p1[0]; p[0] <= outRect.p2[0];
                        p[0] += outStride[0])
                    {
                        float val = lattice.getE(outDir, p);
                        mDatafile.write((char*)(&val),
                            (std::streamsize)sizeof(float));
                    }
                }
                else
                {
                    for (p[2] = outRect.p1[2]; p[2] <= outRect.p2[2];
                        p[2] += outStride[2])
                    for (p[1] = outRect.p1[1]; p[1] <= outRect.p2[1];
                        p[1] += outStride[1])
                    for (p[0] = outRect.p1[0]; p[0] <= outRect.p2[0];
                        p[0] += outStride[0])
                    {
                        float val = lattice.getInterpolatedE(
                            outDir, Vector3f(p)+mInterpolationPoint);
                        mDatafile.write((char*)(&val),
                            (std::streamsize)sizeof(float));
                    }
                }
            }
        }
    }
    mDatafile << flush;
}

void SimpleEHOutput::
writeH(const CalculationPartition & cp)
{
    const InterleavedLattice& lattice(cp.getLattice());
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        for (int outDir = 0; outDir < 3; outDir++)
        {
            Rect3i outRect = mRegions[rr].getYeeCells();
            Vector3i outStride = mRegions[rr].getStride();
            Vector3i p;
            
            if (mWhichH[outDir] != 0)
            {
                if (!mIsInterpolated)
                {
                    for (p[2] = outRect.p1[2]; p[2] <= outRect.p2[2];
                        p[2] += outStride[2])
                    for (p[1] = outRect.p1[1]; p[1] <= outRect.p2[1];
                        p[1] += outStride[1])
                    for (p[0] = outRect.p1[0]; p[0] <= outRect.p2[0];
                        p[0] += outStride[0])
                    {
                        float val = lattice.getH(outDir, p);
                        mDatafile.write((char*)(&val),
                            (std::streamsize)sizeof(float));
                    }
                }
                else
                {
                    for (p[2] = outRect.p1[2]; p[2] <= outRect.p2[2];
                        p[2] += outStride[2])
                    for (p[1] = outRect.p1[1]; p[1] <= outRect.p2[1];
                        p[1] += outStride[1])
                    for (p[0] = outRect.p1[0]; p[0] <= outRect.p2[0];
                        p[0] += outStride[0])
                    {
                        float val = lattice.getInterpolatedH(
                            outDir, Vector3f(p)+mInterpolationPoint);
                        mDatafile.write((char*)(&val),
                            (std::streamsize)sizeof(float));
                    }
                }
            }
        }
    }
    mDatafile << flush;
}

void SimpleEHOutput::
writeDescriptionFile(const VoxelizedPartition & vp,
    const CalculationPartition & cp, string specfile,
    string datafile, string materialfile) const
{
    int nn;
        
    ofstream descFile(specfile.c_str());
    
    //date today = day_clock::local_day();
    //date todayUTC = day_clock::universal_day();
    ptime now(second_clock::local_time());
    
    descFile << "trogdor5output\n";
    descFile << "trogdorMajorVersion " << TROGDOR_MAJOR_VERSION << "\n";
    descFile << "trogdorSVNVersion NOTUSED\n";
    descFile << "trogdorBuildDate " << __DATE__ << "\n";
    descFile << "specfile " << specfile << "\n";
    descFile << "datafile " << datafile << "\n";
    //descFile << "date " << to_iso_extended_string(today) << " "
    descFile << "date " << to_iso_extended_string(now) << "\n";
    descFile << "dxyz " << cp.getDxyz() << "\n";
    descFile << "dt " << cp.getDt() << "\n";
    
    if (!mIsInterpolated)
    {
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (mWhichE[nn])
        {
            descFile << "field e" << char('x' + nn) << " "
                << eFieldPosition(nn) << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (mWhichH[nn])
        {
            descFile << "field h" << char('x' + nn) << " "
                << hFieldPosition(nn) << " 0.5\n";
        }
    }
    else
    {
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (mWhichE[nn])
        {
            descFile << "field e" << char('x' + nn) << " "
                << mInterpolationPoint << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (mWhichH[nn])
        {
            descFile << "field h" << char('x' + nn) << " "
                << mInterpolationPoint << " 0.5 \n";
        }
    }
    
    descFile << "unitVector0 " << Vector3i(1,0,0) << "\n"
        << "unitVector1 " << Vector3i(0,1,0) << "\n"
        << "unitVector2 " << Vector3i(0,0,1) << "\n";
    
    for (nn = 0; nn < mRegions.size(); nn++)
    {
        descFile << "region "
            << mRegions[nn].getYeeCells()-vp.getOriginYee()
            << " stride "
            << mRegions[nn].getStride()
            << "\n";
    }
    
    for (nn = 0; nn < mDurations.size(); nn++)
    {
        descFile << "duration from "
            << mDurations[nn].getFirst() << " to "
            << mDurations[nn].getLast() << " period "
            << mDurations[nn].getPeriod() << "\n";
    }
    
    descFile.close();
}

