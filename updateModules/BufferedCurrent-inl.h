/*
 *  BufferedCurrent-inl.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "BufferedCurrent.h"

const float ONE = 1.0; // we'll access this through a pointer often.

inline void BufferedCurrent::
initLocalE(LocalDataE & data, int dir0)
{
    assert(mPointerJ[dir0].buffer() != 0L);
    assert(mPointerJ[dir0].buffer()->headPointer() != 0L);
    
    data.J = mPointerJ[dir0].pointer();
    
    if (mHasMask)
    {
        assert(mPointerMaskJ[dir0].buffer() != 0L);
        assert(mPointerMaskJ[dir0].buffer()->headPointer() != 0L);
        
        data.mask = mPointerMaskJ[dir0].pointer();
    }
    else
    {
        data.mask = &ONE; // use constant at top of file
    }
    
    data.stride = mStride;
    data.maskStride = mStrideMask;
    data.polarizationFactor = mPolarizationVector[dir0];
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
    //if (*data.J != 0)
    //    LOG << "J" << char(dir0 + 'x') << " = " << *data.J << "\n";
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
    assert(mPointerK[dir0].buffer() != 0L);
    assert(mPointerK[dir0].buffer()->headPointer() != 0L);
    
    data.K = mPointerK[dir0].pointer();
    
    if (mHasMask)
    {
        assert(mPointerMaskK[dir0].buffer() != 0L);
        assert(mPointerMaskK[dir0].buffer()->headPointer() != 0L);
        
        data.mask = mPointerMaskK[dir0].pointer();
    }
    else
    {
        data.mask = &ONE; // use constant at top of file
    }
    
    data.stride = mStride;
    data.maskStride = mStrideMask;
    data.polarizationFactor = mPolarizationVector[dir0];
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

