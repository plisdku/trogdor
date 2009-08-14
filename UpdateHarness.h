/*
 *  UpdateHarness.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/13/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _UPDATEHARNESS_
#define _UPDATEHARNESS_

#include "UpdateEquation.h"
#include "geometry.h"
#include "Runline.h"
#include "Paint.h"
#include <vector>

// TEMPLATE REQUIREMENTS:
//  Material must have constructor with appropriate arguments (TBD?)
//  RunlineClass needs to have constructor for given runline type

// UpdateHarness is templatized by material, runline, PML and current source.
// Really only the update equation requires new code for each combination.
// So, divide some of the functionality among (shared) base classes.

// This class implements
// writeJ()
// writeP()
// writeK()
// writeM()
// getModelName()
template<class MaterialT>
class UpdateHarness_Material;

// This class implements
// getNumRunlinesE()
// getNumRunlinesH()
// getNumHalfCellsE()
// getNumHalfCellsH()
template<class RunlineT>
class UpdateHarness_Runline;

// this may be templatized by runline type as well.
// TEMPLATE REQUIREMENTS:
//  NonPMLMaterial must inherit or look like SimpleMaterial
template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
class UpdateHarness :
    public UpdateHarness_Material<MaterialT>,
    public UpdateHarness_Runline<RunlineT>
{
public:
    UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, Vector3f dxyz, float dt,
        int runlineDirection );
    
    UpdateHarness(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
    virtual void allocateAuxBuffers();
    
private:
    template<int FIELD_DIRECTION_PML>
    void calcE(int fieldDirection);
    
    template<int FIELD_DIRECTION_PML>
    void calcH(int fieldDirection);
    
    Vector3f mDxyz;
    Vector3f mDxyz_inverse;
    float mDt;
    
    int mRunlineDirection;
    
    PMLT mPML;
    CurrentT mCurrent;
};

#include "UpdateHarness-inl.h"



#endif
