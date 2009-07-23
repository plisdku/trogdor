/*
 *  NullCurrent.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _NULLCURRENT_
#define _NULLCURRENT_

class NullCurrent
{
public:
    struct LocalDataE {};
    struct LocalDataH {};   
     
    void allocateAuxBuffers() {}
    
    void initLocalE(LocalDataE & data) {}
    void onStartRunlineE(LocalDataE & data, const SimpleRunline & rl) {}
    void beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk) {}    
    float updateJx(LocalDataE & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    float updateJy(LocalDataE & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    float updateJz(LocalDataE & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    void afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk) {}
    
    void initLocalH(LocalDataH & data) {}
    void onStartRunlineH(LocalDataH & data, const SimpleRunline & rl) {}
    void beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk) {}
    float updateKx(LocalDataH & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    float updateKy(LocalDataH & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    float updateKz(LocalDataH & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    void afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk) {}
};


#endif
