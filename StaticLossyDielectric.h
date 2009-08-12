/*
 *  StaticLossyDielectric.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _STATICLOSSYDIELECTRIC_
#define _STATICLOSSYDIELECTRIC_

#include "SimulationDescription.h"
#include "SimpleSetupMaterial.h"
#include "SimpleMaterialTemplates.h"

class StaticLossyDielectric
{
public:
    StaticLossyDielectric(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt);
    
    std::string getModelName() const;
    
    void writeJ(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    void writeK(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    
    void allocateAuxBuffers() {}
    
    
    struct LocalDataE
    {
        float ce1;
        float ce2;
    };
    
    struct LocalDataH
    {
        float ch1;
    };
    
    
    void initLocalE(LocalDataE & data);
    void onStartRunlineE(LocalDataE & data, const SimpleRunline & rl, int dir);
    void beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    float updateE(LocalDataE & data, int dir, float Ei, float dHj, float dHk,
        float Ji);
    void afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk);
    
    void initLocalH(LocalDataH & data);
    void onStartRunlineH(LocalDataH & data, const SimpleRunline & rl, int dir);
    void beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    float updateH(LocalDataH & data, int dir, float Hi, float dEj, float dEk,
        float Ki);
    void afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk);
    
private:
    float m_epsr;
    float m_mur;
    float m_sigma;
    
    float m_ce1;
    float m_ce2;
    float m_ch1;
};


inline void StaticLossyDielectric::
initLocalE(LocalDataE & data)
{
    data.ce1 = m_ce1;
    data.ce2 = m_ce2;
}

inline void StaticLossyDielectric::
onStartRunlineE(LocalDataE & data, const SimpleRunline & rl, int dir)
{
}

inline void StaticLossyDielectric::
beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}

inline float StaticLossyDielectric::
updateE(LocalDataE & data, int dir, float Ei, float dHj, float dHk, float Ji)
{
    //LOG << "ce1 = " << data.ce1 << "\n";
//    if (Ji != 0)
//        LOG << "dHj " << dHj << " dHk " << dHk << " Ji " << Ji << "\n"; 
    return data.ce1*Ei + data.ce2*(dHk - dHj - Ji);
}


inline void StaticLossyDielectric::
afterUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}


inline void StaticLossyDielectric::
initLocalH(LocalDataH & data)
{
    data.ch1 = m_ch1;
}

inline void StaticLossyDielectric::
onStartRunlineH(LocalDataH & data, const SimpleRunline & rl, int dir)
{
}

inline void StaticLossyDielectric::
beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}

inline float StaticLossyDielectric::
updateH(LocalDataH & data, int dir, float Hi, float dEj, float dEk, float Ki)
{
    //LOG << "ch1 = " << data.ch1 << "\n";
//    if (Ki != 0)
//        LOG << "dEj " << dEj << " dEk " << dEk << " Ki " << Ki << "\n"; 
    return Hi + data.ch1*(-dEk + dEj - Ki);
}

inline void StaticLossyDielectric::
afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}



#endif
