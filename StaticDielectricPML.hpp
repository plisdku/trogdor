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
StaticDielectricPMLDelegate(Vector3i pmlDir) :
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
                depthHalf = float(halfCellsOnSide.p2[pmlDir]-nHalf+ONE)
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
                depthHalf = float(halfCellsOnSide.p2[pmlDir]-nHalf+ONE)
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
    
    // This disgusting dispatch procedure is necessary because the attenuation
    // directions are handled via templates, which are static creatures...
    if (mPMLDir[0] != 0)
    {
        if (mPMLDir[1] && mPMLDir[2])
        {
            m = MaterialPtr(new StaticDielectricPML<1,1,1>(*this,
                cp.getDxyz(), cp.getDt() ));
        }
        else if (mPMLDir[1])
        {
            m = MaterialPtr(new StaticDielectricPML<1,1,0>(*this,
                cp.getDxyz(), cp.getDt() ));
        }
        else if (mPMLDir[2])
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
    else if (mPMLDir[1] != 0)
    {
        if (mPMLDir[2])
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
    else if (mPMLDir[2] != 0)
    {
        m = MaterialPtr(new StaticDielectricPML<0,0,1>(*this,
                cp.getDxyz(), cp.getDt() ));
    }
    else
        assert(!"What, a directionless PML???");
    
    return m;
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
StaticDielectricPML::
StaticDielectricPML(const StaticDielectricPMLDelegate & deleg, Vector3f dxyz,
    float dt) :
    Material(),
    mDxyz(dxyz),
    mDt(dt),
    m_epsr(1.0),
    m_mur(1.0)
{
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
    /*
    // For simplicity all field octants use the same size of PML array.
    for (int attenDir = 0; attenDir < 3; attenDir++)
    if (mPMLDir[attenDir] != 0)
    {
        int depthYee = mPMLHalfCells.p2[attenDir]/2
            - mPMLHalfCells.p1[attenDir]/2 + 1;
        
        mC_JjH[attenDir].resize(depthYee);
        mC_Jjk[attenDir].resize(depthYee);
        mC_PhijH[attenDir].resize(depthYee);
        mC_PhikH[attenDir].resize(depthYee);
        mC_PhijJ[attenDir].resize(depthYee);
        mC_PhikJ[attenDir].resize(depthYee);
        mC_MjE[attenDir].resize(depthYee);
        mC_MkE[attenDir].resize(depthYee);
        mC_PsijE[attenDir].resize(depthYee);
        mC_PsikE[attenDir].resize(depthYee);
        mC_PsijM[attenDir].resize(depthYee);
        mC_PsikM[attenDir].resize(depthYee);
    }
    */
    
    /*
    // Allocate the auxiliary thingies
    int attenDir;
    for (attenDir = 0; attenDir < 3; attenDir++)
    if (mPMLDir[attenDir] != 0)
    {
        int depthYee;
        int jDir = (attenDir+1)%3;
        int kDir = (jDir+1)%3;
        
        for (int nField = 1; nField <= 2; nField++)
        {
            int jkDir = (attenDir+nField)%3;
            
            // E field things!
            Rect3i ePMLYee = rectHalfToYee(mPMLHalfCells,
                eOctantNumber(jkDir));
            depthYee = ePMLYee.size(attenDir)+1;
            
            mC_J
                        
            // H field things!
            Rect3i hPMLYee = rectHalfToYee(mPMLHalfCells,
                hOctantNumber(jkDir));
            depthYee = hPMLYee.size(attenDir)+1;
        }
    }
    */
    
    //LOG << "Created all runlines.\n";
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticDielectricPML::
calcEPhase(int direction)
{
    //LOG << "Calculating E.\n";
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticDielectricPML::
calcHPhase(int direction)
{
    //LOG << "Calculating H.\n";
}


#endif