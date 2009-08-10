/*
 *  SimpleMaterialTemplates.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/17/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _SIMPLEMATERIALTEMPLATES_
#define _SIMPLEMATERIALTEMPLATES_

#include "SimpleSetupMaterial.h"
#include "geometry.h"
#include "Runline.h"

class VoxelizedPartition;
class CalculationPartition;
class Paint;

#pragma mark *** Templates for the setup step ***

// Inheritance from SimpleBulkSetupMaterial provides the runline rules and
// the storage of setup runlines.
template<class MaterialClass, class RunlineT>
class SimpleSetupMaterial : public SimpleBulkSetupMaterial
{
public:
    SimpleSetupMaterial(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, Vector3f dxyz, float dt);
    
    // This returns a material harness with a null PML, a null current and
    // the specified MaterialClass for the update equation.
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
    
private:
    Paint* mParentPaint;
    std::vector<int> mNumCellsE;
    std::vector<int> mNumCellsH;
    Vector3f mDxyz;
    float mDt;
};

// Inheritance from SimpleBulkPMLSetupMaterial provides the runline rules and
// the storage of setup runlines.
template<class MaterialClass, class RunlineT, class PMLFactory>
class SimpleSetupPML : public SimpleBulkPMLSetupMaterial
{
public:
    SimpleSetupPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
    
    // This returns a material harness with the given PML, no current,
    // and the given MaterialClass for the update equation.
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    Paint* mParentPaint;
    std::vector<int> mNumCellsE;
    std::vector<int> mNumCellsH;
    std::vector<Rect3i> mPMLHalfCells;
    Map<Vector3i, Map<std::string,std::string> > mPMLParams;
    Vector3f mDxyz;
    float mDt;
};

#pragma mark *** Calculation harness ***


// TEMPLATE REQUIREMENTS:
//  Material must have constructor with appropriate arguments (TBD?)
//  RunlineClass needs to have constructor for given runline type
template<class RunlineClass>
class WithRunline : public Material
{
public:
    WithRunline();
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


// this may be templatized by runline type as well.
// TEMPLATE REQUIREMENTS:
//  NonPMLMaterial must inherit or look like SimpleMaterial
template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
class UpdateHarness : public WithRunline<RunlineT>
{
public:
    UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, Vector3f dxyz, float dt);
    
    UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
    virtual void allocateAuxBuffers();
    
    template<int DIRECTION_PML>
    void calcE(int fieldDirection);
    
    template<int MEMORY_DIRECTION>
    void calcH(int fieldDirection);
    
    virtual std::string getModelName() const
        { return mMaterial.getModelName(); }
private:
    Vector3f mDxyz;
    Vector3f mDxyz_inverse;
    float mDt;
    
    MaterialT mMaterial;
    PMLT mPML;
    CurrentT mCurrent;
};




#include "SimpleMaterialTemplates.cpp"

#endif