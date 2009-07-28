/*
 *  Performance.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/19/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "Performance.h"
#include "Material.h"

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
            "standard output.\n";
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
setNumHuygensSurfaces(int num)
{
    mHuygensSurfaceMicroseconds.resize(num, 0.0);
}

void PartitionStatistics::
addMaterialMicrosecondsE(int material, double us)
{
    mMaterialMicrosecondsE[material] += us;
}
void PartitionStatistics::
addMaterialMicrosecondsH(int material, double us)
{
    mMaterialMicrosecondsH[material] += us;
}
void PartitionStatistics::
addOutputMicroseconds(int output, double us)
{
    mOutputMicroseconds[output] += us;
}
void PartitionStatistics::
addHardSourceMicroseconds(int source, double us)
{
    mHardSourceMicroseconds[source] += us;
}
void PartitionStatistics::
addSoftSourceMicroseconds(int source, double us)
{
    mSoftSourceMicroseconds[source] += us;
}
void PartitionStatistics::
addHuygensSurfaceMicroseconds(int source, double us)
{
    mHuygensSurfaceMicroseconds[source] += us;
}


void PartitionStatistics::
printForMatlab(std::ostream & str, const string & prefix,
    const vector<MaterialPtr> & materials, long numT)
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
    double totalHuygensSurface_us = accumulate(
        mHuygensSurfaceMicroseconds.begin(), mHuygensSurfaceMicroseconds.end());
    
    str << prefix << "calcETime = " << totalMatE_us*1e-6 << ";\n";
    str << prefix << "calcHTime = " << totalMatH_us*1e-6 << ";\n";
    str << prefix << "calcTime = " << (totalMatE_us+totalMatH_us)*1e-6 << ";\n";
    str << prefix << "outputTime = " << totalOut_us*1e-6 << ";\n";
    str << prefix << "hardSourceTime = " << totalHardSource_us*1e-6 << ";\n";
    str << prefix << "softSourceTime = " << totalSoftSource_us*1e-6<< ";\n";
    str << prefix << "sourceTime = " << (totalHardSource_us+totalSoftSource_us)
        *1e-6 << ";\n";
    str << prefix << "huygensSurfaceTime = " << totalHuygensSurface_us*1e-6
        << ";\n";
    
    for (int nn = 0; nn < mMaterialMicrosecondsE.size(); nn++)
    {
        long runlines = materials[nn]->getNumRunlinesE() +
            materials[nn]->getNumRunlinesH();
        long halfCells = materials[nn]->getNumHalfCellsE() +
            materials[nn]->getNumHalfCellsH();
        double totalMicroseconds = mMaterialMicrosecondsE[nn] +
            mMaterialMicrosecondsH[nn];
        str << prefix << "material{" << nn+1 << "}.name ='"
            << materials[nn]->getSubstanceName() << "';\n";
        str << prefix << "material{" << nn+1 << "}.model = '"
            << materials[nn]->getModelName() << "';\n";
        str << prefix << "material{" << nn+1 << "}.numRunlinesE = "
            << materials[nn]->getNumRunlinesE() << ";\n";
        str << prefix << "material{" << nn+1 << "}.numRunlinesH = "
            << materials[nn]->getNumRunlinesH() << ";\n";
        str << prefix << "material{" << nn+1 << "}.numHalfCellsE = "
            << materials[nn]->getNumHalfCellsE() << ";\n";
        str << prefix << "material{" << nn+1 << "}.numHalfCellsH = "
            << materials[nn]->getNumHalfCellsH() << ";\n";
        str << prefix << "material{" << nn+1 << "}.halfCellTimeE = " <<
            (mMaterialMicrosecondsE[nn]*1e-6/materials[nn]->getNumHalfCellsE()
            /numT) << ";\n";
        str << prefix << "material{" << nn+1 << "}.halfCellTimeH = " <<
            (mMaterialMicrosecondsH[nn]*1e-6/materials[nn]->getNumHalfCellsH()
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












