/*
 *  StaticDielectric.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "StaticDielectric.h"

#include "Log.h"

using namespace std;

StaticDielectricDelegate::
StaticDielectricDelegate() :
	SimpleBulkMaterialDelegate()
{
    
}


MaterialPtr StaticDielectricDelegate::
makeCalcMaterial(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    return MaterialPtr(new StaticDielectric);
}


StaticDielectric::
StaticDielectric() :
    Material()
{
    
}

void StaticDielectric::
calcEPhase(int phasePart)
{
    LOG << "Calculating E.\n";
}

void StaticDielectric::
calcHPhase(int phasePart)
{
    LOG << "Calculating H.\n";
}


