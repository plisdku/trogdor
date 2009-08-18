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

#include "CurrentSource.h"
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

class NeighborBuffer;
typedef Pointer<NeighborBuffer> NeighborBufferPtr;

class Paint
{
public:
	Paint(PaintType inType);
	Paint(const Paint & copyMe); // copy constructor, slightly evil
	~Paint();
	
	friend std::ostream & operator<<(std::ostream & out, const Paint & p);
private:
	Paint(const MaterialDescPtr & material); // bulk constructor
	
public:
	std::string fullName() const;
    
	static Paint* paint(const MaterialDescPtr & material);
    
	Paint* withoutModifications() const { return mBasePaint; }
	Paint* withoutCurlBuffers() const { return mBaseUpdatePaint; }
	Paint* withPML(Vector3i pmlDir) const;
    Paint* withCurlBuffer(int side, const NeighborBuffer* curlBuffer) const;
	Paint* withCurrentSource(CurrentSourceDescPtr currentSource) const;
	
	static const Map<Paint, PaintPtr> & palette() {
		return mPalette; }
	
	static void clearPalette() { mPalette.clear(); }
	
	PaintType type() const { return mType; }
	Vector3i pmlDirections() const { return mPMLDirections; }
    const NeighborBuffer* curlBuffer(int side) const;
	const MaterialDescription* bulkMaterial() const;
    const CurrentSourceDescPtr currentSource() const
        { return mCurrentSource; }
	
	bool equivalentUpdateTo(const Paint & rhs) const;
	bool hasCurlBuffer() const;
	bool hasCurlBuffer(int side) const;
    bool hasCurrentSource() const;
	bool isPML() const;
	
private:
	
	// Cached parent paints (can be self-referential)
	Paint* mBasePaint;				// as painted, without *any* overlays
	Paint* mBaseUpdatePaint;		// as painted but includes current sources
	
	PaintType mType;
	Vector3i mPMLDirections;
    std::vector<const NeighborBuffer*> mCurlBuffers;
	CurrentSourceDescPtr mCurrentSource;
	
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
