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

template<class MaterialClass, class PMLFactory>
SimpleSetupPML<MaterialClass, PMLFactory>::
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


template<class MaterialClass, class PMLFactory>
MaterialPtr SimpleSetupPML<MaterialClass, PMLFactory>::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    SimplePML<MaterialClass, PMLFactory>* m(
        new SimplePML<MaterialClass, PMLFactory>(
            mParentPaint,
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
    return MaterialPtr(m);
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

template<class RunlineClass>
long SimpleMaterial<RunlineClass>::
getNumRunlinesE() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
        total += mRunlinesE[direction].size();
    return total;
}

template<class RunlineClass>
long SimpleMaterial<RunlineClass>::
getNumRunlinesH() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
        total += mRunlinesH[direction].size();
    return total;
}

template<class RunlineClass>
long SimpleMaterial<RunlineClass>::
getNumHalfCellsE() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < mRunlinesE[direction].size(); nn++)
        total += mRunlinesE[direction][nn].length;
    return total;
}

template<class RunlineClass>
long SimpleMaterial<RunlineClass>::
getNumHalfCellsH() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < mRunlinesH[direction].size(); nn++)
        total += mRunlinesH[direction][nn].length;
    return total;
}

#pragma mark *** SimplePML ***

template<class NonPMLMaterial, class PMLFactory>
SimplePML<NonPMLMaterial, PMLFactory>::
SimplePML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt) :
    NonPMLMaterial(*parentPaint->getBulkMaterial(), numCellsE, numCellsH, dxyz,
        dt),
    mPML(PMLFactory::newPML(parentPaint, numCellsE, numCellsH, pmlHalfCells,
        pmlParams, dxyz, dt))
{
}

template<class NonPMLMaterial, class PMLFactory>
void SimplePML<NonPMLMaterial, PMLFactory>::
calcEPhase(int direction)
{
    NonPMLMaterial::calcEPhase(direction);
    if (direction == 0)
        mPML->calcEx( );
    else if (direction == 1)
        mPML->calcEy();
    else
        mPML->calcEz();
}

template<class NonPMLMaterial, class PMLFactory>
void SimplePML<NonPMLMaterial, PMLFactory>::
calcHPhase(int direction)
{
    NonPMLMaterial::calcHPhase(direction);
    if (direction == 0)
        mPML->calcHx();
    else if (direction == 1)
        mPML->calcHy();
    else
        mPML->calcHz();
}

template<class NonPMLMaterial, class PMLFactory>
void SimplePML<NonPMLMaterial, PMLFactory>::
allocateAuxBuffers()
{
    NonPMLMaterial::allocateAuxBuffers();
    mPML->allocateAuxBuffers();
}

template<class NonPMLMaterial, class PMLFactory>
void SimplePML<NonPMLMaterial, PMLFactory>::
setRunlinesE(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    NonPMLMaterial::setRunlinesE(direction, rls);
    mPML->setRunlinesE(direction, rls);
}

template<class NonPMLMaterial, class PMLFactory>
void SimplePML<NonPMLMaterial, PMLFactory>::
setRunlinesH(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    NonPMLMaterial::setRunlinesH(direction, rls);
    mPML->setRunlinesH(direction, rls);
}



















#endif