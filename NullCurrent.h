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

class CurrentSource;

class NullCurrent
{
public:
    struct LocalDataE {};
    struct LocalDataH {};   
    
    void setCurrentSource(CurrentSource* source) {}
    void allocateAuxBuffers() {}
    
    void initLocalE(LocalDataE & data, int dir0) {}
    void onStartRunlineE(LocalDataE & data, const SimpleRunline & rl) {}
    void beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk) {}    
    float updateJ(LocalDataE & data, float Ei, float dHj, float dHk,
        int dir0, int dir1, int dir2)
        { return 0.0; }
    
    void afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk) {}
    
    void initLocalH(LocalDataH & data, int dir0) {}
    void onStartRunlineH(LocalDataH & data, const SimpleRunline & rl) {}
    void beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk) {}  
    float updateK(LocalDataH & data, float Hi, float dEj, float dEk,
        int dir0, int dir1, int dir2)
        { return 0.0; }
    
    void afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk) {}
};


#endif
