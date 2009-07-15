/*
 *  DrudeModel1.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _DRUDEMODEL1_
#define _DRUDEMODEL1_

#include "MaterialBoss.h"
#include "SimulationDescriptionPredeclarations.h"
#include "Map.h"
#include "MemoryUtilities.h"
#include <string>

/**
    Setup Drude model.
    A running equation uses the slash f dollar sign thing, like \f$\sqrt{x}\f$.
    However, if I want to use a display equation I can try it like this,
    
    \f[
        |A| = \left( 5 \right)
    \f]
*/
class SetupDrudeModel1 : public SimpleBulkSetupMaterial
{
public:
	SetupDrudeModel1(const MaterialDescPtr & material);
	
    virtual void setNumCellsE(int fieldDir, int numCells);
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
    
    friend class DrudeModel1;
private:
    MaterialDescPtr mDesc;
	MemoryBufferPtr mCurrents[3];
};


class DrudeModel1 : public Material
{
public:
    DrudeModel1(const SetupDrudeModel1 & deleg,
        const MaterialDescription & descrip,
        Vector3f dxyz, float dt);
    
    virtual void allocateAuxBuffers();
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
    
private:
    Vector3f mDxyz;
    float mDt;
    
    float m_epsrinf;
    float m_mur;
    float m_omegap;
    float m_tauc;
    
    std::vector<float> mCurrents[3];
    MemoryBufferPtr mCurrentBuffers[3];
    std::vector<SimpleAuxRunline> mRunlinesE[3];
    std::vector<SimpleAuxRunline> mRunlinesH[3];
};



#endif
