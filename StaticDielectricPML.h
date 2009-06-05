/*
 *  StaticDielectricPML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */


#ifndef _STATICDIELECTRICPML_
#define _STATICDIELECTRICPML_

#include "SimulationDescription.h"
#include "MaterialBoss.h"

class StaticDielectricPMLDelegate : public SimpleBulkPMLMaterialDelegate
{
public:
    StaticDielectricPMLDelegate();
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    MemoryBufferPtr mBufAccumEj[3], mBufAccumEk[3],
        mBufAccumHj[3], mBufAccumHk[3];
    
    Rect3i mPMLHalfCells;
    Vector3i mPMLDir;
};

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
class StaticDielectricPML : public Material
{
public:
    StaticDielectricPML(const StaticDielectricPMLDelegate & deleg,
        const MaterialDescription & descrip, Rect3i pmlHalfCells,
        Vector3i pmlDir, Vector3f dxyz, float dt);
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
private:
    std::vector<SimplePMLRunline> mRunlines[6];
    
    // The memory buffers define the skeleton of accumulated data but are not
    // responsible for its allocation.
    
    MemoryBufferPtr mBufAccumEj[3], mBufAccumEk[3],
        mBufAccumHj[3], mBufAccumHk[3];
    /*
    MemoryBufferPtr mBufC_JjH[3], mBufC_JkH[3],
        mBufC_PhijH[3], mBufC_PhikH[3],
        mBufC_PhijJ[3], mBufC_PhikJ[3];
    MemoryBufferPtr mBufC_MjE[3], mBufC_MkE[3],
        mBufC_PsijE[3], mBufC_PsikE[3],
        mBufC_PsijM[3], mBufC_PsikM[3];
    */
    
    // These vectors are the actual location of the allocated fields and
    // update constants.
    std::vector<float> mAccumEj[3], mAccumEk[3],
        mAccumHj[3], mAccumHk[3];
    std::vector<float> mC_JjH[3], mC_JkH[3],
        mC_PhijH[3], mC_PhikH[3],
        mC_PhijJ[3], mC_PhikJ[3];
    std::vector<float> mC_MjE[3], mC_MkE[3],
        mC_PsijE[3], mC_PsikE[3],
        mC_PsijM[3], mC_PsikM[3];
    
    
    
    Rect3i mMyPMLHalfCells;
    Vector3i mPMLDir;
    
    Vector3f mDxyz;
    float mDt;
    float m_epsr;
    float m_mur;
};

#include "StaticDielectricPML.cpp"

#endif