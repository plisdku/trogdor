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
    data.J = mSourceOfData->getJ(dir0);
}

inline void BufferedCurrent::
onStartRunlineE(LocalDataE & data, const SimpleRunline & rl)
{
}

inline void BufferedCurrent::
beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}

inline float BufferedCurrent::
updateJ(LocalDataE & data, float Ei, float dHj, float dHk,
    int dir0, int dir1, int dir2)
{
    return data.J;
    //return *data.J;
}

inline void BufferedCurrent::
afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
    //data.J += data.stride;
}

inline void BufferedCurrent::
initLocalH(LocalDataH & data, int dir0)
{
    data.K = mSourceOfData->getK(dir0);
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
    return data.K;
    //return *data.K;
}

inline void BufferedCurrent::
afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
    //data.K += data.stride;
}

