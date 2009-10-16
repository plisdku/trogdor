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
#include "IODescriptionFile.h"
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
    SetupOutput(desc)
{
}

OutputPtr SimpleEHSetupOutput::
makeOutput(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    return OutputPtr(new SimpleEHOutput(description(), vp, cp));
}

#pragma mark *** Output ***

SimpleEHOutput::
SimpleEHOutput(OutputDescPtr description,
    const VoxelizedPartition & vp,
    const CalculationPartition & cp) :
    Output(description),
    mDatafile(),
    mCurrentSampleInterval(0),
    mDurations(description->durations())
{
    // Clip the regions to the current partition bounds (calc bounds, not
    //     allocation bounds!)
//    LOG << "Clipping output regions to partition bounds.  This is not in the"
//        " right place; it should be performed earlier somehow.\n";
    assert(description->regions().size() > 0);
    for (int rr = 0; rr < description->regions().size(); rr++)
    {
        Rect3i outRect(clip(description->regions()[rr].yeeCells(),
            vp.gridYeeCells()));        
        mRegions.push_back(Region(outRect, description->regions()[rr].stride()));
    }
//    LOG << "Truncating durations to simulation duration.  This is in the "
//        "wrong place; can't it be done earlier?\n";
    int numTimesteps = cp.duration();
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].last() > (numTimesteps-1))
        mDurations[dd].setLast(numTimesteps-1);
    
    string specfile(description->file() + string(".txt"));
    string datafile(description->file());
    string materialfile(description->file() + string(".mat"));
    
    IODescriptionFile::write(specfile, description, vp, mRegions, mDurations);
    //writeDescriptionFile(vp, cp, specfile, datafile, materialfile);
    
    mDatafile.open(datafile.c_str());
}

SimpleEHOutput::
~SimpleEHOutput()
{
    mDatafile.close();
}


void SimpleEHOutput::
outputEPhase(const CalculationPartition & cp, long timestep)
{
    if (norm2(description()->whichE()) == 0)
        return;
    if (mCurrentSampleInterval >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentSampleInterval].last())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDurations.size())
            return;
    }
    
    int firstT = mDurations[mCurrentSampleInterval].first();
    int period = mDurations[mCurrentSampleInterval].period();
    
    if (timestep >= firstT && (timestep - firstT)%period == 0)
        writeE(cp);
}

void SimpleEHOutput::
outputHPhase(const CalculationPartition & cp, long timestep)
{
    if (norm2(description()->whichH()) == 0)
        return;
    if (mCurrentSampleInterval >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentSampleInterval].last())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDurations.size())
            return;
    }
    
    int firstT = mDurations[mCurrentSampleInterval].first();
    int period = mDurations[mCurrentSampleInterval].period();
    
    if (timestep >= firstT && (timestep - firstT)%period == 0)
        writeH(cp);
}

void SimpleEHOutput::
writeE(const CalculationPartition & cp)
{   
    const InterleavedLattice & lattice(cp.lattice());
    
    Vector3f interpPoint = description()->interpolationPoint();
    
    for (int outDir = 0; outDir < 3; outDir++)
    {   
        for (unsigned int rr = 0; rr < mRegions.size(); rr++)
        {
            // The regions have been counter-rotated
            Rect3i outRect = mRegions[rr].yeeCells();
            Vector3i outStride = mRegions[rr].stride();
            Vector3i p;
            
            if (description()->whichE()[outDir] != 0)
            {
                if (!description()->isInterpolated())
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
                            outDir, Vector3f(p)+interpPoint);
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
    const InterleavedLattice& lattice(cp.lattice());
    
    Vector3f interpPoint = description()->interpolationPoint();
    
    for (int outDir = 0; outDir < 3; outDir++)
    {
        for (unsigned int rr = 0; rr < mRegions.size(); rr++)
        {
            Rect3i outRect = mRegions[rr].yeeCells();
            Vector3i outStride = mRegions[rr].stride();
            Vector3i p;
            
            if (description()->whichH()[outDir] != 0)
            {
                if (!description()->isInterpolated())
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
                            outDir, Vector3f(p)+interpPoint);
                        mDatafile.write((char*)(&val),
                            (std::streamsize)sizeof(float));
                    }
                }
            }
        }
    }
    mDatafile << flush;
}

/*
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
    descFile << "dxyz " << cp.dxyz() << "\n";
    descFile << "dt " << cp.dt() << "\n";
    
    if (!description()->isInterpolated())
    {
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (description()->whichE()[nn])
        {
            descFile << "field e" << char('x' + nn) << " "
                << eFieldPosition(nn) << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (description()->whichH()[nn])
        {
            descFile << "field h" << char('x' + nn) << " "
                << hFieldPosition(nn) << " 0.5\n";
        }
    }
    else
    {
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (description()->whichE()[nn])
        {
            descFile << "field e" << char('x' + nn) << " "
                << description()->interpolationPoint() << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (description()->whichH()[nn])
        {
            descFile << "field h" << char('x' + nn) << " "
                << description()->interpolationPoint() << " 0.5 \n";
        }
    }
    
    descFile << "unitVector0 " << Vector3i(1,0,0) << "\n"
        << "unitVector1 " << Vector3i(0,1,0) << "\n"
        << "unitVector2 " << Vector3i(0,0,1) << "\n";
    
    for (nn = 0; nn < mRegions.size(); nn++)
    {
        descFile << "region "
            << mRegions[nn].yeeCells()-vp.originYee()
            << " stride "
            << mRegions[nn].stride()
            << "\n";
    }
    
    for (nn = 0; nn < mDurations.size(); nn++)
    {
        descFile << "duration from "
            << mDurations[nn].first() << " to "
            << mDurations[nn].last() << " period "
            << mDurations[nn].period() << "\n";
    }
    
    descFile.close();
}
*/

