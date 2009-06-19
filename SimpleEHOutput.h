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
#include "MemoryUtilities.h"
#include <vector>
#include <fstream>

class MaterialDescription;

class SimpleEHSetupOutput;
typedef Pointer<SimpleEHSetupOutput> SimpleEHSetupOutputPtr;

class SimpleEHOutput;
typedef Pointer<SimpleEHOutput> SimpleEHOutputPtr;

class SimpleEHSetupOutput : public SetupOutput
{
public:
    SimpleEHSetupOutput(const OutputDescPtr & desc);
    
    virtual OutputPtr makeOutput(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    OutputDescPtr mDesc;
};

class SimpleEHOutput : public Output
{
public:
    SimpleEHOutput(const OutputDescription & desc,
        const VoxelizedPartition & vp,
        const CalculationPartition & cp);
    virtual ~SimpleEHOutput();
    
    virtual void outputEPhase(const CalculationPartition & cp, int timestep);
    virtual void outputHPhase(const CalculationPartition & cp, int timestep);
    
private:
    void writeE(const CalculationPartition & cp);
    void writeH(const CalculationPartition & cp);
    
    void writeDescriptionFile(const VoxelizedPartition & vp,
        const CalculationPartition & cp,
        std::string specfile, std::string datafile, std::string materialfile)
        const;
    
    int mCoordPermutation;
    std::ofstream mDatafile;
    long mCurrentSampleInterval;
    
    bool mIsInterpolated;
    Vector3f mInterpolationPoint;
    Vector3i mWhichE;
    Vector3i mWhichH;
    
    Vector3i mAllocYeeOrigin;
    Vector3i mAllocYeeCells;
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
};


#endif
