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

#include "SimulationDescription.h"
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
    //SimpleEHOutput();
    SimpleEHOutput(const OutputDescription & desc, Vector3i origin,
        Vector3f dxyz, float dt);
    
    virtual void outputEPhase(int timestep);
    virtual void outputHPhase(int timestep);
    
private:
    void writeDescriptionFile() const;
    
    
    bool mIsInterpolated;
    Vector3f mInterpolationPoint;
    
    long mCurrentSampleInterval;
    
    Vector3i mWhichE;
    Vector3i mWhichH;
    
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
};


#endif
