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

/**
 * A rectangular (1D, 2D or 3D) Yee lattice, including both electric (E) and
 * magnetic (H) fields.  The interleaved lattice provides a means to get and set
 * field values; the fields are indexed either by Yee cell or half cell.  On
 * construction the lattice does not reserve storage for the fields, but
 * {@link BufferPointer}s are immediately available and will point to the
 * correct E and H field values after the allocate() method is called.
 *
 * @author Paul C. Hansen
 * @see MemoryBuffer, BufferPointer
 */
class InterleavedLattice
{
public:
    /**
     * Initializes a lattice but does not allocate E and H fields yet.
     * 
     * @param bufferNamePrefix "Surname" for the E and H MemoryBuffers
     * @param halfCellBounds Extent in world coordinates of the lattice
     * @param runlineDirection Determines memory ordering; allowed values are
     *  0, 1 and 2 for x, y and z
     * @param nonZeroDimensions Set this to zero along unused directions for 1D
     *  or 2D grids
     */
    InterleavedLattice(const std::string & bufferNamePrefix,
        Rect3i halfCellBounds, int runlineDirection = 0);
    
    /**
     * @deprecated
     * Determine whether the grid is 1D, 2D or 3D, and which dimensions are
     * used.
     *
     * @returns a vector containing 1s (dimension used) and 0s (dimension not 
     *  used)
     */
    Vector3i nonZeroDimensions() const { return mNonZeroDimensions; }
    
    /**
     * Get the bounds of the grid in world coordinates
     * @returns grid bounds in half cells
     */
    const Rect3i & halfCells() const { return mHalfCells; }
    
    /**
     * Get the number of half cells along each dimension.  Because the grid
     * spans an integer number of Yee cells, the returned dimensions will all
     * be even.
     * @returns extent of the grid in half cells along x, y and z
     */
    const Vector3i & numHalfCells() const { return mNumHalfCells; }
    /**
     * Get the number of Yee cells along each dimension.
     * @returns eextent of the grid in Yee cells along x, y and z
     */ 
    const Vector3i & numYeeCells() const { return mNumYeeCells; }
    
    // On all nonzero dimensions, halfCell is wrapped to the interior.
    // The component in the null dimension is unchanged.
    
    /**
     * Treating the grid as periodic in all directions, return the (unique)
     * point inside the grid which is \f$N\f$
     * numHalfCells() away from halfCell for some integer-valued vector
     * \f$N\f$.
     */
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
    int runlineDirection() const { return mRunlineDirection; }
    
    // Access to fields (once allocated)
    void allocate();
    
    float getE(int direction, const Vector3i & yeeCell) const;
    float getH(int direction, const Vector3i & yeeCell) const;
    float getWrappedE(int direction, const Vector3i & yeeCell) const;
    float getWrappedH(int direction, const Vector3i & yeeCell) const;
    float getInterpolatedE(int direction, const Vector3f & position) const;
    float getInterpolatedH(int direction, const Vector3f & position) const;
    
    void setE(int direction, const Vector3i & yeeCell, float value);
    void setH(int direction, const Vector3i & yeeCell, float value);
    
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
    int mRunlineDirection; // 0, 1 or 2
    std::vector<float> mData;
};
typedef Pointer<InterleavedLattice> InterleavedLatticePtr;



#endif
