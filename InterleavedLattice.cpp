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

using namespace std;
using namespace YeeUtilities;

InterleavedLattice::
InterleavedLattice(const string & bufferNamePrefix, Rect3i halfCellBounds,
    Vector3i nonZeroDimensions) :
    mHalfCells(halfCellBounds),
    mNonZeroDimensions(1,1,1),
    mBuffersE(3),
    mBuffersH(3)
{
    Rect3i maximalYeeBounds(halfToYee(mHalfCells));
    mNumYeeCells = maximalYeeBounds.size()+1;
    for (int nn = 0; nn < 3; nn++)
    if (mNonZeroDimensions[nn] == 0)
        mNumYeeCells[nn] = 1;
    mNumHalfCells = 2*mNumYeeCells;
    
    LOG << "Not sure what to do with these cached alloc origins."
        "  It may be okay to just use one value as I'm provisionally doing "
        "here, since I make good use of mNonZeroDimensions.\n";
    for (int nn = 0; nn < 3; nn++)
    {
        mAllocOriginYeeE[nn] = halfToYee(mHalfCells.p1);
        mAllocOriginYeeH[nn] = halfToYee(mHalfCells.p1);
    }
    
    int bufferSize = mNumYeeCells[0]*mNumYeeCells[1]*mNumYeeCells[2];
    const int BUFFERSTRIDE = 1;
    for (int nn = 0; nn < 3; nn++)
    {
        mBuffersE[nn] = MemoryBufferPtr(new MemoryBuffer(bufferNamePrefix+" E"
            +char('x'+nn), bufferSize, BUFFERSTRIDE));
        mBuffersH[nn] = MemoryBufferPtr(new MemoryBuffer(bufferNamePrefix+" H"
            +char('x'+nn), bufferSize, BUFFERSTRIDE));
    }
    mMemStride[0] = BUFFERSTRIDE;
    mMemStride[1] = mNumYeeCells[0]*mMemStride[0];
    mMemStride[2] = mNumYeeCells[1]*mMemStride[1];
    
    for (int nn = 0; nn < 3; nn++)
    if (mNonZeroDimensions[nn] == 0)
        mMemStride[nn] = 0;
    
    for (int nn = 0; nn < 3; nn++)
    {
        mHeadE[nn] = 0L;
        mHeadH[nn] = 0L;
    }
}


Vector3i InterleavedLattice::
wrap(const Vector3i & halfCell) const
{
    Vector3i wrappedHalfCell;
    
    for (int nn = 0; nn < 3; nn++)
    if (mNonZeroDimensions[nn] != 0)
    {   
        if (halfCell[nn] >= mHalfCells.p1[nn])
        {
            wrappedHalfCell[nn] = mHalfCells.p1[nn] +
                (halfCell[nn] - mHalfCells.p1[nn])%mNumHalfCells[nn];
        }
        else
        {
            wrappedHalfCell[nn] = mHalfCells.p1[nn] +
                (mNumHalfCells[nn]-1) -
                (-(halfCell[nn]-mHalfCells.p1[nn])-1)%mNumHalfCells[nn];
        }
        
        assert(wrappedHalfCell[nn] >= mHalfCells.p1[nn]);
        assert(wrappedHalfCell[nn] <= mHalfCells.p2[nn]);
    }
    else
        wrappedHalfCell[nn] = halfCell[nn]; // for null dimensions, nothing done
    
    return wrappedHalfCell;
}

long InterleavedLattice::
linearYeeIndex(const Vector3i & halfCell) const
{
    assert(mHalfCells.encloses(halfCell));
    
    Vector3i displacement(halfCell - mHalfCells.p1);
    /*
    int index = 0;
    for (int nn = 0; nn < 3; nn++)
    if (mNonZeroDimensions[nn])
        index += mMemStride[nn]*displacement[nn];
    */
    int index = dot(displacement/2, mMemStride); // mMemStride == 0 for null dims
    assert(index >= 0);
    assert(index < mNumYeeCells[0]*mNumYeeCells[1]*mNumYeeCells[2]);
    
    return index;
}

// Access to allocation skeleton
BufferPointer InterleavedLattice::
pointer(const Vector3i & halfCell) const
{
    long index = linearYeeIndex(halfCell);
    if (isE(octant(halfCell)))
        return BufferPointer(*mBuffersE[xyz(octant(halfCell))], index);
    else
        return BufferPointer(*mBuffersH[xyz(octant(halfCell))], index);
}

BufferPointer InterleavedLattice::
wrappedPointer(const Vector3i & halfCell) const
{
    Vector3i wrapped(wrap(halfCell));
    long index = linearYeeIndex(wrapped);
    if (isE(octant(wrapped)))
        return BufferPointer(*mBuffersE[xyz(octant(wrapped))], index);
    else
        return BufferPointer(*mBuffersH[xyz(octant(wrapped))], index);
}

BufferPointer InterleavedLattice::
pointerE(int fieldDirection, const Vector3i & yeeCell) const
{
    return pointer(yeeToHalf(yeeCell, octantE(fieldDirection)));
}
BufferPointer InterleavedLattice::
pointerH(int fieldDirection, const Vector3i & yeeCell) const
{
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

// Access to fields (once allocated)
void InterleavedLattice::
allocate()
{
    int nn;
    int bufsize = 0;
    long offset = 0;
    
    for (nn = 0; nn < 3; nn++)
    {
        bufsize += mBuffersE.at(nn)->getLength();
        bufsize += mBuffersH.at(nn)->getLength();
    }
    mData.resize(bufsize);
    
    for (nn = 0; nn < 3; nn++)
    {
        mBuffersE.at(nn)->setHeadPointer(&(mData.at(offset)));
        mHeadE[nn] = mBuffersE.at(nn)->getHeadPointer();
        offset += mBuffersE.at(nn)->getLength();
    }
    for (nn = 0; nn < 3; nn++)
    {
        mBuffersH.at(nn)->setHeadPointer(&(mData.at(offset)));
        mHeadH[nn] = mBuffersH.at(nn)->getHeadPointer();
        offset += mBuffersH.at(nn)->getLength();
    }
}

float InterleavedLattice::
getE(int direction, const Vector3i & yeeCell) const
{
    return mHeadE[direction][dot(yeeCell-mAllocOriginYeeE[direction],
        mMemStride)];
}
float InterleavedLattice::
getH(int direction, const Vector3i & yeeCell) const
{
    return mHeadH[direction][dot(yeeCell-mAllocOriginYeeH[direction],
        mMemStride)];
}
float InterleavedLattice::
getWrappedE(int direction, const Vector3i & yeeCell) const
{
    return mHeadE[direction][
        dot( (yeeCell-mAllocOriginYeeE[direction]+mNumYeeCells)%mNumYeeCells,
        mMemStride)];
}
float InterleavedLattice::
getWrappedH(int direction, const Vector3i & yeeCell) const
{
    return mHeadE[direction][
        dot( (yeeCell-mAllocOriginYeeH[direction]+mNumYeeCells)%mNumYeeCells,
        mMemStride)];
}

float InterleavedLattice::
setE(int direction, const Vector3i & yeeCell, float value)
{
    mHeadE[direction][dot(yeeCell-mAllocOriginYeeE[direction], mMemStride)]
        = value;
}
float InterleavedLattice::
setH(int direction, const Vector3i & yeeCell, float value)
{
    mHeadH[direction][dot(yeeCell-mAllocOriginYeeH[direction], mMemStride)]
        = value;
}

void InterleavedLattice::
printE(std::ostream & str, int fieldDirection, float scale) const
{
}

void InterleavedLattice::
printH(std::ostream & str, int fieldDirection, float scale) const
{
    /*
    float spaceMax = 0.2*scale;
    float periodMax = 0.5*scale;
    int direction;
    bool isEHere = isE(field);
    Rect3i r(mAllocOriginYee, mAllocOriginYee+mAllocYeeCells-1);
    
    int ni, nj, nk;
    
    if (isEHere)
    {
        direction = xyz(field);
        assert(direction != -1);
        for (nk = r.p1[2]; nk <= r.p2[2]; nk++)
        {
            str << "+";
            for (ni = r.p1[0]; ni <= r.p2[0]; ni++)
                str << "-";
            str << "+\n";
            for (nj = r.p2[1]; nj >= r.p1[1]; nj--)
            {
                str << "|";
                for (ni = r.p1[0]; ni <= r.p2[0]; ni++)
                {
                    float field = fabs(getE(direction, ni, nj, nk));
                    if (field < spaceMax)
                        str << " ";
                    else if (field < periodMax)
                        str << ".";
                    else
                        str << "•";
                }
                str << "|\n";
            }
            str << "+";
            for (ni = r.p1[0]; ni <= r.p2[0]; ni++)
                str << "-";
            str << "+\n";
        }
    }
    else
    {
        direction = xyz(field);
        assert(direction != -1);
        for (int nk = r.p1[2]; nk <= r.p2[2]; nk++)
        {
            for (int nj = r.p2[1]; nj >= r.p1[1]; nj--)
            {
                for (int ni = r.p1[0]; ni <= r.p2[0]; ni++)
                {
                    float field = fabs(getH(direction, ni, nj, nk));
                    if (field < spaceMax)
                        str << " ";
                    else if (field < periodMax)
                        str << ".";
                    else
                        str << "•";
                }
                str << "\n";
            }
        }
    }*/
}




