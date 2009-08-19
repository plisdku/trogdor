/*
 *  RunlineEncoder.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "RunlineEncoder.h"

#include "VoxelizedPartition.h"
#include "Paint.h"
#include "YeeUtilities.h"
#include "Log.h"

using namespace std;
using namespace YeeUtilities;

RunlineEncoder::
RunlineEncoder() :
    mRunlineDirection(0),
    mFieldContinuity(kNeighborFieldNone),
    mAuxContinuity(kNeighborAuxiliaryNone),
    mMatContinuity(kMaterialContinuityNone),
    mLineContinuity(kLineContinuityNone)
{
}

RunlineEncoder::
~RunlineEncoder()
{
}

// Things called by the... iterator on the grid, I guess it's called.
void RunlineEncoder::
startRunline(const VoxelizedPartition & vp, const Vector3i & beginningHalfCell)
{
    mFirstHalfCell = beginningHalfCell;
    mOctant = octant(beginningHalfCell);
    mFieldDirection = xyz(mOctant);
    mLength = 1;
    mRunlineDirection = vp.lattice().runlineDirection();
    mFirstPaint = vp.voxels()(beginningHalfCell);
    
    initFirstNeighborFields(vp, beginningHalfCell);
    initFirstNeighborAuxiliaryIndices(vp, beginningHalfCell);
}

bool RunlineEncoder::
canContinueRunline(const VoxelizedPartition & vp,
    const Vector3i & newHalfCell,
    const Paint* newPaint) const
{
    bool canContinue = (
        neighborFieldContinuityOK(vp, newHalfCell, newPaint) &&
        neighborAuxContinuityOK(vp, newHalfCell, newPaint) &&
        materialContinuityOK(vp, newHalfCell, newPaint) &&
        lineContinuityOK(vp, newHalfCell, newPaint)
        );
    
    return canContinue;
}

void RunlineEncoder::
continueRunline()
{
    mLength++;
}

// Hey user, you can implement this to make it do your bidding!
void RunlineEncoder::
endRunline(const VoxelizedPartition & vp)
{
    LOG << "Ending runline, doing nothing.\n";
}




void RunlineEncoder::
initFirstNeighborFields(const VoxelizedPartition & vp,
    const Vector3i & firstHalfCell)
{
    int side;
    
    // Set the used neighbor indices according to mFieldContinuity.
    for (side = 0; side < 6; side++)
    {
        if (mFieldContinuity == kNeighborFieldNone)
            mUsedNeighborFields[side] = 0;
        else if (mFieldContinuity == kNeighborFieldTransverse4)
            mUsedNeighborFields[side] = (side/2 != mFieldDirection);
        else if (mFieldContinuity == kNeighborFieldNearby6)
            mUsedNeighborFields[side] = 1;
        else
        {
            cerr << "Error: unsupported NeighborFieldContinuity "
                << mFieldContinuity << ".\n";
            assert(!"This is a problem.");
            exit(1);
        }
    }
    
    // Set the neighbor pointers.  This includes the unused ones; it's
    // just easy that way.
    for (side = 0; side < 6; side++)
    {
        if (mFirstPaint->hasCurlBuffer(side))
        {
            mFirstNeighborFields[side] = mFirstPaint->curlBuffer(side)->
                lattice()->wrappedPointer(mFirstHalfCell+cardinal(side));
        }
        else
        {
            mFirstNeighborFields[side] = vp.lattice().wrappedPointer(
                firstHalfCell+cardinal(side));
        }
    }
}

void RunlineEncoder::
initFirstNeighborAuxiliaryIndices(const VoxelizedPartition & vp,
    const Vector3i & beginningHalfCell)
{
    int side;
    
    for (side = 0; side < 6; side++)
    {
        if (mAuxContinuity == kNeighborAuxiliaryNone)
            mUsedNeighborAuxIndices[side] = 0;
        else if (mAuxContinuity == kNeighborAuxiliaryTransverse4)
            mUsedNeighborAuxIndices[side] = (side/2 != mFieldDirection);
        else if (mAuxContinuity == kNeighborAuxiliaryNearby6)
            mUsedNeighborAuxIndices[side] = 1;
        else
        {
            cerr << "Error: unsupported NeighborAuxiliaryContinuity "
                << mAuxContinuity << ".\n";
            assert(!"This is a problem.");
            exit(1);
        }
    }
    
    for (side = 0; side < 6; side++)
    {
        mFirstNeighborAuxIndices[side] =
            vp.indices()(beginningHalfCell+cardinal(side));
    }
}


bool RunlineEncoder::
neighborFieldContinuityOK(const VoxelizedPartition & vp,
    const Vector3i & newHalfCell, const Paint* newPaint) const
{
    if (mFieldContinuity == kNeighborFieldNone)
        return 1;
    
    BufferPointer neighbor;
    for (int side = 0; side < 6; side++)
    if (mUsedNeighborFields[side])
    {
        neighbor = vp.lattice().wrappedPointer(newHalfCell+cardinal(side));
        if (neighbor.buffer() != mFirstNeighborFields[side].buffer())
            return 0;
        if (mFirstNeighborFields[side].offset() + mLength != neighbor.offset())
            return 0;
    }
    
    return 1;
}


bool RunlineEncoder::
neighborAuxContinuityOK(const VoxelizedPartition & vp,
    const Vector3i & newHalfCell, const Paint* newPaint) const
{
    if (mAuxContinuity == kNeighborAuxiliaryNone)
        return 1;
    
    long neighbor;
    for (int side = 0; side < 6; side++)
    if (mUsedNeighborAuxIndices[side])
    {
        neighbor = vp.indices()(newHalfCell+cardinal(side));
        if (mFirstNeighborAuxIndices[side] + mLength != neighbor)
            return 0;
    }
    
    return 1;
}


bool RunlineEncoder::
materialContinuityOK(const VoxelizedPartition & vp,
    const Vector3i & newHalfCell, const Paint* newPaint) const
{
    if (mMatContinuity == kMaterialContinuityNone)
        return 1;
    
    return newPaint->withoutCurlBuffers() == mFirstPaint->withoutCurlBuffers();
}


bool RunlineEncoder::
lineContinuityOK(const VoxelizedPartition & vp,
    const Vector3i & newHalfCell, const Paint* newPaint) const
{
    if (mLineContinuity == kLineContinuityNone)
        return 1;
    
    const int transverseDir1 = (mRunlineDirection+1)%3;
    const int transverseDir2 = (transverseDir1+1)%3;
    
    return mFirstHalfCell[transverseDir1] == newHalfCell[transverseDir1] &&
        mFirstHalfCell[transverseDir2] == newHalfCell[transverseDir2];
}




