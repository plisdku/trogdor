/*
 *  SourceEH.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _SOURCEEH_
#define _SOURCEEH_

#include "SourceBoss.h"
#include "FieldInput.h"
#include "SimulationDescription.h"

#include <vector>

class CalculationPartition;
class VoxelizedPartition;

class SetupSourceEH : public SetupSource
{
public:
    SetupSourceEH(const SourceDescPtr sourceDescription);
    virtual ~SetupSourceEH();
    
    virtual SourcePtr makeSource(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    SourceDescPtr mDescription;
};

class SourceEH : public Source
{
public:
    SourceEH(const SourceDescPtr sourceDescription,
        const VoxelizedPartition & vp,
        const CalculationPartition & cp);
    virtual ~SourceEH();
    
    virtual void sourceEPhase(CalculationPartition & cp, int timestep);
    virtual void sourceHPhase(CalculationPartition & cp, int timestep);
private:
    void doSourceE(CalculationPartition & cp, int timestep);
    void doSourceH(CalculationPartition & cp, int timestep);
    
    FieldInput mFieldInput;
    
    bool mIsSoft;
    int mCurrentDuration;
    SourceFields mFields;
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
};





#endif
