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
    
    template<int MEMORYDIRECTION>
    struct LocalDataE
    {
    };
    template<int MEMORYDIRECTION>
    struct LocalDataH
    {
    };
    
    void onStartRunlineE(LocalDataE<0> & data, const SimpleRunline & rl,
        int dir0, int dir1, int dir2) {}
    void onStartRunlineE(LocalDataE<1> & data, const SimpleRunline & rl,
        int dir0, int dir1, int dir2) {}
    void onStartRunlineE(LocalDataE<2> & data, const SimpleRunline & rl,
        int dir0, int dir1, int dir2) {}
    
    float updateJ(LocalDataE<0> & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    float updateJ(LocalDataE<1> & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    float updateJ(LocalDataE<2> & data, float Ei, float dHj, float dHk)
        { return 0.0; }
    
    void onStartRunlineH(LocalDataH<0> & data, const SimpleRunline & rl,
        int dir0, int dir1, int dir2) {}
    void onStartRunlineH(LocalDataH<1> & data, const SimpleRunline & rl,
        int dir0, int dir1, int dir2) {}
    void onStartRunlineH(LocalDataH<2> & data, const SimpleRunline & rl,
        int dir0, int dir1, int dir2) {}
    
    float updateK(LocalDataH<0> & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    float updateK(LocalDataH<1> & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    float updateK(LocalDataH<2> & data, float Hi, float dEj, float dEk)
        { return 0.0; }
    
    void allocateAuxBuffers() {}
};


#endif
