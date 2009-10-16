/*
 *  ModularUpdateEquation.h
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

// ModularUpdateEquation is templatized by material, runline, PML and current source.
// Really only the update equation requires new code for each combination.
// So, divide some of the functionality among (shared) base classes.

// This class implements
// writeJ()
// writeP()
// writeK()
// writeM()
// modelName()
template<class MaterialT>
class ModularUpdateEquation_Material;

// This class implements
// numRunlinesE()
// numRunlinesH()
// numHalfCellsE()
// numHalfCellsH()
template<class RunlineT>
class ModularUpdateEquation_Runline;

// this may be templatized by runline type as well.
// TEMPLATE REQUIREMENTS:
//  NonPMLMaterial must inherit or look like SimpleMaterial
template<class MaterialT, class RunlineT, class PMLT, class CurrentT>
class ModularUpdateEquation :
    public ModularUpdateEquation_Material<MaterialT>,
    public ModularUpdateEquation_Runline<RunlineT>
{
public:
    ModularUpdateEquation(Paint* parentPaint, std::vector<long> numCellsE,
        std::vector<long> numCellsH, Vector3f dxyz, float dt,
        int runlineDirection );
    
    ModularUpdateEquation(Paint* parentPaint, std::vector<long> numCellsE,
        std::vector<long> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt, int runlineDirection );
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
    virtual void setCurrentSource(CurrentSource* source);
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

#include "ModularUpdateEquation-inl.h"



#endif
