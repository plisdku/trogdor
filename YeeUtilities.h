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
int octant(const Vector3i & v);

// returns a vector with origin at 0 pointing to the given Yee octant
const Vector3i & halfCellOffset(int octant);

const Vector3i & cardinal(int directionIndex);

int xyz(int octant);
bool isE(int octant);
bool isH(int octant);

int octantE(int directionIndex); // returns octant 1 for dir 0 (Ex), etc.
int octantH(int directionIndex); // returns octant 6 for dir 0 (Hx), etc.

Vector3f eFieldPosition(int fieldIndex);
Vector3f hFieldPosition(int fieldIndex);
Vector3i eFieldOffset(int directionIndex);
Vector3i hFieldOffset(int directionIndex);

// returns halfCell/2
Vector3i halfToYee(const Vector3i & halfCell);

// returns  2*yeeCell + halfCellOffset
Vector3i yeeToHalf(const Vector3i & yeeCell,
	const Vector3i & halfCellOffset);

// returns 2*yeeCell + halfCellOffset(halfCellIndex)
Vector3i yeeToHalf(const Vector3i & yeeCell, int halfCellIndex);

// returns smallest Yee rect containing all points in halfRect
Rect3i halfToYee(const Rect3i & halfRect);

// returns smallest Yee rect containing all points at given halfCellIndex
// in halfRect
Rect3i halfToYee(const Rect3i & halfRect, int halfCellIndex);

// returns smallest Yee rect containing all points at given halfCellOffset
// in halfRect
Rect3i halfToYee(const Rect3i & halfRect, const Vector3i & halfCellOffset);

// returns smallest half cell rect containing all points in given Yee rect
// at given halfCellIndex
Rect3i yeeToHalf(const Rect3i & yeeRect, int halfCellIndex);

// returns smallest half cell rect containing all points in given Yee rect
// at given half cell offset
Rect3i yeeToHalf(const Rect3i & yeeRect, const Vector3i & halfCellOffset);

// returns smallest Yee rect containing all points in given Yee rect
Rect3i yeeToHalf(const Rect3i & yeeRect);

// equivalent to yeeToHalf(rectHalftoYee(halfRect))
Rect3i expandToYeeRect(const Rect3i & halfRect);

// returns the last layer of the rect on the side with normal along sideIndex
Rect3i edgeOfRect(const Rect3i & rect, int sideIndex);

}; // namespace YeeUtilities

#endif
