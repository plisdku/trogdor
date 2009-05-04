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
#include <vector>

class VoxelizedPartition;
class NeighborBufferDescription;
typedef Pointer<NeighborBufferDescription> NeighborBufferDescPtr;

class CalculationPartition
{
public:
    CalculationPartition(const VoxelizedPartition & vg);
    ~CalculationPartition();

private:
    std::vector<MaterialPtr> mMaterials;
    
    EHBufferSetPtr mEHBuffers;
    Map<NeighborBufferDescPtr, EHBufferSet> mNBBuffers;
    
    
    
};
typedef Pointer<CalculationPartition> CalculationPartitionPtr;


#endif

