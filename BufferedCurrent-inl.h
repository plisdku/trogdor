/*
 *  BufferedCurrent-inl.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "BufferedCurrent.h"


inline void BufferedCurrent::
initLocalE(LocalDataE & data, int dir0)
{
    /*
    mSourceOfData->loadSingleTimestepE(mSingleTimestepDataE[dir0]);
    data.J = &(mSourceOfDataE[dir0][0]);
    
    data.mask = &(mMaskE[dir0][0]);
    
    if (mSingleTimestepDataE[dir0].size() > 1)
        data.stride = 1;
    else
        data.stride = 0;
    
    if (mMaskE[dir0].size() > 1)
        data.maskStride = 1;
    else
        data.maskStride = 0;
    */
}

inline void BufferedCurrent::
onStartRunlineE(LocalDataE & data, const SimpleRunline & rl)
{
    // Nothing... easiest that way.
}

inline void BufferedCurrent::
beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}

inline float BufferedCurrent::
updateJ(LocalDataE & data, float Ei, float dHj, float dHk,
    int dir0, int dir1, int dir2)
{
    return data.polarizationFactor * (*data.J) * (*data.mask);
}

inline void BufferedCurrent::
afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
    data.J += data.stride;
    data.mask += data.maskStride;
}

inline void BufferedCurrent::
initLocalH(LocalDataH & data, int dir0)
{
    /*
    mSourceOfData->loadSingleTimestepH(mSingleTimestepDataH[dir0]);
    data.K = &(mSourceOfDataH[dir0][0]);
    
    data.mask = &(mMaskH[dir0][0]);
    
    if (mSingleTimestepDataH[dir0].size() > 1)
        data.stride = 1;
    else
        data.stride = 0;
    
    if (mMaskH[dir0].size() > 1)
        data.maskStride = 1;
    else
        data.maskStride = 0;
    */
}

inline void BufferedCurrent::
onStartRunlineH(LocalDataH & data, const SimpleRunline & rl)
{
}

inline void BufferedCurrent::
beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}

inline float BufferedCurrent::
updateK(LocalDataH & data, float Hi, float dEj, float dEk,
    int dir0, int dir1, int dir2)
{
    return data.polarizationFactor * (*data.K) * (*data.mask);
}

inline void BufferedCurrent::
afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
    data.K += data.stride;
    data.mask += data.maskStride;
}

