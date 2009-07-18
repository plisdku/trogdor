/*
 *  StandardPML.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/15/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _STANDARDPML_
#define _STANDARDPML_
#if 0

#include "geometry.h"
#include "MemoryUtilities.h"
#include "Paint.h"
#include "SimpleSetupMaterial.h"

#include <string>
#include <vector>

class AbstractPML
{
public:
    virtual ~AbstractPML();
    virtual void calcEx(std::vector<SimpleAuxPMLRunline> & runlines) = 0;
    virtual void calcEy(std::vector<SimpleAuxPMLRunline> & runlines) = 0;
    virtual void calcEz(std::vector<SimpleAuxPMLRunline> & runlines) = 0;
    virtual void calcHx(std::vector<SimpleAuxPMLRunline> & runlines) = 0;
    virtual void calcHy(std::vector<SimpleAuxPMLRunline> & runlines) = 0;
    virtual void calcHz(std::vector<SimpleAuxPMLRunline> & runlines) = 0;
    
    virtual void allocateAuxBuffers() = 0;
};
typedef Pointer<AbstractPML> AbstractPMLPtr;
/*
class SetupStandardPML
{
public:
    SetupStandardPML(
        const Map<Vector3i, Map<std::string, std::string> > & pmlParams,
        Paint* parentPaint, Vector3f dxyz, float dt);
    
    virtual AbstractPMLPtr newPML() const;
    
    void setNumCellsE(int fieldDir, int numCells, Paint* parentPaint);
    void setNumCellsH(int fieldDir, int numCells, Paint* parentPaint);
    void setPMLHalfCells(int pmlDir, Rect3i halfCellsOnSide,
        const GridDescription & gridDesc, Paint* parentPaint);
    
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
    
    std::vector<Rect3i> mPMLHalfCells;
    Map<Vector3i, Map<std::string, std::string> > mPMLParams;
    Paint* mParentPaint;
    Vector3f mDxyz;
    float dt;
    
    // Indexing is [fieldDir][pmlDir].  The diagonal elements are unused because
    // Ex is not attenuated in the x direction, etc.
    std::vector<float> mSigmaE[3][3];
    std::vector<float> mSigmaH[3][3];
    std::vector<float> mAlphaE[3][3];
    std::vector<float> mAlphaH[3][3];
    std::vector<float> mKappaE[3][3];
    std::vector<float> mKappaH[3][3];
};
*/

class StandardPMLFactory
{
public:
    static AbstractPMLPtr newPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
};

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
class StandardPML : public AbstractPML
{
public:
    StandardPML(Paint* parentPaint, std::vector<int> numCellsE,
        std::vector<int> numCellsH, std::vector<Rect3i> pmlHalfCells,
        Map<Vector3i, Map<std::string,std::string> > pmlParams, Vector3f dxyz,
        float dt);
    
    // three setup functions
    void setNumCellsE(int fieldDir, int numCells, Paint* parentPaint);
    void setNumCellsH(int fieldDir, int numCells, Paint* parentPaint);
    void setPMLHalfCells(int pmlDir, Rect3i halfCellsOnSide,
        Paint* parentPaint);
    
    virtual void allocateAuxBuffers();
    
    Vector3i getPMLDirection() const { return mPMLDirection; }

    std::vector<float> & getAccumEj(int direction)
        { return mAccumEj[direction]; }
    std::vector<float> & getAccumEk(int direction)
        { return mAccumEk[direction]; }
    std::vector<float> & getAccumHj(int direction)
        { return mAccumHj[direction]; }
    std::vector<float> & getAccumHk(int direction)
        { return mAccumHk[direction]; }
    
    std::vector<float> & getC_JjH(int direction)
        { return mC_JjH[direction]; }
    std::vector<float> & getC_JkH(int direction)
        { return mC_JkH[direction]; }
    std::vector<float> & getC_PhijH(int direction)
        { return mC_PhijH[direction]; }
    std::vector<float> & getC_PhikH(int direction)
        { return mC_PhikH[direction]; }
    std::vector<float> & getC_PhijJ(int direction)
        { return mC_PhijJ[direction]; }
    std::vector<float> & getC_PhikJ(int direction)
        { return mC_PhikJ[direction]; }
    
    std::vector<float> & getC_MjE(int direction)
        { return mC_MjE[direction]; }
    std::vector<float> & getC_MkE(int direction)
        { return mC_MkE[direction]; }
    std::vector<float> & getC_PsijE(int direction)
        { return mC_PsijE[direction]; }
    std::vector<float> & getC_PsikE(int direction)
        { return mC_PsikE[direction]; }
    std::vector<float> & getC_PsijM(int direction)
        { return mC_PsijM[direction]; }
    std::vector<float> & getC_PsikM(int direction)
        { return mC_PsikM[direction]; }
    
    virtual void calcEx(std::vector<SimpleAuxPMLRunline> & runlines);
    virtual void calcEy(std::vector<SimpleAuxPMLRunline> & runlines);
    virtual void calcEz(std::vector<SimpleAuxPMLRunline> & runlines);
    virtual void calcHx(std::vector<SimpleAuxPMLRunline> & runlines);
    virtual void calcHy(std::vector<SimpleAuxPMLRunline> & runlines);
    virtual void calcHz(std::vector<SimpleAuxPMLRunline> & runlines);
    
private:
    // Formerly setup things:
    std::vector<float> mSigmaE[3][3];
    std::vector<float> mSigmaH[3][3];
    std::vector<float> mAlphaE[3][3];
    std::vector<float> mAlphaH[3][3];
    std::vector<float> mKappaE[3][3];
    std::vector<float> mKappaH[3][3];
    
    std::vector<Rect3i> mPMLHalfCells;
    Map<Vector3i, Map<std::string, std::string> > mPMLParams;
    
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




#include "StandardPML.cpp"

#endif // 0

#endif
