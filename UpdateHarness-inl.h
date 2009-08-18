/*
 *  UpdateHarness.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/13/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "UpdateHarness.h"

template<class RunlineClass>
class UpdateHarness_Runline : virtual public UpdateEquation
{
public:
    UpdateHarness_Runline();
    //  Because templated virtual functions are not allowed in C++, I have to
    // overload the setRunlines functions manually.  What a pain!
    
    virtual void setRunlinesE(int direction,
        const std::vector<SBMRunlinePtr> & rls);
    virtual void setRunlinesE(int direction,
        const std::vector<SBPMRunlinePtr> & rls);
    
    virtual void setRunlinesH(int direction,
        const std::vector<SBMRunlinePtr> & rls);
    virtual void setRunlinesH(int direction,
        const std::vector<SBPMRunlinePtr> & rls);
    
    std::vector<RunlineClass> & getRunlinesE(int direction)
        { return mRunlinesE[direction]; }
    std::vector<RunlineClass> & getRunlinesH(int direction)
        { return mRunlinesH[direction]; }
    
    virtual long getNumRunlinesE() const;
    virtual long getNumRunlinesH() const;
    virtual long getNumHalfCellsE() const;
    virtual long getNumHalfCellsH() const;
protected:
    std::vector<RunlineClass> mRunlinesE[3];
    std::vector<RunlineClass> mRunlinesH[3];
};

template<class MaterialClass>
class UpdateHarness_Material : virtual public UpdateEquation
{
public:
    UpdateHarness_Material(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, Vector3f dxyz, float dt);
        
    virtual void writeJ(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeP(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeK(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeM(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
        
    virtual std::string modelName() const;
protected:
    MaterialClass mMaterial;
};

#pragma mark *** UpdateHarness_Runline ***

template<class RunlineClass>
UpdateHarness_Runline<RunlineClass>::
UpdateHarness_Runline()
{
}

template<class RunlineClass>
void UpdateHarness_Runline<RunlineClass>::
setRunlinesE(int direction, const std::vector<SBMRunlinePtr> & rls)
{
    mRunlinesE[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesE[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void UpdateHarness_Runline<RunlineClass>::
setRunlinesH(int direction, const std::vector<SBMRunlinePtr> & rls)
{
    mRunlinesH[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesH[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void UpdateHarness_Runline<RunlineClass>::
setRunlinesE(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    mRunlinesE[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesE[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
void UpdateHarness_Runline<RunlineClass>::
setRunlinesH(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    mRunlinesH[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
        mRunlinesH[direction][nn] = RunlineClass(*rls[nn]);
}

template<class RunlineClass>
long UpdateHarness_Runline<RunlineClass>::
getNumRunlinesE() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
        total += mRunlinesE[direction].size();
    return total;
}

template<class RunlineClass>
long UpdateHarness_Runline<RunlineClass>::
getNumRunlinesH() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
        total += mRunlinesH[direction].size();
    return total;
}

template<class RunlineClass>
long UpdateHarness_Runline<RunlineClass>::
getNumHalfCellsE() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < mRunlinesE[direction].size(); nn++)
        total += mRunlinesE[direction][nn].length;
    return total;
}

template<class RunlineClass>
long UpdateHarness_Runline<RunlineClass>::
getNumHalfCellsH() const
{
    long total = 0;
    for (int direction = 0; direction < 3; direction++)
    for (int nn = 0; nn < mRunlinesH[direction].size(); nn++)
        total += mRunlinesH[direction][nn].length;
    return total;
}

#pragma mark *** UpdateHarness_Material ***

template<class MaterialT>
UpdateHarness_Material<MaterialT>::
UpdateHarness_Material(Paint* parentPaint, std::vector<int> numCellsE,
    std::vector<int> numCellsH, Vector3f dxyz, float dt) :
    UpdateEquation(),
    mMaterial(*parentPaint->getBulkMaterial(), numCellsE, numCellsH, dxyz, dt)
{
}
        
template<class MaterialT>
void UpdateHarness_Material<MaterialT>::
writeJ(int direction, std::ostream & binaryStream,
    long startingIndex, const float* startingField, long length) const
{
    mMaterial.writeJ(direction, binaryStream, startingIndex, startingField,
        length);
}

template<class MaterialT>
void UpdateHarness_Material<MaterialT>::
writeP(int direction, std::ostream & binaryStream,
    long startingIndex, const float* startingField, long length) const
{
    mMaterial.writeP(direction, binaryStream, startingIndex, startingField,
        length);
}

template<class MaterialT>
void UpdateHarness_Material<MaterialT>::
writeK(int direction, std::ostream & binaryStream,
    long startingIndex, const float* startingField, long length) const
{
    mMaterial.writeK(direction, binaryStream, startingIndex, startingField,
        length);
}

template<class MaterialT>
void UpdateHarness_Material<MaterialT>::
writeM(int direction, std::ostream & binaryStream,
    long startingIndex, const float* startingField, long length) const
{
    mMaterial.writeM(direction, binaryStream, startingIndex, startingField,
        length);
}

template<class MaterialT>
std::string UpdateHarness_Material<MaterialT>::
modelName() const
{
    return mMaterial.modelName();
}


#pragma mark *** UpdateHarness ***

template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, Vector3f dxyz, float dt,
        int runlineDirection ) :
    UpdateHarness_Material<MaterialT>(parentPaint, numCellsE, numCellsH,
        dxyz, dt),
    UpdateHarness_Runline<RunlineT>(),
    mDxyz(dxyz),
    mDt(dt),
    mRunlineDirection(runlineDirection),
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
    UpdateHarness_Material<MaterialT>(parentPaint, numCellsE, numCellsH,
        dxyz, dt),
    UpdateHarness_Runline<RunlineT>(),
    mDxyz(dxyz),
    mDt(dt),
    mRunlineDirection(runlineDirection),
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
allocateAuxBuffers()
{
    UpdateHarness_Material<MaterialT>::mMaterial.allocateAuxBuffers();
    mPML.allocateAuxBuffers();
}


template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
void UpdateHarness<MaterialT, RunlineT, PMLT, CurrentT>::
setCurrentSource(CurrentSource* source)
{
    mCurrent.setCurrentSource(source);
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
    
    UpdateHarness_Material<MaterialT>::mMaterial.initLocalE(materialData);
    //mPML.initLocalE(pmlData);
    mCurrent.initLocalE(currentData, dir0);
    
    std::vector<RunlineT> & runlines =
        UpdateHarness_Runline<RunlineT>::getRunlinesE(fieldDirection);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
//        LOG << rl << "\n";
        UpdateHarness_Material<MaterialT>::mMaterial.onStartRunlineE(
            materialData, rl, dir0);
        mPML.onStartRunlineE(pmlData, rl, dir0, dir1, dir2);
        mCurrent.onStartRunlineE(currentData, rl);
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dHj = (*gjHigh - *gjLow)*dk_inv;
            float dHk = (*gkHigh - *gkLow)*dj_inv;
            
            UpdateHarness_Material<MaterialT>::mMaterial.beforeUpdateE(
                materialData, *fi, dHj, dHk);
            //mPML.beforeUpdateE(pmlData, *fi, dHj, dHk, dir0, dir1, dir2);
            mCurrent.beforeUpdateE(currentData, *fi, dHj, dHk);
            
            *fi = UpdateHarness_Material<MaterialT>::mMaterial.updateE(
                materialData, fieldDirection, *fi, dHj, dHk,
                mPML.updateJ(pmlData, *fi, dHj, dHk) +
                mCurrent.updateJ(currentData, *fi, dHj, dHk, dir0, dir1, dir2));
            
            UpdateHarness_Material<MaterialT>::mMaterial.afterUpdateE(
                materialData, *fi, dHj, dHk);
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
    
    UpdateHarness_Material<MaterialT>::mMaterial.initLocalH(materialData);
    //mPML.initLocalH(pmlData);
    mCurrent.initLocalH(currentData, dir0);
    
    std::vector<RunlineT> & runlines =
        UpdateHarness_Runline<RunlineT>::getRunlinesH(fieldDirection);
    for (int nRL = 0; nRL < runlines.size(); nRL++)
    {
        RunlineT & rl(runlines[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
//        LOG << rl << "\n";
        UpdateHarness_Material<MaterialT>::mMaterial.onStartRunlineH(
            materialData, rl, dir0);
        mPML.onStartRunlineH(pmlData, rl, dir0, dir1, dir2);
        mCurrent.onStartRunlineH(currentData, rl);
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float dEj = (*gjHigh - *gjLow)*dk_inv;
            float dEk = (*gkHigh - *gkLow)*dj_inv;
            
            UpdateHarness_Material<MaterialT>::mMaterial.beforeUpdateH(
                materialData, *fi, dEj, dEk);
            //mPML.beforeUpdateH(pmlData, *fi, dEj, dEk, dir0, dir1, dir2);
            mCurrent.beforeUpdateH(currentData, *fi, dEj, dEk);
            
            *fi = UpdateHarness_Material<MaterialT>::mMaterial.updateH(
                materialData, fieldDirection, *fi, dEj, dEk,
                mPML.updateK(pmlData, *fi, dEj, dEk) +
                mCurrent.updateK(currentData, *fi, dEj, dEk, dir0, dir1, dir2));
            
            UpdateHarness_Material<MaterialT>::mMaterial.afterUpdateH(
                materialData, *fi, dEj, dEk);
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





