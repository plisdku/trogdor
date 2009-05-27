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
#include "MaterialBoss.h"

class StaticLossyDielectricDelegate : public SimpleBulkMaterialDelegate
{
public:
	StaticLossyDielectricDelegate();
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    
};


class StaticLossyDielectric : public Material
{
public:
    StaticLossyDielectric(const StaticLossyDielectricDelegate & deleg,
        const MaterialDescription & descrip,
        Vector3f dxyz, float dt);
        
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
private:
    float m_epsr;
    float m_mur;
    float m_sigma;
    std::vector<SimpleRunline> mRunlines[6];
};

#endif
