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

#include <cmath>

static const int PREFETCH_CELLS_AHEAD = 3;
static const bool PREFETCH_E = 1;
static const bool PREFETCH_H = 1;

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
MaterialPtr SimpleSetupMaterial<MaterialClass, RunlineT>::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{    
    WithRunline<RunlineT>* h =
        new UpdateHarness<MaterialClass, RunlineT, NullPML, NullCurrent>(
        mParentPaint,
        mNumCellsE,
        mNumCellsH,
        mDxyz,
        mDt);
    
    for (int nn = 0; nn < 3; nn++)
    {
        h->setRunlinesE(nn, getRunlinesE(nn));
        h->setRunlinesH(nn, getRunlinesH(nn));
    }
    
    return MaterialPtr(h);
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
MaterialPtr SimpleSetupPML<MaterialClass, RunlineT, PMLT>::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    WithRunline<RunlineT>* h =
        new UpdateHarness<MaterialClass, RunlineT, PMLT, NullCurrent>(
        mParentPaint,
        mNumCellsE,
        mNumCellsH,
        mPMLHalfCells,
        mPMLParams,
        mDxyz,
        mDt);
    
    for (int nn = 0; nn < 3; nn++)
    {
        h->setRunlinesE(nn, getRunlinesE(nn));
        h->setRunlinesH(nn, getRunlinesH(nn));
    }
    
    return MaterialPtr(h);
    
    //return MaterialPtr(0L);
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
        std::vector<int> numCellsH, Vector3f dxyz, float dt) :
    WithRunline<RunlineT>(),
    mDxyz(dxyz),
    mDt(dt),
    mMaterial(*parentPaint->getBulkMaterial(), numCellsE, numCellsH, dxyz, dt),
    mPML(),
    mCurrent()
{
    mDxyz_inverse = 1.0 / mDxyz;
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt) :
    WithRunline<RunlineT>(),
    mDxyz(dxyz),
    mDt(dt),
    mMaterial(*parentPaint->getBulkMaterial(), numCellsE, numCellsH, dxyz, dt),
    mPML(parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams, dxyz, dt),
    mCurrent()
{
    mDxyz_inverse = 1.0 / mDxyz;
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
    
    float dj_inv = mDxyz_inverse[(DIRECTION+1)%3];  // e.g. dy
    float dk_inv = mDxyz_inverse[(DIRECTION+2)%3];  // e.g. dz
    
    typename MaterialT::LocalDataE materialData;
    typename PMLT::LocalDataEx pmlData;
    typename CurrentT::LocalDataE currentData;
    
    mMaterial.initLocalE(materialData);
    mPML.initLocalEx(pmlData);
    mCurrent.initLocalE(currentData);
    
    std::vector<RunlineT> & runlines =
        WithRunline<RunlineT>::getRunlinesE(DIRECTION);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
//        LOG << rl << "\n";
        mMaterial.onStartRunlineEx(materialData, rl);
        mPML.onStartRunlineEx(pmlData, rl);
        mCurrent.onStartRunlineE(currentData, rl);
        
        /*
        if (nRL < runlines.size()-1)
        {
            RunlineT & rlAhead(runlines[nRL+1]);
            __builtin_prefetch(rlAhead.fi, 1); // 1 indicates write
            __builtin_prefetch(rlAhead.gj[0], 0);
            __builtin_prefetch(rlAhead.gj[1], 0);
            __builtin_prefetch(rlAhead.gk[0], 0);
            __builtin_prefetch(rlAhead.gk[1], 0);
        }
        */
        
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dHj = (*gjHigh - *gjLow)*dk_inv;
            float dHk = (*gkHigh - *gkLow)*dj_inv;
            
            mMaterial.beforeUpdateE(materialData, *fi, dHj, dHk);
            mPML.beforeUpdateEx(pmlData, *fi, dHj, dHk);
            mCurrent.beforeUpdateE(currentData, *fi, dHj, dHk);
            
            *fi = mMaterial.updateEx(materialData, *fi, dHj, dHk,
                mPML.updateJx(pmlData, *fi, dHj, dHk) +
                mCurrent.updateJx(currentData, *fi, dHj, dHk) );
            
            mMaterial.afterUpdateE(materialData, *fi, dHj, dHk);
            mPML.afterUpdateEx(pmlData, *fi, dHj, dHk);
            mCurrent.afterUpdateE(currentData, *fi, dHj, dHk);
            
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
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 1;
    const int STRIDE = 1;
    
    float dj_inv = mDxyz_inverse[(DIRECTION+1)%3];  // e.g. dy
    float dk_inv = mDxyz_inverse[(DIRECTION+2)%3];  // e.g. dz
    
    typename MaterialT::LocalDataE materialData;
    typename PMLT::LocalDataEy pmlData;
    typename CurrentT::LocalDataE currentData;
    
    mMaterial.initLocalE(materialData);
    mPML.initLocalEy(pmlData);
    mCurrent.initLocalE(currentData);
    
    std::vector<RunlineT> & runlines =
        WithRunline<RunlineT>::getRunlinesE(DIRECTION);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
//        LOG << rl << "\n";
        mMaterial.onStartRunlineEy(materialData, rl);
        mPML.onStartRunlineEy(pmlData, rl);
        mCurrent.onStartRunlineE(currentData, rl);
        
        /*
        if (nRL < runlines.size()-1)
        {
            RunlineT & rlAhead(runlines[nRL+1]);
            __builtin_prefetch(rlAhead.fi, 1); // 1 indicates write
            __builtin_prefetch(rlAhead.gj[0], 0);
            __builtin_prefetch(rlAhead.gj[1], 0);
            __builtin_prefetch(rlAhead.gk[0], 0);
            __builtin_prefetch(rlAhead.gk[1], 0);
        }
        */
        
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dHj = (*gjHigh - *gjLow)*dk_inv;
            float dHk = (*gkHigh - *gkLow)*dj_inv;
            
            mMaterial.beforeUpdateE(materialData, *fi, dHj, dHk);
            mPML.beforeUpdateEy(pmlData, *fi, dHj, dHk);
            mCurrent.beforeUpdateE(currentData, *fi, dHj, dHk);
            
            *fi = mMaterial.updateEy(materialData, *fi, dHj, dHk,
                mPML.updateJy(pmlData, *fi, dHj, dHk) +
                mCurrent.updateJy(currentData, *fi, dHj, dHk) );
            
            mMaterial.afterUpdateE(materialData, *fi, dHj, dHk);
            mPML.afterUpdateEy(pmlData, *fi, dHj, dHk);
            mCurrent.afterUpdateE(currentData, *fi, dHj, dHk);
            
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
calcEz()
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 2;
    const int STRIDE = 1;
    
    float dj_inv = mDxyz_inverse[(DIRECTION+1)%3];  // e.g. dy
    float dk_inv = mDxyz_inverse[(DIRECTION+2)%3];  // e.g. dz
    
    typename MaterialT::LocalDataE materialData;
    typename PMLT::LocalDataEz pmlData;
    typename CurrentT::LocalDataE currentData;
    
    mMaterial.initLocalE(materialData);
    mPML.initLocalEz(pmlData);
    mCurrent.initLocalE(currentData);
    
    std::vector<RunlineT> & runlines =
        WithRunline<RunlineT>::getRunlinesE(DIRECTION);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
//        LOG << rl << "\n";
        
        mMaterial.onStartRunlineEz(materialData, rl);
        mPML.onStartRunlineEz(pmlData, rl);
        mCurrent.onStartRunlineE(currentData, rl);
        
        /*
        if (nRL < runlines.size()-1)
        {
            RunlineT & rlAhead(runlines[nRL+1]);
            __builtin_prefetch(rlAhead.fi, 1); // 1 indicates write
            __builtin_prefetch(rlAhead.gj[0], 0);
            __builtin_prefetch(rlAhead.gj[1], 0);
            __builtin_prefetch(rlAhead.gk[0], 0);
            __builtin_prefetch(rlAhead.gk[1], 0);
        }
        */
        
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dHj = (*gjHigh - *gjLow)*dk_inv;
            float dHk = (*gkHigh - *gkLow)*dj_inv;
            
            mMaterial.beforeUpdateE(materialData, *fi, dHj, dHk);
            mPML.beforeUpdateEz(pmlData, *fi, dHj, dHk);
            mCurrent.beforeUpdateE(currentData, *fi, dHj, dHk);
            
            *fi = mMaterial.updateEz(materialData, *fi, dHj, dHk,
                //mPML.updateJz(pmlData, *fi, dHj, dHk) +
                //Jz + 
                mPML.updateJz(pmlData, *fi, dHj, dHk) +
                mCurrent.updateJz(currentData, *fi, dHj, dHk) );
            
            mMaterial.afterUpdateE(materialData, *fi, dHj, dHk);
            mPML.afterUpdateEz(pmlData, *fi, dHj, dHk);
            mCurrent.afterUpdateE(currentData, *fi, dHj, dHk);
            
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
calcHx()
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 0;
    const int STRIDE = 1;
    
    float dj_inv = mDxyz_inverse[(DIRECTION+1)%3];  // e.g. dy
    float dk_inv = mDxyz_inverse[(DIRECTION+2)%3];  // e.g. dz
    
    typename MaterialT::LocalDataH materialData;
    typename PMLT::LocalDataHx pmlData;
    typename CurrentT::LocalDataH currentData;
    
    mMaterial.initLocalH(materialData);
    mPML.initLocalHx(pmlData);
    mCurrent.initLocalH(currentData);
    
    std::vector<RunlineT> & runlines =
        WithRunline<RunlineT>::getRunlinesH(DIRECTION);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        mMaterial.onStartRunlineHx(materialData, rl);
        mPML.onStartRunlineHx(pmlData, rl);
        mCurrent.onStartRunlineH(currentData, rl);
        
        /*
        if (nRL < runlines.size()-1)
        {
            RunlineT & rlAhead(runlines[nRL+1]);
            __builtin_prefetch(rlAhead.fi, 1); // 1 indicates write
            __builtin_prefetch(rlAhead.gj[0], 0);
            __builtin_prefetch(rlAhead.gj[1], 0);
            __builtin_prefetch(rlAhead.gk[0], 0);
            __builtin_prefetch(rlAhead.gk[1], 0);
        }
        */
        
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dEj = (*gjHigh - *gjLow)*dk_inv;
            float dEk = (*gkHigh - *gkLow)*dj_inv;
            
            mMaterial.beforeUpdateH(materialData, *fi, dEj, dEk);
            mPML.beforeUpdateHx(pmlData, *fi, dEj, dEk);
            mCurrent.beforeUpdateH(currentData, *fi, dEj, dEk);
            
            *fi = mMaterial.updateHx(materialData, *fi, dEj, dEk,
                mPML.updateKx(pmlData, *fi, dEj, dEk) +
                mCurrent.updateKx(currentData, *fi, dEj, dEk) );
            
            mMaterial.afterUpdateH(materialData, *fi, dEj, dEk);
            mPML.afterUpdateHx(pmlData, *fi, dEj, dEk);
            mCurrent.afterUpdateH(currentData, *fi, dEj, dEk);
            
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
calcHy()
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 1;
    const int STRIDE = 1;
    
    float dj_inv = mDxyz_inverse[(DIRECTION+1)%3];  // e.g. dy
    float dk_inv = mDxyz_inverse[(DIRECTION+2)%3];  // e.g. dz
    
    typename MaterialT::LocalDataH materialData;
    typename PMLT::LocalDataHy pmlData;
    typename CurrentT::LocalDataH currentData;
    
    mMaterial.initLocalH(materialData);
    mPML.initLocalHy(pmlData);
    mCurrent.initLocalH(currentData);
    
    std::vector<RunlineT> & runlines =
        WithRunline<RunlineT>::getRunlinesH(DIRECTION);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        mMaterial.onStartRunlineHy(materialData, rl);
        mPML.onStartRunlineHy(pmlData, rl);
        mCurrent.onStartRunlineH(currentData, rl);
        
        /*
        if (nRL < runlines.size()-1)
        {
            RunlineT & rlAhead(runlines[nRL+1]);
            __builtin_prefetch(rlAhead.fi, 1); // 1 indicates write
            __builtin_prefetch(rlAhead.gj[0], 0);
            __builtin_prefetch(rlAhead.gj[1], 0);
            __builtin_prefetch(rlAhead.gk[0], 0);
            __builtin_prefetch(rlAhead.gk[1], 0);
        }
        */
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dEj = (*gjHigh - *gjLow)*dk_inv;
            float dEk = (*gkHigh - *gkLow)*dj_inv;
            
            mMaterial.beforeUpdateH(materialData, *fi, dEj, dEk);
            mPML.beforeUpdateHy(pmlData, *fi, dEj, dEk);
            mCurrent.beforeUpdateH(currentData, *fi, dEj, dEk);
                            
            *fi = mMaterial.updateHy(materialData, *fi, dEj, dEk,
                mPML.updateKy(pmlData, *fi, dEj, dEk) +
                mCurrent.updateKy(currentData, *fi, dEj, dEk) );
                        
            mMaterial.afterUpdateH(materialData, *fi, dEj, dEk);
            mPML.afterUpdateHy(pmlData, *fi, dEj, dEk);
            mCurrent.afterUpdateH(currentData, *fi, dEj, dEk);
            
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
calcHz()
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 2;
    const int STRIDE = 1;
    
    float dj_inv = mDxyz_inverse[(DIRECTION+1)%3];  // e.g. dy
    float dk_inv = mDxyz_inverse[(DIRECTION+2)%3];  // e.g. dz
    
    typename MaterialT::LocalDataH materialData;
    typename PMLT::LocalDataHz pmlData;
    typename CurrentT::LocalDataH currentData;
    
    mMaterial.initLocalH(materialData);
    mPML.initLocalHz(pmlData);
    mCurrent.initLocalH(currentData);
    
    std::vector<RunlineT> & runlines =
        WithRunline<RunlineT>::getRunlinesH(DIRECTION);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        mMaterial.onStartRunlineHz(materialData, rl);
        mPML.onStartRunlineHz(pmlData, rl);
        mCurrent.onStartRunlineH(currentData, rl);
        
        /*
        if (nRL < runlines.size()-1)
        {
            RunlineT & rlAhead(runlines[nRL+1]);
            __builtin_prefetch(rlAhead.fi, 1); // 1 indicates write
            __builtin_prefetch(rlAhead.gj[0], 0);
            __builtin_prefetch(rlAhead.gj[1], 0);
            __builtin_prefetch(rlAhead.gk[0], 0);
            __builtin_prefetch(rlAhead.gk[1], 0);
        }
        */
        
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dEj = (*gjHigh - *gjLow)*dk_inv;
            float dEk = (*gkHigh - *gkLow)*dj_inv;
            
            mMaterial.beforeUpdateH(materialData, *fi, dEj, dEk);
            mPML.beforeUpdateHz(pmlData, *fi, dEj, dEk);
            mCurrent.beforeUpdateH(currentData, *fi, dEj, dEk);
                
            *fi = mMaterial.updateHz(materialData, *fi, dEj, dEk,
                mPML.updateKz(pmlData, *fi, dEj, dEk) +
                mCurrent.updateKz(currentData, *fi, dEj, dEk) );
            
            mMaterial.afterUpdateH(materialData, *fi, dEj, dEk);
            mPML.afterUpdateHz(pmlData, *fi, dEj, dEk);
            mCurrent.afterUpdateH(currentData, *fi, dEj, dEk);
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

















#endif