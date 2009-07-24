/*
 *  FileSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/24/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _FILESOURCE_
#define _FILESOURCE_

#include <fstream>

class FileSetupSource : public SetupSource
{
public:
    FileSetupSource(const SourceDescPtr & desc);
    
    virtual SourcePtr makeSource(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    SourceDescPtr mDesc;
};

class FileSource : public Source
{
public:
    FileSource(const SourceDescription & desc,
        const VoxelizedPartition & vp, const CalculationPartition & cp);
    
    virtual void sourceEPhase(CalculationPartition & cp, int timestep);
    virtual void sourceHPhase(CalculationPartition & cp, int timestep);
private:
    void uniformSourceE(CalculationPartition & cp, int timestep);
    void uniformSourceH(CalculationPartition & cp, int timestep);
    void polarizedSourceE(CalculationPartition & cp, int timestep);
    void polarizedSourceH(CalculationPartition & cp, int timestep);
    
    // I can't figure out a good way to make use of these functions, because
    // the hard source has such weird behavior in Yee grids.
    //void sourceE(Vector3f value);
    //void sourceH(Vector3f value);
    
    std::ifstream mFileStream;
    
    int mCurrentDuration;
    SourceFields mFields;
    
    float mDt;
    bool mIsSpaceVarying;
    bool mIsSoft;
    
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
    
    Map<std::string, std::string> mParams;
};









#endif
