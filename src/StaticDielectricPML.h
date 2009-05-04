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

#include "MaterialDelegate.h"

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
    StaticDielectricPML();
        
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
private:
    
};

#endif