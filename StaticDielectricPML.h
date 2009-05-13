/*
 *  StaticDielectricPML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */


#ifndef _STATICDIELECTRICPML_
#define _STATICDIELECTRICPML_

#include "SimulationDescription.h"
#include "MaterialBoss.h"

class StaticDielectricPMLDelegate : public SimpleBulkPMLMaterialDelegate
{
public:
    StaticDielectricPMLDelegate();
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
};


class StaticDielectricPML : public Material
{
public:
    StaticDielectricPML(const StaticDielectricPMLDelegate & deleg,
        const MaterialDescription & descrip, Vector3f dxyz, float dt);
        
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
private:
    std::vector<SimplePMLRunline> mRunlines[6];
    
    float m_epsr;
    float m_mur;
};

#include "StaticDielectricPML.cpp"

#endif