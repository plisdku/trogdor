/*
 *  CalculationPartition.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _CALCULATIONPARTITION_
#define _CALCULATIONPARTITION_

#include "UpdateEquation.h"
#include "Output.h"
#include "Source.h"
#include "CurrentSource.h"
#include "HuygensSurface.h"
#include "InterleavedLattice.h"
#include "Performance.h"

#include "Pointer.h"
#include <vector>
#include <string>
#include <iostream>

class VoxelizedPartition;

class CalculationPartition
{
public:
    CalculationPartition(const VoxelizedPartition & vg,
        Vector3f dxyz, float dt, long numT);
    ~CalculationPartition();
    
    GridDescPtr gridDescription() const { return mGridDescription; }
    const InterleavedLattice& lattice() const;
    InterleavedLattice& lattice();
    
    // used by PartitionStatistics
    const std::vector<UpdateEquationPtr> materials() const
        { return mMaterials; }
    
    // instruct all materials and outputs to allocate space for extra fields
    // and accumulation variables (e.g. for output interpolation, currents...)
    void allocateAuxBuffers();
    
    // returns        cell size in meters (same for all grids, all partitions)
    Vector3f dxyz() const { return m_dxyz; }
    
    // returns        timestep duration in seconds (all grids/partitions same)
    float dt() const { return m_dt; }
    
    // returns        number of timesteps in simulation, including initial cond.
    long duration() const { return m_numT; }
    
    void updateE(long timestep);
    void sourceE(long timestep);
    void outputE(long timestep);
    void updateH(long timestep);
    void sourceH(long timestep);
    void outputH(long timestep);
    
    void timedUpdateE(long timestep);
    void timedSourceE(long timestep);
    void timedOutputE(long timestep);
    void timedUpdateH(long timestep);
    void timedSourceH(long timestep);
    void timedOutputH(long timestep);
    
    void printFields(std::ostream & str, int octant, float scale);
    
    void printPerformanceForMatlab(std::ostream & str, std::string prefix);
    
private:
    const GridDescPtr mGridDescription;
    
    Vector3f m_dxyz;
    float m_dt;
    long m_numT;
    
    std::vector<UpdateEquationPtr> mMaterials;
    std::vector<OutputPtr> mOutputs;
    std::vector<SourcePtr> mHardSources;
    std::vector<SourcePtr> mSoftSources;
    std::vector<CurrentSourcePtr> mCurrentSources;
    std::vector<HuygensSurfacePtr> mHuygensSurfaces;
    
    PartitionStatistics mStatistics;
    
    InterleavedLatticePtr mLattice;
};
typedef Pointer<CalculationPartition> CalculationPartitionPtr;


#endif

