/*
 *  Performance.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/19/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "Performance.h"
#include "UpdateEquation.h"

// for accumulate()
#include <numeric>

using namespace std;

#pragma mark *** GlobalStatistics ***

GlobalStatistics::
GlobalStatistics() :
    mReadDescriptionMicroseconds(0.0),
    mVoxelizeMicroseconds(0.0),
    mSetupCalculationMicroseconds(0.0),
    mRunCalculationMicroseconds(0.0),
    mPrintTimestepMicroseconds(0.0),
    mTimestepStartTimes()
{
}

void GlobalStatistics::
setNumTimesteps(long numT)
{
    if (numT > mTimestepStartTimes.max_size())
        cerr << "Warning: storage of " << numT << " timestep start times will"
        " likely fail (std::vector::max_size = "
        << mTimestepStartTimes.max_size() << ")" << endl;
    mTimestepStartTimes.resize(numT);
}

void GlobalStatistics::
setReadDescriptionMicroseconds(double us)
{
    mReadDescriptionMicroseconds = us;
}

void GlobalStatistics::
setVoxelizeMicroseconds(double us)
{
    mVoxelizeMicroseconds = us;
}

void GlobalStatistics::
setSetupCalculationMicroseconds(double us)
{
    mSetupCalculationMicroseconds = us;
}

void GlobalStatistics::
setRunCalculationMicroseconds(double us)
{
    mRunCalculationMicroseconds = us;
}

void GlobalStatistics::
setPrintTimestepMicroseconds(double us)
{
    mPrintTimestepMicroseconds = us;
}

void GlobalStatistics::
setTimestepMicroseconds(long timestep, double us)
{
    mTimestepStartTimes[timestep] = us;
}

void GlobalStatistics::
printForMatlab(std::ostream & str)
{
    str << "trogdor.readDescriptionTime = "
        << mReadDescriptionMicroseconds*1e-6 << ";\n";
    str << "trogdor.voxelizeTime = " << mVoxelizeMicroseconds*1e-6 << ";\n";
    str << "trogdor.setupCalculationTime = " <<
        mSetupCalculationMicroseconds*1e-6 << ";\n";
    str << "trogdor.setupTime = " << (mReadDescriptionMicroseconds +
        mVoxelizeMicroseconds + mSetupCalculationMicroseconds)*1e-6 << ";\n";
        
    str << "% Post-simulation analysis\n"
        "% Runtime is the total after the setup is complete.\n"
        "% Work time is the time not including printing timesteps to "
            "standard output.\n"
        "% printTimestepTime is exactly what it sounds like.\n";
    str << "trogdor.runtime = " << mRunCalculationMicroseconds*1e-6 << ";\n";
    str << "trogdor.printTimestepTime = " << mPrintTimestepMicroseconds*1e-6
        << ";\n";
    str << "trogdor.workTime = " << (mRunCalculationMicroseconds -
        mPrintTimestepMicroseconds)*1e-6 << ";\n";
    
}

#pragma mark *** PartitionStatistics ***

PartitionStatistics::
PartitionStatistics()
{
}

void PartitionStatistics::
setNumMaterials(int num)
{
    mMaterialMicrosecondsE.resize(num, 0.0);
    mMaterialMicrosecondsH.resize(num, 0.0);
}
void PartitionStatistics::
setNumOutputs(int num)
{
    mOutputMicroseconds.resize(num, 0.0);
}
void PartitionStatistics::
setNumHardSources(int num)
{
    mHardSourceMicroseconds.resize(num, 0.0);
}
void PartitionStatistics::
setNumSoftSources(int num)
{
    mSoftSourceMicroseconds.resize(num, 0.0);
}
void PartitionStatistics::
setNumCurrentSources(int num)
{
    mCurrentSourceMicroseconds.resize(num, 0.0);
}
void PartitionStatistics::
setNumHuygensSurfaces(int num)
{
    mHuygensSurfaceMicroseconds.resize(num, 0.0);
}

void PartitionStatistics::
addMaterialMicrosecondsE(int material, double us)
{
    mMaterialMicrosecondsE.at(material) += us;
}
void PartitionStatistics::
addMaterialMicrosecondsH(int material, double us)
{
    mMaterialMicrosecondsH.at(material) += us;
}
void PartitionStatistics::
addOutputMicroseconds(int output, double us)
{
    mOutputMicroseconds[output] += us;
}
void PartitionStatistics::
addHardSourceMicroseconds(int source, double us)
{
    mHardSourceMicroseconds.at(source) += us;
}
void PartitionStatistics::
addSoftSourceMicroseconds(int source, double us)
{
    mSoftSourceMicroseconds.at(source) += us;
}
void PartitionStatistics::
addCurrentSourceMicroseconds(int source, double us)
{
    mCurrentSourceMicroseconds.at(source) += us;
}
void PartitionStatistics::
addHuygensSurfaceMicroseconds(int source, double us)
{
    mHuygensSurfaceMicroseconds.at(source) += us;
}


void PartitionStatistics::
printForMatlab(std::ostream & str, const string & prefix,
    const vector<UpdateEquationPtr> & materials, long numT)
{
    assert(materials.size() == mMaterialMicrosecondsE.size());
    
    double totalMatE_us = accumulate(mMaterialMicrosecondsE.begin(),
        mMaterialMicrosecondsE.end(), 0.0);
    double totalMatH_us = accumulate(mMaterialMicrosecondsH.begin(),
        mMaterialMicrosecondsH.end(), 0.0);
    double totalOut_us = accumulate(mOutputMicroseconds.begin(),
        mOutputMicroseconds.end(), 0.0);
    double totalHardSource_us = accumulate(mHardSourceMicroseconds.begin(),
        mHardSourceMicroseconds.end(), 0.0);
    double totalSoftSource_us = accumulate(mSoftSourceMicroseconds.begin(),
        mSoftSourceMicroseconds.end(), 0.0);
    double totalCurrentSource_us = accumulate(
        mCurrentSourceMicroseconds.begin(),
        mCurrentSourceMicroseconds.end(), 0.0);
    double totalHuygensSurface_us = accumulate(
        mHuygensSurfaceMicroseconds.begin(), mHuygensSurfaceMicroseconds.end(),
        0.0);
    
    str << prefix << "calcETime = " << totalMatE_us*1e-6 << ";\n";
    str << prefix << "calcHTime = " << totalMatH_us*1e-6 << ";\n";
    str << prefix << "calcTime = " << (totalMatE_us+totalMatH_us)*1e-6 << ";\n";
    str << prefix << "outputTime = " << totalOut_us*1e-6 << ";\n";
    str << prefix << "hardSourceTime = " << totalHardSource_us*1e-6 << ";\n";
    str << prefix << "softSourceTime = " << totalSoftSource_us*1e-6<< ";\n";
    str << prefix << "currentSourceTime = " << totalCurrentSource_us*1e-6
        << ";\n";
    str << prefix << "sourceTime = " <<
        (totalHardSource_us + totalSoftSource_us + totalCurrentSource_us)
        *1e-6 << ";\n";
    str << prefix << "huygensSurfaceTime = " << totalHuygensSurface_us*1e-6
        << ";\n";
    
    for (int nn = 0; nn < mMaterialMicrosecondsE.size(); nn++)
    {
        long runlines = materials[nn]->numRunlinesE() +
            materials[nn]->numRunlinesH();
        long halfCells = materials[nn]->numHalfCellsE() +
            materials[nn]->numHalfCellsH();
        double totalMicroseconds = mMaterialMicrosecondsE[nn] +
            mMaterialMicrosecondsH[nn];
        str << prefix << "material{" << nn+1 << "}.name ='"
            << materials[nn]->substanceName() << "';\n";
        str << prefix << "material{" << nn+1 << "}.model = '"
            << materials[nn]->modelName() << "';\n";
        str << prefix << "material{" << nn+1 << "}.numRunlinesE = "
            << materials[nn]->numRunlinesE() << ";\n";
        str << prefix << "material{" << nn+1 << "}.numRunlinesH = "
            << materials[nn]->numRunlinesH() << ";\n";
        str << prefix << "material{" << nn+1 << "}.numHalfCellsE = "
            << materials[nn]->numHalfCellsE() << ";\n";
        str << prefix << "material{" << nn+1 << "}.numHalfCellsH = "
            << materials[nn]->numHalfCellsH() << ";\n";
        str << prefix << "material{" << nn+1 << "}.halfCellTimeE = " <<
            (mMaterialMicrosecondsE[nn]*1e-6/materials[nn]->numHalfCellsE()
            /numT) << ";\n";
        str << prefix << "material{" << nn+1 << "}.halfCellTimeH = " <<
            (mMaterialMicrosecondsH[nn]*1e-6/materials[nn]->numHalfCellsH()
            /numT) << ";\n";
        str << prefix << "material{" << nn+1 << "}.numRunlines = " << runlines
            << ";\n";
        str << prefix << "material{" << nn+1 << "}.numHalfCells = " << halfCells
            << ";\n";
        str << prefix << "material{" << nn+1 << "}.numYeeCells = " <<
            halfCells/6.0 << ";\n";
        str << prefix << "material{" << nn+1 << "}.totalTime = " <<
            totalMicroseconds*1e-6 << ";\n";
        str << prefix << "material{" << nn+1 << "}.halfCellTime = " <<
            (totalMicroseconds*1e-6/halfCells/numT) << ";\n";
    }
}












