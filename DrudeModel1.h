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

class DrudeModel1Delegate : public SimpleBulkMaterialDelegate
{
public:
	DrudeModel1Delegate(const MaterialDescPtr & material);
	
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
    DrudeModel1(const DrudeModel1Delegate & deleg,
        const MaterialDescription & descrip,
        Vector3f dxyz, float dt);
    
    virtual void allocateAuxBuffers();
    virtual void calcEPhase(int phasePart = 0);
    virtual void calcHPhase(int phasePart = 0);
    
private:
    float m_mur;
    float m_omegap;
    float m_tauc;
    float m_epsrinf;
    
    std::vector<float> mCurrents[3];
    MemoryBufferPtr mCurrentBuffers[3];
    std::vector<SimpleAuxRunline> mRunlines[6];
};



#endif
