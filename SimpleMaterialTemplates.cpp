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
UpdateEquationPtr SimpleSetupMaterial<MaterialClass, RunlineT>::
makeUpdateEquation(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{    
    WithRunline<RunlineT>* h =
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
    WithRunline<RunlineT>* h =
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
        std::vector<int> numCellsH, Vector3f dxyz, float dt,
        int runlineDirection ) :
    WithRunline<RunlineT>(),
    mDxyz(dxyz),
    mDt(dt),
    mRunlineDirection(runlineDirection),
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
        float dt, int runlineDirection) :
    WithRunline<RunlineT>(),
    mDxyz(dxyz),
    mDt(dt),
    mRunlineDirection(runlineDirection),
    mMaterial(*parentPaint->getBulkMaterial(), numCellsE, numCellsH, dxyz, dt),
    mPML(parentPaint, numCellsE, numCellsH, pmlHalfCells, pmlParams, dxyz, dt,
        runlineDirection),
    mCurrent()
{
    mDxyz_inverse = 1.0 / mDxyz;
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcEPhase(int direction)
{
    // If the memory direction is 0 (x), then Ex updates use calcE<0>
    // If the memory direction is 1 (y), then Ex updates use calcE<2>
    // If the memory direction is 2 (z), then Ex updates use calcE<1>
    int pmlFieldDirection = (3-mRunlineDirection+direction)%3;
    assert(pmlFieldDirection >= 0);
    assert(pmlFieldDirection < 3);
    
    if (pmlFieldDirection == 0)
        calcE<0>(direction);
    else if (pmlFieldDirection == 1)
        calcE<1>(direction);
    else
        calcE<2>(direction);
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcHPhase(int direction)
{
    // If the memory direction is 0 (x), then Hx updates use calcH<0>
    // If the memory direction is 1 (y), then Hx updates use calcH<2>
    // If the memory direction is 2 (z), then Hx updates use calcH<1>
    int pmlFieldDirection = (3-mRunlineDirection+direction)%3;
    assert(pmlFieldDirection >= 0);
    assert(pmlFieldDirection < 3);
    
    if (pmlFieldDirection == 0)
        calcH<0>(direction);
    else if (pmlFieldDirection == 1)
        calcH<1>(direction);
    else
        calcH<2>(direction);
}


template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
writeJ(int direction, std::ostream & binaryStream,
    long startingIndex, const float* startingField, long length) const
{
    mMaterial.writeJ(direction, binaryStream, startingIndex, startingField,
        length);
}

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
writeK(int direction, std::ostream & binaryStream,
    long startingIndex, const float* startingField, long length) const
{
    mMaterial.writeK(direction, binaryStream, startingIndex, startingField,
        length);
}


template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
allocateAuxBuffers()
{
    mMaterial.allocateAuxBuffers();
    mPML.allocateAuxBuffers();
}

// The PML is already templated appropriately to know the memory direction.
// Consequently all I need to worry about is telling it to use the MEM+0, MEM+1,
// or MEM+3 update equation.

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
template<int FIELD_DIRECTION_PML>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcE(int fieldDirection)
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int STRIDE = 1;
    
    const int dir0 = fieldDirection;
    const int dir1 = (dir0+1)%3;
    const int dir2 = (dir1+1)%3;
    
    float dj_inv = mDxyz_inverse[dir1];  // e.g. dy
    float dk_inv = mDxyz_inverse[dir2];  // e.g. dz
    
    // Thanks to polymorphism, the only place this function needs to mention
    // the FIELD_DIRECTION_PML parameter is here, in the declaration of pmlData.
    // The PML is polymorphic by LocalDataE<int>, so the calls below to
    // onStartRunlineE and updateJ (and anything else with the PML) will do
    // different things as well.  It's just a little hard to see.
    typename MaterialT::LocalDataE materialData;
    typename PMLT::template LocalDataE<FIELD_DIRECTION_PML> pmlData;
    typename CurrentT::LocalDataE currentData;
    
    mMaterial.initLocalE(materialData);
    //mPML.initLocalE(pmlData);
    mCurrent.initLocalE(currentData);
    
    std::vector<RunlineT> & runlines =
        WithRunline<RunlineT>::getRunlinesE(fieldDirection);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
//        LOG << rl << "\n";
        mMaterial.onStartRunlineE(materialData, rl, dir0);
        mPML.onStartRunlineE(pmlData, rl, dir0, dir1, dir2);
        //mCurrent.onStartRunlineE(currentData, rl, dir0, dir1, dir2);
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dHj = (*gjHigh - *gjLow)*dk_inv;
            float dHk = (*gkHigh - *gkLow)*dj_inv;
            
            mMaterial.beforeUpdateE(materialData, *fi, dHj, dHk);
            //mPML.beforeUpdateE(pmlData, *fi, dHj, dHk, dir0, dir1, dir2);
            mCurrent.beforeUpdateE(currentData, *fi, dHj, dHk);
            
            *fi = mMaterial.updateE(materialData, fieldDirection, *fi, dHj, dHk,
                mPML.updateJ(pmlData, *fi, dHj, dHk) +
                mCurrent.updateJ(currentData, *fi, dHj, dHk, dir0, dir1, dir2));
            
            mMaterial.afterUpdateE(materialData, *fi, dHj, dHk);
            //mPML.afterUpdateE(pmlData, *fi, dHj, dHk, dir0, dir1, dir2);
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
template<int FIELD_DIRECTION_PML>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
calcH(int fieldDirection)
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int STRIDE = 1;
    
    const int dir0 = fieldDirection;
    const int dir1 = (dir0+1)%3;
    const int dir2 = (dir1+1)%3;
    
    float dj_inv = mDxyz_inverse[dir1];  // e.g. dy
    float dk_inv = mDxyz_inverse[dir2];  // e.g. dz
    
    typename MaterialT::LocalDataH materialData;
    typename PMLT::template LocalDataH<FIELD_DIRECTION_PML> pmlData;
    typename CurrentT::LocalDataH currentData;
    
    mMaterial.initLocalH(materialData);
    //mPML.initLocalH(pmlData);
    mCurrent.initLocalH(currentData);
    
    std::vector<RunlineT> & runlines =
        WithRunline<RunlineT>::getRunlinesH(fieldDirection);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
//        LOG << rl << "\n";
        mMaterial.onStartRunlineH(materialData, rl, dir0);
        mPML.onStartRunlineH(pmlData, rl, dir0, dir1, dir2);
        //mCurrent.onStartRunlineH(currentData, rl, dir0, dir1, dir2);
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dEj = (*gjHigh - *gjLow)*dk_inv;
            float dEk = (*gkHigh - *gkLow)*dj_inv;
            
            mMaterial.beforeUpdateH(materialData, *fi, dEj, dEk);
            //mPML.beforeUpdateH(pmlData, *fi, dEj, dEk, dir0, dir1, dir2);
            mCurrent.beforeUpdateH(currentData, *fi, dEj, dEk);
            
            *fi = mMaterial.updateH(materialData, fieldDirection, *fi, dEj, dEk,
                mPML.updateK(pmlData, *fi, dEj, dEk) +
                mCurrent.updateK(currentData, *fi, dEj, dEk, dir0, dir1, dir2));
            
            mMaterial.afterUpdateH(materialData, *fi, dEj, dEk);
            //mPML.afterUpdateH(pmlData, *fi, dEj, dEk, dir0, dir1, dir2);
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