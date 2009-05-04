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

#include "MaterialBoss.h"

class StaticDielectricDelegate : public SimpleBulkMaterialDelegate
{
public:
	StaticDielectricDelegate();
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    
};


class StaticDielectric : public Material
{
public:
    StaticDielectric();
        
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
private:
    
};

#endif
