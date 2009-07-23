/*
 *  NullPML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _NULLPML_
#define _NULLPML_



class NullPML
{
public:
    struct LocalDataEx {};
    struct LocalDataEy {};
    struct LocalDataEz {};
    struct LocalDataHx {}; 
    struct LocalDataHy {}; 
    struct LocalDataHz {};   
     
    void allocateAuxBuffers() {}
    
    void initLocalEx(LocalDataEx & data) {}
    void initLocalEy(LocalDataEy & data) {}
    void initLocalEz(LocalDataEz & data) {}
    void onStartRunlineEx(LocalDataEx & data, const SimpleRunline & rl) {}
    void onStartRunlineEy(LocalDataEy & data, const SimpleRunline & rl) {}
    void onStartRunlineEz(LocalDataEz & data, const SimpleRunline & rl) {}
    void beforeUpdateEx(LocalDataEx & data, float Ei, float dHj, float dHk) {}
    void beforeUpdateEy(LocalDataEy & data, float Ei, float dHj, float dHk) {}
    void beforeUpdateEz(LocalDataEz & data, float Ei, float dHj, float dHk) {} 
    float updateJx(LocalDataEx & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    float updateJy(LocalDataEy & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    float updateJz(LocalDataEz & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    void afterUpdateEx(LocalDataEx & data, float Ei, float dHj, float dHk) {}
    void afterUpdateEy(LocalDataEy & data, float Ei, float dHj, float dHk) {}
    void afterUpdateEz(LocalDataEz & data, float Ei, float dHj, float dHk) {}
    
    void initLocalHx(LocalDataHx & data) {}
    void initLocalHy(LocalDataHy & data) {}
    void initLocalHz(LocalDataHz & data) {}
    void onStartRunlineHx(LocalDataHx & data, const SimpleRunline & rl) {}
    void onStartRunlineHy(LocalDataHy & data, const SimpleRunline & rl) {}
    void onStartRunlineHz(LocalDataHz & data, const SimpleRunline & rl) {}
    void beforeUpdateHx(LocalDataHx & data, float Hi, float dEj, float dEk) {}
    void beforeUpdateHy(LocalDataHy & data, float Hi, float dEj, float dEk) {}
    void beforeUpdateHz(LocalDataHz & data, float Hi, float dEj, float dEk) {}
    float updateKx(LocalDataHx & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    float updateKy(LocalDataHy & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    float updateKz(LocalDataHz & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    void afterUpdateHx(LocalDataHx & data, float Hi, float dEj, float dEk) {}
    void afterUpdateHy(LocalDataHy & data, float Hi, float dEj, float dEk) {}
    void afterUpdateHz(LocalDataHz & data, float Hi, float dEj, float dEk) {}
};


#endif
