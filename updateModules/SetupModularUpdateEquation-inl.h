/*
 *  SimpleMaterialTemplates.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/17/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */


#ifdef _SIMPLEMATERIALTEMPLATES_

#include "VoxelizedPartition.h"
#include "Paint.h"
#include "CalculationPartition.h"
#include "Pointer.h"
#include "PhysicalConstants.h"

#include "ModularUpdateEquation.h"

#include "NullPML.h"
#include "NullCurrent.h"

#include <cmath>


#pragma mark *** SetupModularUpdateEquation ***

template<class MaterialClass, class RunlineT, class CurrentT>
SetupModularUpdateEquation<MaterialClass, RunlineT, CurrentT>::
SetupModularUpdateEquation(Paint* parentPaint, std::vector<int> numCellsE,
    std::vector<int> numCellsH, Vector3f dxyz, float dt) :
    BulkSetupUpdateEquation(MaterialDescPtr(parentPaint->bulkMaterial())),
    mParentPaint(parentPaint),
    mNumCellsE(numCellsE),
    mNumCellsH(numCellsH),
    mDxyz(dxyz),
    mDt(dt)
{
    assert(mParentPaint != 0L);
}

template<class MaterialClass, class RunlineT, class CurrentT>
UpdateEquationPtr SetupModularUpdateEquation<MaterialClass, RunlineT, CurrentT>::
makeUpdateEquation(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{    
    ModularUpdateEquation<MaterialClass, RunlineT, NullPML, CurrentT>* h =
        new ModularUpdateEquation<MaterialClass, RunlineT, NullPML, CurrentT>(
        mParentPaint,
        mNumCellsE,
        mNumCellsH,
        mDxyz,
        mDt,
        cp.lattice().runlineDirection());
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        h->setRunlinesE(xyz, runlinesE(xyz));
        h->setRunlinesH(xyz, runlinesH(xyz));
    }
    
    return UpdateEquationPtr(h);
}

#pragma mark *** SimpleSetupPML ***

template<class MaterialClass, class RunlineT, class CurrentT, class PMLT>
SimpleSetupPML<MaterialClass, RunlineT, CurrentT, PMLT>::
SimpleSetupPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt) :
    BulkPMLSetupUpdateEquation(MaterialDescPtr(parentPaint->bulkMaterial())),
    mParentPaint(parentPaint),
    mNumCellsE(numCellsE),
    mNumCellsH(numCellsH),
    mPMLHalfCells(pmlHalfCells),
    mPMLParams(pmlParams),
    mDxyz(dxyz),
    mDt(dt)
{
}


template<class MaterialClass, class RunlineT, class CurrentT, class PMLT>
UpdateEquationPtr SimpleSetupPML<MaterialClass, RunlineT, CurrentT, PMLT>::
makeUpdateEquation(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    ModularUpdateEquation<MaterialClass, RunlineT, PMLT, CurrentT>* h =
        new ModularUpdateEquation<MaterialClass, RunlineT, PMLT, CurrentT>(
        mParentPaint,
        mNumCellsE,
        mNumCellsH,
        mPMLHalfCells,
        mPMLParams,
        mDxyz,
        mDt,
        cp.lattice().runlineDirection());
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        h->setRunlinesE(xyz, runlinesE(xyz));
        h->setRunlinesH(xyz, runlinesH(xyz));
    }
    
    return UpdateEquationPtr(h);
}










#endif