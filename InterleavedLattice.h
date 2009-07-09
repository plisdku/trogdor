/*
 *  InterleavedLattice.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/8/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _INTERLEAVEDLATTICE_
#define _INTERLEAVEDLATTICE_

#include "MemoryUtilities.h"
#include "geometry.h"
#include <vector>
#include <iostream>
#include <string>

class InterleavedLattice
{
public:
    InterleavedLattice(const std::string & bufferNamePrefix,
        Rect3i halfCellBounds, Vector3i nonZeroDimensions = Vector3i(1,1,1));
    
    Vector3i nonZeroDimensions() const { return mNonZeroDimensions; }
    
    const Rect3i & halfCells() const { return mHalfCells; }
    
    // On all nonzero dimensions, halfCell is wrapped to the interior.
    // The component in the null dimension is unchanged.
    Vector3i wrap(const Vector3i & halfCell) const;
    long linearYeeIndex(const Vector3i & halfCell) const;
    
    // Access to allocation skeleton
    BufferPointer pointer(const Vector3i & halfCell) const;
    BufferPointer wrappedPointer(const Vector3i & halfCell) const;
    
    BufferPointer pointerE(int fieldDirection, const Vector3i & yeeCell) const;
    BufferPointer pointerH(int fieldDirection, const Vector3i & yeeCell) const;
    
    BufferPointer wrappedPointerE(int fieldDirection, const Vector3i & yeeCell)
        const;
    BufferPointer wrappedPointerH(int fieldDirection, const Vector3i & yeeCell)
        const;
    
    Vector3i fieldStride() const { return mMemStride; }
    
    // Access to fields (once allocated)
    void allocate();
    
    float getE(int direction, const Vector3i & yeeCell) const;
    float getH(int direction, const Vector3i & yeeCell) const;
    float getWrappedE(int direction, const Vector3i & yeeCell) const;
    float getWrappedH(int direction, const Vector3i & yeeCell) const;
    
    float setE(int direction, const Vector3i & yeeCell, float value);
    float setH(int direction, const Vector3i & yeeCell, float value);
    
    void printE(std::ostream & str, int fieldDirection, float scale) const;
    void printH(std::ostream & str, int fieldDirection, float scale) const;
    
private:
    Rect3i mHalfCells;
    Vector3i mNonZeroDimensions;
    Vector3i mNumYeeCells;    
    Vector3i mNumHalfCells; // == 2*mNumYeeCells
    Vector3i mAllocOriginYeeE[3]; // cached halfToYee(mHalfCells.p1, octant)
    Vector3i mAllocOriginYeeH[3];
    
    std::vector<MemoryBufferPtr> mBuffersE;
    std::vector<MemoryBufferPtr> mBuffersH;
    
    float* mHeadE[3];
    float* mHeadH[3];
    Vector3i mMemStride;
    std::vector<float> mData;
};




#endif
