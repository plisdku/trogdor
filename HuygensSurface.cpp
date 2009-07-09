/*
 *  HuygensSurface.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "HuygensSurface.h"
#include "VoxelizedPartition.h"
#include "SimulationDescription.h"
#include "Exception.h"

#include "HuygensLink.h"

using namespace std;

SetupHuygensSurfacePtr HuygensSurfaceFactory::
newSetupHuygensSurface(const VoxelizedPartition & vp,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
    const HuygensSurfaceDescPtr & desc)
{
    SetupHuygensSurfacePtr hs;
    
    if (desc->getType() == kLink)
    {
        hs = SetupHuygensSurfacePtr(new SetupHuygensLink(
            vp, grids, desc));
    }
    /*
    else if (desc->getType() == kTFSFSource)
    {
    }
    else if (desc->getType() == kCustomTFSFSource)
    {
    }
    */
    else
        throw(Exception("Unknown HuygensSurface type!"));
    
    return hs;
}

SetupHuygensSurface::
SetupHuygensSurface()
    //mNeighborBuffers(6)
{
}

HuygensSurface::
HuygensSurface()
    //mNeighborBuffers(6)
{
}
/*
SetupNeighborBuffer::
SetupNeighborBuffer(int side) 
{
    assert(side >= 0 && side <= 5);
    mInfo.side = side;
    mInfo.nonZeroDimensions = Vector3i(1,1,1);
    mInfo.nonZeroDimensions[side/2] = 0;
}


BufferPointer SetupNeighborBuffer::
getE(int fieldDirection, const Vector3i & yeeCell) const
{
    Vector3i p1Yee = halfToYee(mInfo.destHalfRect.p1);
    Vector3i displacement = yeeCell - p1Yee;
    displacement[mInfo.side/2] = 0;
    
    int index = displacement[0] + mInfo.numYeeCells[0]*displacement[1]
        + mInfo.numYeeCells[0]*mInfo.numYeeCells[1]*displacement[2];
    
    return BufferPointer(*mInfo.buffersE[fieldDirection], index);
}


BufferPointer SetupNeighborBuffer::
getH(int fieldDirection, const Vector3i & yeeCell) const
{
    Vector3i p1Yee = halfToYee(mInfo.destHalfRect.p1);
    Vector3i displacement = yeeCell - p1Yee;
    displacement[mInfo.side/2] = 0;
    
    int index = displacement[0] + mInfo.numYeeCells[0]*displacement[1]
        + mInfo.numYeeCells[0]*mInfo.numYeeCells[1]*displacement[2];
    
    return BufferPointer(*mInfo.buffersH[fieldDirection], index);
}


NeighborBuffer::
NeighborBuffer(const SetupNeighborBuffer & setupNB) :
    mInfo(setupNB.getInfo())
{
    mFields.resize(6*mInfo.buffersE[0]->
}


float NeighborBuffer::
getE(int fieldDirection, const Vector3i & yeeCell) const
{
    Vector3i p1Yee = halfToYee(mInfo.destHalfRect.p1);
    Vector3i displacement(yeeCell-p1Yee);
    displacement[mInfo.side/2] = 0;
    int index = dot(displacement, mMemStride);
    
    return mHeadE[fieldDirection][index];
}

float NeighborBuffer::
getH(int fieldDirection, const Vector3i & yeeCell) const
{
    Vector3i p1Yee = halfToYee(mInfo.destHalfRect.p1);
    Vector3i displacement(yeeCell-p1Yee);
    displacement[mInfo.side/2] = 0;
    int index = dot(displacement, mMemStride);
    
    return mHeadH[fieldDirection][index];
}

void NeighborBuffer::
setE(int fieldDirection, const Vector3i & yeeCell, float value)
{
    Vector3i p1Yee = halfToYee(mInfo.destHalfRect.p1);
    Vector3i displacement(yeeCell-p1Yee);
    displacement[mInfo.side/2] = 0;
    int index = dot(displacement, mMemStride);
    mHeadE[fieldDirection][index] = value;
}

void NeighborBuffer::
setH(int fieldDirection, const Vector3i & yeeCell, float value)
{
    Vector3i p1Yee = halfToYee(mInfo.destHalfRect.p1);
    Vector3i displacement(yeeCell-p1Yee);
    displacement[mInfo.side/2] = 0;
    int index = dot(displacement, mMemStride);
    mHeadH[fieldDirection][index] = value;
}
*/


