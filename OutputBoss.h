/*
 *  OutputBoss.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/6/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _OUTPUTBOSS_
#define _OUTPUTBOSS_

#include "Pointer.h"
#include "SimulationDescriptionPredeclarations.h"

class VoxelizedPartition;
class CalculationPartition;
typedef Pointer<CalculationPartition> CalculationPartitionPtr;

class SetupOutput;
typedef Pointer<SetupOutput> SetupOutputPtr;

class Output;
typedef Pointer<Output> OutputPtr;

class OutputFactory
{
public:
    static SetupOutputPtr newSetupOutput(const VoxelizedPartition & vp,
        const OutputDescPtr & desc);
    
private:
    OutputFactory();
};


class SetupOutput
{
public:
    SetupOutput() {}
    virtual ~SetupOutput() {}
    
    // Setting up runtime outputs
    virtual OutputPtr makeOutput(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const = 0;
};


class Output
{
public:
    Output();
    virtual ~Output();
    
    // A chance to do runlines...
    //virtual void setupFromGrid(const VoxelizedPartition & vp);
    
    virtual void outputEPhase(const CalculationPartition & cp, int timestep);
    virtual void outputHPhase(const CalculationPartition & cp, int timestep);
    
    virtual void allocateAuxBuffers();
private:
    
};
typedef Pointer<Output> OutputPtr;



#endif
