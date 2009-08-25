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
#include "OutputBoss.h"
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
    
    void updateE(int timestep);
    void sourceE(int timestep);
    void outputE(int timestep);
    void updateH(int timestep);
    void sourceH(int timestep);
    void outputH(int timestep);
    
    void timedUpdateE(int timestep);
    void timedSourceE(int timestep);
    void timedOutputE(int timestep);
    void timedUpdateH(int timestep);
    void timedSourceH(int timestep);
    void timedOutputH(int timestep);
    
    void printFields(std::ostream & str, int octant, float scale);
    
    void printPerformanceForMatlab(std::ostream & str, std::string prefix);
    
private:
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

