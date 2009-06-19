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
#include "MaterialBoss.h"

class SetupStaticDielectric : public SimpleBulkSetupMaterial
{
public:
	SetupStaticDielectric();
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    
};


class StaticDielectric : public Material
{
public:
    StaticDielectric(const SetupStaticDielectric & deleg,
        const MaterialDescription & descrip,
        Vector3f dxyz, float dt);
        
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
private:
    Vector3f mDxyz;
    float mDt;
    
    float m_epsr;
    float m_mur;
    std::vector<SimpleRunline> mRunlines[6];
};

#endif
