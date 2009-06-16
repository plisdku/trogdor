/*
 *  NeighborBuffer.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _NEIGHBORBUFFER_
#define _NEIGHBORBUFFER_

#include "Pointer.h"
#include "MemoryUtilities.h"
#include <vector>

class NeighborBufferDelegate
{
public:
    NeighborBufferDelegate();
    const std::vector<MemoryBufferPtr> & getBuffers() const { return mBuffers; }
private:
    std::vector<MemoryBufferPtr> mBuffers;
};
typedef Pointer<NeighborBufferDelegate> NeighborBufferDelegatePtr;

class NeighborBuffer
{
public:
    NeighborBuffer(const NeighborBufferDelegate & delegate);
    
    void updateE();
    void updateH();
private:
    float* mHeadPointersE[3];
    float* mHeadPointersH[3];
    Vector3i mStridesE[3];
    Vector3i mStridesH[3];
    Vector3i mNumYeeCells;
};
typedef Pointer<NeighborBuffer> NeighborBufferPtr;




#endif
