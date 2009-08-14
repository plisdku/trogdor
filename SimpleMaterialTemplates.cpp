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

template<class MaterialClass, class RunlineT>
SimpleSetupMaterial<MaterialClass, RunlineT>::
SimpleSetupMaterial(Paint* parentPaint, std::vector<int> numCellsE,
    std::vector<int> numCellsH, Vector3f dxyz, float dt) :
    mParentPaint(parentPaint),
    mNumCellsE(numCellsE),
    mNumCellsH(numCellsH),
    mDxyz(dxyz),
    mDt(dt)
{
    assert(mParentPaint != 0L);
}

template<class MaterialClass, class RunlineT>
UpdateEquationPtr SimpleSetupMaterial<MaterialClass, RunlineT>::
makeUpdateEquation(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{    
    UpdateHarness<MaterialClass, RunlineT, NullPML, NullCurrent>* h =
        new UpdateHarness<MaterialClass, RunlineT, NullPML, NullCurrent>(
        mParentPaint,
        mNumCellsE,
        mNumCellsH,
        mDxyz,
        mDt,
        cp.getLattice().runlineDirection());
    
    for (int nn = 0; nn < 3; nn++)
    {
        h->setRunlinesE(nn, getRunlinesE(nn));
        h->setRunlinesH(nn, getRunlinesH(nn));
    }
    
    return UpdateEquationPtr(h);
}

#pragma mark *** SimpleSetupPML ***

template<class MaterialClass, class RunlineT, class PMLT>
SimpleSetupPML<MaterialClass, RunlineT, PMLT>::
SimpleSetupPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt) :
    mParentPaint(parentPaint),
    mNumCellsE(numCellsE),
    mNumCellsH(numCellsH),
    mPMLHalfCells(pmlHalfCells),
    mPMLParams(pmlParams),
    mDxyz(dxyz),
    mDt(dt)
{
}


template<class MaterialClass, class RunlineT, class PMLT>
UpdateEquationPtr SimpleSetupPML<MaterialClass, RunlineT, PMLT>::
makeUpdateEquation(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    UpdateHarness<MaterialClass, RunlineT, PMLT, NullCurrent>* h =
        new UpdateHarness<MaterialClass, RunlineT, PMLT, NullCurrent>(
        mParentPaint,
        mNumCellsE,
        mNumCellsH,
        mPMLHalfCells,
        mPMLParams,
        mDxyz,
        mDt,
        cp.getLattice().runlineDirection());
    
    for (int nn = 0; nn < 3; nn++)
    {
        h->setRunlinesE(nn, getRunlinesE(nn));
        h->setRunlinesH(nn, getRunlinesH(nn));
    }
    
    return UpdateEquationPtr(h);
    
    //return UpdateEquationPtr(0L);
}










#endif