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

#include "UpdateEquation.h"
#include "BulkSetupMaterials.h"
#include "geometry.h"
#include "Runline.h"

class VoxelizedPartition;
class CalculationPartition;
class Paint;

#pragma mark *** Templates for the setup step ***

// Inheritance from BulkSetupMaterial provides the runline rules and
// the storage of setup runlines.
template<class MaterialClass, class RunlineT, class CurrentT>
class SimpleSetupMaterial : public BulkSetupMaterial
{
public:
    SimpleSetupMaterial(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, Vector3f dxyz, float dt);
    
    // This returns a material harness with a null PML, a null current and
    // the specified MaterialClass for the update equation.
    virtual UpdateEquationPtr makeUpdateEquation(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
    
private:
    Paint* mParentPaint;
    std::vector<int> mNumCellsE;
    std::vector<int> mNumCellsH;
    Vector3f mDxyz;
    float mDt;
};

// Inheritance from BulkPMLSetupMaterial provides the runline rules and
// the storage of setup runlines.
template<class MaterialClass, class RunlineT, class CurrentT, class PMLT>
class SimpleSetupPML : public BulkPMLSetupMaterial
{
public:
    SimpleSetupPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
    
    // This returns a material harness with the given PML, no current,
    // and the given MaterialClass for the update equation.
    virtual UpdateEquationPtr makeUpdateEquation(const VoxelizedPartition & vp,
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




#include "SimpleMaterialTemplates.cpp"

#endif