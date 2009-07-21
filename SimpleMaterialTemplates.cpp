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

#include "NullPML.h"
#include "NullCurrent.h"

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
    /*
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
    */
    
    Material* h = new UpdateHarness<MaterialClass, NullPML, NullCurrent>(
        *mParentPaint->getBulkMaterial(),
        mNumCellsE,
        mNumCellsH,
        mDxyz,
        mDt);
    
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
    /*
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
    */
    
    Material* h = new UpdateHarness<MaterialClass, NullPML, NullCurrent>(
        *mParentPaint->getBulkMaterial(),
        mNumCellsE,
        mNumCellsH,
        mDxyz,
        mDt);
    
    for (int nn = 0; nn < 3; nn++)
    {
        m->setRunlinesE(nn, getRunlinesE(nn));
        m->setRunlinesH(nn, getRunlinesH(nn));
    }
    
    return MaterialPtr(m);
}


#pragma mark *** WithRunline ***

template<class RunlineClass>
WithRunline<RunlineClass>::
WithRunline()
{
}

template<class RunlineClass>
void WithRunline<RunlineClass>::
setRunlinesE(int direction, const std::vector<SBMRunlinePtr> & rls)
{
    mRunlinesE[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesE[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void WithRunline<RunlineClass>::
setRunlinesH(int direction, const std::vector<SBMRunlinePtr> & rls)
{
    mRunlinesH[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesH[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void WithRunline<RunlineClass>::
setRunlinesE(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    mRunlinesE[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesE[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void WithRunline<RunlineClass>::
setRunlinesH(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    mRunlinesH[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesH[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
long WithRunline<RunlineClass>::
getNumRunlinesE() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
        total += mRunlinesE[direction].size();
    return total;
}

template<class RunlineClass>
long WithRunline<RunlineClass>::
getNumRunlinesH() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
        total += mRunlinesH[direction].size();
    return total;
}

template<class RunlineClass>
long WithRunline<RunlineClass>::
getNumHalfCellsE() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < mRunlinesE[direction].size(); nn++)
        total += mRunlinesE[direction][nn].length;
    return total;
}

template<class RunlineClass>
long WithRunline<RunlineClass>::
getNumHalfCellsH() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < mRunlinesH[direction].size(); nn++)
        total += mRunlinesH[direction][nn].length;
    return total;
}

#pragma mark *** SimplePML ***

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt) :
    WithRunline<RunlineT>(),
    mMaterial(*parentPaint->getBulkMaterial(), numCellsE, numCellsH, dxyz, dt),
    mPML(parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams, dxyz, dt),
    mCurrent(),
    mDxyz(dxyz),
    mDt(dt)
{
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcEPhase(int direction)
{
    if (direction == 0)
        calcEx();
    else if (direction == 1)
        calcEy();
    else
        calcEz();
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcHPhase(int direction)
{
    if (direction == 0)
        calcHx();
    else if (direction == 1)
        calcHy();
    else
        calcHz();
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
allocateAuxBuffers()
{
    mMaterial.allocateAuxBuffers();
    mPML.allocateAuxBuffers();
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcEx()
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 0;
    const int STRIDE = 1;
    
    float dj = mDxyz[(DIRECTION+1)%3];  // e.g. dy
    float dk = mDxyz[(DIRECTION+2)%3];  // e.g. dz
    
    std::vector<RunlineT> & runlines =
        SimpleMaterial<RunlineT>::getRunlinesE(DIRECTION);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        typename MaterialT::LocalData materialData;
        typename PMLT::LocalData pmlData;
        //typename CurrentT::LocalData currentData;
        
        mMaterial.onStartRunline(materialData, rl);
        mPML.onStartRunline(pmlData, rl);
        //mCurrent.onStartRunline(pmlData, rl);
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dHj = (*gjHigh - *gjLow)/dk;
            float dHk = (*gkHigh - *gkLow)/dj;
            
            mMaterial.beforeUpdateEx(materialData, *fi, dHj, dHk);
            mPML.beforeUpdateEx(pmlData, *fi, dHj, dHk);
            //mCurrent.beforeUpdateEx(currentData, *fi, dHj, dHk);
            
            *fi += (mDt/Constants::eps0)*mMaterial.dDxdt(materialData, dHj, dHk,
                    *fi, mPML.getCurrent(pmlData, dHj, dHk));
                    //mCurrent.getCurrent(currentData, nn));
            
            mMaterial.afterUpdateEx(materialData, *fi, dHj, dHk);
            mPML.afterUpdateEx(materialData, *fi, dHj, dHk);
            //mCurrent.afterUpdateEx(currentData, *fi, dHj, dHk);
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcEy()
{
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcEz()
{
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcHx()
{
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcHy()
{
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcHz()
{
}

















#endif