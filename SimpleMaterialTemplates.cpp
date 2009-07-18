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
    MaterialClass* m(new MaterialClass(
        *mParentPaint->getBulkMaterial(),
        mNumCellsE,
        mNumCellsH,
        mDxyz,
        mDt));
    
    for (int nn = 0; nn < 3; nn++)
    {
        m->setRunlinesE(nn, getRunlinesE(nn));
        m->setRunlinesH(nn, getRunlinesH(nn));
    }
    return MaterialPtr(m);
}

#pragma mark *** SimpleSetupPML ***

template<class MaterialClass, class PMLImplementationClass>
SimpleSetupPML<MaterialClass, PMLImplementationClass>::
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


template<class MaterialClass, class PMLImplementationClass>
MaterialPtr SimpleSetupPML<MaterialClass, PMLImplementationClass>::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    SimplePML<MaterialClass, PMLImplementationClass>* m(
        new SimplePML<MaterialClass, PMLImplementationClass>(
            *mParentPaint->getBulkMaterial(),
            mNumCellsE,
            mNumCellsH,
            mPMLHalfCells,
            mPMLParams,
            mDxyz,
            mDt));
    
    for (int nn = 0; nn < 3; nn++)
    {
        m->setRunlinesE(nn, getRunlinesE(nn));
        m->setRunlinesH(nn, getRunlinesH(nn));
    }
    return m;
}


#pragma mark *** SimpleMaterial ***

template<class RunlineClass>
SimpleMaterial<RunlineClass>::
SimpleMaterial()
{
}

template<class RunlineClass>
void SimpleMaterial<RunlineClass>::
setRunlinesE(int direction, const std::vector<SBMRunlinePtr> & rls)
{
    mRunlinesE[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesE[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void SimpleMaterial<RunlineClass>::
setRunlinesH(int direction, const std::vector<SBMRunlinePtr> & rls)
{
    mRunlinesH[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesH[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void SimpleMaterial<RunlineClass>::
setRunlinesE(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    mRunlinesE[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesE[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void SimpleMaterial<RunlineClass>::
setRunlinesH(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    mRunlinesH[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesH[direction][nn] = RunlineClass(*rls[nn]);
}


#pragma mark *** SimplePML ***

template<class NonPMLMaterial, class PMLImplementationClass>
SimplePML<NonPMLMaterial, PMLImplementationClass>::
SimplePML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt) :
    NonPMLMaterial(*parentPaint->getBulkMaterial(), numCellsE, numCellsH, dxyz,
        dt)
    
{
    //mPML(parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams, dxyz, dt)
}

template<class NonPMLMaterial, class PMLImplementationClass>
void SimplePML<NonPMLMaterial, PMLImplementationClass>::
calcEPhase(int direction)
{
    NonPMLMaterial::calcEPhase(direction);
    //mPML.calcEPhase(direction, getRunlinesE(direction));
}

template<class NonPMLMaterial, class PMLImplementationClass>
void SimplePML<NonPMLMaterial, PMLImplementationClass>::
calcHPhase(int direction)
{
    NonPMLMaterial::calcHPhase(direction);
    //mPML.calcEPhase(direction, getRunlinesH(direction));
}

template<class NonPMLMaterial, class PMLImplementationClass>
void SimplePML<NonPMLMaterial, PMLImplementationClass>::
allocateAuxBuffers()
{
    NonPMLMaterial::allocateAuxBuffers();
    mPML->allocateAuxBuffers;
}




















#endif