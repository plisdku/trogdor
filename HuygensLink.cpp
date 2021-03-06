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
#include "CalculationPartition.h"
#include "SimulationDescription.h"
#include "YeeUtilities.h"
#include "Log.h"

using namespace YeeUtilities;
using namespace std;

HuygensLink::
HuygensLink(const HuygensSurface & hs)
{
}

void HuygensLink::
updateE(HuygensSurface & hs, CalculationPartition & cp, long timestep)
{
    //LOG << "Update E.\n";
    const std::vector<NeighborBufferPtr> & nbs(hs.neighborBuffers());
    InterleavedLatticePtr destLattice(hs.destLattice());
    InterleavedLatticePtr sourceLattice(hs.sourceLattice());
    
    for (int bufNum = 0; bufNum < nbs.size(); bufNum++)
    if (nbs.at(bufNum) != 0L)
    {
        for (int fieldDirection = 0; fieldDirection < 3; fieldDirection++)
        {
            InterleavedLatticePtr bufferLattice = nbs[bufNum]->lattice();
            // other lattices: mDestLattice, mSourceLattice
            
            Rect3i destYeeCells = halfToYee(nbs[bufNum]->destHalfCells(),
                octantE(fieldDirection));
            Rect3i sourceYeeCells = halfToYee(nbs[bufNum]->
                sourceHalfCells(), octantE(fieldDirection));
            
            const float srcFactor =
                nbs[bufNum]->sourceFactorE(fieldDirection);
            const float destFactor =
                nbs[bufNum]->destFactorE(fieldDirection);
            
            Vector3i srcStride = hs.sourceLattice()->fieldStride();
            Vector3i bufStride = bufferLattice->fieldStride();
            Vector3i destStride = hs.destLattice()->fieldStride();
            
            BufferPointer srcp = hs.sourceLattice()->wrappedPointerE(
                fieldDirection, sourceYeeCells.p1);
            BufferPointer destp = hs.destLattice()->wrappedPointerE(
                fieldDirection, destYeeCells.p1);
            BufferPointer bufp = bufferLattice->wrappedPointerE(
                fieldDirection, destYeeCells.p1);
            
            float *psrc, *pdest, *pbuf;
            psrc = srcp.pointer();
            pdest = destp.pointer();
            pbuf = bufp.pointer();
            
            float srcField, destField, bufField;
            Vector3i yee;
            
            for (yee[2] = 0; yee[2] < destYeeCells.num(2); yee[2]++)
            {
                float* srcy = psrc;
                float* desty = pdest;
                float* bufy = pbuf;
                for (yee[1] = 0; yee[1] < destYeeCells.num(1); yee[1]++)
                {
                    float* srcx = srcy;
                    float* destx = desty;
                    float* bufx = bufy;
                    for (yee[0] = 0; yee[0] < destYeeCells.num(0); yee[0]++)
                    {
                        srcField = *srcx;
                        destField = *destx;
                        bufField = srcFactor*srcField + destFactor*destField;
                        /*
                        float testSrc = sourceLattice->getWrappedE(
                            fieldDirection, yee+sourceYeeCells.p1);
                        float testDest = destLattice->getWrappedE(
                            fieldDirection, yee+destYeeCells.p1);
                        
                        assert(srcField == testSrc);
                        assert(destField == testDest);
                        */
                        
                        *bufx = bufField;
                        
                        srcx += srcStride[0];
                        bufx += bufStride[0];
                        destx += destStride[0];
                    }
                    srcy += srcStride[1];
                    bufy += bufStride[1];
                    desty += destStride[1];
                }
                psrc += srcStride[2];
                pdest += destStride[2];
                pbuf += bufStride[2];
            }
            
            /*
            LOG << "Source " << sourceYeeCells << " dest " << destYeeCells
                << "\n";
            LOG << "Source factor " << srcFactor << "\n";
            */
            
            /*
            for (yee[2] = 0; yee[2] < destYeeCells.num(2); yee[2]++)
            for (yee[1] = 0; yee[1] < destYeeCells.num(1); yee[1]++)
            for (yee[0] = 0; yee[0] < destYeeCells.num(0); yee[0]++)
            {
                srcField = sourceLattice->getWrappedE(fieldDirection,
                    yee+sourceYeeCells.p1);
                destField = destLattice->getWrappedE(fieldDirection,
                    yee+destYeeCells.p1);
                bufField = srcFactor*srcField + destFactor*destField;
                
                bufferLattice->setE(fieldDirection,
                    yee+destYeeCells.p1, bufField);
                
                const int TESTME = 0;
                if (TESTME)
                {
                    float tempval = bufferLattice->getWrappedE(fieldDirection,
                        yee+destYeeCells.p1);
                        
                    assert(tempval == bufField);
                        
                    if (bufField != 0)
                    {
                        BufferPointer buf = bufferLattice->wrappedPointerE(
                            fieldDirection, yee+destYeeCells.p1);
                        BufferPointer src = sourceLattice->wrappedPointerE(
                            fieldDirection, yee+sourceYeeCells.p1);
                        BufferPointer dest = destLattice->wrappedPointerE(
                            fieldDirection, yee+destYeeCells.p1);
                        LOG << MemoryBuffer::identify(buf.pointer()) << " is "
                            << *(buf.pointer()) << "\n";
                        LOG << MemoryBuffer::identify(src.pointer()) << " is "
                            << *(src.pointer()) << "\n";
                        LOG << MemoryBuffer::identify(dest.pointer()) << " is "
                            << *(dest.pointer()) << "\n";
                        
                        LOG << "Source field " << srcField << " dest " <<
                            destField << " buf field " << bufField << "\n";
                        
                        LOG << "Source " << sourceYeeCells.p1+yee << " dest "
                            << destYeeCells.p1+yee << "\n";
                        if (bufField != *(buf.pointer()))
                            assert(bufField == *(buf.pointer()));
                    }
                }
            }
            */
        }
    }
}

void HuygensLink::
updateH(HuygensSurface & hs, CalculationPartition & cp, long timestep)
{
    //LOG << "Update H.\n";
    const std::vector<NeighborBufferPtr> & nbs(hs.neighborBuffers());
    InterleavedLatticePtr destLattice(hs.destLattice());
    InterleavedLatticePtr sourceLattice(hs.sourceLattice());
    
    for (int bufNum = 0; bufNum < nbs.size(); bufNum++)
    if (nbs.at(bufNum) != 0L)
    {
        for (int fieldDirection = 0; fieldDirection < 3; fieldDirection++)
        {
            InterleavedLatticePtr bufferLattice = nbs[bufNum]->lattice();
            // other lattices: mDestLattice, mSourceLattice
            
            Rect3i destYeeCells = halfToYee(nbs[bufNum]->destHalfCells(),
                octantH(fieldDirection));
            Rect3i sourceYeeCells = halfToYee(nbs[bufNum]->
                sourceHalfCells(), octantH(fieldDirection));
            
            const float srcFactor =
                nbs[bufNum]->sourceFactorH(fieldDirection);
            const float destFactor =
                nbs[bufNum]->destFactorH(fieldDirection);
            
            Vector3i srcStride = hs.sourceLattice()->fieldStride();
            Vector3i bufStride = bufferLattice->fieldStride();
            Vector3i destStride = hs.destLattice()->fieldStride();
            
            BufferPointer srcp = hs.sourceLattice()->wrappedPointerH(
                fieldDirection, sourceYeeCells.p1);
            BufferPointer destp = hs.destLattice()->wrappedPointerH(
                fieldDirection, destYeeCells.p1);
            BufferPointer bufp = bufferLattice->wrappedPointerH(
                fieldDirection, destYeeCells.p1);
            
            float *psrc, *pdest, *pbuf;
            psrc = srcp.pointer();
            pdest = destp.pointer();
            pbuf = bufp.pointer();
            
            float srcField, destField, bufField;
            Vector3i yee;
            
            for (yee[2] = 0; yee[2] < destYeeCells.num(2); yee[2]++)
            {
                float* srcy = psrc;
                float* desty = pdest;
                float* bufy = pbuf;
                for (yee[1] = 0; yee[1] < destYeeCells.num(1); yee[1]++)
                {
                    float* srcx = srcy;
                    float* destx = desty;
                    float* bufx = bufy;
                    for (yee[0] = 0; yee[0] < destYeeCells.num(0); yee[0]++)
                    {
                        srcField = *srcx;
                        destField = *destx;
                        bufField = srcFactor*srcField + destFactor*destField;
                        
                        *bufx = bufField;
                        
                        srcx += srcStride[0];
                        bufx += bufStride[0];
                        destx += destStride[0];
                    }
                    srcy += srcStride[1];
                    bufy += bufStride[1];
                    desty += destStride[1];
                }
                psrc += srcStride[2];
                pdest += destStride[2];
                pbuf += bufStride[2];
            }
        }
    }
}















