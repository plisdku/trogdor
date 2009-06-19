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

class SetupStaticDielectricPML : public SimpleBulkPMLSetupMaterial
{
public:
    SetupStaticDielectricPML(
        const Map<Vector3i, Map<std::string, std::string> > & pmlParams);
    
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
        
    virtual void setNumCellsE(int fieldDir, int numCells);
    virtual void setNumCellsH(int fieldDir, int numCells);
    virtual void setPMLHalfCells(int pmlDir, Rect3i halfCellsOnSide,
        const GridDescription & gridDesc);
    
    const std::vector<float> & getSigmaE(int fieldDir, int pmlDir) const
        { return mSigmaE[fieldDir][pmlDir]; }
    const std::vector<float> & getAlphaE(int fieldDir, int pmlDir) const
        { return mAlphaE[fieldDir][pmlDir]; }
    const std::vector<float> & getKappaE(int fieldDir, int pmlDir) const
        { return mKappaE[fieldDir][pmlDir]; }
    
    const std::vector<float> & getSigmaH(int fieldDir, int pmlDir) const
        { return mSigmaH[fieldDir][pmlDir]; }
    const std::vector<float> & getAlphaH(int fieldDir, int pmlDir) const
        { return mAlphaH[fieldDir][pmlDir]; }
    const std::vector<float> & getKappaH(int fieldDir, int pmlDir) const
        { return mKappaH[fieldDir][pmlDir]; }
    
    MemoryBufferPtr getBufAccumEj(int fieldDir) const
        { return mBufAccumEj[fieldDir]; }
    MemoryBufferPtr getBufAccumEk(int fieldDir) const
        { return mBufAccumEk[fieldDir]; }
    MemoryBufferPtr getBufAccumHj(int fieldDir) const
        { return mBufAccumHj[fieldDir]; }
    MemoryBufferPtr getBufAccumHk(int fieldDir) const
        { return mBufAccumHk[fieldDir]; }
    
    
private:
    MemoryBufferPtr mBufAccumEj[3], mBufAccumEk[3],
        mBufAccumHj[3], mBufAccumHk[3];
    
    Map<Vector3i, Map<std::string, std::string> > mPMLParams;
    
    // Indexing is [fieldDir][pmlDir].  The diagonal elements are unused because
    // Ex is not attenuated in the x direction, etc.
    std::vector<float> mSigmaE[3][3];
    std::vector<float> mSigmaH[3][3];
    std::vector<float> mAlphaE[3][3];
    std::vector<float> mAlphaH[3][3];
    std::vector<float> mKappaE[3][3];
    std::vector<float> mKappaH[3][3];
};

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
class StaticDielectricPML : public Material
{
public:
    StaticDielectricPML(const SetupStaticDielectricPML & deleg,
        Vector3f dxyz, float dt);
    
    virtual void calcEPhase(int direction);
    virtual void calcHPhase(int direction);
    
    virtual void allocateAuxBuffers();
private:
    void calcEx();
    void calcEy();
    void calcEz();
    void calcHx();
    void calcHy();
    void calcHz();
    
    std::vector<SimpleAuxPMLRunline> mRunlinesE[3];
    std::vector<SimpleAuxPMLRunline> mRunlinesH[3];
    
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
    
    MemoryBufferPtr mBufAccumEj[3], mBufAccumEk[3],
        mBufAccumHj[3], mBufAccumHk[3];
    Vector3i mPMLDirection;
    
    Vector3f mDxyz;
    float mDt;
    float m_epsr;
    float m_mur;
};

#include "StaticDielectricPML.hpp"

#endif