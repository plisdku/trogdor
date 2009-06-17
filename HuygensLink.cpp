/*
 *  HuygensLink.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "HuygensLink.h"
#include "VoxelizedPartition.h"
#include "SimulationDescription.h"
#include "Log.h"

using namespace std;

HuygensLinkDelegate::
HuygensLinkDelegate(const VoxelizedPartition & vp,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
    const HuygensSurfaceDescPtr & desc)
{
    const Map<NeighborBufferDescPtr, vector<MemoryBufferPtr> > & memBufs =
        vp.getNBBuffers();
    const vector<NeighborBufferDescPtr> & buffers = desc->getBuffers();
    GridDescPtr g = desc->getSourceGrid();
    VoxelizedPartitionPtr srcVP = grids[g];
    
    for (int nn = 0; nn < 6; nn++)
    if (buffers[nn] != 0L)
    {
        LinkNeighborBufferDeleg b;
        Vector3i headYeeCell = vecHalfToYee(buffers[nn]->getDestHalfRect().p1);
        
        assert(memBufs.count(buffers[nn]) != 0);
        const vector<MemoryBufferPtr> & mbs = memBufs[buffers[nn]];
        
        for (int fieldDir = 0; fieldDir < 3; fieldDir++)
        {
            b.mDestEHeadPtr[fieldDir] = vp.getE(fieldDir, headYeeCell);
            //b.mSrcEHeadPtr[fieldDir] = vp.getE(fieldDir, headYeeCell);
            b.mBufEHeadPtr[fieldDir] = vp.getE(fieldDir, headYeeCell);
        }
        
        b.mSourceStride = srcVP->getFieldStride();
        b.mDestStride = vp.getFieldStride();
        //b.mNumYeeCells = buffers[nn]->getBufferYeeBounds().size()+1;
    }
}

HuygensSurfacePtr HuygensLinkDelegate::
makeHuygensSurface() const
{
    return HuygensSurfacePtr(new HuygensLink(mNBs));
}

HuygensLink::
HuygensLink(const vector<LinkNeighborBufferDeleg> & nbs)
{
    LOG << "Constructor.\n";
    
    unsigned int nn, mm;
    for (nn = 0; nn < nbs.size(); nn++)
    {
        LinkNeighborBuffer b;
        for (mm = 0; mm < 3; mm++)
        {
            b.mDestEHeadPtr[mm] = nbs[nn].mDestEHeadPtr[mm].getPointer();
            b.mDestHHeadPtr[mm] = nbs[nn].mDestHHeadPtr[mm].getPointer();
            b.mSrcEHeadPtr[mm] = nbs[nn].mSrcEHeadPtr[mm].getPointer();
            b.mSrcHHeadPtr[mm] = nbs[nn].mSrcHHeadPtr[mm].getPointer();
            b.mBufEHeadPtr[mm] = nbs[nn].mBufEHeadPtr[mm].getPointer();
            b.mBufHHeadPtr[mm] = nbs[nn].mBufHHeadPtr[mm].getPointer();
        }
        b.mSourceStride = nbs[nn].mSourceStride;
        b.mDestStride = nbs[nn].mDestStride;
        b.mNumYeeCells = nbs[nn].mNumYeeCells;
        b.mSrcFactorsE = nbs[nn].mSrcFactorsE;
        b.mSrcFactorsH = nbs[nn].mSrcFactorsH;
        b.mDestFactorsE = nbs[nn].mDestFactorsE;
        b.mDestFactorsH = nbs[nn].mDestFactorsH;
    }
}


void HuygensLink::
updateE()
{
    LOG << "Update E.\n";
    
    const float *srcPtr, *srcStart2, *srcStart1;
    const float* destPtr, *destStart2, *destStart1;
    float *bufPtr, *bufStart2, *bufStart1;
    
    for (int bufNum = 0; bufNum < mNBs.size(); bufNum++)
    {
        const Vector3i srcStride(mNBs[bufNum].mSourceStride);
        const Vector3i destStride(mNBs[bufNum].mDestStride);
        for (int fieldNum = 0; fieldNum < 3; fieldNum++)
        {
            srcStart2 = mNBs[bufNum].mSrcEHeadPtr[fieldNum];
            destStart2 = mNBs[bufNum].mDestEHeadPtr[fieldNum];
            bufStart2 = mNBs[bufNum].mBufEHeadPtr[fieldNum];
            
            const float srcFactor = mNBs[bufNum].mSrcFactorsE[fieldNum];
            const float destFactor = mNBs[bufNum].mDestFactorsE[fieldNum];
            const Vector3i numYeeCells = mNBs[bufNum].mNumYeeCells;
            
            for (int nk = 0; nk < numYeeCells[2]; nk++)
            {
                srcStart1 = srcStart2;
                destStart1 = destStart2;
                bufStart1 = bufStart2;
                for (int nj = 0; nj < numYeeCells[1]; nj++)
                {
                    srcPtr = srcStart1;
                    destPtr = destStart1;
                    bufPtr = bufStart1;
                    for (int ni = 0; ni < numYeeCells[0]; ni++)
                    {
                        *bufPtr = *srcPtr*srcFactor + *destPtr*destFactor;
                        srcPtr += srcStride[0];
                        destPtr += destStride[0];
                        bufPtr += destStride[0];
                    }
                    srcStart1 += srcStride[1];
                    destStart1 += destStride[1];
                    bufStart1 += destStride[1];
                }
                srcStart2 += srcStride[2];
                destStart2 += destStride[2];
                bufStart2 += destStride[2];
            }
        }
    }
}

void HuygensLink::
updateH()
{
    /*
    LOG << "Update H.\n";
    
    float srcFactor, destFactor;
    
    const float *srcPtr, *srcStart2, *srcStart1;
    const float* destPtr, *destStart2, *destStart1;
    float *bufPtr, *bufStart2, *bufStart1;
    
    for (int fieldNum = 0; fieldNum < 3; fieldNum++)
    {
        srcStart2 = mSrcHHeadPtr[fieldNum];
        destStart2 = mDestHHeadPtr[fieldNum];
        bufStart2 = mBufHHeadPtr[fieldNum];  // buffer start head pointer
        
        for (int nk = 0; nk < mNumYeeCells[2]; nk++)
        {
            srcStart1 = srcStart2;
            destStart1 = destStart2;
            bufStart1 = bufStart2;
            for (int nj = 0; nj < mNumYeeCells[1]; nj++)
            {
                srcPtr = srcStart1;
                destPtr = destStart1;
                bufPtr = bufStart1;
                for (int ni = 0; ni < mNumYeeCells[0]; ni++)
                {
                    *bufPtr = *srcPtr*srcFactor + *destPtr*destFactor;
                    srcPtr += mSourceStride[0];
                    destPtr += mDestStride[0];
                    bufPtr += mDestStride[0];
                }
                srcStart1 += mSourceStride[1];
                destStart1 += mDestStride[1];
                bufStart1 += mDestStride[1];
            }
            srcStart2 += mSourceStride[2];
            destStart2 += mDestStride[2];
            bufStart2 += mDestStride[2];
        }
    }
    */
}


LinkNeighborBufferDeleg::
LinkNeighborBufferDeleg() :
    mDestEHeadPtr(3),
    mDestHHeadPtr(3),
    mSrcEHeadPtr(3),
    mSrcHHeadPtr(3),
    mBufEHeadPtr(3),
    mBufHHeadPtr(3),
    mSrcFactorsE(3),
    mDestFactorsE(3),
    mSrcFactorsH(3),
    mDestFactorsH(3)
{
}

LinkNeighborBuffer::
LinkNeighborBuffer() :
    mDestEHeadPtr(3),
    mDestHHeadPtr(3),
    mSrcEHeadPtr(3),
    mSrcHHeadPtr(3),
    mBufEHeadPtr(3),
    mBufHHeadPtr(3),
    mSrcFactorsE(3),
    mDestFactorsE(3),
    mSrcFactorsH(3),
    mDestFactorsH(3)
{
}






















