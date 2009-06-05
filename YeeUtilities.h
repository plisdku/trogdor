/*
 *  YeeUtilities.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _YEEUTILITIES_
#define _YEEUTILITIES_

#include "geometry.h"

// TO-DO: call things "octants" as applicable

namespace YeeUtilities
{

// returns a number from [0,7] telling which octant of the Yee cell v is in
int halfCellIndex(const Vector3i & v);

// returns a vector with origin at 0 pointing to the given Yee octant
const Vector3i & halfCellOffset(int halfCellIndex);

const Vector3i & cardinalDirection(int directionIndex);

int halfCellFieldIndex(int fieldIndex);  // returns halfCellIndices of EM fields
const Vector3i & halfCellFieldOffset(int fieldIndex); // offsets of EM fields

int octantFieldNumber(int octant); // returns field indices or -1
int octantFieldNumber(Vector3i octant); // returns field indices or -1
int octantENumber(int octant); // returns 0,1,2 for Ex,y,z or -1
int octantHNumber(int octant); // returns 0,1,2 for Hx,y,z or -1
int octantFieldDirection(int octant); // returns 0 for Ex or Hx, 1 for y, etc.
int octantFieldDirection(Vector3i octant);
int eOctantNumber(int directionIndex); // returns octant 1 for dir 0 (Ex), etc.
int hOctantNumber(int directionIndex); // returns octant 6 for dir 0 (Hx), etc.
int eFieldNumber(int directionIndex);
int hFieldNumber(int directionIndex);

Vector3f eFieldPosition(int fieldIndex);
Vector3f hFieldPosition(int fieldIndex);
Vector3i eFieldOffset(int directionIndex);
Vector3i hFieldOffset(int directionIndex);

// returns halfCell/2
Vector3i vecHalfToYee(const Vector3i & halfCell);

// returns  2*yeeCell + halfCellOffset
Vector3i vecYeeToHalf(const Vector3i & yeeCell,
	const Vector3i & halfCellOffset);

// returns 2*yeeCell + halfCellOffset(halfCellIndex)
Vector3i vecYeeToHalf(const Vector3i & yeeCell, int halfCellIndex);

// returns smallest Yee rect containing all points in halfRect
Rect3i rectHalfToYee(const Rect3i & halfRect);

// returns smallest Yee rect containing all points at given halfCellIndex
// in halfRect
Rect3i rectHalfToYee(const Rect3i & halfRect, int halfCellIndex);

// returns smallest Yee rect containing all points at given halfCellOffset
// in halfRect
Rect3i rectHalfToYee(const Rect3i & halfRect, const Vector3i & halfCellOffset);

// returns smallest half cell rect containing all points in given Yee rect
// at given halfCellIndex
Rect3i rectYeeToHalf(const Rect3i & yeeRect, int halfCellIndex);

// returns smallest half cell rect containing all points in given Yee rect
// at given half cell offset
Rect3i rectYeeToHalf(const Rect3i & yeeRect, const Vector3i & halfCellOffset);

// returns smallest Yee rect containing all points in given Yee rect
Rect3i rectYeeToHalf(const Rect3i & yeeRect);

// equivalent to rectYeeToHalf(rectHalftoYee(halfRect))
Rect3i expandToYeeRect(const Rect3i & halfRect);

// returns the last layer of the rect on the side with normal along sideIndex
Rect3i edgeOfRect(const Rect3i & rect, int sideIndex);

}; // namespace YeeUtilities

#endif
