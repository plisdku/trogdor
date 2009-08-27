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

#include "UpdateHarness.h"

#include "NullPML.h"
#include "NullCurrent.h"

#include <cmath>


#pragma mark *** SimpleSetupMaterial ***

template<class MaterialClass, class RunlineT, class CurrentT>
SimpleSetupMaterial<MaterialClass, RunlineT, CurrentT>::
SimpleSetupMaterial(Paint* parentPaint, std::vector<int> numCellsE,
    std::vector<int> numCellsH, Vector3f dxyz, float dt) :
    BulkSetupMaterial(MaterialDescPtr(parentPaint->bulkMaterial())),
    mParentPaint(parentPaint),
    mNumCellsE(numCellsE),
    mNumCellsH(numCellsH),
    mDxyz(dxyz),
    mDt(dt)
{
    assert(mParentPaint != 0L);
}

template<class MaterialClass, class RunlineT, class CurrentT>
UpdateEquationPtr SimpleSetupMaterial<MaterialClass, RunlineT, CurrentT>::
makeUpdateEquation(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{    
    UpdateHarness<MaterialClass, RunlineT, NullPML, CurrentT>* h =
        new UpdateHarness<MaterialClass, RunlineT, NullPML, CurrentT>(
        mParentPaint,
        mNumCellsE,
        mNumCellsH,
        mDxyz,
        mDt,
        cp.lattice().runlineDirection());
    
    for (int nn = 0; nn < 3; nn++)
    {
        h->setRunlinesE(nn, runlinesE(nn));
        h->setRunlinesH(nn, runlinesH(nn));
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
    BulkPMLSetupMaterial(MaterialDescPtr(parentPaint->bulkMaterial())),
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
    UpdateHarness<MaterialClass, RunlineT, PMLT, CurrentT>* h =
        new UpdateHarness<MaterialClass, RunlineT, PMLT, CurrentT>(
        mParentPaint,
        mNumCellsE,
        mNumCellsH,
        mPMLHalfCells,
        mPMLParams,
        mDxyz,
        mDt,
        cp.lattice().runlineDirection());
    
    for (int nn = 0; nn < 3; nn++)
    {
        h->setRunlinesE(nn, runlinesE(nn));
        h->setRunlinesH(nn, runlinesH(nn));
    }
    
    return UpdateEquationPtr(h);
}










#endif