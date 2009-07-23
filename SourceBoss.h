/*
 *  SourceBoss.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/7/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _SOURCEBOSS_
#define _SOURCEBOSS_

#include "Pointer.h"
#include "SimulationDescriptionPredeclarations.h"

class VoxelizedPartition;
class CalculationPartition;

class SetupSource;
typedef Pointer<SetupSource> SetupSourcePtr;

class Source;
typedef Pointer<Source> SourcePtr;

class SourceFactory
{
public:
    static SetupSourcePtr newSetupSource(const VoxelizedPartition & vp,
        const SourceDescPtr & desc);
    
private:
    SourceFactory();
};


class SetupSource
{
public:
    SetupSource() {}
    virtual ~SetupSource() {}
    
    // Setting up runtime outputs
    virtual SourcePtr makeSource(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const = 0;
};

class Source
{
public:
    Source();
    virtual ~Source();
    
    virtual void sourceEPhase(CalculationPartition & cp, int timestep);
    virtual void sourceHPhase(CalculationPartition & cp, int timestep);
    
private:
    
};
typedef Pointer<Source> SourcePtr;






#endif
