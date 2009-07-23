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
    void onStartRunlineEx(LocalDataE & data, const SimpleAuxRunline & rl);
    void onStartRunlineEy(LocalDataE & data, const SimpleAuxRunline & rl);
    void onStartRunlineEz(LocalDataE & data, const SimpleAuxRunline & rl);
    void beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);    
    float updateEx(LocalDataE & data, float Ei, float dHj, float dHk, float Ji);
    float updateEy(LocalDataE & data, float Ei, float dHj, float dHk, float Ji);
    float updateEz(LocalDataE & data, float Ei, float dHj, float dHk, float Ji);
    void afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    
    void initLocalH(LocalDataH & data);
    void onStartRunlineHx(LocalDataH & data, const SimpleAuxRunline & rl);
    void onStartRunlineHy(LocalDataH & data, const SimpleAuxRunline & rl);
    void onStartRunlineHz(LocalDataH & data, const SimpleAuxRunline & rl);
    void beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    float updateHx(LocalDataH & data, float Hi, float dEj, float dEk, float Ki);
    float updateHy(LocalDataH & data, float Hi, float dEj, float dEk, float Ki);
    float updateHz(LocalDataH & data, float Hi, float dEj, float dEk, float Ki);
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
onStartRunlineEx(LocalDataE & data, const SimpleAuxRunline & rl)
{
    data.Ji = &(mCurrents[0][rl.auxIndex]);
}

inline void DrudeModel1::
onStartRunlineEy(LocalDataE & data, const SimpleAuxRunline & rl)
{
    data.Ji = &(mCurrents[1][rl.auxIndex]);
}

inline void DrudeModel1::
onStartRunlineEz(LocalDataE & data, const SimpleAuxRunline & rl)
{
    data.Ji = &(mCurrents[2][rl.auxIndex]);
}

inline void DrudeModel1::
beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}

inline float DrudeModel1::
updateEx(LocalDataE & data, float Ei, float dHj, float dHk, float Ji)
{
    //LOG << "ce1 = " << data.ce1 << "\n";
//    if (Ji != 0)
//        LOG << "dHj " << dHj << " dHk " << dHk << " Ji " << Ji << "\n"; 
    return Ei + data.ce*(dHk - dHj - Ji - *data.Ji++);
}

inline float DrudeModel1::
updateEy(LocalDataE & data, float Ei, float dHj, float dHk, float Ji)
{
    //LOG << "ce1 = " << data.ce1 << "\n";
//    if (Ji != 0)
//        LOG << "dHj " << dHj << " dHk " << dHk << " Ji " << Ji << "\n"; 
    return Ei + data.ce*(dHk - dHj - Ji - *data.Ji++);
}

inline float DrudeModel1::
updateEz(LocalDataE & data, float Ei, float dHj, float dHk, float Ji)
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
onStartRunlineHx(LocalDataH & data, const SimpleAuxRunline & rl) {}
inline void DrudeModel1::
onStartRunlineHy(LocalDataH & data, const SimpleAuxRunline & rl) {}
inline void DrudeModel1::
onStartRunlineHz(LocalDataH & data, const SimpleAuxRunline & rl) {}

inline void DrudeModel1::
beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}

inline float DrudeModel1::
updateHx(LocalDataH & data, float Hi, float dEj, float dEk, float Ki)
{
    return Hi + data.ch*(-dEk + dEj - Ki);
}

inline float DrudeModel1::
updateHy(LocalDataH & data, float Hi, float dEj, float dEk, float Ki)
{
    return Hi + data.ch*(-dEk + dEj - Ki);
}

inline float DrudeModel1::
updateHz(LocalDataH & data, float Hi, float dEj, float dEk, float Ki)
{
    return Hi + data.ch*(-dEk + dEj - Ki);
}

inline void DrudeModel1::
afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}



#endif
