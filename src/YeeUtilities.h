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

// returns the last layer of the rect on the side with normal along sideIndex
Rect3i edgeOfRect(const Rect3i & rect, int sideIndex);

}; // namespace YeeUtilities

#endif
