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
#include "CurrentSource.h"

class BufferedCurrent
{
public:
    BufferedCurrent();
    
    struct LocalDataE {
        //float* J;
        //long stride;
        float J;
    };
    
    struct LocalDataH {
        //float* K;
        //long stride;
        float K;
    };
    
    void setCurrentSource(CurrentSource* source);
    void allocateAuxBuffers();
    
    void initLocalE(LocalDataE & data, int dir0);
    void onStartRunlineE(LocalDataE & data, const SimpleRunline & rl);
    void beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    float updateJ(LocalDataE & data, float Ei, float dHj, float dHk,
        int dir0, int dir1, int dir2);
    void afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    
    void initLocalH(LocalDataH & data, int dir0);
    void onStartRunlineH(LocalDataH & data, const SimpleRunline & rl);
    void beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    float updateK(LocalDataH & data, float Hi, float dEj, float dEk,
        int dir0, int dir1, int dir2);
    void afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    
private:
    CurrentSource* mSourceOfData;
    //std::vector<float> mSingleTimestepData;
    //MemoryBuffer mBuffer; // identifying information for mSingleTimestepData.
    //long mStride;  // 0 or 1 typically
};

#include "BufferedCurrent-inl.h"


#endif
