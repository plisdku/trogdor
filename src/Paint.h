/*
 *  Paint.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _PAINT_
#define _PAINT_


// Basic paint types: bulk materials, boundaries, PMLs.  These correspond to
// actually different update equations.
//
// Paint modifications: curl (neighbor) buffers, current buffers
// The curl buffer doesn't change the update equation
// The current buffer does change the update equation, I think, unless it's
// implemented by a four-direction curl buffer.  I suppose it could be, for
// transparent use by all material models.  I'd need a way to compose buffer
// operations then, or combine them in more complicated buffers.
//
// I'm leaning towards combining the effects of TFSF and currents into one
// TFSF buffer method... it would be transparent from a material implementation
// standpoint, which is nice, although it would also suffer from some pretty
// extensive extra buffer use.
//
// So, scratch that.  Now I'm leaning towards implementing the current source as
// a locally different update equation.  It should be easy enough to turn on
// by using templates, if needed; anyway the modification is simple enough.
//
// Perhaps marking it will be enough, for now.

#include "SimulationDescriptionPredeclarations.h"

#include "geometry.h"
#include <vector>
#include <iostream>

enum PaintType
{
	kBulkPaintType,
	kBoundaryPaintType
};

class Paint;
typedef Pointer<Paint> PaintPtr;

class Paint
{
public:
	Paint(PaintType inType);
	~Paint();
	
	friend std::ostream & operator<<(std::ostream & out, const Paint & p);
private:
	Paint(const MaterialDescPtr & material); // bulk constructor
	Paint(const Paint & parent, int sideNum, // curl buffer constructor
		NeighborBufferDescPtr & curlBuffer);
	Paint(const Paint & parent, Vector3i pmlDir); // pml constructor
	Paint(const Paint & parent, int donothing); // parent paint constructor (no modification)
public:
	static Paint* getPaint(const MaterialDescPtr & material);
	static Paint* getCurlBufferedPaint(Paint* basePaint, int sideNum,
		NeighborBufferDescPtr & curlBuffer);
	static Paint* getPMLPaint(Paint* basePaint, Vector3i pmlDir);
	static Paint* getParentPaint(Paint* basePaint);
	static Paint* retrieveCurlBufferParentPaint(Paint* basePaint);
	
	static const Map<Paint, PaintPtr> & getPalette() {
		return mPalette; }
	
	static void clearPalette() { mPalette.clear(); }
	
	PaintType getType() const { return mType; }
	
	bool equivalentUpdateTo(const Paint & rhs) const;
	
private:
	
	PaintType mType;
	Vector3i mPMLDirections;
	std::vector<NeighborBufferDescPtr> mCurlBuffers;
	int mCurrentBufferIndex;
	
	// for BulkPaint
	MaterialDescPtr mBulkMaterial;
	
	// for BoundaryPaint
	// . . . nothing here yet
	
	static Map<Paint, PaintPtr> mPalette;
	
	friend bool operator<(const Paint & lhs, const Paint & rhs);
};
bool operator<(const Paint & lhs, const Paint & rhs);
std::ostream & operator<<(std::ostream & out, const Paint & p);



#endif
