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
#include "YeeUtilities.h"
#include "Log.h"

using namespace YeeUtilities;
using namespace std;

SetupHuygensLink::
SetupHuygensLink(const VoxelizedPartition & vp,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
    const HuygensSurfaceDescPtr & desc)
{
    const Map<NeighborBufferDescPtr, vector<MemoryBufferPtr> > & memBufsE =
        vp.getNBBuffersE();
    const Map<NeighborBufferDescPtr, vector<MemoryBufferPtr> > & memBufsH =
        vp.getNBBuffersH();
    const vector<NeighborBufferDescPtr> & buffers = desc->getBuffers();
    GridDescPtr srcGrid = desc->getSourceGrid();
    VoxelizedPartitionPtr srcVP = grids[srcGrid];
    Rect3i destYee, srcYee;
    
    LOG << "Setup!  Buffer sourcing from " << srcGrid->getName() << "\n";
    //LOGMORE << "Membufs size " << memBufs.size() << "\n";
    LOGMORE << "Buffers size " << buffers.size() << "\n";
    
    //LOGMORE << "Contents of buffers: " << "\n";
    //LOGMORE << buffers << "\n";
    int dir;
    //for (int sideNum = 0; sideNum < 3; sideNum++)
    for (vector<NeighborBufferDescPtr>::const_iterator itr = buffers.begin();
        itr != buffers.end(); itr++)
    if (memBufsE.count(*itr) != 0)
    {
        assert(memBufsH.count(*itr) != 0);
        LinkNeighborBufferDeleg b;
        
        LOG << "New E buffers:\n";
        for (dir = 0; dir < 3; dir++)
        {
            destYee = halfToYee((*itr)->getDestHalfRect(),
                octantE(dir));
            srcYee = halfToYee((*itr)->getSourceHalfRect(),
                octantE(dir));
            
            LOGMORE << "E" << dir << " src " << srcYee << " dest " << destYee
                << "\n";
            LOGMORE << "From half cells " <<
                (*itr)->getSourceHalfRect() << " and " << 
                (*itr)->getDestHalfRect() << "\n";
            
            b.mDestEHeadPtr[dir] = vp.getE(dir, destYee.p1);
            b.mSrcEHeadPtr[dir] = srcVP->getE(dir, srcYee.p1);
            b.mBufEHeadPtr[dir] = vp.getE(*itr, dir,
                destYee.p1);
            
            b.mSrcFactorsE[dir] =
                (*itr)->getSourceFactorsE()[dir];
            b.mDestFactorsE[dir] =
                (*itr)->getDestFactorsE()[dir];
            
            LOGMORE << b.mSrcEHeadPtr[dir] << "\n";
            LOGMORE << b.mDestEHeadPtr[dir] << "\n";
            LOGMORE << b.mBufEHeadPtr[dir] << "\n";
        }
        
        LOG << "New H buffers:\n";
        
        for (dir = 0; dir < 3; dir++)
        {
            destYee = halfToYee((*itr)->getDestHalfRect(),
                octantH(dir));
            srcYee = halfToYee((*itr)->getSourceHalfRect(),
                octantH(dir));
            
            b.mDestHHeadPtr[dir] = vp.getH(dir, destYee.p1);
            b.mSrcHHeadPtr[dir] = srcVP->getH(dir, srcYee.p1);
            b.mBufHHeadPtr[dir] = vp.getH(*itr, dir,
                destYee.p1);
            
            b.mSrcFactorsH[dir] =
                (*itr)->getSourceFactorsH()[dir];
            b.mDestFactorsH[dir] =
                (*itr)->getDestFactorsH()[dir];
            
            LOGMORE << b.mSrcHHeadPtr[dir] << "\n";
            LOGMORE << b.mDestHHeadPtr[dir] << "\n";
            LOGMORE << b.mBufHHeadPtr[dir] << "\n";
        }
        
        b.mSourceStride = srcVP->getFieldStride();
        b.mDestStride = vp.getFieldStride();
        
        // this calculation is based on VoxelizedPartition::getFieldStride
        int stride = memBufsE[*itr][0]->getStride();
        Vector3i bufDims = halfToYee((*itr)->getBufferHalfRect())
            .size() + 1;
            
        Vector3i stride3d(stride, stride*bufDims[0],
            stride*bufDims[0]*bufDims[1]);
        if (bufDims[0] == 1)
            stride3d[0] = 0;
        if (bufDims[1] == 1)
            stride3d[1] = 0;
        if (bufDims[2] == 1)
            stride3d[2] = 0;
        b.mStride = stride3d;
        
        LOG << "Buffer stride " << b.mStride << "\n";
        
        Rect3i nbYeeRect = halfToYee((*itr)->getBufferHalfRect());
        b.mNumYeeCells = nbYeeRect.size()+1; 
        mNBs.push_back(b);
    }
}

HuygensSurfacePtr SetupHuygensLink::
makeHuygensSurface() const
{
    return HuygensSurfacePtr(new HuygensLink(mNBs));
}

HuygensLink::
HuygensLink(const vector<LinkNeighborBufferDeleg> & nbs)
{
    //LOG << "Constructor.\n";
    
    unsigned int nn, mm;
    for (nn = 0; nn < nbs.size(); nn++)
    {
        LinkNeighborBuffer b;
        for (mm = 0; mm < 3; mm++)
        {
//            LOG << "nn = " << nn << " mm = " << mm << "\n";
            b.mDestEHeadPtr[mm] = nbs[nn].mDestEHeadPtr[mm].getPointer();
            b.mDestHHeadPtr[mm] = nbs[nn].mDestHHeadPtr[mm].getPointer();
            b.mSrcEHeadPtr[mm] = nbs[nn].mSrcEHeadPtr[mm].getPointer();
            b.mSrcHHeadPtr[mm] = nbs[nn].mSrcHHeadPtr[mm].getPointer();
            b.mBufEHeadPtr[mm] = nbs[nn].mBufEHeadPtr[mm].getPointer();
            b.mBufHHeadPtr[mm] = nbs[nn].mBufHHeadPtr[mm].getPointer();
            /*
            LOG << "E: \n";
            LOG << b.mSrcEHeadPtr[mm] << " is " <<
                MemoryBuffer::identify(b.mSrcEHeadPtr[mm]) << "\n";
            LOG << MemoryBuffer::identify(b.mDestEHeadPtr[mm]) << "\n";
            LOG << MemoryBuffer::identify(b.mBufEHeadPtr[mm]) << "\n";
            LOG << "H: \n";
            LOG << b.mSrcHHeadPtr[mm] << " is " <<
                MemoryBuffer::identify(b.mSrcHHeadPtr[mm]) << "\n";
            LOG << MemoryBuffer::identify(b.mDestHHeadPtr[mm]) << "\n";
            LOG << MemoryBuffer::identify(b.mBufHHeadPtr[mm]) << "\n";
            */
        }
        b.mSourceStride = nbs[nn].mSourceStride;
        b.mDestStride = nbs[nn].mDestStride;
        b.mStride = nbs[nn].mStride;
        b.mNumYeeCells = nbs[nn].mNumYeeCells;
        b.mSrcFactorsE = nbs[nn].mSrcFactorsE;
        b.mSrcFactorsH = nbs[nn].mSrcFactorsH;
        b.mDestFactorsE = nbs[nn].mDestFactorsE;
        b.mDestFactorsH = nbs[nn].mDestFactorsH;
        
        mNBs.push_back(b);
    }
}


void HuygensLink::
updateE()
{
    //LOG << "Update E.\n";
    
    const float *srcPtr, *srcStart2, *srcStart1;
    const float* destPtr, *destStart2, *destStart1;
    float *bufPtr, *bufStart2, *bufStart1;
    
    for (int bufNum = 0; bufNum < mNBs.size(); bufNum++)
    {
        const Vector3i srcStride(mNBs[bufNum].mSourceStride);
        const Vector3i destStride(mNBs[bufNum].mDestStride);
        const Vector3i bufStride(mNBs[bufNum].mStride);
        for (int fieldNum = 0; fieldNum < 3; fieldNum++)
        {
            srcStart2 = mNBs[bufNum].mSrcEHeadPtr[fieldNum];
            destStart2 = mNBs[bufNum].mDestEHeadPtr[fieldNum];
            bufStart2 = mNBs[bufNum].mBufEHeadPtr[fieldNum];
            /*
            LOG << "bufNum = " << bufNum << " fieldNum = " << fieldNum << "\n";
            LOG << srcStart2 << " is " << 
                MemoryBuffer::identify(srcStart2) << "\n";
            LOG << MemoryBuffer::identify(destStart2) << "\n";
            LOG << MemoryBuffer::identify(bufStart2) << "\n";
            */
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
                        //LOG << "src " << *srcPtr << " dest " << *destPtr
                        //    << "\n";
                        
                        /*
                        LOG << MemoryBuffer::identify(srcPtr) << "\n"
                            << MemoryBuffer::identify(destPtr) << "\n"
                            << MemoryBuffer::identify(bufPtr) << "\n";
                        LOGMORE << "src " << srcFactor << " dest " << destFactor
                            << "\n";
                        */
                        
                        *bufPtr = *srcPtr*srcFactor + *destPtr*destFactor;
                        srcPtr += srcStride[0];
                        destPtr += destStride[0];
                        bufPtr += bufStride[0];
                    }
                    srcStart1 += srcStride[1];
                    destStart1 += destStride[1];
                    bufStart1 += bufStride[1];
                }
                srcStart2 += srcStride[2];
                destStart2 += destStride[2];
                bufStart2 += bufStride[2];
            }
        }
    }
}

void HuygensLink::
updateH()
{
    //LOG << "Update H\n";
    const float *srcPtr, *srcStart2, *srcStart1;
    const float* destPtr, *destStart2, *destStart1;
    float *bufPtr, *bufStart2, *bufStart1;
    
    for (int bufNum = 0; bufNum < mNBs.size(); bufNum++)
    {
        const Vector3i srcStride(mNBs[bufNum].mSourceStride);
        const Vector3i destStride(mNBs[bufNum].mDestStride);
        const Vector3i bufStride(mNBs[bufNum].mStride);
        for (int fieldNum = 0; fieldNum < 3; fieldNum++)
        {
            srcStart2 = mNBs[bufNum].mSrcHHeadPtr[fieldNum];
            destStart2 = mNBs[bufNum].mDestHHeadPtr[fieldNum];
            bufStart2 = mNBs[bufNum].mBufHHeadPtr[fieldNum];
            /*
            LOG << "bufNum = " << bufNum << " fieldNum = " << fieldNum << "\n";
            LOG << srcStart2 << " is " <<
                MemoryBuffer::identify(srcStart2) << "\n";
            LOG << MemoryBuffer::identify(destStart2) << "\n";
            LOG << MemoryBuffer::identify(bufStart2) << "\n";
            */
            const float srcFactor = mNBs[bufNum].mSrcFactorsH[fieldNum];
            const float destFactor = mNBs[bufNum].mDestFactorsH[fieldNum];
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
                        //LOG << "src " << *srcPtr << " dest " << *destPtr
                        //    << "\n";
                        
                        /*
                        LOG << MemoryBuffer::identify(srcPtr) << "\n"
                            << MemoryBuffer::identify(destPtr) << "\n"
                            << MemoryBuffer::identify(bufPtr) << "\n";
                        LOGMORE << "src " << srcFactor << " dest " << destFactor
                            << "\n";
                        */
                        
                        *bufPtr = *srcPtr*srcFactor + *destPtr*destFactor;
                        srcPtr += srcStride[0];
                        destPtr += destStride[0];
                        bufPtr += bufStride[0];
                    }
                    srcStart1 += srcStride[1];
                    destStart1 += destStride[1];
                    bufStart1 += bufStride[1];
                }
                srcStart2 += srcStride[2];
                destStart2 += destStride[2];
                bufStart2 += bufStride[2];
            }
        }
    }
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






















