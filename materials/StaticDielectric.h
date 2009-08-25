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

#include "Material.h"
#include "BulkSetupMaterials.h"
#include "SimpleMaterialTemplates.h"
#include <string>
#include "Log.h"

class MaterialDescription;

class StaticDielectric : public Material
{
public:
    StaticDielectric(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt);
    
    std::string modelName() const;
    
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
onStartRunlineE(LocalDataE & data, const SimpleRunline & rl, int dir)
{
}

inline void StaticDielectric::
beforeUpdateE(LocalDataE & data, float Ei, float dHj, float dHk)
{
}

inline float StaticDielectric::
updateE(LocalDataE & data, int dir, float Ei, float dHj, float dHk, float Ji)
{
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
onStartRunlineH(LocalDataH & data, const SimpleRunline & rl, int dir)
{
}

inline void StaticDielectric::
beforeUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}

inline float StaticDielectric::
updateH(LocalDataH & data, int dir, float Hi, float dEj, float dEk, float Ki)
{
    return Hi + data.ch1*(-dEk + dEj - Ki);
}

inline void StaticDielectric::
afterUpdateH(LocalDataH & data, float Hi, float dEj, float dEk)
{
}





#endif
