/*
 *  IndexStencil.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/27/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "IndexStencil.h"
#include <cassert>
using namespace std;


IndexStencil::
IndexStencil()
{
    //clear();
}


IndexStencil::
~IndexStencil()
{
}

IndexStencil& IndexStencil::
operator=(const IndexStencil& rhs)
{
    if (&rhs == this)
        return *this;
	
	// New
	for (int n = 0; n < 6; n++)
		mIndices[n] = rhs.mIndices[n];
	
	// Old
	for (int n = 0; n < 2; n++)
    {
        mXIndices[n] = rhs.mXIndices[n];
        mYIndices[n] = rhs.mYIndices[n];
        mZIndices[n] = rhs.mZIndices[n];
    }
    
    return *this;
}

bool IndexStencil::
operator==(const IndexStencil & rhs)
{
	// New
	for (int n = 0; n < 5; n++)
		if (mIndices[n] != rhs.mIndices[n])
			return 0;
	
	// Old
    for (int n = 0; n < 2; n++)
    {
        if (mXIndices[n] != rhs.mXIndices[n])
            return 0;
        if (mYIndices[n] != rhs.mYIndices[n])
            return 0;
        if (mZIndices[n] != rhs.mZIndices[n])
            return 0;
    }
    
    return 1;
}

// redacted
/*
void IndexStencil::
clear()
{
	// New  (but what does -1 signify?)
	for (int n = 0; n < 6; n++)
		mIndices[n] = -1;
	
	
	// Old
    for (int n = 0; n < 2; n++)
    {
        mXIndices[n] = -1;
        mYIndices[n] = -1;
        mZIndices[n] = -1;
    }
}
*/

int IndexStencil::
getIndex(int faceNumber) const
{
	assert(faceNumber >= 0 && faceNumber <= 5);
	return mIndices[faceNumber];
}

void IndexStencil::
setIndex(int faceNumber, int index)
{
	assert(faceNumber >= 0 && faceNumber <= 5);
	mIndices[faceNumber] = index;
}

int IndexStencil::
getXIndex(int i) const
{
    assert(i >= 0 && i <= 2);
    return mXIndices[i];
}

int IndexStencil::
getYIndex(int i) const
{
    assert(i >= 0);
    assert(i <= 2);
    return mYIndices[i];
}

int IndexStencil::
getZIndex(int i) const
{
    assert(i >= 0);
    assert(i < 2);
    return mZIndices[i];
}

void IndexStencil::
setXIndex(int i, int index)
{
    assert(i >= 0);
    assert(i <= 2);
    mXIndices[i] = index;
}

void IndexStencil::
setYIndex(int i, int index)
{
    assert(i >= 0);
    assert(i <= 2);
    mYIndices[i] = index;
}

void IndexStencil::
setZIndex(int i, int index)
{
    assert(i >= 0);
    assert(i <= 2);
    mZIndices[i] = index;
}


unsigned int IndexStencil::
compareShiftedOldish(int myShiftCells, const IndexStencil & s2)
{
	unsigned int sameFlags = 1 + 2 + 4 + 8 + 16 + 32;
	
    for (int n = 0; n < 2; n++)
    {
        if (mXIndices[n] != -1 && s2.mXIndices[n] != -1)
        {
            if (mXIndices[n] + myShiftCells != s2.mXIndices[n])
                sameFlags ^= 2 >> n;
        }
        else if (mXIndices[n] != -1 || s2.mXIndices[n] != -1)
            sameFlags ^= 2 >> n;
        
        if (mYIndices[n] != -1 && s2.mYIndices[n] != -1)
        {
            if (mYIndices[n] + myShiftCells != s2.mYIndices[n])
                sameFlags ^= 8 >> n;
        }
        else if (mYIndices[n] != -1 || s2.mYIndices[n] != -1)
			sameFlags ^= 8 >> n;
        
        if (mZIndices[n] != -1 && s2.mZIndices[n] != -1)
        {
            if (mZIndices[n] + myShiftCells != s2.mZIndices[n])
                sameFlags ^= 32 >> n;
        }
        else if (mZIndices[n] != -1 || s2.mZIndices[n] != -1)
            sameFlags ^= 32 >> n;
    }
    return sameFlags;
}


bool IndexStencil::
compareToIncremented(const IndexStencil & lastStencil, int increment,
	const bitset<6> & mask)
{
	for (int n = 0; n < 6; n++)
	if (mask[n])
	if (mIndices[n] != lastStencil.mIndices[n] - increment)
		return 0;
	
	return 1;
}

/*
bool IndexStencil::
compareShifted(int myShiftCells, const IndexStencil & s2, bool compareXAxis,
    bool compareYAxis, bool compareZAxis)
{
    int n;
    
    if (compareXAxis)
    for (n = 0; n < 2; n++)
    {
        if (mXIndices[n] != -1 && s2.mXIndices[n] != -1)
        {
            if (mXIndices[n] + myShiftCells != s2.mXIndices[n])
                return 0;
        }
        else if (mXIndices[n] != -1 || s2.mXIndices[n] != -1)
            return 0;
    }
    
    if (compareYAxis)
    for (n = 0; n < 2; n++)
    {
        if (mYIndices[n] != -1 && s2.mYIndices[n] != -1)
        {
            if (mYIndices[n] + myShiftCells != s2.mYIndices[n])
                return 0;
        }
        else if (mYIndices[n] != -1 || s2.mYIndices[n] != -1)
            return 0;
    }
    
    if (compareZAxis)
    for (n = 0; n < 2; n++)
    {
        if (mZIndices[n] != -1 && s2.mZIndices[n] != -1)
        {
            if (mZIndices[n] + myShiftCells != s2.mZIndices[n])
                return 0;
        }
        else if (mZIndices[n] != -1 || s2.mZIndices[n] != -1)
            return 0;
    }
    return 1;
}
*/

void IndexStencil::
print(ostream & str) const
{
    int n;
	
	for (n = 0; n < 6; n++)
	{
		str << mIndices[n] << " ";
	}
	str << "\n";
	
	/*
    str << "x: ";
    //for (n = 0; n < 4; n++)
    for (n = 0; n < 2; n++)
        str << mXIndices[n] << " ";
    str << "\ny: ";
    //for (n = 0; n < 4; n++)
    for (n = 0; n < 2; n++)
        str << mYIndices[n] << " ";
    str << "\nz: ";
    //for (n = 0; n < 4; n++)
    for (n = 0; n < 2; n++)
        str << mZIndices[n] << " ";
    str << "\n";
	*/
}

