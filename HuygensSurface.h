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
#include "Map.h"
#include <vector>

class SetupHuygensSurface;
typedef Pointer<SetupHuygensSurface> SetupHuygensSurfacePtr;
class HuygensSurface;
typedef Pointer<HuygensSurface> HuygensSurfacePtr;
class VoxelizedPartition;
typedef Pointer<VoxelizedPartition> VoxelizedPartitionPtr;

class SetupNeighborBuffer;
typedef Pointer<SetupNeighborBuffer> SetupNeighborBufferPtr;
class NeighborBuffer;
typedef Pointer<NeighborBuffer> NeighborBufferPtr;

class HuygensSurfaceFactory
{
public:
    static SetupHuygensSurfacePtr newSetupHuygensSurface(
        const VoxelizedPartition & vp,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
        const HuygensSurfaceDescPtr & desc);
private:
    HuygensSurfaceFactory() {}
};

class SetupHuygensSurface
{
public:
    SetupHuygensSurface();
    virtual ~SetupHuygensSurface() {}
    //const std::vector<MemoryBufferPtr> & getBuffers() const { return mBuffers; }
    
    virtual HuygensSurfacePtr makeHuygensSurface() const = 0;
    
    /*
    bool hasBuffer(int side)
        { return mNeighborBuffers.at(side) != 0L; }
    SetupNeighborBufferPtr & getNeighborBuffer(int side)
        { return mNeighborBuffers.at(side); }
    */
    
protected:
    //std::vector<SetupNeighborBufferPtr> mNeighborBuffers;
};
typedef Pointer<SetupHuygensSurface> SetupHuygensSurfacePtr;

class HuygensSurface
{
public:
    HuygensSurface();
    virtual ~HuygensSurface() {}
    
    virtual void updateE() {}
    virtual void updateH() {}
    /*
    bool hasBuffer(int side)
        { return mNeighborBuffers.at(side) != 0L; }
    NeighborBufferPtr & getNeighborBuffer(int side)
        { return mNeighborBuffers.at(side); }
    */
private:
    //std::vector<NeighborBufferPtr> mNeighborBuffers;
};
typedef Pointer<HuygensSurface> HuygensSurfacePtr;

/*
struct NeighborBufferInfo
{
    int side;
    Rect3i destHalfRect;
    Vector3i numYeeCells;
    Vector3i nonZeroDimensions;
	std::vector<float> destFactorsE;
	std::vector<float> srcFactorsE;
	std::vector<float> destFactorsH;
	std::vector<float> srcFactorsH;
    std::vector<MemoryBufferPtr> buffersE;
    std:::vector<MemoryBufferPtr> buffersH;
};

class SetupNeighborBuffer
{
public:
    SetupNeighborBuffer(int side);
    
    const Rect3i & getDestRect() const
        { return mInfo.destHalfRect; }
    float getDestFactorE(int fieldDirection) const
        { return mInfo.destFactorsE[fieldDirection]; }
    float getDestFactorH(int fieldDirection) const;
        { return mInfo.destFactorsH[fieldDirection]; }
    float getSourceFactorE(int fieldDirection) const;
        { return mInfo.sourceFactorsE[fieldDirection]; }
    float getSourceFactorH(int fieldDirection) const;
        { return mInfo.sourceFactorsH[fieldDirection]; }
    
    BufferPointer getE(int fieldDirection, const Vector3i & yeeCell) const;
    BufferPointer getH(int fieldDirection, const Vector3i & yeeCell) const;
    
    const NeighborBufferInfo & getInfo() const { return mInfo; }
private:
    NeighborBufferInfo mInfo;
};

class NeighborBuffer
{
public:
    NeighborBuffer(const SetupNeighborBuffer & setupNB);
    
    const Rect3i & getDestRect() const
        { return mInfo.destHalfRect; }
    float getDestFactorE(int fieldDirection) const
        { return mInfo.destFactorsE[fieldDirection]; }
    float getDestFactorH(int fieldDirection) const;
        { return mInfo.destFactorsH[fieldDirection]; }
    float getSourceFactorE(int fieldDirection) const;
        { return mInfo.sourceFactorsE[fieldDirection]; }
    float getSourceFactorH(int fieldDirection) const;
        { return mInfo.sourceFactorsH[fieldDirection]; }
    
    float getE(int fieldDirection, const Vector3i & yeeCell) const;
    float getH(int fieldDirection, const Vector3i & yeeCell) const;
    void setE(int fieldDirection, const Vector3i & yeeCell, float value);
    void setH(int fieldDirection, const Vector3i & yeeCell, float value);
    
    const NeighborBufferInfo & getInfo() const { return mInfo; }
private:
    float* mHeadE[3];
    float* mHeadH[3];
    Vector3i mMemStride;
    Vector3f mEOffset[3];
    Vector3f mHOffset[3];
    
    std::vector<float> mFields;
    
    NeighborBufferInfo mInfo;
};
*/


#endif
