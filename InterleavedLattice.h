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
#include "Pointer.h"

class InterleavedLattice
{
public:
    InterleavedLattice(const std::string & bufferNamePrefix,
        Rect3i halfCellBounds, Vector3i nonZeroDimensions = Vector3i(1,1,1));
    
    Vector3i nonZeroDimensions() const { return mNonZeroDimensions; }
    
    const Rect3i & halfCells() const { return mHalfCells; }
    const Vector3i & numHalfCells() const { return mNumHalfCells; }
    const Vector3i & numYeeCells() const { return mNumYeeCells; }
    
    // On all nonzero dimensions, halfCell is wrapped to the interior.
    // The component in the null dimension is unchanged.
    Vector3i wrap(const Vector3i & halfCell) const;
    long linearYeeIndex(const Vector3i & halfCell) const;
    long wrappedLinearYeeIndex(const Vector3i & halfCell) const;
    
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
    float getInterpolatedE(int direction, const Vector3f & position) const;
    float getInterpolatedH(int direction, const Vector3f & position) const;
    
    float setE(int direction, const Vector3i & yeeCell, float value);
    float setH(int direction, const Vector3i & yeeCell, float value);
    
    void printE(std::ostream & str, int fieldDirection, float scale) const;
    void printH(std::ostream & str, int fieldDirection, float scale) const;
    
private:
    static const int STRIDE = 1;
    
    Rect3i mHalfCells;
    Vector3i mNonZeroDimensions;
    Vector3i mNumYeeCells;
    Vector3i mNumHalfCells; // == 2*mNumYeeCells
    Vector3i mOriginYeeE[3]; // cached halfToYee(mHalfCells.p1, octant)
    Vector3i mOriginYeeH[3];
    
    std::vector<MemoryBufferPtr> mBuffersE; // 3
    std::vector<MemoryBufferPtr> mBuffersH; // 3
    std::vector<MemoryBufferPtr> mOctantBuffers; // 8
    
    bool mFieldsAreAllocated;
    float* mHeadE[3];
    float* mHeadH[3];
    Vector3i mMemStride;
    std::vector<float> mData;
};
typedef Pointer<InterleavedLattice> InterleavedLatticePtr;



#endif
