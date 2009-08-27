/*
 *  Output.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/6/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _OUTPUT_
#define _OUTPUT_

#include "Pointer.h"
#include "SimulationDescriptionPredeclarations.h"

class VoxelizedPartition;
class CalculationPartition;

// Two predeclarations (definitions below)
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
    SetupOutput(OutputDescPtr description) :
        mDescription(description) {}
    virtual ~SetupOutput() {}
    OutputDescPtr description() const { return mDescription; }
    
    // Setting up runtime outputs
    virtual OutputPtr makeOutput(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const = 0;
    
private:
    OutputDescPtr mDescription;
};


class Output
{
public:
    Output(OutputDescPtr description);
    virtual ~Output();
    
    OutputDescPtr description() const { return mDescription; }
    
    // A chance to do runlines...
    //virtual void setupFromGrid(const VoxelizedPartition & vp);
    
    virtual void outputEPhase(const CalculationPartition & cp, int timestep);
    virtual void outputHPhase(const CalculationPartition & cp, int timestep);
    
    virtual void allocateAuxBuffers();
    
private:
    OutputDescPtr mDescription;
};
typedef Pointer<Output> OutputPtr;



#endif
