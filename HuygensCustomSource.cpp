/*
 *  HuygensCustomSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 10/20/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "HuygensCustomSource.h"
#include "CalculationPartition.h"
#include "VoxelizedPartition.h"
#include "SimulationDescription.h"
#include "YeeUtilities.h"
#include "Log.h"

using namespace YeeUtilities;
using namespace std;

HuygensCustomSource::
HuygensCustomSource(const HuygensSurface & hs) :
    mDescription(hs.description()),
    mFieldInput(hs.description()),
    mDuration(hs.description()->duration())
{
}

void HuygensCustomSource::
updateE(HuygensSurface & hs, CalculationPartition & cp, long timestep)
{
    LOG << "Update E.\n";
    
    mFieldInput.startHalfTimestepE(timestep, cp.dt()*timestep);
    
    const std::vector<NeighborBufferPtr> & nbs(hs.neighborBuffers());
    InterleavedLatticePtr destLattice(hs.destLattice());
    
    for (int fieldDirection = 0; fieldDirection < 3; fieldDirection++)
    {
        mFieldInput.restartMaskPointer(fieldDirection);
        for (int bufNum = 0; bufNum < nbs.size(); bufNum++)
        if (nbs.at(bufNum) != 0L)
        {
            InterleavedLatticePtr bufferLattice = nbs[bufNum]->lattice();
            
            Rect3i destYeeCells = halfToYee(nbs[bufNum]->destHalfCells(),
                octantE(fieldDirection));
            Rect3i sourceYeeCells = halfToYee(nbs[bufNum]->
                sourceHalfCells(), octantE(fieldDirection));
            
            const float srcFactor =
                nbs[bufNum]->sourceFactorE(fieldDirection);
            const float destFactor =
                nbs[bufNum]->destFactorE(fieldDirection);
            
            Vector3i bufStride = bufferLattice->fieldStride();
            Vector3i destStride = hs.destLattice()->fieldStride();
            
            BufferPointer destp = hs.destLattice()->wrappedPointerE(
                fieldDirection, destYeeCells.p1);
            BufferPointer bufp = bufferLattice->wrappedPointerE(
                fieldDirection, destYeeCells.p1);
            
            float *pdest, *pbuf;
            pdest = destp.pointer();
            pbuf = bufp.pointer();
            
            float srcField, destField, bufField;
            Vector3i yee;
            
            for (yee[2] = 0; yee[2] < destYeeCells.num(2); yee[2]++)
            {
                float* desty = pdest;
                float* bufy = pbuf;
                for (yee[1] = 0; yee[1] < destYeeCells.num(1); yee[1]++)
                {
                    float* destx = desty;
                    float* bufx = bufy;
                    for (yee[0] = 0; yee[0] < destYeeCells.num(0); yee[0]++)
                    {
                        srcField = mFieldInput.getFieldE(fieldDirection);
                        destField = *destx;
                        bufField = srcFactor*srcField + destFactor*destField;
                        *bufx = bufField;
                        
                        bufx += bufStride[0];
                        destx += destStride[0];
                    }
                    bufy += bufStride[1];
                    desty += destStride[1];
                }
                pdest += destStride[2];
                pbuf += bufStride[2];
            }
        }
    }
}

void HuygensCustomSource::
updateH(HuygensSurface & hs, CalculationPartition & cp, long timestep)
{
    LOG << "Update H.\n";
    
    mFieldInput.startHalfTimestepH(timestep, cp.dt()*(timestep+0.5));
    
    const std::vector<NeighborBufferPtr> & nbs(hs.neighborBuffers());
    InterleavedLatticePtr destLattice(hs.destLattice());
    
    for (int fieldDirection = 0; fieldDirection < 3; fieldDirection++)
    {
        mFieldInput.restartMaskPointer(fieldDirection);
        for (int bufNum = 0; bufNum < nbs.size(); bufNum++)
        if (nbs.at(bufNum) != 0L)
        {
            InterleavedLatticePtr bufferLattice = nbs[bufNum]->lattice();
            
            Rect3i destYeeCells = halfToYee(nbs[bufNum]->destHalfCells(),
                octantH(fieldDirection));
            Rect3i sourceYeeCells = halfToYee(nbs[bufNum]->
                sourceHalfCells(), octantH(fieldDirection));
            
            const float srcFactor =
                nbs[bufNum]->sourceFactorH(fieldDirection);
            const float destFactor =
                nbs[bufNum]->destFactorH(fieldDirection);
            
            Vector3i bufStride = bufferLattice->fieldStride();
            Vector3i destStride = hs.destLattice()->fieldStride();
            
            BufferPointer destp = hs.destLattice()->wrappedPointerH(
                fieldDirection, destYeeCells.p1);
            BufferPointer bufp = bufferLattice->wrappedPointerH(
                fieldDirection, destYeeCells.p1);
            
            float *pdest, *pbuf;
            pdest = destp.pointer();
            pbuf = bufp.pointer();
            
            float srcField, destField, bufField;
            Vector3i yee;
            
            for (yee[2] = 0; yee[2] < destYeeCells.num(2); yee[2]++)
            {
                float* desty = pdest;
                float* bufy = pbuf;
                for (yee[1] = 0; yee[1] < destYeeCells.num(1); yee[1]++)
                {
                    float* destx = desty;
                    float* bufx = bufy;
                    for (yee[0] = 0; yee[0] < destYeeCells.num(0); yee[0]++)
                    {
                        srcField = mFieldInput.getFieldH(fieldDirection);
                        destField = *destx;
                        bufField = srcFactor*srcField + destFactor*destField;
                        *bufx = bufField;
                        
                        bufx += bufStride[0];
                        destx += destStride[0];
                    }
                    bufy += bufStride[1];
                    desty += destStride[1];
                }
                pdest += destStride[2];
                pbuf += bufStride[2];
            }
        }
    }
}
