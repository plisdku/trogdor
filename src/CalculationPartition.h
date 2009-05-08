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

class CalculationPartition
{
public:
    CalculationPartition(const VoxelizedPartition & vg,
        Vector3f dxyz, float dt, long numT);
    ~CalculationPartition();
    
    void allocateAuxBuffers();
    
    Vector3f getDxyz() const { return m_dxyz; }
    float getDt() const { return m_dt; }
    long getDuration() const { return m_numT; }
    
    void calcE();
    void calcAfterE();
    void calcH();
    void calcAfterH();
    

private:
    Vector3f m_dxyz;
    float m_dt;
    long m_numT;
    
    void allocate(std::vector<float> & data, EHBufferSet& buffers);
    
    std::vector<MaterialPtr> mMaterials;
    std::vector<OutputPtr> mOutputs;
    
    EHBufferSetPtr mEHBuffers;
    std::vector<float> mFields;
    
    Map<NeighborBufferDescPtr, EHBufferSet> mNBBuffers;
    Map<NeighborBufferDescPtr, std::vector<float> > mNBFields;
    
};
typedef Pointer<CalculationPartition> CalculationPartitionPtr;


#endif

