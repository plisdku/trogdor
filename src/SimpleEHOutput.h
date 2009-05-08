/*
 *  SimpleEHOutput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/7/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _SIMPLEEHOUTPUT_
#define _SIMPLEEHOUTPUT_

#include "SimulationDescriptionPredeclarations.h"
#include "OutputBoss.h"
#include "geometry.h"
#include <vector>

class MaterialDescription;

class SimpleEHOutputDelegate;
typedef Pointer<SimpleEHOutputDelegate> SimpleEHOutputDelegatePtr;

class SimpleEHOutput;
typedef Pointer<SimpleEHOutput> SimpleEHOutputPtr;

class SimpleEHOutputDelegate : public OutputDelegate
{
public:
    SimpleEHOutputDelegate(const OutputDescPtr & desc);
    
    virtual OutputPtr makeOutput(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    OutputDescPtr mDesc;
};


class SimpleEHOutput : public Output
{
public:
    SimpleEHOutput();
private:
    SimpleEHOutput(const OutputDescription & desc);
    
public:
    static OutputPtr makeOneFieldOutput(const CalculationPartition & cp,
        const OutputDescription & desc);
    static OutputPtr makeThreeFieldOutput(const CalculationPartition & cp,
        const OutputDescription & desc);
    static OutputPtr makeColocatedOutput(const CalculationPartition & cp,
        const OutputDescription & desc);
    
private:
    bool mIsInterpolated;
    Vector3f mInterpolationPoint;
    
    long mCurrentSampleInterval;
    
    Vector3b mWhichE;
    Vector3b mWhichH;
    
    std::vector<long> mFirstSamples;
    std::vector<long> mLastSamples;
    std::vector<long> mSamplePeriods;
    std::vector<Rect3i> mYeeRects;
    std::vector<Vector3i> mStrides;
};


#endif
