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
#include "InterleavedLattice.h"
#include "Pointer.h"
#include "MemoryUtilities.h"
#include "Map.h"
#include "geometry.h"
#include <vector>

class HuygensSurface;
typedef Pointer<HuygensSurface> HuygensSurfacePtr;
class VoxelizedPartition;
typedef Pointer<VoxelizedPartition> VoxelizedPartitionPtr;

class NeighborBuffer;
typedef Pointer<NeighborBuffer> NeighborBufferPtr;

class HuygensUpdate;
typedef Pointer<HuygensUpdate> HuygensUpdatePtr;

class HuygensSurfaceFactory
{
public:
    static HuygensSurfacePtr newHuygensSurface(
        std::string namePrefix,
        const VoxelizedPartition & vp,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
        const HuygensSurfaceDescPtr & desc);
private:
    HuygensSurfaceFactory() {}
};

class HuygensSurface
{
public:
    HuygensSurface(std::string namePrefix, const VoxelizedPartition & vp,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
        const HuygensSurfaceDescPtr & desc);
    
    const Rect3i & halfCells() const { return mHalfCells; }
    bool hasBuffer(int side) const { return mNeighborBuffers.at(side) != 0L; }
    NeighborBufferPtr buffer(int side) const
        { return mNeighborBuffers.at(side); }
    const std::vector<NeighborBufferPtr> & neighborBuffers() const
        { return mNeighborBuffers; }
    
    InterleavedLatticePtr destLattice() const { return mDestLattice; }
    InterleavedLatticePtr sourceLattice() const { return mSourceLattice; }
    void allocate();
    
    void setUpdater(HuygensUpdatePtr update) { mUpdate = update; }
    void updateE();
    void updateH();
private:
    Rect3i mHalfCells;
    HuygensUpdatePtr mUpdate;
    std::vector<NeighborBufferPtr> mNeighborBuffers;
    InterleavedLatticePtr mDestLattice;
    InterleavedLatticePtr mSourceLattice;
};

class HuygensUpdate
{
public:
    HuygensUpdate() {}
    virtual ~HuygensUpdate() {}
    
    virtual void updateE(HuygensSurface & hs) {}
    virtual void updateH(HuygensSurface & hs) {}
};

class NeighborBuffer
{
public:
    NeighborBuffer(std::string prefix,
        const Rect3i & huygensHalfCells, int sideNum,
        float incidentFieldFactor);
    NeighborBuffer(std::string prefix,
        const Rect3i & huygensHalfCells, 
        const Rect3i & sourceHalfCells,
        int sideNum,
        float incidentFieldFactor);
    
    const Rect3i & destHalfCells() const;
    const Rect3i & sourceHalfCells() const;
    float destFactorE(int fieldDirection) const;
    float destFactorH(int fieldDirection) const;
    float sourceFactorE(int fieldDirection) const;
    float sourceFactorH(int fieldDirection) const;
    
    InterleavedLatticePtr lattice() const { return mLattice; }
private:
    Rect3i edgeHalfCells(const Rect3i & halfCells, int nSide);
    void initFactors(const Rect3i & huygensHalfCells, int sideNum,
        float incidentFieldFactor);
    
    int side;
    InterleavedLatticePtr mLattice;
    
    Rect3i mSourceHalfCells; // not always used
    
    std::vector<float> mDestFactorsE;
	std::vector<float> mSourceFactorsE;
	std::vector<float> mDestFactorsH;
	std::vector<float> mSourceFactorsH;
};


#endif
