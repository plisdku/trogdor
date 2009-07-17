/*
 *  FreshMaterials.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/17/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef AAA
#define AAA

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


template<class MaterialClass, class PMLImplementationClass>
class SimpleSetupPML : public SimpleBulkPMLSetupMaterial
{
public:
    SimpleSetupPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<string,string> > pmlParams, Vector3f dxyz,
        float dt);
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
};

// TEMPLATE REQUIREMENTS:
//  Material must have constructor with appropriate arguments (TBD?)
//  RunlineClass needs to have constructor for given runline type
template<class RunlineClass>
class SimpleMaterial : public Material
{
public:
    template<class SetupRunlineClass>
    virtual void setRunlinesE(int direction,
        const std::vector<SBMRunline> & rls);
    template<class SetupRunlineClass>
    virtual void setRunlinesH(int direction,
        const std::vector<SBMRunline> & rls);
    
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
    SimplePML();
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
    virtual void allocateAuxBuffers();
private:
    PMLImplementationClass mPML;
};














































#endif