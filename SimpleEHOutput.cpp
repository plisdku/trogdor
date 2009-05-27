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
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "Version.h"

using namespace std;
using namespace boost::gregorian;
using namespace YeeUtilities;

#pragma mark *** Delegate ***

SimpleEHOutputDelegate::
SimpleEHOutputDelegate(const OutputDescPtr & desc) :
    OutputDelegate(),
    mDesc(desc)
{
}

OutputPtr SimpleEHOutputDelegate::
makeOutput(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    //string outputClass(mDesc->getClass());
    
    if (norm2(mDesc->getWhichJ()) == 0 &&
        norm2(mDesc->getWhichP()) == 0 &&
        norm2(mDesc->getWhichM()) == 0)
    {
        return OutputPtr(new SimpleEHOutput(*mDesc, vp.getOriginYee(),
            cp.getDxyz(), cp.getDt()));
    }
    else
    {
        cerr << "Warning: can't handle outputs for other than E or H.\n";
        exit(1);
    }
    
    return OutputPtr(0L);
}

#pragma mark *** Output ***

/*
SimpleEHOutput::
SimpleEHOutput() :
    mIsInterpolated(0),
    mWhichE(0,0,0),
    mWhichH(0,0,0),
    mRegions(1,Region()),
    mDurations(1,Duration())
{
}
*/

SimpleEHOutput::
SimpleEHOutput(const OutputDescription & desc, Vector3i origin, Vector3f dxyz,
    float dt) :
    mIsInterpolated(0),
    mInterpolationPoint(0,0,0),
    mCurrentSampleInterval(0),
    mWhichE(desc.getWhichE()),
    mWhichH(desc.getWhichH()),
    mRegions(desc.getRegions()),
    mDurations(desc.getDurations())
{
    writeDescriptionFile();
}


void SimpleEHOutput::
outputEPhase(int timestep)
{
    if (norm2(mWhichE) != 0)
        LOG << "Output E.\n";
}

void SimpleEHOutput::
outputHPhase(int timestep)
{
    if (norm2(mWhichH) != 0)
        LOG << "Output H.\n";
}


void SimpleEHOutput::
writeDescriptionFile() const
{
    /*
    int nn, dir;
    Mat3i permuteBackwardi;
    Mat3f permuteBackwardf;
    Mat3i unpermutei(1);
    Mat3f unpermutef(1);
    permuteBackwardi = 0,1,0,0,0,1,1,0,0;
    permuteBackwardf = 0,1,0,0,0,1,1,0,0;
    int permutation = desc.getPermutation();
    
    for (nn = 0; nn < permutation; nn++)
    {
        unpermutei = permuteBackwardi * unpermutei;
        unpermutef = permuteBackwardf * unpermutef;
    }
    
    // Write description file
    string specfile(desc.getFile() + string(".txt"));
    string datafile(desc.getFile());
    string materialfile(desc.getFile() + string(".mat"));
    
    ofstream descFile(specfile.c_str());
    //descFile.open();
    
    date today = day_clock::local_day();
    date todayUTC = day_clock::universal_day();
    descFile << "trogdor5output\n";
    descFile << "trogdorMajorVersion " << TROGDOR_MAJOR_VERSION << "\n";
    descFile << "trogdorSVNVersion NOTUSED\n";
    descFile << "trogdorBuildDate " << __DATE__ << "\n";
    descFile << "specfile " << specfile << "\n";
    descFile << "datafile " << datafile << "\n";
    descFile << "date " << to_iso_extended_string(today) << "\n";
    descFile << "dxyz " << dxyz << "\n";
    descFile << "dt " << dt << "\n";
    
    if (desc.isInterpolated())
    {
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (desc.getWhichE()[nn])
        {
            dir = (nn+3-permutation)%3;  // undo permutation
            descFile << "field e" << char('x' + dir) << " "
                << eFieldPosition(dir) << "0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (desc.getWhichH()[nn])
        {
            dir = (nn+3-permutation)%3;  // undo permutation
            descFile << "field h" << char('x' + dir) << " "
                << hFieldPosition(dir) << " 0.5\n";
        }
    }
    else
    {
        Vector3f interp(unpermutef * desc.getInterpolationPoint());
        // E fields
        for (nn = 0; nn < 3; nn++)
        if (desc.getWhichE()[nn])
        {
            dir = (nn+3-permutation)%3;  // undo permutation
            descFile << "field e" << char('x' + dir) << " " << interp
                << " 0.0 \n";
        }
        
        // H fields
        for (nn = 0; nn < 3; nn++)
        if (desc.getWhichH()[nn])
        {
            dir = (nn+3-permutation)%3;  // undo permutation
            descFile << "field h" << char('x' + dir) << " " << interp
                << " 0.5 \n";
        }
    }
    
    const vector<Region> & regions = desc.getRegions();
    const vector<Duration> & durations = desc.getDurations();
    
    for (nn = 0; nn < regions.size(); nn++)
    {
        descFile << "region " << unpermutei*(regions[nn].getYeeCells() - origin)
            << " stride " << unpermutei*regions[nn].getStride() << "\n";
    }
    
    for (nn = 0; nn < durations.size(); nn++)
    {
        descFile << "duration from " << durations[nn].getFirst() << " to "
            << durations[nn].getLast() << " period "
            << durations[nn].getPeriod() << "\n";
    }
    
    descFile.close();
    */
}

