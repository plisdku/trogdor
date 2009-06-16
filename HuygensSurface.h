/*
 *  HuygensSurface.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _HUYGENSSURFACE_
#define _HUYGENSSURFACE_

#include "SimulationDescriptionPredeclarations.h"
#include "Pointer.h"
#include "MemoryUtilities.h"
#include <vector>

class HuygensSurfaceDelegate;
typedef Pointer<HuygensSurfaceDelegate> HuygensSurfaceDelegatePtr;
class HuygensSurface;
typedef Pointer<HuygensSurface> HuygensSurfacePtr;
class VoxelizedPartition;
typedef Pointer<VoxelizedPartition> VoxelizedPartitionPtr;

class HuygensSurfaceFactory
{
public:
    static HuygensSurfaceDelegatePtr getDelegate(
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
        const HuygensSurfaceDescPtr & desc);
private:
    HuygensSurfaceFactory() {}
};

class HuygensSurfaceDelegate
{
public:
    HuygensSurfaceDelegate() {}
    virtual ~HuygensSurfaceDelegate() {}
    //const std::vector<MemoryBufferPtr> & getBuffers() const { return mBuffers; }
    
    virtual HuygensSurfacePtr makeHuygensSurface() const = 0;
    
private:
    //std::vector<MemoryBufferPtr> mBuffers;
};
typedef Pointer<HuygensSurfaceDelegate> HuygensSurfaceDelegatePtr;

class HuygensSurface
{
public:
    HuygensSurface() = 0;
    virtual ~HuygensSurface() {}
    
    virtual void updateE() {}
    virtual void updateH() {}
private:
    /*
    float* mHeadPointersE[3];
    float* mHeadPointersH[3];
    Vector3i mStridesE[3];
    Vector3i mStridesH[3];
    Vector3i mNumYeeCells;
    */
};
typedef Pointer<HuygensSurface> HuygensSurfacePtr;




#endif
