/*
 *  Source.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _SOURCE_
#define _SOURCE_

#include "StreamedFieldInput.h"
#include "SimulationDescription.h"

#include <vector>

class CalculationPartition;
class VoxelizedPartition;

class Source;

class SetupSource
{
public:
    SetupSource(const SourceDescPtr sourceDescription);
    virtual ~SetupSource();
    SourceDescPtr description() const { return mDescription; }
    
    Pointer<Source> makeSource(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    SourceDescPtr mDescription;
};
typedef Pointer<SetupSource> SetupSourcePtr;

class Source
{
public:
    Source(const SourceDescPtr sourceDescription,
        const VoxelizedPartition & vp,
        const CalculationPartition & cp);
    virtual ~Source();
    
    void sourceEPhase(CalculationPartition & cp, long timestep);
    void sourceHPhase(CalculationPartition & cp, long timestep);
private:
    void doSourceE(CalculationPartition & cp, long timestep);
    void doSourceH(CalculationPartition & cp, long timestep);
    
    SourceDescPtr mDescription;
    StreamedFieldInput mFieldInput;
    
    bool mIsSoft;
    long mCurrentDuration;
    SourceFields mFields;
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
};
typedef Pointer<Source> SourcePtr;





#endif
