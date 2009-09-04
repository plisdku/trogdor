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
#include "HuygensSurface.h"
#include <sstream>
using namespace std;

Map<Paint, PaintPtr> Paint::mPalette;

#pragma mark *** Paint ***

Paint::
Paint(PaintType inType) :
	mBasePaint(this),
	mBaseUpdatePaint(this),
	mType(inType),
	mPMLDirections(0,0,0),
	mCurlBuffers(6),
	mCurrentSource(0L),
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
Paint(const Paint & copyMe)
{
	mBasePaint = copyMe.mBasePaint;
	mBaseUpdatePaint = copyMe.mBaseUpdatePaint;
	mType = copyMe.mType;
	mPMLDirections = copyMe.mPMLDirections;
	mCurlBuffers = copyMe.mCurlBuffers;
	mCurrentSource = copyMe.mCurrentSource;
	mBulkMaterial = copyMe.mBulkMaterial;
}


Paint::
Paint(const MaterialDescPtr & material) :
	mBasePaint(this),
	mBaseUpdatePaint(this),
	mType(kBulkPaintType),
	mPMLDirections(0,0,0),
	mCurlBuffers(6),
	mCurrentSource(0L),
	mBulkMaterial(material)
{
	assert(material != 0L);
}

string Paint::
fullName() const
{
    ostringstream str;
    str << mBulkMaterial->name();
    
	if (mType == kBulkPaintType)
		str << " bulk";
	else
		str << " boundary";
	
	if (norm2(mPMLDirections) != 0)
		str << " PML " << mPMLDirections;
    
    if (mCurrentSource != 0L)
        str << " current";
    
    return str.str();
}

Paint* Paint::
paint(const MaterialDescPtr & material)
{
	PaintPtr bulkPaint(new Paint(material));
	if (mPalette.count(*bulkPaint) != 0)
		return mPalette[*bulkPaint];
	mPalette[*bulkPaint] = bulkPaint;
	return bulkPaint;
}

Paint* Paint::
withPML(Vector3i pmlDir) const
{
	assert(!isPML());
	PaintPtr p(new Paint(*this));
	p->mBaseUpdatePaint = p;
	p->mPMLDirections = pmlDir;
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
}

Paint* Paint::
withCurlBuffer(int side, const NeighborBuffer* curlBuffer) const
{
	assert(mCurlBuffers[side] == 0L);
	PaintPtr p(new Paint(*this));
	p->mCurlBuffers[side] = curlBuffer;
	
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
}

Paint* Paint::
withCurrentSource(CurrentSourceDescPtr currentSource) const
{
	assert(mCurrentSource == 0L);
	PaintPtr p(new Paint(*this));
	p->mBaseUpdatePaint = p;
	p->mCurrentSource = currentSource;
	
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
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
	if (mCurrentSource != rhs.mCurrentSource)
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

bool Paint::
hasCurrentSource() const
{
    return (mCurrentSource != 0L);
}

const NeighborBuffer* Paint::
curlBuffer(int side) const
{
	assert(side >= 0);
	assert(side < 6);
	return mCurlBuffers[side];
}

MaterialDescPtr Paint::
bulkMaterial() const
{
    return mBulkMaterial;
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
	else if (lhs.mCurrentSource < rhs.mCurrentSource)
		return 1;
	else if (lhs.mCurrentSource > rhs.mCurrentSource)
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
    
    if (p.mCurrentSource != 0L)
        out << " (current source too!)"; 
	
	return out;
}

