/*
 *  DrudeModel1.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _DRUDEMODEL1_
#define _DRUDEMODEL1_

#include "SimpleSetupMaterial.h"
#include "SimpleMaterialTemplates.h"
#include "SimulationDescriptionPredeclarations.h"
#include "Map.h"
#include "MemoryUtilities.h"
#include <string>

/**
    Drude metal model with one pole or whatever.
    A running equation uses the slash f dollar sign thing, like \f$\sqrt{x}\f$.
    However, if I want to use a display equation I can try it like this,
    
    \f[
        |A| = \left( 5 \right)
    \f]
*/

class DrudeModel1
{
public:
    DrudeModel1(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt);
    
    virtual ~DrudeModel1() {}
    
    std::string getModelName() const;
    virtual void writeJ(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void allocateAuxBuffers();
    
    struct LocalDataE
    {
        float cj1;
        float cj2;
        float ce;
        float* Ji;
    };
    
    struct LocalDataH
    {
        float ch;
    };
    
    void initLocalE(LocalDataE & data);
    void onStartRunlineE(LocalDataE & data, const SimpleAuxRunline & rl,
        int dir);
    void beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    float updateE(LocalDataE & data, int dir, float Ei, float dHj, float dHk,
        float Ji);
    void afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    
    void initLocalH(LocalDataH & data);
    void onStartRunlineH(LocalDataH & data, const SimpleAuxRunline & rl,
        int dir);
    void beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    float updateH(LocalDataH & data, int dir, float Hi, float dEj, float dEk,
        float Ki);
    void afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    
private:
    Vector3f mDxyz;
    float mDt;
    
    float m_epsrinf;
    float m_mur;
    float m_omegap;
    float m_tauc;
    
    float m_cj1, m_cj2, m_ce, m_ch;
    
    std::vector<float> mCurrents[3];
    MemoryBufferPtr mCurrentBuffers[3];
};


inline void DrudeModel1::
initLocalE(LocalDataE & data)
{
    data.ce = m_ce;
    data.cj1 = m_cj1;
    data.cj2 = m_cj2;
}

inline void DrudeModel1::
onStartRunlineE(LocalDataE & data, const SimpleAuxRunline & rl, int dir)
{
    data.Ji = &(mCurrents[dir][rl.auxIndex]);
}

inline void DrudeModel1::
beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}

inline float DrudeModel1::
updateE(LocalDataE & data, int dir, float Ei, float dHj, float dHk, float Ji)
{
    //LOG << "ce1 = " << data.ce1 << "\n";
//    if (Ji != 0)
//        LOG << "dHj " << dHj << " dHk " << dHk << " Ji " << Ji << "\n"; 
    return Ei + data.ce*(dHk - dHj - Ji - *data.Ji++);
}

inline void DrudeModel1::
afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}

inline void DrudeModel1::
initLocalH(LocalDataH & data)
{
    data.ch = m_ch;
}

inline void DrudeModel1::
onStartRunlineH(LocalDataH & data, const SimpleAuxRunline & rl, int dir)
{
}

inline void DrudeModel1::
beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}

inline float DrudeModel1::
updateH(LocalDataH & data, int dir, float Hi, float dEj, float dEk, float Ki)
{
    return Hi + data.ch*(-dEk + dEj - Ki);
}

inline void DrudeModel1::
afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}



#endif
