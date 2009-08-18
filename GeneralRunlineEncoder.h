/*
 *  GeneralRunlineEncoder.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _GENERALRUNLINEENCODER_
#define _GENERALRUNLINEENCODER_

#include "geometry.h"

class VoxelizedPartition;
class Paint;

class GeneralRunlineEncoder
{
public:
    GeneralRunlineEncoder();
    virtual ~GeneralRunlineEncoder();
    
    // Setup.  Set all the flags...
    void setRunlineDirection(int runlineDirection)
        { mRunlineDirection = runlineDirection; }
    
    // Accessors for the current runline's info
    const Vector3i & firstHalfCell() const
        { return mFirstHalfCell; }
    const Vector3i & lastHalfCell() const
        { return mLastHalfCell; }
    long length() const
        { return mLength; }
    
    // Things called by the... iterator on the grid, I guess it's called.
    void startRunline(const VoxelizedPartition & vp,
        const Vector3i & beginningHalfCell);
    bool canContinueRunline(const VoxelizedPartition & vp,
        const Vector3i & oldHalfCell,
        const Vector3i & newHalfCell,
        const Paint* newPaint) const;
    void continueRunline();
    
    // Hey user, you can implement this to make it do your bidding!
    virtual void endRunline();
private:
    Vector3i mFirstHalfCell;
    Vector3i mLastHalfCell;
    long mLength;
    int mRunlineDirection;
};





#endif
