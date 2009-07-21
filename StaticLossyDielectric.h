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
/*
class StaticLossyDielectric : public SimpleMaterial<SimpleRunline>
{
public:
    StaticLossyDielectric(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt);
    
    virtual std::string getModelName() const;
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
private:
    float m_epsr;
    float m_mur;
    float m_sigma;
};
*/

#endif
