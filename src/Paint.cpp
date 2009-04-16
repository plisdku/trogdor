/*
 *  Paint.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "Paint.h"
#include "Map.h"
#include "SimulationDescription.h"

using namespace std;

Map<Paint, PaintPtr> Paint::mPalette;

#pragma mark *** Paint ***

Paint::
Paint(PaintType inType) :
	mType(inType),
	mPMLDirections(0,0,0),
	mCurlBuffers(6),
	mCurrentBufferIndex(-1),
	mBulkMaterial(0L)
{
	assert(!"This is evil.");
}

Paint::
~Paint()
{
	//LOG << "Killing me: \n"; // << hex << this << dec << endl;
}

Paint::
Paint(const MaterialDescPtr & material) :
	mType(kBulkPaintType),
	mPMLDirections(0,0,0),
	mCurlBuffers(6),
	mCurrentBufferIndex(-1),
	mBulkMaterial(material)
{
	assert(material != 0L);
}

Paint::
Paint(const Paint & parent, int sideNum, NeighborBufferDescPtr & curlBuffer) :
	mType(parent.mType),
	mPMLDirections(parent.mPMLDirections),
	mCurlBuffers(parent.mCurlBuffers),
	mCurrentBufferIndex(parent.mCurrentBufferIndex),
	mBulkMaterial(parent.mBulkMaterial)
{
	//LOG << mCurlBuffers[sideNum] << "\n";
	assert(mCurlBuffers[sideNum] == 0L);
	assert(mBulkMaterial != 0L);
	mCurlBuffers[sideNum] = curlBuffer;
	//LOG << "side num " << sideNum << " buffer " << hex
	//	<< (void*)mCurlBuffers[sideNum] << dec << "\n";
}

Paint::
Paint(const Paint & parent, Vector3i pmlDirection) :
	mType(parent.mType),
	mPMLDirections(pmlDirection),
	mCurlBuffers(6),
	mCurrentBufferIndex(-1),
	mBulkMaterial(parent.mBulkMaterial)
{
	assert(parent.mPMLDirections == Vector3i(0,0,0));
	assert(mBulkMaterial != 0L);
}


Paint::
Paint(const Paint & parent, int donothing) :
	mType(parent.mType),
	mPMLDirections(0,0,0),
	mCurlBuffers(6),
	mCurrentBufferIndex(-1),
	mBulkMaterial(parent.mBulkMaterial)
{
	assert(mBulkMaterial != 0L);
}



Paint* Paint::
getPaint(const MaterialDescPtr & material)
{
	PaintPtr bulkPaint(new Paint(material));
	if (mPalette.count(*bulkPaint) != 0)
		return mPalette[*bulkPaint];
	mPalette[*bulkPaint] = bulkPaint;
	return bulkPaint;
}

Paint* Paint::
getCurlBufferedPaint(Paint* basePaint, int sideNum,
	NeighborBufferDescPtr & curlBuffer)
{
	assert(basePaint != 0L);
	PaintPtr p(new Paint(*basePaint, sideNum, curlBuffer));
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
}

Paint* Paint::
getPMLPaint(Paint* basePaint, Vector3i pmlDir)
{
	assert(basePaint != 0L);
	PaintPtr p(new Paint(*basePaint, pmlDir));
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
}

Paint* Paint::
getParentPaint(Paint* basePaint)
{
	assert(basePaint != 0L);
	if (!basePaint->hasCurlBuffer() && !basePaint->isPML() &&
		basePaint->mCurrentBufferIndex == -1)
		return basePaint;
	
	PaintPtr p(new Paint(*basePaint, 0));
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
}


Paint* Paint::
retrieveCurlBufferParentPaint(Paint* basePaint)
{
	assert(basePaint != 0L);
	PaintPtr p(new Paint(*basePaint));
	p->mCurlBuffers.resize(6, NeighborBufferDescPtr(0L));
	assert(mPalette.count(*p) != 0);
	return mPalette[*p];
}


bool Paint::
equivalentUpdateTo(const Paint & rhs) const
{
	if (mType != rhs.mType)
		return 0;
	if (mType == kBulkPaintType)
	if (mBulkMaterial != rhs.mBulkMaterial)
		return 0;
	if (mPMLDirections != rhs.mPMLDirections)
		return 0;
	if (mCurrentBufferIndex != rhs.mCurrentBufferIndex)
		return 0;
	return 1;
}

bool Paint::
hasCurlBuffer() const
{
	assert(mCurlBuffers.size() == 6);
	for (unsigned int nn = 0; nn < 6; nn++)
		if (mCurlBuffers[nn] != 0L)
			return 1;
	return 0;
}

bool Paint::
hasCurlBuffer(int side) const
{
	assert(mCurlBuffers.size() == 6);
	assert(side >= 0);
	assert(side < 6);
	return (mCurlBuffers[side] != 0L);
}

const NeighborBufferDescPtr & Paint::
getCurlBuffer(int side) const
{
	assert(side >= 0);
	assert(side < 6);
	return mCurlBuffers[side];
}
	
bool Paint::
isPML() const
{
	return (mPMLDirections[0] != 0 ||
		mPMLDirections[1] != 0 ||
		mPMLDirections[2] != 0);
}

bool
operator<(const Paint & lhs, const Paint & rhs)
{
	if (&lhs == &rhs)
		return 0;
	if (lhs.mType < rhs.mType)
		return 1;
	else if (lhs.mType > rhs.mType)
		return 0;
	if (lhs.mPMLDirections < rhs.mPMLDirections)
		return 1;
	else if (lhs.mPMLDirections > rhs.mPMLDirections)
		return 0;
	else if (lhs.mCurlBuffers < rhs.mCurlBuffers)
		return 1;
	else if (lhs.mCurlBuffers > rhs.mCurlBuffers)
		return 0;
	else if (lhs.mCurrentBufferIndex < rhs.mCurrentBufferIndex)
		return 1;
	else if (lhs.mCurrentBufferIndex > rhs.mCurrentBufferIndex)
		return 0;
	else if (lhs.mBulkMaterial < rhs.mBulkMaterial) // if unused, these are 0L
		return 1;
	else if (lhs.mBulkMaterial > rhs.mBulkMaterial)
		return 0;
	
	return 0;
}

ostream &
operator<<(ostream & out, const Paint & p)
{
	if (p.mBulkMaterial != 0L)
		out << *p.mBulkMaterial;
	else
		out << "Null";
	if (p.mType == kBulkPaintType)
		out << " bulk";
	else
		out << " boundary";
	
	if (norm2(p.mPMLDirections) != 0)
		out << " PML " << p.mPMLDirections;
	
	for (int nSide = 0; nSide < 6; nSide++)
	if (p.mCurlBuffers[nSide] != 0L)
	{
		out << " (side " << nSide << " buffer " << hex <<
			(void*)p.mCurlBuffers[nSide] << dec << ")";
	}
	
	return out;
}

