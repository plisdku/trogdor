/*
 *  FileCurrentSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/17/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _FILECURRENTSOURCE_
#define _FILECURRENTSOURCE_

#include "CurrentSource.h"
#include <fstream>

class SetupFileCurrentSource : public SetupCurrentSource
{
public:
    SetupFileCurrentSource(const CurrentSourceDescPtr & description);
    
    virtual Pointer<CurrentSource> makeCurrentSource(
        const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
};


class FileCurrentSource : public CurrentSource
{
public:
    FileCurrentSource(const CurrentSourceDescPtr & description,
        const VoxelizedPartition & vp, const CalculationPartition & cp);
    
    virtual void prepareJ(long timestep);
    virtual void prepareK(long timestep);
    
    virtual float getJ(int direction) const;
    virtual float getK(int direction) const;
private:
    void updateJ(long timestep);
    void updateK(long timestep);
    
    std::string mFormula;
    std::ifstream mFileStream;
    SourceCurrents mCurrents;
    
    float mDt;
    
    std::vector<Duration> mDurations;
    long mCurrentDuration;
    
    Vector3f mJ;
    Vector3f mK;
};


#endif
