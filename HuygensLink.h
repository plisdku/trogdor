/*
 *  HuygensLink.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _HUYGENSLINK_
#define _HUYGENSLINK_

#include "HuygensSurface.h"
#include "geometry.h"
#include <vector>

struct LinkNeighborBufferDeleg;
struct LinkNeighborBuffer;

class SetupHuygensLink : public SetupHuygensSurface
{
public:
    SetupHuygensLink(
        const VoxelizedPartition & vp,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
        const HuygensSurfaceDescPtr & desc);
    
    virtual HuygensSurfacePtr makeHuygensSurface() const;
private:
    std::vector<LinkNeighborBufferDeleg> mNBs;
};


class HuygensLink : public HuygensSurface
{
public:
    HuygensLink(const std::vector<LinkNeighborBufferDeleg> & nbs);
    
    virtual void updateE();
    virtual void updateH();
private:
    std::vector<LinkNeighborBuffer> mNBs;
};


struct LinkNeighborBufferDeleg
{
    LinkNeighborBufferDeleg();
    std::vector<BufferPointer> mDestEHeadPtr;
    std::vector<BufferPointer> mDestHHeadPtr;
    std::vector<BufferPointer> mSrcEHeadPtr;
    std::vector<BufferPointer> mSrcHHeadPtr;
    std::vector<BufferPointer> mBufEHeadPtr;
    std::vector<BufferPointer> mBufHHeadPtr;
    Vector3i mSourceStride;
    Vector3i mDestStride;
    Vector3i mNumYeeCells;
    std::vector<float> mSrcFactorsE;
    std::vector<float> mDestFactorsE;
    std::vector<float> mSrcFactorsH;
    std::vector<float> mDestFactorsH;
};

struct LinkNeighborBuffer
{
    LinkNeighborBuffer();
    std::vector<float*> mDestEHeadPtr;
    std::vector<float*> mDestHHeadPtr;
    std::vector<float*> mSrcEHeadPtr;
    std::vector<float*> mSrcHHeadPtr;
    std::vector<float*> mBufEHeadPtr;
    std::vector<float*> mBufHHeadPtr;
    Vector3i mSourceStride;
    Vector3i mDestStride;
    Vector3i mNumYeeCells;
    std::vector<float> mSrcFactorsE;
    std::vector<float> mDestFactorsE;
    std::vector<float> mSrcFactorsH;
    std::vector<float> mDestFactorsH;
};

#endif
