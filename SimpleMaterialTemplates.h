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

#include "MaterialBoss.h"
#include "geometry.h"

class VoxelizedPartition;
class CalculationPartition;
class Paint;

template<class MaterialClass>
class SimpleSetupMaterial : public SimpleBulkSetupMaterial
{
public:
    SimpleSetupMaterial(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, Vector3f dxyz, float dt);
    
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
template<class MaterialClass, class PMLImplementationClass>
class SimpleSetupPML : public SimpleBulkPMLSetupMaterial
{
public:
    SimpleSetupPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
    
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

// TEMPLATE REQUIREMENTS:
//  Material must have constructor with appropriate arguments (TBD?)
//  RunlineClass needs to have constructor for given runline type
template<class RunlineClass>
class SimpleMaterial : public Material
{
public:
    SimpleMaterial();
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
private:
    std::vector<RunlineClass> mRunlinesE[3];
    std::vector<RunlineClass> mRunlinesH[3];
};


// this may be templatized by runline type as well.
// TEMPLATE REQUIREMENTS:
//  NonPMLMaterial must inherit or look like SimpleMaterial
template<class NonPMLMaterial, class PMLImplementationClass>
class SimplePML : public NonPMLMaterial
{
public:
    SimplePML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
    virtual void allocateAuxBuffers();
private:
    Pointer<PMLImplementationClass> mPML;
};




#include "SimpleMaterialTemplates.cpp"




#endif