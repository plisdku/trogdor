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
    const Map<NeighborBufferDescPtr, vector<MemoryBufferPtr> > & memBufs =
        vp.getNBBuffers();
    const vector<NeighborBufferDescPtr> & buffers = desc->getBuffers();
    GridDescPtr srcGrid = desc->getSourceGrid();
    VoxelizedPartitionPtr srcVP = grids[srcGrid];
    /*
    LOG << "Setup!  Buffer sourcing from " << srcGrid->getName() << "\n";
    LOGMORE << "Membufs size " << memBufs.size() << "\n";
    LOGMORE << "Buffers size " << buffers.size() << "\n";
    */
    //LOGMORE << "Contents of buffers: " << "\n";
    //LOGMORE << buffers << "\n";
    
    for (int nn = 0; nn < 6; nn++)
    if (memBufs.count(buffers[nn]) != 0)
    {
        LinkNeighborBufferDeleg b;
        
        //assert(memBufs.count(buffers[nn]) != 0);
        const vector<MemoryBufferPtr> & mbs = memBufs[buffers[nn]];
        
        for (int fieldDir = 0; fieldDir < 3; fieldDir++)
        {
            Rect3i destYee = rectHalfToYee(buffers[nn]->getDestHalfRect(),
                eOctantNumber(fieldDir));
            Rect3i srcYee = rectHalfToYee(buffers[nn]->getSourceHalfRect(),
                eOctantNumber(fieldDir));
            
            LOG << "E src " << srcYee << " dest " << destYee << "\n";
            
            b.mDestEHeadPtr[fieldDir] = vp.getE(fieldDir, destYee.p1);
            b.mSrcEHeadPtr[fieldDir] = srcVP->getE(fieldDir, srcYee.p1);
            b.mBufEHeadPtr[fieldDir] = vp.getE(buffers[nn], fieldDir,
                destYee.p1);
            
            LOG << "H src " << srcYee << " dest " << destYee << "\n";
            
            destYee = rectHalfToYee(buffers[nn]->getDestHalfRect(),
                hOctantNumber(fieldDir));
            srcYee = rectHalfToYee(buffers[nn]->getSourceHalfRect(),
                hOctantNumber(fieldDir));
            
            b.mDestHHeadPtr[fieldDir] = vp.getH(fieldDir, destYee.p1);
            b.mSrcHHeadPtr[fieldDir] = srcVP->getH(fieldDir, srcYee.p1);
            b.mBufHHeadPtr[fieldDir] = vp.getH(buffers[nn], fieldDir,
                destYee.p1);
            
            b.mSrcFactorsE[fieldDir] = buffers[nn]->getSourceFactors()
                [eOctantNumber(fieldDir)];
            b.mSrcFactorsH[fieldDir] = buffers[nn]->getSourceFactors()
                [hOctantNumber(fieldDir)];
            b.mDestFactorsE[fieldDir] = buffers[nn]->getDestFactors()
                [eOctantNumber(fieldDir)];
            b.mDestFactorsH[fieldDir] = buffers[nn]->getDestFactors()
                [hOctantNumber(fieldDir)];
            /*
            LOG << "What the buffer?\n";
            LOGMORE << b.mSrcEHeadPtr[fieldDir] << "\n";
            LOGMORE << b.mDestEHeadPtr[fieldDir] << "\n";
            LOGMORE << b.mBufEHeadPtr[fieldDir] << "\n";
            LOGMORE << b.mSrcHHeadPtr[fieldDir] << "\n";
            LOGMORE << b.mDestHHeadPtr[fieldDir] << "\n";
            LOGMORE << b.mBufHHeadPtr[fieldDir] << "\n";
            */
        }
        
        b.mSourceStride = srcVP->getFieldStride();
        b.mDestStride = vp.getFieldStride();
        
        Rect3i nbYeeRect = rectHalfToYee(buffers[nn]->getBufferHalfRect());
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
            //LOG << "nn = " << nn << " mm = " << mm << "\n";
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
                        LOG << MemoryBuffer::identify(srcPtr) << "\n"
                            << MemoryBuffer::identify(destPtr) << "\n"
                            << MemoryBuffer::identify(bufPtr) << "\n";
                        LOGMORE << "src " << srcFactor << " dest " << destFactor
                            << "\n";
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
    //LOG << "Update H\n";
    const float *srcPtr, *srcStart2, *srcStart1;
    const float* destPtr, *destStart2, *destStart1;
    float *bufPtr, *bufStart2, *bufStart1;
    
    for (int bufNum = 0; bufNum < mNBs.size(); bufNum++)
    {
        const Vector3i srcStride(mNBs[bufNum].mSourceStride);
        const Vector3i destStride(mNBs[bufNum].mDestStride);
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
                        LOG << MemoryBuffer::identify(srcPtr) << "\n"
                            << MemoryBuffer::identify(destPtr) << "\n"
                            << MemoryBuffer::identify(bufPtr) << "\n";
                        LOGMORE << "src " << srcFactor << " dest " << destFactor
                            << "\n";
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






















