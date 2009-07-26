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
    mCoordPermutation(desc.getPermutation()),
    mDatafile(),
    mCurrentSampleInterval(0),
    mIsInterpolated(desc.isInterpolated()),
    mInterpolationPoint(desc.getInterpolationPoint()),
    mWhichE(desc.getWhichE()),
    mWhichH(desc.getWhichH()),
    mAllocYeeOrigin(vp.getAllocYeeCells().p1),
    mAllocYeeCells(vp.getAllocYeeCells().size()+1),
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
        outRect.p1 = cyclicPermute(outRect.p1, 3-mCoordPermutation);
        outRect.p2 = cyclicPermute(outRect.p2, 3-mCoordPermutation);
        
        Vector3i outStride(cyclicPermute(desc.getRegions()[rr].getStride(),
            3-mCoordPermutation));
        
        mRegions.push_back(Region(outRect, outStride));
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
    // Variables prefixed by "out" are as seen by the output file and user.
    // Variables prefixed by "in" are the corresponding permuted ones, to deal
    // with "invisible" grid rotations.
    
    const InterleavedLattice & lattice(cp.getLattice());
    
    int in0 = (3-mCoordPermutation)%3;  // direction corresponding to outside x
    int in1 = (4-mCoordPermutation)%3;  // ... outside y
    int in2 = (5-mCoordPermutation)%3;  // ... outside z
    
    // If there is no grid rotation, in0 = 0, in1 = 1, in2 = 2 because the
    // inside and outside xyz axes point the same directions...
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        for (int outDir = 0; outDir < 3; outDir++)
        {
            int inDir = (outDir + 3 - mCoordPermutation)%3;
            
            Rect3i rect = mRegions[rr].getYeeCells();
            Vector3i stride = mRegions[rr].getStride();
            Vector3i p;
            
            if (mWhichE[inDir] != 0)
            {
                if (!mIsInterpolated)
                {
                    for (p[in2] = rect.p1[in2]; p[in2] <= rect.p2[in2];
                        p[in2] += stride[in2])
                    for (p[in1] = rect.p1[in1]; p[in1] <= rect.p2[in1];
                        p[in1] += stride[in1])
                    for (p[in0] = rect.p1[in0]; p[in0] <= rect.p2[in0];
                        p[in0] += stride[in0])
                    {
                        //float val = cp.getE(inDir, p);
                        float val = lattice.getE(inDir, p);
                        mDatafile.write((char*)(&val),
                            (std::streamsize)sizeof(float));
                    }
                }
                else
                {
                    for (p[in2] = rect.p1[in2]; p[in2] <= rect.p2[in2];
                        p[in2] += stride[in2])
                    for (p[in1] = rect.p1[in1]; p[in1] <= rect.p2[in1];
                        p[in1] += stride[in1])
                    for (p[in0] = rect.p1[in0]; p[in0] <= rect.p2[in0];
                        p[in0] += stride[in0])
                    {
                        float val = lattice.getInterpolatedE(
                            inDir, Vector3f(p)+mInterpolationPoint);
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
    // Variables prefixed by "out" are as seen by the output file and user.
    // Variables prefixed by "in" are the corresponding permuted ones, to deal
    // with "invisible" grid rotations.
    
    const InterleavedLattice& lattice(cp.getLattice());
    
    int in0 = (3-mCoordPermutation)%3;  // direction corresponding to outside x
    int in1 = (4-mCoordPermutation)%3;  // ... outside y
    int in2 = (5-mCoordPermutation)%3;  // ... outside z
    
    // If there is no grid rotation, in0 = 0, in1 = 1, in2 = 2 because the
    // inside and outside xyz axes point the same directions...
    
    for (unsigned int rr = 0; rr < mRegions.size(); rr++)
    {
        for (int outDir = 0; outDir < 3; outDir++)
        {
            int inDir = (outDir + 3 - mCoordPermutation)%3;
            
            Rect3i rect = mRegions[rr].getYeeCells();
            Vector3i stride = mRegions[rr].getStride();
            Vector3i p;
            
            if (mWhichH[inDir] != 0)
            {
                if (!mIsInterpolated)
                {
                    for (p[in2] = rect.p1[in2]; p[in2] <= rect.p2[in2];
                        p[in2] += stride[in2])
                    for (p[in1] = rect.p1[in1]; p[in1] <= rect.p2[in1];
                        p[in1] += stride[in1])
                    for (p[in0] = rect.p1[in0]; p[in0] <= rect.p2[in0];
                        p[in0] += stride[in0])
                    {
                        float val = lattice.getH(inDir, p);
                        mDatafile.write((char*)(&val),
                            (std::streamsize)sizeof(float));
                    }
                }
                else
                {
                    for (p[in2] = rect.p1[in2]; p[in2] <= rect.p2[in2];
                        p[in2] += stride[in2])
                    for (p[in1] = rect.p1[in1]; p[in1] <= rect.p2[in1];
                        p[in1] += stride[in1])
                    for (p[in0] = rect.p1[in0]; p[in0] <= rect.p2[in0];
                        p[in0] += stride[in0])
                    {
                        float val = lattice.getInterpolatedH(
                            inDir, Vector3f(p)+mInterpolationPoint);
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
    int nn, dir;
    int unpermute = 3-mCoordPermutation;
        
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
            dir = (nn+3-mCoordPermutation)%3;  // undo permutation
            descFile << "field e" << char('x' + dir) << " "
                << eFieldPosition(dir) << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (mWhichH[nn])
        {
            dir = (nn+3-mCoordPermutation)%3;  // undo permutation
            descFile << "field h" << char('x' + dir) << " "
                << hFieldPosition(dir) << " 0.5\n";
        }
    }
    else
    {
        Vector3f interp(cyclicPermute(mInterpolationPoint, unpermute));
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (mWhichE[nn])
        {
            dir = (nn+3-mCoordPermutation)%3;  // undo permutation
            descFile << "field e" << char('x' + dir) << " " << interp
                << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (mWhichH[nn])
        {
            dir = (nn+3-mCoordPermutation)%3;  // undo permutation
            descFile << "field h" << char('x' + dir) << " " << interp
                << " 0.5 \n";
        }
    }
    
    for (nn = 0; nn < mRegions.size(); nn++)
    {
        descFile << "region "
            << cyclicPermute(mRegions[nn].getYeeCells()-vp.getOriginYee(),
                unpermute)
            << " stride "
            << cyclicPermute(mRegions[nn].getStride(), unpermute)
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

