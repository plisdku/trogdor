/*
 *  CalculationPartition.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "CalculationPartition.h"
#include "SimulationDescription.h"

#include "Log.h"
#include "VoxelizedPartition.h"
#include "Map.h"

using namespace std;

CalculationPartition::
CalculationPartition(const VoxelizedPartition & vp) :
    mEHBuffers(vp.getEHBuffers())
{
    LOG << "New calc partition.\n";
    
    const Map<Paint*, MaterialDelegatePtr> & delegs = vp.getDelegates();
    map<Paint*, MaterialDelegatePtr>::const_iterator itr;
    for (itr = delegs.begin(); itr != delegs.end(); itr++)
    {
        LOG << "Dealing with paint " << *itr->first << endl;
        mMaterials.push_back(itr->second->makeCalcMaterial(vp, *this));
    }
}

CalculationPartition::
~CalculationPartition()
{
}


