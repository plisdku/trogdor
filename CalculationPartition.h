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

#include "MaterialBoss.h"
#include "Pointer.h"
#include "Map.h"
#include <vector>

class VoxelizedPartition;
class NeighborBufferDescription;
typedef Pointer<NeighborBufferDescription> NeighborBufferDescPtr;

class Output;
typedef Pointer<Output> OutputPtr;
class Source;
typedef Pointer<Source> SourcePtr;
class HuygensSurface;
typedef Pointer<HuygensSurface> HuygensSurfacePtr;

class CalculationPartition
{
public:
    CalculationPartition(const VoxelizedPartition & vg,
        Vector3f dxyz, float dt, long numT);
    ~CalculationPartition();
    
    // instruct all materials and outputs to allocate space for extra fields
    // and accumulation variables (e.g. for output interpolation, currents...)
    void allocateAuxBuffers();
    
    // returns        cell size in meters (same for all grids, all partitions)
    Vector3f getDxyz() const { return m_dxyz; }
    
    // returns        timestep duration in seconds (all grids/partitions same)
    float getDt() const { return m_dt; }
    
    // returns        number of timesteps in simulation, including initial cond.
    long getDuration() const { return m_numT; }
    
    /*
    // update E fields
    void calcE();
    void calcBeforeE(int timestep);
    void calcAfterE(int timestep);
    
    // update H fields
    void calcH();
    void calcBeforeH(int timestep);
    void calcAfterH(int timestep);
    */
    void updateE(int timestep);
    void sourceE(int timestep);
    void outputE(int timestep);
    void updateH(int timestep);
    void sourceH(int timestep);
    void outputH(int timestep);
    
    
    // These field accessors exist solely for the convenience of the Output
    // classes.  To improve speed a little, the SetupOutput can prepare in
    // advance using VoxelizedPartition::fieldPointer.
    
    // direction      0,1,2 for x,y,z
    // xi,xj,xk       global Yee cell coordinates
    // returns        EM field in cell, with partition wraparound
    float getE(int direction, int xi, int xj, int xk) const;
    float getH(int direction, int xi, int xj, int xk) const;
    float getE(int direction, Vector3i xx) const;
    float getH(int direction, Vector3i xx) const;
    
    // direction      0,1,2 for x,y,z
    // xi,xj,xk       global coordinates in meters (!)
    // returns        appropriate trilinearly interpolated field value
    //                (this takes into account the Yee cell offsets)
    float getE(int direction, float xi, float xj, float xk) const;
    float getH(int direction, float xi, float xj, float xk) const;
    float getE(int direction, Vector3f xx) const;
    float getH(int direction, Vector3f xx) const;
    
    void setE(int direction, int xi, int xj, int xk, float val);
    void setH(int direction, int xi, int xj, int xk, float val);
    
    void printFields(std::ostream & str, int field, float scale);
    
    void createHuygensSurfaces(const VoxelizedPartition & vp);

private:
    Vector3f m_dxyz;
    float m_dt;
    long m_numT;
    
    // All the variables in this section are used for getting field values.
    float* mHeadE[3];
    float* mHeadH[3];
    Vector3i mAllocOriginYee;
    Vector3i mAllocYeeCells;
    Vector3i mMemStride;
    Vector3f mEOffset[3];
    Vector3f mHOffset[3];
    // end of field accessor vars
    
    void allocate(std::vector<float> & data,
        std::vector<MemoryBufferPtr> & buffers);
    
    std::vector<MaterialPtr> mMaterials;
    std::vector<OutputPtr> mOutputs;
    std::vector<SourcePtr> mHardSources;
    std::vector<SourcePtr> mSoftSources;
    std::vector<HuygensSurfacePtr> mHuygensSurfaces;
    
    //EHBufferSetPtr mEHBuffers;
    std::vector<MemoryBufferPtr> mEHBuffers;
    std::vector<float> mFields;
    
    Map<NeighborBufferDescPtr, std::vector<MemoryBufferPtr> > mNBBuffers;
    Map<NeighborBufferDescPtr, std::vector<float> > mNBFields;
    
};
typedef Pointer<CalculationPartition> CalculationPartitionPtr;


#endif

