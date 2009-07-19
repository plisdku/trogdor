/*
 *  Performance.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/19/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _PERFORMANCE_
#define _PERFORMANCE_

#include "Pointer.h"
#include <vector>
#include <iostream>

class Material;
typedef Pointer<Material> MaterialPtr;

class GlobalStatistics
{
public:
    GlobalStatistics();
    void setNumTimesteps(long numT);
    
    void setReadDescriptionMicroseconds(double us);
    void setVoxelizeMicroseconds(double us);
    void setSetupCalculationMicroseconds(double us);
    void setRunCalculationMicroseconds(double us);
    void setPrintTimestepMicroseconds(double us);
    
    void setTimestepMicroseconds(long timestep, double us);
    
    void printForMatlab(std::ostream & str);
private:
    double mReadDescriptionMicroseconds;
    double mVoxelizeMicroseconds;
    double mSetupCalculationMicroseconds;
    double mRunCalculationMicroseconds;
    double mPrintTimestepMicroseconds;
    
    std::vector<double> mTimestepStartTimes;
};

class PartitionStatistics
{
public:
    PartitionStatistics();
    void setNumMaterials(int num);
    void setNumOutputs(int num);
    void setNumHardSources(int num);
    void setNumSoftSources(int num);
    void setNumHuygensSurfaces(int num);
    
    void addMaterialMicrosecondsE(int material, double us);
    void addMaterialMicrosecondsH(int material, double us);
    void addOutputMicroseconds(int output, double us);
    void addHardSourceMicroseconds(int source, double us);
    void addSoftSourceMicroseconds(int source, double us);
    void addHuygensSurfaceMicroseconds(int source, double us);
    
    void printForMatlab(std::ostream & str, const std::string & prefix,
        const std::vector<MaterialPtr> & materials, long numT);
private:
    std::vector<double> mMaterialMicrosecondsE;
    std::vector<double> mMaterialMicrosecondsH;
    std::vector<double> mOutputMicroseconds;
    std::vector<double> mHardSourceMicroseconds;
    std::vector<double> mSoftSourceMicroseconds;
	std::vector<double> mHuygensSurfaceMicroseconds;
};










#endif
