/*
 *  InterleavedLattice.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/8/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "InterleavedLattice.h"
#include "YeeUtilities.h"
#include "Exception.h"

using namespace std;
using namespace YeeUtilities;


InterleavedLattice::
InterleavedLattice(const string & bufferNamePrefix, Rect3i halfCellBounds,
    int runlineDirection) :
    mHalfCells(halfCellBounds),
    mNonZeroDimensions(1,1,1),
    mBuffersE(3),
    mBuffersH(3),
    mOctantBuffers(8),
    mFieldsAreAllocated(0),
    mRunlineDirection(runlineDirection)
{
    if (mHalfCells.num()%2 != Vector3i(0,0,0))
        throw(Exception("Interleaved lattice must span full Yee cells!"));
    mNumYeeCells = mHalfCells.num()/2;
    mNumHalfCells = mHalfCells.num();
    
    assert(runlineDirection >= 0 && runlineDirection < 3);
    
    //LOG << "Rect " << halfCellBounds << " yee " << mNumYeeCells << "\n";
    
    // Cache the origins.  This saves cycles in all the accessor functions.
    for (int xyz = 0; xyz < 3; xyz++)
    {
        mOriginYeeE[xyz] = halfToYee(mHalfCells, octantE(xyz)).p1;
        mOriginYeeH[xyz] = halfToYee(mHalfCells, octantH(xyz)).p1;
    }
    
    // Create the buffers
    int bufferSize = mNumYeeCells[0]*mNumYeeCells[1]*mNumYeeCells[2];
    for (int xyz = 0; xyz < 3; xyz++)
    {
        mBuffersE[xyz] = MemoryBufferPtr(new MemoryBuffer(
            bufferNamePrefix + " E" +char('x'+xyz), bufferSize, STRIDE));
        mBuffersH[xyz] = MemoryBufferPtr(new MemoryBuffer(
            bufferNamePrefix + " H" +char('x'+xyz), bufferSize, STRIDE));
        mOctantBuffers[octantE(xyz)] = mBuffersE[xyz];
        mOctantBuffers[octantH(xyz)] = mBuffersH[xyz];
    }
    
    // For run length encoding, sometimes it'll be nice to pretend that there
    // are also field values in the (unused) octants 0 and 7.  We can set them
    // up here without need to actually allocate them.
    mOctantBuffers[0] = MemoryBufferPtr(new MemoryBuffer(
        bufferNamePrefix + " empty (0,0,0)", bufferSize, STRIDE));
    mOctantBuffers[7] = MemoryBufferPtr(new MemoryBuffer(
        bufferNamePrefix + " empyt (1,1,1)", bufferSize, STRIDE));
    
    // Calculate the memory stride.  It is useful, perhaps kludgy though, to
    // set the memory stride equal to zero along dimensions which are not used
    // (e.g. the z direction for a 2D, XY-plane lattice).
    
    int alloc0 = runlineDirection;
    int alloc1 = (alloc0+1)%3;
    int alloc2 = (alloc1+1)%3;
    mMemStride[alloc0] = STRIDE;
    mMemStride[alloc1] = mNumYeeCells[alloc0]*mMemStride[alloc0];
    mMemStride[alloc2] = mNumYeeCells[alloc1]*mMemStride[alloc1];
    
    for (int xyz = 0; xyz < 3; xyz++)
    if (mNonZeroDimensions[xyz] == 0 || mNumYeeCells[xyz] == 1)
        mMemStride[xyz] = 0;
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        mHeadE[xyz] = 0L;
        mHeadH[xyz] = 0L;
    }
}

#pragma mark *** Methods for not-yet-allocated fields

Vector3i InterleavedLattice::
wrap(const Vector3i & halfCell) const
{
    assert(mNumHalfCells == 2*mNumYeeCells); // so num half cells is even.
    
    Vector3i wrappedHalfCell;
    
    for (int xyz = 0; xyz < 3; xyz++)
    if (mNonZeroDimensions[xyz] != 0)
    {   
        if (halfCell[xyz] >= mHalfCells.p1[xyz])
        {
            wrappedHalfCell[xyz] = mHalfCells.p1[xyz] +
                (halfCell[xyz] - mHalfCells.p1[xyz])%mNumHalfCells[xyz];
        }
        else
        {
            wrappedHalfCell[xyz] = mHalfCells.p1[xyz] +
                (mNumHalfCells[xyz]-1) -
                (mHalfCells.p1[xyz]-halfCell[xyz]-1)%mNumHalfCells[xyz];
        }
        
        assert(wrappedHalfCell[xyz] >= mHalfCells.p1[xyz]);
        assert(wrappedHalfCell[xyz] <= mHalfCells.p2[xyz]);
    }
    else
        wrappedHalfCell[xyz] = halfCell[xyz]; // for null dimensions, nothing done
    
    return wrappedHalfCell;
}

long InterleavedLattice::
linearYeeIndex(const Vector3i & halfCell) const
{
    assert(mHalfCells.encloses(halfCell));
    
    Vector3i displacement(halfCell - mHalfCells.p1);
    int index = dot(displacement/2, mMemStride); // mMemStride == 0 for null dims
    assert(index >= 0);
    assert(index < mNumYeeCells[0]*mNumYeeCells[1]*mNumYeeCells[2]);
    
    return index;
}

long InterleavedLattice::
wrappedLinearYeeIndex(const Vector3i & halfCell) const
{
    Vector3i displacement(wrap(halfCell) - mHalfCells.p1);
    int index = dot(displacement/2, mMemStride); // mMemStride == 0 for null dims
    assert(index >= 0);
    assert(index < mNumYeeCells[0]*mNumYeeCells[1]*mNumYeeCells[2]);
    
    return index;
}

// Access to allocation skeleton
BufferPointer InterleavedLattice::
pointer(const Vector3i & halfCell) const
{
    assert(mHalfCells.encloses(halfCell));
    assert(isE(octant(halfCell)) || isH(octant(halfCell)));
    
    long index = linearYeeIndex(halfCell);
    
    return BufferPointer(*mOctantBuffers[octant(halfCell)], index);
}

BufferPointer InterleavedLattice::
wrappedPointer(const Vector3i & halfCell) const
{
    Vector3i wrapped(wrap(halfCell));
    long index = linearYeeIndex(wrapped);
    
    assert(mOctantBuffers[octant(halfCell)] != 0L);
    
    return BufferPointer(*mOctantBuffers[octant(halfCell)], index);
}

BufferPointer InterleavedLattice::
pointerE(int fieldDirection, const Vector3i & yeeCell) const
{
    assert(mHalfCells.encloses(yeeToHalf(yeeCell, octantE(fieldDirection))));
    return pointer(yeeToHalf(yeeCell, octantE(fieldDirection)));
}

BufferPointer InterleavedLattice::
pointerH(int fieldDirection, const Vector3i & yeeCell) const
{
    assert(mHalfCells.encloses(yeeToHalf(yeeCell, octantH(fieldDirection))));
    return pointer(yeeToHalf(yeeCell, octantH(fieldDirection)));
}

BufferPointer InterleavedLattice::
wrappedPointerE(int fieldDirection, const Vector3i & yeeCell)
    const
{
    return wrappedPointer(yeeToHalf(yeeCell, octantE(fieldDirection)));
}

BufferPointer InterleavedLattice::
wrappedPointerH(int fieldDirection, const Vector3i & yeeCell)
    const
{
    return wrappedPointer(yeeToHalf(yeeCell, octantH(fieldDirection)));
}

#pragma mark *** Allocate ***
// Access to fields (once allocated)
void InterleavedLattice::
allocate()
{
    int nn;
    int bufsize = 0;
    long offset = 0;
    
    for (nn = 0; nn < 3; nn++)
    {
        bufsize += mBuffersE.at(nn)->length();
        bufsize += mBuffersH.at(nn)->length();
    }
    mData.resize(bufsize);
    
    for (nn = 0; nn < 3; nn++)
    {
        mBuffersE.at(nn)->setHeadPointer(&(mData.at(offset)));
        mHeadE[nn] = mBuffersE.at(nn)->headPointer();
        offset += mBuffersE.at(nn)->length();
    }
    for (nn = 0; nn < 3; nn++)
    {
        mBuffersH.at(nn)->setHeadPointer(&(mData.at(offset)));
        mHeadH[nn] = mBuffersH.at(nn)->headPointer();
        offset += mBuffersH.at(nn)->length();
    }
    mFieldsAreAllocated = 1;
}

#pragma mark *** Methods for allocated fields ***

float InterleavedLattice::
getE(int direction, const Vector3i & yeeCell) const
{
    assert(mFieldsAreAllocated);
    assert(mHalfCells.encloses(yeeToHalf(yeeCell, octantE(direction))));
    
    float* ptr;
    ptr = mHeadE[direction] + dot(yeeCell-mOriginYeeE[direction], mMemStride);
    
    assert(mBuffersE[direction]->includes(ptr));
    
    return *ptr;
}

float InterleavedLattice::
getH(int direction, const Vector3i & yeeCell) const
{
    assert(mFieldsAreAllocated);
    assert(mHalfCells.encloses(yeeToHalf(yeeCell, octantH(direction))));
    
    float* ptr;
    ptr = mHeadH[direction] + dot(yeeCell-mOriginYeeH[direction], mMemStride);
    
    assert(mBuffersH[direction]->includes(ptr));
    
    return *ptr;
}

float InterleavedLattice::
getWrappedE(int direction, const Vector3i & yeeCell) const
{
    assert(mFieldsAreAllocated);
    
    float* ptr;
    ptr = mHeadE[direction] +
        dot( (yeeCell-mOriginYeeE[direction]+mNumYeeCells)%mNumYeeCells,
            mMemStride);
    assert(mBuffersE[direction]->includes(ptr));
    
    return *ptr;
}

float InterleavedLattice::
getWrappedH(int direction, const Vector3i & yeeCell) const
{
    assert(mFieldsAreAllocated);
    
    float* ptr;
    ptr = mHeadH[direction] +
        dot( (yeeCell-mOriginYeeH[direction]+mNumYeeCells)%mNumYeeCells,
            mMemStride);
    assert(mBuffersH[direction]->includes(ptr));
    
    return *ptr;
}

float InterleavedLattice::
getInterpolatedE(int direction, const Vector3f & position) const
{
    Vector3f x = position - eFieldPosition(direction);
    const int ilow(floor(x[0])), jlow(floor(x[1])), klow(floor(x[2]));
    const int ihigh(ilow+1), jhigh(jlow+1), khigh(klow+1);
    const float dx(x[0]-floor(x[0])),
        dy(x[1]-floor(x[1])),
        dz(x[2]-floor(x[2]));
    
    return dx*dy*dz*getWrappedE(direction, Vector3i(ihigh, jhigh, khigh)) +
        (1.0f-dx)*dy*dz*getWrappedE(direction, Vector3i(ilow, jhigh, khigh)) +
        dx*(1.0f-dy)*dz*getWrappedE(direction, Vector3i(ihigh, jlow, khigh)) +
        (1.0f-dx)*(1.0f-dy)*dz*getWrappedE(direction, Vector3i(ilow, jlow, khigh)) +
        dx*dy*(1.0f-dz)*getWrappedE(direction, Vector3i(ihigh, jhigh, klow)) +
        (1.0f-dx)*dy*(1.0f-dz)*getWrappedE(direction, Vector3i(ilow, jhigh, klow)) +
        dx*(1.0f-dy)*(1.0f-dz)*getWrappedE(direction, Vector3i(ihigh, jlow, klow)) +
        (1.0f-dx)*(1.0f-dy)*(1.0f-dz)*getWrappedE(direction, Vector3i(ilow, jlow, klow)
        );
}

float InterleavedLattice::
getInterpolatedH(int direction, const Vector3f & position) const
{
    Vector3f x = position - hFieldPosition(direction);
    const int ilow(floor(x[0])), jlow(floor(x[1])), klow(floor(x[2]));
    const int ihigh(ilow+1), jhigh(jlow+1), khigh(klow+1);
    const float dx(x[0]-floor(x[0])),
        dy(x[1]-floor(x[1])),
        dz(x[2]-floor(x[2]));
    
    return dx*dy*dz*getWrappedH(direction, Vector3i(ihigh, jhigh, khigh)) +
        (1.0f-dx)*dy*dz*getWrappedH(direction, Vector3i(ilow, jhigh, khigh)) +
        dx*(1.0f-dy)*dz*getWrappedH(direction, Vector3i(ihigh, jlow, khigh)) +
        (1.0f-dx)*(1.0f-dy)*dz*getWrappedH(direction, Vector3i(ilow, jlow, khigh)) +
        dx*dy*(1.0f-dz)*getWrappedH(direction, Vector3i(ihigh, jhigh, klow)) +
        (1.0f-dx)*dy*(1.0f-dz)*getWrappedH(direction, Vector3i(ilow, jhigh, klow)) +
        dx*(1.0f-dy)*(1.0f-dz)*getWrappedH(direction, Vector3i(ihigh, jlow, klow)) +
        (1.0f-dx)*(1.0f-dy)*(1.0f-dz)*getWrappedH(direction, Vector3i(ilow, jlow, klow)
        );
}


void InterleavedLattice::
setE(int direction, const Vector3i & yeeCell, float value)
{
    assert(mFieldsAreAllocated);
    assert(mHalfCells.encloses(yeeToHalf(yeeCell, octantE(direction))));
    
    float* ptr;
    ptr = mHeadE[direction] + dot(yeeCell-mOriginYeeE[direction], mMemStride);
    
    //LOG << MemoryBuffer::identify(ptr) << "\n";
    
    assert(mBuffersE[direction]->includes(ptr));
    
    *ptr = value;
}
void InterleavedLattice::
setH(int direction, const Vector3i & yeeCell, float value)
{
    assert(mFieldsAreAllocated);
    assert(mHalfCells.encloses(yeeToHalf(yeeCell, octantH(direction))));
    
    float* ptr;
    ptr = mHeadH[direction] + dot(yeeCell-mOriginYeeH[direction], mMemStride);
    
    assert(mBuffersH[direction]->includes(ptr));
    
    *ptr = value;
}


void InterleavedLattice::
printE(std::ostream & str, int fieldDirection, float scale) const
{
    float periodMax = 0.2*scale;
    float dotMax = 0.5*scale;
    Rect3i r(halfToYee(mHalfCells, octantE(fieldDirection)));
    
    int dir0, dir1, dir2;
    
    dir0 = 0;
    for (int xyz = 0; xyz < 3; xyz++)
    if (mNumYeeCells[xyz] == 1)
        dir0 = (xyz+1)%3;
    dir1 = (dir0+1)%3;
    for (int xyz = 1; xyz < 3; xyz++)
    if (mNumYeeCells[ (dir0+xyz)%3 ] != 1)
        dir1 = (dir0+xyz)%3;
    dir2 = 3 - (dir0 + dir1);
    
    LOG << "Axes " << dir0 << " " << dir1 << " " << dir2 << "\n";
    
    Vector3i ijk;
    
    for (ijk[dir2] = r.p1[dir2]; ijk[dir2] <= r.p2[dir2]; ijk[dir2]++)
    {
        str << "+";
        for (int ni = r.p1[dir0]; ni <= r.p2[dir0]; ni++)
            str << "-";
        str << "+\n";
        for (ijk[dir1] = r.p2[dir1]; ijk[dir1] >= r.p1[dir1]; ijk[dir1]--)
        {
            str << "|";
            for (ijk[dir0] = r.p1[dir0]; ijk[dir0] <= r.p2[dir0]; ijk[dir0]++)
            {
                float field = fabs(getE(fieldDirection, ijk));
                if (field == 0)
                    str << " ";
                else if (field < periodMax)
                    str << ".";
                else if (field < dotMax)
                    str << "o";
                else if (isinf(field))
                    str << "∞";
                else if (isnan(field))
                    str << "N";
                else
                    str << "O";
            }
            str << "|\n";
        }
        str << "+";
        for (int ni = r.p1[dir0]; ni <= r.p2[dir0]; ni++)
            str << "-";
        str << "+\n";
    }
}

void InterleavedLattice::
printH(std::ostream & str, int fieldDirection, float scale) const
{
    float periodMax = 0.2*scale;
    float dotMax = 0.5*scale;
    Rect3i r(halfToYee(mHalfCells, octantH(fieldDirection)));
    
    int dir0, dir1, dir2;
    
    dir0 = 0;
    for (int xyz = 0; xyz < 3; xyz++)
    if (mNumYeeCells[xyz] == 1)
        dir0 = (xyz+1)%3;
    dir1 = (dir0+1)%3;
    for (int xyz = 1; xyz < 3; xyz++)
    if (mNumYeeCells[ (dir0+xyz)%3 ] != 1)
        dir1 = (dir0+xyz)%3;
    dir2 = 3 - (dir0 + dir1);
    
    LOG << "Axes " << dir0 << " " << dir1 << " " << dir2 << "\n";
    
    Vector3i ijk;
    
    for (ijk[dir2] = r.p1[dir2]; ijk[dir2] <= r.p2[dir2]; ijk[dir2]++)
    {
        str << "+";
        for (int ni = r.p1[dir0]; ni <= r.p2[dir0]; ni++)
            str << "-";
        str << "+\n";
        for (ijk[dir1] = r.p2[dir1]; ijk[dir1] >= r.p1[dir1]; ijk[dir1]--)
        {
            str << "|";
            for (ijk[dir0] = r.p1[dir0]; ijk[dir0] <= r.p2[dir0]; ijk[dir0]++)
            {
                float field = fabs(getH(fieldDirection, ijk));
                if (field == 0)
                    str << " ";
                else if (field < periodMax)
                    str << ".";
                else if (field < dotMax)
                    str << "o";
                else if (isinf(field))
                    str << "∞";
                else if (isnan(field))
                    str << "N";
                else
                    str << "O";
            }
            str << "|\n";
        }
        str << "+";
        for (int ni = r.p1[dir0]; ni <= r.p2[dir0]; ni++)
            str << "-";
        str << "+\n";
    }
}


