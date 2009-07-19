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


class StaticDielectric : public SimpleMaterial<SimpleRunline>
{
public:
    StaticDielectric(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt);
    
    virtual std::string getModelName() const;
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
private:
    Vector3f mDxyz;
    float mDt;
    
    float m_epsr;
    float m_mur;
};


#endif
