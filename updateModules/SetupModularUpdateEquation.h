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

// Inheritance from BulkSetupUpdateEquation provides the runline rules and
// the storage of setup runlines.
template<class MaterialClass, class RunlineT, class CurrentT>
class SetupModularUpdateEquation : public BulkSetupUpdateEquation
{
public:
    SetupModularUpdateEquation(Paint* parentPaint, std::vector<long> numCellsE,
        std::vector<long> numCellsH, Vector3f dxyz, float dt);
    
    // This returns a material harness with a null PML, a null current and
    // the specified MaterialClass for the update equation.
    virtual UpdateEquationPtr makeUpdateEquation(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
    
private:
    Paint* mParentPaint;
    std::vector<long> mNumCellsE;
    std::vector<long> mNumCellsH;
    Vector3f mDxyz;
    float mDt;
};

// Inheritance from BulkPMLSetupUpdateEquation provides the runline rules and
// the storage of setup runlines.
template<class MaterialClass, class RunlineT, class CurrentT, class PMLT>
class SimpleSetupPML : public BulkPMLSetupUpdateEquation
{
public:
    SimpleSetupPML(Paint* parentPaint, std::vector<long> numCellsE,
        std::vector<long> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
    
    // This returns a material harness with the given PML, no current,
    // and the given MaterialClass for the update equation.
    virtual UpdateEquationPtr makeUpdateEquation(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    Paint* mParentPaint;
    std::vector<long> mNumCellsE;
    std::vector<long> mNumCellsH;
    std::vector<Rect3i> mPMLHalfCells;
    Map<Vector3i, Map<std::string,std::string> > mPMLParams;
    Vector3f mDxyz;
    float mDt;
};

#pragma mark *** Calculation harness ***




#include "SetupModularUpdateEquation-inl.h"

#endif