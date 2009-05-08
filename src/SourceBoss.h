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

class SourceDelegate;
typedef Pointer<SourceDelegate> SourceDelegatePtr;

class Source;
typedef Pointer<Source> SourcePtr;

class SourceFactory
{
public:
    static SourceDelegatePtr getDelegate(const VoxelizedPartition & vp,
        const SourceDescPtr & desc);
    
private:
    SourceFactory();
};


class SourceDelegate
{
public:
    SourceDelegate() {}
    
    // Setting up runtime outputs
    virtual SourcePtr makeSource(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const = 0;
};

class Source
{
public:
    Source();
    virtual ~Source();
    
    virtual void sourceEPhase(int timestep);
    virtual void sourceHPhase(int timestep);
    
private:
    
};
typedef Pointer<Source> SourcePtr;






#endif