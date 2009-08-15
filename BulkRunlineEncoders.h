/*
 *  BulkRunlineEncoders.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SIMPLESETUPMATERIAL_
#define _SIMPLESETUPMATERIAL_

#include "RunlineEncoder.h"
#include "Pointer.h"
#include "geometry.h"
#include "MemoryUtilities.h"
#include "Runline.h"
#include <vector>

class Paint;

class VoxelizedPartition;
class GridDescription;


class BulkRunlineEncoder : public RunlineEncoder
{
public:
	BulkRunlineEncoder();
	virtual ~BulkRunlineEncoder();
    
	// Runline handling
	virtual void startRunline(const VoxelizedPartition & vp,
		const Vector3i & startPos);
	virtual bool canContinueRunline(const VoxelizedPartition & vp,
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint,
        int runlineDirection) const;
	virtual void continueRunline(const Vector3i & newPos);
	virtual void endRunline();
	
	virtual void printRunlines(std::ostream & out) const;
	
    const std::vector<SBMRunlinePtr> & getRunlinesE(int dir) const
        { return mRunlinesE[dir]; }
    const std::vector<SBMRunlinePtr> & getRunlinesH(int dir) const
        { return mRunlinesH[dir]; }
    Paint* getPaint() const { return mStartPaint; }
protected:
	std::vector<SBMRunlinePtr> mRunlinesE[3];
	std::vector<SBMRunlinePtr> mRunlinesH[3];
	
	// Runline generation storage
	SBMRunline mCurrentRunline;
	Paint* mStartPaint;
	Vector3i mStartPoint;
	int mStartOctant;
	long mStartNeighborIndices[6];
	bool mUsedNeighborIndices[6];
};

class BulkPMLRunlineEncoder : public RunlineEncoder
{
public:
	BulkPMLRunlineEncoder();
	
	// Runline handling
	virtual void startRunline(const VoxelizedPartition & vp,
		const Vector3i & startPos);
	virtual bool canContinueRunline(const VoxelizedPartition & vp,
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint,
        int runlineDirection) const;
	virtual void continueRunline(const Vector3i & newPos);
	virtual void endRunline();
	
	virtual void printRunlines(std::ostream & out) const;
	    
    const std::vector<SBPMRunlinePtr> & getRunlinesE(int dir) const
        { return mRunlinesE[dir]; }
    const std::vector<SBPMRunlinePtr> & getRunlinesH(int dir) const
        { return mRunlinesH[dir]; }
    
protected:
	std::vector<SBPMRunlinePtr> mRunlinesE[3];
	std::vector<SBPMRunlinePtr> mRunlinesH[3];
	
	// Runline generation storage
	SBPMRunline mCurrentRunline;
	Paint* mStartPaint;
	Vector3i mStartPoint;
	int mStartOctant;
	long mStartNeighborIndices[6];
	bool mUsedNeighborIndices[6];
	Rect3i mPMLRect;
};














#endif
