/*
 *  SetupTFSFBufferSet.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/21/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "SetupTFSFBufferSet.h"

#include "SetupGrid.h"
#include "SetupLink.h"
#include "SetupTFSFSource.h"

#include <sstream>

using namespace std;

SetupTFSFBufferSet::
SetupTFSFBufferSet(const SetupLink& inLink, SetupGridPtr auxGrid,
	const Rect3i & activeRegion) :
	mType(kLinkType),
	mOmitSideFlags(inLink.getOmitSideFlags()),
	mTFRect(inLink.getDestRect()),
	mTFSFType(inLink.getLinkType()),
	mAuxGrid(auxGrid),
	mAuxRect(inLink.getSourceRect()),
	mParams()
{
	// Augment omitted sides.  The first set of omitted sides comes from the
	// user, but these come from considerations of the dimension of the space.
	for (int ndir = 0; ndir < 3; ndir++)
	{
		if (!activeRegion.encloses(getOuterBufferRect(2*ndir)))
			omitSide(2*ndir);
		if (!activeRegion.encloses(getOuterBufferRect(2*ndir+1)))
			omitSide(2*ndir+1);
	}
}


SetupTFSFBufferSet::
SetupTFSFBufferSet(const SetupTFSFSource& inSource,
	const Rect3i & activeRegion) :
	mType(kFileType),
	mOmitSideFlags(inSource.getOmitSideFlags()),
	mTFRect(inSource.getTFRect()),
	mTFSFType(inSource.getType()),
	mAuxGrid(0L),
	mAuxRect(),
	mParams(inSource.getParameters())
{	
	// Augment omitted sides.  The first set of omitted sides comes from the
	// user, but these come from considerations of the dimension of the space. 
	for (int ndir = 0; ndir < 3; ndir++)
	{
		if (!activeRegion.encloses(getOuterBufferRect(2*ndir)))
			omitSide(2*ndir);
		if (!activeRegion.encloses(getOuterBufferRect(2*ndir+1)))
			omitSide(2*ndir+1);
	}
}

SetupTFSFBufferSet::
~SetupTFSFBufferSet()
{
}


void SetupTFSFBufferSet::
omitSide(int sideNum)
{
	assert(sideNum >= 0);
	assert(sideNum <= 5);
	mOmitSideFlags[sideNum] = 1;
}


const SetupGridPtr & SetupTFSFBufferSet:: 
getAuxGrid() const
{
	return mAuxGrid;
}

const std::string & SetupTFSFBufferSet::
getAuxGridName() const
{
	return mAuxGrid->getName();
}


Rect3i SetupTFSFBufferSet::
getOuterBufferRect(int bufferNumber) const
{
	assert(bufferNumber >= 0 && bufferNumber <= 5);
	Rect3i outerRect(mTFRect);
	int ndir = bufferNumber/2;  // 0 = x, 1 = y, 2 = z.
	
	if (bufferNumber % 2 == 0)	 // even numbers are -e_i oriented faces
	{
		outerRect.p1[ndir] -= 1; // shift to just outside the TF rect
		outerRect.p2[ndir] = outerRect.p1[ndir];
	}
	else  // odd numbers are +e_i oriented faces
	{
		outerRect.p2[ndir] += 1; // shift to just outside the TF rect
		outerRect.p1[ndir] = outerRect.p2[ndir];
	}
	
	return outerRect;
}


Rect3i SetupTFSFBufferSet::
getInnerBufferRect(int bufferNumber) const
{
	assert(bufferNumber >= 0 && bufferNumber <= 5);
	Rect3i innerRect(mTFRect);
	int ndir = bufferNumber/2;  // 0 = x, 1 = y, 2 = z.
	
	if (bufferNumber % 2 == 0)	 // even numbers are -e_i oriented faces
		innerRect.p2[ndir] = innerRect.p1[ndir];
	else  // odd numbers are -e_i oriented faces
		innerRect.p1[ndir] = innerRect.p2[ndir];
	
	return innerRect;
}

Rect3i SetupTFSFBufferSet::
getInnerAuxBufferRect(int bufferNumber) const
{
	assert(bufferNumber >= 0 && bufferNumber <= 5);
	Rect3i innerRect(mAuxRect);
	Rect3i innerTFRect(getInnerBufferRect(bufferNumber));
	int ndir = bufferNumber/2;  // 0 = x, 1 = y, 2 = z.
	
	// This task is a little more complicated than you'd think, because it's
	// possible to sensibly set up a TFSF region that extends from odd-numbered
	// to even-numbered cells (i.e. not on normal Yee cell boundaries), yet
	// still reads from a lower-dimensional aux grid (one yee-cell across).
	// We need to do a little careful checking of boundaries and such to
	// account for this.
	//Vector3b singularFlags = mAuxGrid->getSingularDimensions();
	//Vector3b periodicFlags = mAuxGrid->getPeriodicDimensions();
	//Vector3i dimensions(mAuxGrid->get_nnx(), mAuxGrid->get_nny(),
	//	mAuxGrid->get_nnz());
	
	if (bufferNumber % 2 == 0)	 // even numbers are -e_i oriented faces
	{
		/*
		if (innerRect.p1[ndir] % 2 != innerTFRect.p2[ndir] % 2)
		{
			assert(singularFlags[ndir]);
			innerRect.p1[ndir] = innerTFRect.p1[ndir] % 2;
		}
		*/
		innerRect.p2[ndir] = innerRect.p1[ndir];
	}
	else  // odd numbers are e_i oriented faces
	{
		/*
		if (innerRect.p2[ndir] % 2 != innerTFRect.p2[ndir] % 2)
		{
			assert(singularFlags[ndir]);
			innerRect.p2[ndir] = innerTFRect.p2[ndir] % 2;
		}
		*/
		innerRect.p1[ndir] = innerRect.p2[ndir];
	}
	
	return innerRect;
}



Rect3i SetupTFSFBufferSet::
getYeeBufferRect(int bufferNumber, Vector3i parity) const
{
	// The TFSF buffer set is actually six buffers (Ex... Hz) each located on
	// the six sides of the TFSF region.  This function returns the Yee cell
	// coordinates of one of these 36 buffers, based on a point that accesses
	// a buffer for neighbor data and the direction that point reaches for
	// data along.
	//
	// The function works by calculating the origin o and opposite corner p
	// of the buffer rect, in Yee cells.
	
	assert(parity == parity % 2);
	
	Rect3i innerBuffer = getInnerBufferRect(bufferNumber);
	Vector3i o(innerBuffer.p1);  // origin of the buffer in half cells
	Vector3i p(innerBuffer.p2);  // opposite corner of the buffer in half cells
	
	int direction = bufferNumber/2;
	int dir_j = (direction+1)%3;
	int dir_k = (direction+2)%3;
	
	if (o[direction] % 2 != parity[direction] % 2)
	{
		if (bufferNumber % 2 == 0)
		{
			o[direction] -= 1;
			p[direction] -= 1;
		}
		else
		{
			o[direction] += 1;
			p[direction] += 1;
		}
	}
	
	if (o[dir_j] % 2 != parity[dir_j])
		o[dir_j] += 1;
	if (o[dir_k] % 2 != parity[dir_k])
		o[dir_k] += 1;
	if (p[dir_j] % 2 != parity[dir_j])
		p[dir_j] -= 1;
	if (p[dir_k] % 2 != parity[dir_k])
		p[dir_k] -= 1;
	
	//LOG << innerBuffer << " ";
	//LOGMORE << Rect3i(o/2, p/2) << "\n";
	
	return Rect3i(o/2, p/2);
}


Rect3i SetupTFSFBufferSet::
getYeeAuxBufferRect(int bufferNumber, Vector3i pt) const
{
	assert(mType == kLinkType);
	
	Rect3i innerBuffer = getInnerAuxBufferRect(bufferNumber);
	Vector3i o(innerBuffer.p1);  // origin of the buffer in half cells
	Vector3i p(innerBuffer.p2);  // opposite corner of the buffer in half cells
	
	int direction = bufferNumber/2;
	int dir_j = (direction+1)%3;
	int dir_k = (direction+2)%3;
	
	if (o[direction] % 2 != pt[direction] % 2)
	{
		if (bufferNumber % 2 == 0)
		{
			o[direction] -= 1;
			p[direction] -= 1;
		}
		else
		{
			o[direction] += 1;
			p[direction] += 1;
		}
	}
	
	if (o[dir_j] % 2 != pt[dir_j] % 2)
		o[dir_j] += 1;
	if (o[dir_k] % 2 != pt[dir_k] % 2)
		o[dir_k] += 1;
	if (p[dir_j] % 2 != pt[dir_j] % 2)
		p[dir_j] -= 1;
	if (p[dir_k] % 2 != pt[dir_k] % 2)
		p[dir_k] -= 1;
	
	// Now we need to correct it for periodicity.  This is the coordinate-
	// wrapping part.
	Vector3b periodicFlags = mAuxGrid->getPeriodicDimensions(mAuxRect);
	Rect3i bounds(0, 0, 0,
		mAuxGrid->get_nx()-1, mAuxGrid->get_ny()-1, mAuxGrid->get_nz()-1);
	
	for (int ndim = 0; ndim < 3; ndim++)
	if (periodicFlags[ndim])
	{
		if (o[ndim] < bounds.p1[ndim])
			o[ndim] = bounds.p2[ndim];
		else if (o[ndim] > bounds.p2[ndim])
			o[ndim] = bounds.p1[ndim];
		
		if (p[ndim] < bounds.p1[ndim])
			p[ndim] = bounds.p2[ndim];
		else if (p[ndim] > bounds.p2[ndim])
			p[ndim] = bounds.p1[ndim];
	}
	
	assert(bounds.encloses(Rect3i(o/2, p/2)));
	
	return Rect3i(o/2, p/2);
}


int SetupTFSFBufferSet::
getBufferNeighborIndex(int bufferNumber, const Vector3i & pt) const
{
	Rect3i innerBuffer = getInnerBufferRect(bufferNumber);
	Vector3i o(innerBuffer.p1);  // origin of the buffer
	Vector3i p(innerBuffer.p2);  // opposite corner of the buffer
	
	int direction = bufferNumber/2;
	int dir_j = (direction+1)%3;
	int dir_k = (direction+2)%3;
	
	o[direction] = pt[direction];
	p[direction] = pt[direction];
	
	if (o[dir_j] % 2 != pt[dir_j] % 2)
		o[dir_j] += 1;
	if (o[dir_k] % 2 != pt[dir_k] % 2)
		o[dir_k] += 1;
	if (p[dir_j] % 2 != pt[dir_j] % 2)
		p[dir_j] -= 1;
	if (p[dir_k] % 2 != pt[dir_k] % 2)
		p[dir_k] -= 1;
	
	Vector3i dimensions = (p - o)/2 + Vector3i(1,1,1);
	
	Vector3i dp = pt - o;
	
	int index = dp[0] + dimensions[0]*dp[1] + dimensions[0]*dimensions[1]*dp[2];
	
	return index;
}

Vector3i SetupTFSFBufferSet::
getBufferSize(int bufferNumber, const Vector3i & pt) const
{
	Rect3i innerBuffer = getInnerBufferRect(bufferNumber);
	Vector3i o(innerBuffer.p1);  // origin of the buffer
	Vector3i p(innerBuffer.p2);  // opposite corner of the buffer
	
	int direction = bufferNumber/2;
	int dir_j = (direction+1)%3;
	int dir_k = (direction+2)%3;
	
	o[direction] = pt[direction];
	p[direction] = pt[direction];
	
	if (o[dir_j] % 2 != pt[dir_j] % 2)
		o[dir_j] += 1;
	if (o[dir_k] % 2 != pt[dir_k] % 2)
		o[dir_k] += 1;
	if (p[dir_j] % 2 != pt[dir_j] % 2)
		p[dir_j] -= 1;
	if (p[dir_k] % 2 != pt[dir_k] % 2)
		p[dir_k] -= 1;
	
	Vector3i dimensions = (p - o)/2 + Vector3i(1,1,1);
	
	return dimensions;
}


TFSFType SetupTFSFBufferSet::
getBufferType(int bufferNumber, const Vector3i & field_pt) const
{
	bool isOuterType;
	int ndir = bufferNumber/2;
	int lowHigh = bufferNumber%2;
	
	if (lowHigh == 0) // low-side correction
	{
		if (field_pt[ndir] % 2 == mTFRect.p1[ndir] % 2)
			isOuterType = 1;
		else
			isOuterType = 0;
	}
	else
	{
		if (field_pt[ndir] % 2 == mTFRect.p2[ndir] % 2)
			isOuterType = 1;
		else
			isOuterType = 0;
	}
	
	if (mTFSFType == kTFType)
	{
		if (isOuterType)
			return kSFType;
		else
			return kTFType;
	}
	else
	{
		if (isOuterType)
			return kTFType;
		else
			return kSFType;
	}
}

string SetupTFSFBufferSet::
getDescription() const
{
    ostringstream str;
    str << "TFSF type = ";
    if (mTFSFType == kNoType)
        str << "no type,";
    else if (mTFSFType == kTFType)
        str << "TF type,";
    else if (mTFSFType == kSFType)
        str << "SF type,";
    else
        str << "unknown (this is bad),";
    
	str << " buffer type = ";
	if (mType == kFileType)
	{
		str << "file type,";
		str << " TFRect = " << mTFRect;
	}
	else if (mType == kLinkType)
	{
		str << "link type,";
		str << " aux grid name = " << mAuxGrid;
		str << " TFRect = " << mTFRect;
		str << " aux Rect = " << mAuxRect;
	}
	else
		str << "unknown (this is bad),";
	
    
    if (mOmitSideFlags.count())
    {
        str << "omitting sides";
		/*
        for (set<Vector3i>::iterator itr = mOmittedSides.begin();
            itr != mOmittedSides.end(); itr++)
            str << " " << *itr;
		*/
		for (int nn = 0; nn < 6; nn++)
		if (mOmitSideFlags[nn])
			str << " " << nn;
    }
    str << "\n";
    
    return str.str();
}


bool operator<(const SetupTFSFBufferSet & lhs, const SetupTFSFBufferSet & rhs)
{
	if (lhs.mType < rhs.mType)
		return 1;
	else if (lhs.mType > rhs.mType)
		return 0;
	else if (lhs.mTFSFType < rhs.mTFSFType)
		return 1;
	else if (lhs.mTFSFType > rhs.mTFSFType)
		return 0;
	else if (lhs.mTFRect < rhs.mTFRect)
		return 1;
	else if (lhs.mTFRect > rhs.mTFRect)
		return 0;
	
	else if (lhs.mType == kLinkType)
	{
		if (lhs.mAuxGrid < rhs.mAuxGrid)
			return 1;
		else if (lhs.mAuxGrid > rhs.mAuxGrid)
			return 0;
		else if (lhs.mAuxRect < rhs.mAuxRect)
			return 1;
	}
	else if (lhs.mType == kFileType)
	{
		if (lhs.mParams < rhs.mParams)
			return 1;
		else if (lhs.mParams > rhs.mParams)
			return 0;
	}
	else if (lhs.mOmitSideFlags.to_ulong() < rhs.mOmitSideFlags.to_ulong())
		return 1;
	else if (lhs.mOmitSideFlags.to_ulong() > rhs.mOmitSideFlags.to_ulong())
		return 0;
	
	return 0;
}






