/*
 *  BufferedCurrent.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _BUFFEREDCURRENT_
#define _BUFFEREDCURRENT_

#include <vector>
#include "MemoryUtilities.h"
#include "Runline.h"

class BufferedCurrent
{
public:
    BufferedCurrent();
    
    struct LocalDataE {
        float* J;
        long stride;
    };
    
    struct LocalDataH {
        float* K;
        long stride;
    };   
     
    void allocateAuxBuffers();
    
    void initLocalE(LocalDataE & data);
    void onStartRunlineE(LocalDataE & data, const SimpleRunline & rl);
    void beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    float updateJ(LocalDataE & data, float Ei, float dHj, float dHk,
        int dir0, int dir1, int dir2);
    void afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    
    void initLocalH(LocalDataH & data);
    void onStartRunlineH(LocalDataH & data, const SimpleRunline & rl);
    void beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    float updateK(LocalDataH & data, float Hi, float dEj, float dEk,
        int dir0, int dir1, int dir2);
    void afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    
private:
    std::vector<float> mSingleTimestepData;
    MemoryBuffer mBuffer; // identifying information for mSingleTimestepData.
    long mStride;  // 0 or 1 typically
};

#include "BufferedCurrent-inl.h"


#endif
