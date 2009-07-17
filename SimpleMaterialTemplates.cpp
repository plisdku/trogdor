/*
 *  FreshMaterials.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/17/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */


#ifdef AAA

#include "FreshMaterials.h"
#include "VoxelizedPartition.h"
#include "Paint.h"
#include "CalculationPartition.h"
#include "Pointer.h"

#pragma mark *** SimpleSetupMaterial ***

template<class MaterialClass>
SimpleSetupMaterial<MaterialClass>::
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

template<class MaterialClass>
MaterialPtr SimpleSetupMaterial<MaterialClass>::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    MaterialPtr m(new MaterialClass(
        mParentPaint->getBulkMaterial()),
        mNumCellsE,
        mNumCellsH,
        mDxyz,
        mDt);
    return m;
}

#pragma mark *** SimpleSetupPML ***

template<class MaterialClass, class PMLImplementationClass>
MaterialPtr SimpleSetupPML<MaterialClass, PMLImplementationClass>::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    MaterialPtr m;
    //MaterialPtr m(new SimplePML<MaterialClass, PMLImplementationClass>);
    return m;
}


#pragma mark *** SimpleMaterial ***


template<class RunlineClass>
template<class SetupRunlineClass>
void SimpleMaterial::
setRunlinesE(int direction, const std::vector<SetupRunlineClass> & rls)
{
    mRunlinesE[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesE[direction][nn] = RunlineClass(rls[nn]);
}

template<class RunlineClass>
template<class SetupRunlineClass>
void SimpleMaterial::
setRunlinesH(int direction, const std::vector<SetupRunlineClass> & rls)
{
    mRunlinesH.resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesH[direction][nn] = RunlineClass(rls[nn]);
}



#pragma mark *** SimplePML ***

template<class NonPMLMaterial, class PMLImplementationClass>
SimplePML<NonPMLMaterial, PMLImplementationClass>::
SimplePML() :
    NonPMLMaterial(),
    mPML()
{
}

template<class NonPMLMaterial, class PMLImplementationClass>
void SimplePML<NonPMLMaterial, PMLImplementationClass>::
calcEPhase(int direction)
{
    NonPMLMaterial::calcEPhase(direction);
    mPML.calcEPhase(direction, getRunlinesE(direction));
}

template<class NonPMLMaterial, class PMLImplementationClass>
void SimplePML<NonPMLMaterial, PMLImplementationClass>::
calcHPhase(int direction)
{
    NonPMLMaterial::calcHPhase(direction);
    mPML.calcEPhase(direction, getRunlinesH(direction));
}

template<class NonPMLMaterial, class PMLImplementationClass>
void SimplePML<NonPMLMaterial, PMLImplementationClass>::
allocateAuxBuffers()
{
    NonPMLMaterial::allocateAuxBuffers();
    mPML.calcEPhase(direction);
}




















#endif