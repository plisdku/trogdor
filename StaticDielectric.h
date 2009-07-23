/*
 *  StaticDielectric.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _STATICDIELECTRIC_
#define _STATICDIELECTRIC_

#include "SimulationDescription.h"
#include "SimpleSetupMaterial.h"
#include "SimpleMaterialTemplates.h"
#include <string>
#include "Log.h"

class StaticDielectric
{
public:
    StaticDielectric(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt);
    
    std::string getModelName() const;
    void allocateAuxBuffers();
    
    struct LocalDataE
    {
        float ce1;
    };
    
    struct LocalDataH
    {
        float ch1;
    };
    
    void initLocalE(LocalDataE & data);
    void onStartRunlineEx(LocalDataE & data, const SimpleRunline & rl);
    void onStartRunlineEy(LocalDataE & data, const SimpleRunline & rl);
    void onStartRunlineEz(LocalDataE & data, const SimpleRunline & rl);
    void beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);    
    float updateEx(LocalDataE & data, float Ei, float dHj, float dHk, float Ji);
    float updateEy(LocalDataE & data, float Ei, float dHj, float dHk, float Ji);
    float updateEz(LocalDataE & data, float Ei, float dHj, float dHk, float Ji);
    void afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    
    void initLocalH(LocalDataH & data);
    void onStartRunlineHx(LocalDataH & data, const SimpleRunline & rl);
    void onStartRunlineHy(LocalDataH & data, const SimpleRunline & rl);
    void onStartRunlineHz(LocalDataH & data, const SimpleRunline & rl);
    void beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    float updateHx(LocalDataH & data, float Hi, float dEj, float dEk, float Ki);
    float updateHy(LocalDataH & data, float Hi, float dEj, float dEk, float Ki);
    float updateHz(LocalDataH & data, float Hi, float dEj, float dEk, float Ki);
    void afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);

private:
    Vector3f mDxyz;
    float mDt;
    
    float m_ce1, m_ch1;
    
    float m_epsr;
    float m_mur;
};

inline void StaticDielectric::
initLocalE(LocalDataE & data)
{
    data.ce1 = m_ce1;
}

inline void StaticDielectric::
onStartRunlineEx(LocalDataE & data, const SimpleRunline & rl) {}
inline void StaticDielectric::
onStartRunlineEy(LocalDataE & data, const SimpleRunline & rl) {}
inline void StaticDielectric::
onStartRunlineEz(LocalDataE & data, const SimpleRunline & rl) {}

inline void StaticDielectric::
beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}

inline float StaticDielectric::
updateEx(LocalDataE & data, float Ei, float dHj, float dHk, float Ji)
{
    //LOG << "ce1 = " << data.ce1 << "\n";
//    if (Ji != 0)
//        LOG << "dHj " << dHj << " dHk " << dHk << " Ji " << Ji << "\n"; 
    return Ei + data.ce1*(dHk - dHj - Ji);
}

inline float StaticDielectric::
updateEy(LocalDataE & data, float Ei, float dHj, float dHk, float Ji)
{
    //LOG << "ce1 = " << data.ce1 << "\n";
//    if (Ji != 0)
//        LOG << "dHj " << dHj << " dHk " << dHk << " Ji " << Ji << "\n"; 
    return Ei + data.ce1*(dHk - dHj - Ji);
}

inline float StaticDielectric::
updateEz(LocalDataE & data, float Ei, float dHj, float dHk, float Ji)
{
    //LOG << "ce1 = " << data.ce1 << "\n";
//    if (Ji != 0)
//        LOG << "dHj " << dHj << " dHk " << dHk << " Ji " << Ji << "\n"; 
    return Ei + data.ce1*(dHk - dHj - Ji);
}

inline void StaticDielectric::
afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}


inline void StaticDielectric::
initLocalH(LocalDataH & data)
{
    data.ch1 = m_ch1;
}

inline void StaticDielectric::
onStartRunlineHx(LocalDataH & data, const SimpleRunline & rl) {}
inline void StaticDielectric::
onStartRunlineHy(LocalDataH & data, const SimpleRunline & rl) {}
inline void StaticDielectric::
onStartRunlineHz(LocalDataH & data, const SimpleRunline & rl) {}

inline void StaticDielectric::
beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}

inline float StaticDielectric::
updateHx(LocalDataH & data, float Hi, float dEj, float dEk, float Ki)
{
    //LOG << "ch1 = " << data.ch1 << "\n";
//    if (Ki != 0)
//        LOG << "dEj " << dEj << " dEk " << dEk << " Ki " << Ki << "\n"; 
    return Hi + data.ch1*(-dEk + dEj - Ki);
}

inline float StaticDielectric::
updateHy(LocalDataH & data, float Hi, float dEj, float dEk, float Ki)
{
    //LOG << "ch1 = " << data.ch1 << "\n";
//    if (Ki != 0)
//        LOG << "dEj " << dEj << " dEk " << dEk << " Ki " << Ki << "\n";
    return Hi + data.ch1*(-dEk + dEj - Ki);
}

inline float StaticDielectric::
updateHz(LocalDataH & data, float Hi, float dEj, float dEk, float Ki)
{
    //LOG << "ch1 = " << data.ch1 << "\n";
//    if (Ki != 0)
//        LOG << "dEj " << dEj << " dEk " << dEk << " Ki " << Ki << "\n";
    return Hi + data.ch1*(-dEk + dEj - Ki);
}

inline void StaticDielectric::
afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}





#endif
