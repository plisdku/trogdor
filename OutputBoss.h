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

class OutputDelegate;
typedef Pointer<OutputDelegate> OutputDelegatePtr;

class Output;
typedef Pointer<Output> OutputPtr;

class OutputFactory
{
public:
    static OutputDelegatePtr getDelegate(const VoxelizedPartition & vp,
        const OutputDescPtr & desc);
    
private:
    OutputFactory();
};


class OutputDelegate
{
public:
    OutputDelegate() {}
    
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
    
    virtual void outputEPhase(int timestep);
    virtual void outputHPhase(int timestep);
    
    virtual void allocateAuxBuffers();
private:
    
};
typedef Pointer<Output> OutputPtr;



#endif