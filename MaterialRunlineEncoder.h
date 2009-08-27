/*
 *  MaterialRunlineEncoder.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/19/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _MATERIALRUNLINEENCODER_
#define _MATERIALRUNLINEENCODER_

#include "RunlineEncoder.h"
#include "Runline.h"

#include <vector>

class BulkMaterialRLE : public RunlineEncoder
{
public:
    BulkMaterialRLE();
    
    virtual void endRunline(const VoxelizedPartition & vp,
        const Vector3i & lastHalfCell);
    
    const std::vector<SBMRunlinePtr> & runlinesE(int dir) const
        { return mRunlinesE[dir]; }
    const std::vector<SBMRunlinePtr> & runlinesH(int dir) const
        { return mRunlinesH[dir]; }
protected:
	std::vector<SBMRunlinePtr> mRunlinesE[3];
	std::vector<SBMRunlinePtr> mRunlinesH[3];
};

class BulkPMLRLE : public RunlineEncoder
{
public:
    BulkPMLRLE();
    
    virtual void endRunline(const VoxelizedPartition & vp,
        const Vector3i & lastHalfCell);
    
    const std::vector<SBPMRunlinePtr> & runlinesE(int dir) const
        { return mRunlinesE[dir]; }
    const std::vector<SBPMRunlinePtr> & runlinesH(int dir) const
        { return mRunlinesH[dir]; }
protected:
	std::vector<SBPMRunlinePtr> mRunlinesE[3];
	std::vector<SBPMRunlinePtr> mRunlinesH[3];
};





#endif
