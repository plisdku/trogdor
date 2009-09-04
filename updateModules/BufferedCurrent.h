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
    BufferedCurrent(std::vector<int> numCellsE, std::vector<int> numCellsH);
    
    struct LocalDataE {
        float* J;
        const float* mask;
        float polarizationFactor;
        long stride;
        long maskStride;
    };
    
    struct LocalDataH {
        float* K;
        const float* mask;
        float polarizationFactor;
        long stride;
        long maskStride;
    };
    
    void setCurrentSource(CurrentSource* source, int myMaterialID);
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
    
    bool mHasMask;
    BufferPointer mPointerJ[3];
    BufferPointer mPointerK[3];
    BufferPointer mPointerMaskJ[3];
    BufferPointer mPointerMaskK[3];
    int mStride;
    int mStrideMask;
    
    Vector3f mPolarizationVector; // defaults to (1.0, 1.0, 1.0); always used
    
    //std::vector<float> mSingleTimestepData;
    //MemoryBuffer mBuffer; // identifying information for mSingleTimestepData.
    //long mStride;  // 0 or 1 typically
};

#include "BufferedCurrent-inl.h"


#endif
