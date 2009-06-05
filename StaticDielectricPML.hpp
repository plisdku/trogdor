/*
 *  StaticDielectricPML.hh
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifdef _STATICDIELECTRICPML_

#include "StaticDielectricPML.h"
#include "CalculationPartition.h"
#include "Paint.h"
#include "YeeUtilities.h"
#include <sstream>

using namespace YeeUtilities;
using namespace std;

StaticDielectricPMLDelegate::
StaticDielectricPMLDelegate() :
    SimpleBulkPMLMaterialDelegate()
{
}

void StaticDielectricPMLDelegate::
setNumCellsE(int fieldDir, int numCells)
{
    const int STRIDE = 1;
    int jDir = (fieldDir+1)%3;
    int kDir = (jDir+1)%3;
    char fieldChar = '0' + fieldDir;
    Vector3i pmlDir = getParentPaint()->getPMLDirections();
    string prefix = getParentPaint()->getBulkMaterial()->getName() + " accum E";
    
    if (pmlDir[jDir])
    {
        mBufAccumEj[fieldDir] = MemoryBufferPtr(
            new MemoryBuffer( prefix + 'j' + fieldChar, numCells, STRIDE ) );
    }
    if (pmlDir[kDir])
    {
        mBufAccumEk[fieldDir] = MemoryBufferPtr(
            new MemoryBuffer( prefix + 'k' + fieldChar, numCells, STRIDE ) );
    }
}

void StaticDielectricPMLDelegate::
setNumCellsH(int fieldDir, int numCells)
{
    const int STRIDE = 1;
    int jDir = (fieldDir+1)%3;
    int kDir = (jDir+1)%3;
    char fieldChar = '0' + fieldDir;
    Vector3i pmlDir = getParentPaint()->getPMLDirections();
    string prefix = getParentPaint()->getBulkMaterial()->getName() + " accum H";
    
    if (pmlDir[jDir])
    {
        mBufAccumHj[fieldDir] = MemoryBufferPtr(
            new MemoryBuffer( prefix + 'j' + fieldChar, numCells, STRIDE ) );
    }
    if (pmlDir[kDir])
    {
        mBufAccumHk[fieldDir] = MemoryBufferPtr(
            new MemoryBuffer( prefix + 'k' + fieldChar, numCells, STRIDE ) );
    }
}

void StaticDielectricPMLDelegate::
setPMLHalfCells(int pmlDir, Rect3i halfCellsOnSide)
{
    Rect3i pmlYee;
    int pmlDepthYee;
    int nYee, nHalf, nHalf0;
    int fieldDir;
    float depthHalf, depthFrac;
    float pmlDepthHalf = halfCellsOnSide.size(pmlDir)+2; // bigger by 1!!!
    
    const int ONE = 0; // if I set it to 0, the PML layer includes depth==0.
    
    for (fieldDir = 0; fieldDir < 3; fieldDir++)
    if (fieldDir != pmlDir)
    {
        // E field auxiliary constants
        pmlYee = rectHalfToYee(halfCellsOnSide, eOctantNumber(fieldDir));
        pmlDepthYee = pmlYee.size(pmlDir)+1;
        mSigmaE[fieldDir][pmlDir].resize(pmlDepthYee);
        mAlphaE[fieldDir][pmlDir].resize(pmlDepthYee);
        mKappaE[fieldDir][pmlDir].resize(pmlDepthYee);
        
        nHalf0 = rectYeeToHalf(pmlYee, eOctantNumber(fieldDir)).p1[pmlDir];
        
        for (nYee = 0, nHalf=nHalf0; nYee < pmlDepthYee; nYee++, nHalf+=2)
        {
            if (pmlDir%2 == 0) // going left/down etc. (negative direction)
                depthHalf = float(halfCellsOnSide.p2[pmlDir]-nHalf+ONE);
            else
                depthHalf = float(nHalf-halfCellsOnSide.p1[pmlDir]+ONE);
            depthFrac = depthHalf / pmlDepthHalf;
            mSigmaE[fieldDir][pmlDir][nYee] = depthFrac;
            mKappaE[fieldDir][pmlDir][nYee] = 1.0;
            mAlphaE[fieldDir][pmlDir][nYee] = 0.0;
        }
        
        // H field auxiliary constants
        pmlYee = rectHalfToYee(halfCellsOnSide, hOctantNumber(fieldDir));
        pmlDepthYee = pmlYee.size(pmlDir)+1;
        mSigmaH[fieldDir][pmlDir].resize(pmlDepthYee);
        mAlphaH[fieldDir][pmlDir].resize(pmlDepthYee);
        mKappaH[fieldDir][pmlDir].resize(pmlDepthYee);
        
        nHalf0 = rectYeeToHalf(pmlYee, hOctantNumber(fieldDir)).p1[pmlDir];
        
        for (nYee = 0, nHalf=nHalf0; nYee < pmlDepthYee; nYee++, nHalf+=2)
        {
            if (pmlDir%2 == 0) // going left/down etc. (negative direction)
                depthHalf = float(halfCellsOnSide.p2[pmlDir]-nHalf+ONE);
            else
                depthHalf = float(nHalf-halfCellsOnSide.p1[pmlDir]+ONE);
            depthFrac = depthHalf / pmlDepthHalf;
            mSigmaH[fieldDir][pmlDir][nYee] = depthFrac;
            mKappaH[fieldDir][pmlDir][nYee] = 1.0;
            mAlphaH[fieldDir][pmlDir][nYee] = 0.0;
        }

    }
    
    if (ONE != 1)
        LOG << "Warning: ONE is not 1.\n";
}

MaterialPtr StaticDielectricPMLDelegate::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    MaterialPtr m;
    Vector3i pmlDir = getParentPaint()->getPMLDirections();
    
    // This disgusting dispatch procedure is necessary because the attenuation
    // directions are handled via templates, which are static creatures...
    if (pmlDir[0] != 0)
    {
        if (pmlDir[1] && pmlDir[2])
        {
            m = MaterialPtr(new StaticDielectricPML<1,1,1>(*this,
                cp.getDxyz(), cp.getDt() ));
        }
        else if (pmlDir[1])
        {
            m = MaterialPtr(new StaticDielectricPML<1,1,0>(*this,
                cp.getDxyz(), cp.getDt() ));
        }
        else if (pmlDir[2])
        {
            m = MaterialPtr(new StaticDielectricPML<1,0,1>(*this,
                cp.getDxyz(), cp.getDt() ));
        }
        else
        {
            m = MaterialPtr(new StaticDielectricPML<1,0,0>(*this,
                cp.getDxyz(), cp.getDt() ));
        }
    }
    else if (pmlDir[1] != 0)
    {
        if (pmlDir[2])
        {
            m = MaterialPtr(new StaticDielectricPML<0,1,1>(*this,
                cp.getDxyz(), cp.getDt() ));
        }
        else
        {
            m = MaterialPtr(new StaticDielectricPML<0,1,0>(*this,
                cp.getDxyz(), cp.getDt() ));
        }
    }
    else if (pmlDir[2] != 0)
    {
        m = MaterialPtr(new StaticDielectricPML<0,0,1>(*this,
                cp.getDxyz(), cp.getDt() ));
    }
    else
        assert(!"What, a directionless PML???");
    
    return m;
}


template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
StaticDielectricPML(const StaticDielectricPMLDelegate & deleg, Vector3f dxyz,
    float dt) :
    Material(),
    mDxyz(dxyz),
    mDt(dt),
    m_epsr(1.0),
    m_mur(1.0)
{
    int fieldDir, jDir, kDir;
    MaterialDescPtr desc = deleg.getParentPaint()->getBulkMaterial();
     
    if (desc->getParams().count("epsr"))
        istringstream(desc->getParams()["epsr"]) >> m_epsr;
    if (desc->getParams().count("mur"))
        istringstream(desc->getParams()["mur"]) >> m_mur;
    
    // Make the runlines
    for (int field = 0; field < 6; field++)
    {
        const std::vector<SBPMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
            mRunlines[field][nn] = SimplePMLRunline(*setupRunlines[nn]);
    }
    
    // PML STUFF HERE
    Vector3i pmlDir = deleg.getParentPaint()->getPMLDirections();
    
    // Allocate auxiliary variables
    MemoryBufferPtr p;
    for (fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        jDir = (fieldDir+1)%3;
        kDir = (fieldDir+2)%3;
        
        if (pmlDir[jDir] != 0)
        {
            p = deleg.getBufAccumEj(fieldDir);
            mAccumEj[fieldDir].resize(p->getLength());
            p->setHeadPointer(&(mAccumEj[fieldDir][0]));
            
            p = deleg.getBufAccumHj(fieldDir);
            mAccumHj[fieldDir].resize(p->getLength());
            p->setHeadPointer(&(mAccumHj[fieldDir][0]));
        }
        
        if (pmlDir[kDir] != 0)
        {
            p = deleg.getBufAccumEk(fieldDir);
            mAccumEk[fieldDir].resize(p->getLength());
            p->setHeadPointer(&(mAccumEk[fieldDir][0]));
            
            p = deleg.getBufAccumHk(fieldDir);
            mAccumHk[fieldDir].resize(p->getLength());
            p->setHeadPointer(&(mAccumHk[fieldDir][0]));
        }
    }
    
    // Allocate and calculate update constants
    for (fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        jDir = (fieldDir+1)%3;
        kDir = (fieldDir+2)%3;
        
        if (pmlDir[jDir] != 0)
        {
            mC_JjH[fieldDir].resize(deleg.getSigmaE(fieldDir, jDir).size());
            mC_PhijH[fieldDir].resize(deleg.getSigmaE(fieldDir, jDir).size());
            mC_PhijJ[fieldDir].resize(deleg.getSigmaE(fieldDir, jDir).size());
            mC_MjE[fieldDir].resize(deleg.getSigmaH(fieldDir, jDir).size());
            mC_PsijE[fieldDir].resize(deleg.getSigmaH(fieldDir, jDir).size());
            mC_PsijM[fieldDir].resize(deleg.getSigmaH(fieldDir, jDir).size());
        }
        
        if (pmlDir[kDir] != 0)
        {
            mC_JkH[fieldDir].resize(deleg.getSigmaE(fieldDir, kDir).size());
            mC_PhikH[fieldDir].resize(deleg.getSigmaE(fieldDir, kDir).size());
            mC_PhikJ[fieldDir].resize(deleg.getSigmaE(fieldDir, kDir).size());
            mC_MkE[fieldDir].resize(deleg.getSigmaH(fieldDir, kDir).size());
            mC_PsikE[fieldDir].resize(deleg.getSigmaH(fieldDir, kDir).size());
            mC_PsikM[fieldDir].resize(deleg.getSigmaH(fieldDir, kDir).size());
        }
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcEPhase(int direction)
{
    //LOG << "Calculating E.\n";
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcHPhase(int direction)
{
    //LOG << "Calculating H.\n";
}


#endif