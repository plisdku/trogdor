/*
 *  GeneralRunlineEncoder.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "GeneralRunlineEncoder.h"

#include "VoxelizedPartition.h"
#include "Paint.h"
#include "Log.h"

using namespace std;

GeneralRunlineEncoder::
GeneralRunlineEncoder()
{
}

GeneralRunlineEncoder::
~GeneralRunlineEncoder()
{
}


// Things called by the... iterator on the grid, I guess it's called.
void GeneralRunlineEncoder::
startRunline(const VoxelizedPartition & vp, const Vector3i & beginningHalfCell)
{
    mFirstHalfCell = beginningHalfCell;
}

bool GeneralRunlineEncoder::
canContinueRunline(const VoxelizedPartition & vp,
    const Vector3i & oldHalfCell,
    const Vector3i & newHalfCell,
    const Paint* newPaint) const
{
}

void GeneralRunlineEncoder::
continueRunline()
{
    mLength++;
}

// Hey user, you can implement this to make it do your bidding!
void GeneralRunlineEncoder::
endRunline()
{
    LOG << "Ending runline, doing nothing.\n";
}
