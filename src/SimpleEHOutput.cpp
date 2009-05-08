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
#include <cstdlib>
#include <iostream>

using namespace std;

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
    string outputClass(mDesc->getClass());
    
    LOG << "Making output of class " << outputClass << endl;
    
    if (outputClass == "OneFieldOutput")
        return SimpleEHOutput::makeOneFieldOutput(cp, *mDesc);
    else if (outputClass == "ThreeFieldOutput")
        return SimpleEHOutput::makeThreeFieldOutput(cp, *mDesc);
    else if (outputClass == "ColocatedOutput")
        return SimpleEHOutput::makeColocatedOutput(cp, *mDesc);
    
    LOG << "Using default (null) output... I mean, crashing immediately.\n";
    exit(1);
    
    return OutputPtr(0L);
}

#pragma mark *** Output ***

SimpleEHOutput::
SimpleEHOutput() :
    mIsInterpolated(0),
    mCurrentSampleInterval(0),
    mWhichE(0,0,0),
    mWhichH(0,0,0)
{
}

SimpleEHOutput::
SimpleEHOutput(const OutputDescription & desc) :
    mIsInterpolated(0),
    mInterpolationPoint(0,0,0),
    mCurrentSampleInterval(0),
    mWhichE(0,0,0),
    mWhichH(0,0,0),
    mFirstSamples(desc.getFirstSamples()),
    mLastSamples(desc.getLastSamples()),
    mSamplePeriods(desc.getSamplePeriods()),
    mYeeRects(desc.getYeeRects()),
    mStrides(desc.getStrides())
{
}

OutputPtr SimpleEHOutput::
makeOneFieldOutput(const CalculationPartition & cp,
    const OutputDescription & desc)
{
    SimpleEHOutput* out = new SimpleEHOutput(desc);
        
    return OutputPtr(out);
}

OutputPtr SimpleEHOutput::
makeThreeFieldOutput(const CalculationPartition & cp,
const OutputDescription & desc)
{
    SimpleEHOutput* out = new SimpleEHOutput(desc);
        
    return OutputPtr(out);
}

OutputPtr SimpleEHOutput::
makeColocatedOutput(const CalculationPartition & cp,
    const OutputDescription & desc)
{
    SimpleEHOutput* out = new SimpleEHOutput(desc);
    out->mIsInterpolated = 1;
    out->mInterpolationPoint = Vector3f(0.5, 0.5, 0.5);
        
    return OutputPtr(out);
}
