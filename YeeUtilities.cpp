/*
 *  YeeUtilities.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "YeeUtilities.h"

#include <cassert>

namespace YeeUtilities
{

static Vector3i sCardinals[6] =
	{ Vector3i(-1,0,0),
	  Vector3i(1,0,0),
	  Vector3i(0,-1,0),
	  Vector3i(0,1,0),
	  Vector3i(0,0,-1),
	  Vector3i(0,0,1) };

static Vector3i sOffsets[8] =
	{ Vector3i(0,0,0),
	  Vector3i(1,0,0),
	  Vector3i(0,1,0),
	  Vector3i(1,1,0),
	  Vector3i(0,0,1),
	  Vector3i(1,0,1),
	  Vector3i(0,1,1),
	  Vector3i(1,1,1) };

static Vector3i sFieldOffsets[6] =
	{ Vector3i(1,0,0), // Ex
	  Vector3i(0,1,0), // Ey
	  Vector3i(1,1,0), // Hz
	  Vector3i(0,0,1), // Ez
	  Vector3i(1,0,1), // Hy
	  Vector3i(0,1,1) }; // Hx

static int sHalfCellFieldIndices[6] =
	{ 1, 2, 3, 4, 5, 6 }; // indices of the field offsets (above) for E, H.

static int sOctantFieldIndices[8] =
	{ -1, 0, 1, 2, 3, 4, 5, -1 }; // indices of field offsets

static int sOctantEIndices[8] =
	{ -1, 0, 1, -1, 2, -1, -1, -1 }; // Ex = 0, etc.

static int sOctantHIndices[8] =
	{ -1, -1, -1, 2, -1, 1, 0, -1 }; // Hx = 0, etc.

static int sOctantFieldDirections[8] =
	{ -1, 0, 1, 2, 2, 1, 0, -1 }; // x = 0, y = 1, z = 2

int halfCellIndex(const Vector3i & v)
{
	return abs(v[0]%2 + 2*(v[1]%2) + 4*(v[2]%2)); // abs deals with -1
}

const Vector3i & halfCellOffset(int halfCellIndex)
{
	assert(halfCellIndex >= 0);
	assert(halfCellIndex < 8);
	return sOffsets[halfCellIndex];
}

const Vector3i & cardinalDirection(int directionIndex)
{
	assert(directionIndex >= 0);
	assert(directionIndex < 6);
	return sCardinals[directionIndex];
}

int halfCellFieldIndex(int fieldIndex)
{
	assert(fieldIndex >= 0);
	assert(fieldIndex < 6);
	return sHalfCellFieldIndices[fieldIndex];
}

const Vector3i & halfCellFieldOffset(int fieldIndex)
{
	assert(fieldIndex >= 0);
	assert(fieldIndex < 6);
	return sFieldOffsets[fieldIndex];
}


int octantFieldNumber(int octant)
{
	assert(octant >= 0);
	assert(octant < 8);
	return sOctantFieldIndices[octant];
}

int octantFieldNumber(Vector3i octant)
{
	return octantFieldNumber(halfCellIndex(octant));
}

int octantENumber(int octant)
{
	assert(octant >= 0);
	assert(octant < 8);
	return sOctantEIndices[octant];
}

int octantHNumber(int octant)
{
	assert(octant >= 0);
	assert(octant < 8);
	return sOctantHIndices[octant];
}

int octantFieldDirection(int octant)
{
	assert(octant >= 0);
	assert(octant < 8);
	return sOctantFieldDirections[octant];
}

int octantFieldDirection(Vector3i octant)
{
	return octantFieldDirection(halfCellIndex(octant));
}

Vector3f eFieldPosition(int fieldNum)
{
    Vector3f v(0.0, 0.0, 0.0);
    v[fieldNum] = 0.5;
    return v;
}

Vector3f hFieldPosition(int fieldNum)
{
    Vector3f v(0.5, 0.5, 0.5);
    v[fieldNum] = 0.0;
    return v;
}


// returns halfCell/2
Vector3i vecHalfToYee(const Vector3i & halfCell)
{
	return halfCell/2;
}

// returns  2*yeeCell + halfCellOffset
Vector3i vecYeeToHalf(const Vector3i & yeeCell,
	const Vector3i & halfCellOffset)
{
	return 2*yeeCell + halfCellOffset;
}

// returns 2*yeeCell + halfCellOffset(halfCellIndex)
Vector3i vecYeeToHalf(const Vector3i & yeeCell, int halfCellIndex)
{
	return 2*yeeCell + halfCellOffset(halfCellIndex);
}

// returns smallest Yee rect containing all points in halfRect
Rect3i rectHalfToYee(const Rect3i & halfRect)
{
	return halfRect/2;
}

// returns smallest Yee rect containing all points at given halfCellIndex
// in halfRect
Rect3i rectHalfToYee(const Rect3i & halfRect, int halfCellIndex)
{
	// if halfRect.p1%2 == offset%2, then p1 = halfRect.p1/2
	// else p1 = (halfRect.p1 + 1)/2
	
	// if halfRect.p2%2 == offset%2, then p2 = halfRect.p2/2
	// else p2 = (halfRect.p2 - 1)/2
	
	// so formulae:
	// p1 = (halfRect.p1 + (offset%2 != halfRect.p1%2) elementwise )/2
	// p2 = (halfRect.p2 - (offset%2 != halfRect.p2%2) elementwise )/2
	//
	// since elementwise != is not defined, I can instead use
	// (offset%2 != halfRect.p1%2)  equiv to   (offset+halfRect.p1)%2
	
	const Vector3i & offset = halfCellOffset(halfCellIndex);
	return Rect3i( (halfRect.p1 + Vector3i(offset+halfRect.p1)%2)/2,
		(halfRect.p2 - Vector3i(offset+halfRect.p2)%2)/2 );
	
}

// returns smallest Yee rect containing all points at given halfCellOffset
// in halfRect
Rect3i rectHalfToYee(const Rect3i & halfRect, const Vector3i & halfCellOffset)
{
	// see above
	return Rect3i( (halfRect.p1 + Vector3i(halfCellOffset+halfRect.p1)%2)/2,
		(halfRect.p2 - Vector3i(halfCellOffset+halfRect.p2)%2)/2 );	
}

// returns smallest half cell rect containing all points in given Yee rect
// at given halfCellIndex
Rect3i rectYeeToHalf(const Rect3i & yeeRect, int halfCellIndex)
{
	return Rect3i(2*yeeRect + halfCellOffset(halfCellIndex));
}

// returns smallest half cell rect containing all points in given Yee rect
// at given half cell offset
Rect3i rectYeeToHalf(const Rect3i & yeeRect, const Vector3i & halfCellOffset)
{
	return Rect3i(2*yeeRect + halfCellOffset);
}

// returns smallest Yee rect containing all points in given Yee rect
Rect3i rectYeeToHalf(const Rect3i & yeeRect)
{
	return Rect3i(2*yeeRect.p1, 2*yeeRect.p2 + Vector3i(1,1,1));
}

Rect3i expandToYeeRect(const Rect3i & halfRect)
{
	return rectYeeToHalf(rectHalfToYee(halfRect));
}


// returns the last layer of the rect on the side with normal along sideIndex
Rect3i edgeOfRect(const Rect3i & rect, int sideIndex)
{
	Rect3i retval(rect);
	if (sideIndex%2 == 0)
		retval.p2[sideIndex/2] = retval.p1[sideIndex/2];
	else
		retval.p1[sideIndex/2] = retval.p2[sideIndex/2];
	return retval;
}

}; // namespace YeeUtilities