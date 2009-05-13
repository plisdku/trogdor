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

// Output types:
// subset of (Ex Ey Ez Hx Hy Hz) in series of Yee rects with given decimation
// subset of resampled/colocated (E H) in series of Yee rects, decimated
// aux field output in series of Yee rects, so RLE needed rectwise
// full material aux field dump, all (?) aux data; may need space-varying params
// custom calculation on E and H, in series of Yee rects, given decimation

// Also: specify timesteps in series of intervals with given period

// Report types:
// material params in cells, whatever may be available (?) (does this go here?)


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
    
private:
    
};
typedef Pointer<Output> OutputPtr;



#endif
