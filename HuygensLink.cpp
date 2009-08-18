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

HuygensLink::
HuygensLink(const HuygensSurface & hs)
{
}


void HuygensLink::
updateE(HuygensSurface & hs)
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
            
            /*
            LOG << "Source " << sourceYeeCells << " dest " << destYeeCells
                << "\n";
            LOG << "Source factor " << srcFactor << "\n";
            */
            float srcField, destField, bufField;
            Vector3i yee;
            
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
                /*
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
                    
                    LOG << "Source field " << srcField << " dest " << destField
                        << " buf field " << bufField << "\n";
                    
                    LOG << "Source " << sourceYeeCells.p1+yee << " dest "
                        << destYeeCells.p1+yee << "\n";
                    if (bufField != *(buf.pointer()))
                        assert(bufField == *(buf.pointer()));
                }
                */
            }
        }
    }
}

void HuygensLink::
updateH(HuygensSurface & hs)
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
            /*
            LOG << "Source " << sourceYeeCells << " dest " << destYeeCells
                << "\n";
            LOG << "Source factor " << srcFactor << "\n";
            */
            float srcField, destField, bufField;
            Vector3i yee;
            
            for (yee[2] = 0; yee[2] < destYeeCells.num(2); yee[2]++)
            for (yee[1] = 0; yee[1] < destYeeCells.num(1); yee[1]++)
            for (yee[0] = 0; yee[0] < destYeeCells.num(0); yee[0]++)
            {
                srcField = sourceLattice->getWrappedH(fieldDirection,
                    yee+sourceYeeCells.p1);
                destField = destLattice->getWrappedH(fieldDirection,
                    yee+destYeeCells.p1);
                bufField = srcFactor*srcField + destFactor*destField;
                
                bufferLattice->setH(fieldDirection,
                    yee+destYeeCells.p1, bufField);
                /*
                if (bufField != 0)
                {
                    BufferPointer buf = bufferLattice->wrappedPointerH(
                        fieldDirection, yee+destYeeCells.p1);
                    BufferPointer src = sourceLattice->wrappedPointerH(
                        fieldDirection, yee+sourceYeeCells.p1);
                    BufferPointer dest = destLattice->wrappedPointerH(
                        fieldDirection, yee+destYeeCells.p1);
                    LOG << MemoryBuffer::identify(buf.pointer()) << " is "
                        << *(buf.pointer()) << "\n";
                    LOG << MemoryBuffer::identify(src.pointer()) << " is "
                        << *(src.pointer()) << "\n";
                    LOG << MemoryBuffer::identify(dest.pointer()) << " is "
                        << *(dest.pointer()) << "\n";
                    
                    LOG << "Source " << sourceYeeCells.p1+yee << " dest "
                        << destYeeCells.p1+yee << "\n";
                }
                */
            }
        }
    }
}















