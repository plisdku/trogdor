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
class DrudeModel1 : public SimpleMaterial<SimpleAuxRunline>
{
public:
    DrudeModel1(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt);
    
    virtual std::string getModelName() const;
    
    virtual void allocateAuxBuffers();
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
    
private:
    Vector3f mDxyz;
    float mDt;
    
    float m_epsrinf;
    float m_mur;
    float m_omegap;
    float m_tauc;
    
    std::vector<float> mCurrents[3];
    MemoryBufferPtr mCurrentBuffers[3];
};



#endif
