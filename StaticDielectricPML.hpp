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
#include "PhysicalConstants.h"
#include <sstream>

using namespace YeeUtilities;
using namespace std;


static vector<float>
calcC_JH(const vector<float> & kappa,
    const vector<float> & sigma, const vector<float> & alpha, float dt)
{
    assert(kappa.size() == sigma.size() && sigma.size() == alpha.size());
    vector<float> c(sigma.size());
    for (unsigned int nn = 0; nn < sigma.size(); nn++)
    {
        c[nn] = ( Constants::eps0 + 0.5*alpha[nn]*dt ) /
            ( kappa[nn]*Constants::eps0 +
            0.5*dt*(kappa[nn]*alpha[nn]+sigma[nn]) )
            - 1.0;
    }
    return c;
}

static vector<float>
calcC_PhiH(const vector<float> & kappa,
    const vector<float> & sigma, const vector<float> & alpha, float dt)
{
    assert(kappa.size() == sigma.size() && sigma.size() == alpha.size());
    vector<float> c(sigma.size());
    for (unsigned int nn = 0; nn < sigma.size(); nn++)
    {
        c[nn] = dt*( (1.0-kappa[nn])*alpha[nn] - sigma[nn] ) /
            (kappa[nn]*Constants::eps0 +
            0.5*dt*(kappa[nn]*alpha[nn]+sigma[nn]));
    }
    return c;
}

static vector<float>
calcC_PhiJ(const vector<float> & kappa,
    const vector<float> & sigma, const vector<float> & alpha, float dt)
{
    assert(kappa.size() == sigma.size() && sigma.size() == alpha.size());
    vector<float> c(sigma.size());
    for (unsigned int nn = 0; nn < sigma.size(); nn++)
    {
        c[nn] = dt*( kappa[nn]*alpha[nn] + sigma[nn] ) /
            ( kappa[nn]*Constants::eps0 +
                0.5*dt*(kappa[nn]*alpha[nn]+sigma[nn]) );
    }
    return c;
}


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
setPMLHalfCells(int faceNum, Rect3i halfCellsOnSide)
{
    Rect3i pmlYee;
    int pmlDepthYee;
    int nYee, nHalf, nHalf0;
    int fieldDir;
    float depthHalf, depthFrac;
    float pmlDepthHalf = halfCellsOnSide.size(faceNum/2)+2; // bigger by 1!!!
    
    const int ONE = 0; // if I set it to 0, the PML layer includes depth==0.
    
    for (fieldDir = 0; fieldDir < 3; fieldDir++)
    if (fieldDir != faceNum/2)
    {
        // E field auxiliary constants
        pmlYee = rectHalfToYee(halfCellsOnSide, eOctantNumber(fieldDir));
        pmlDepthYee = pmlYee.size(faceNum/2)+1;
        mSigmaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mAlphaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mKappaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        
        nHalf0 = rectYeeToHalf(pmlYee, eOctantNumber(fieldDir)).p1[faceNum/2];
        
        for (nYee = 0, nHalf=nHalf0; nYee < pmlDepthYee; nYee++, nHalf+=2)
        {
            if (faceNum%2 == 0) // going left/down etc. (negative direction)
                depthHalf = float(halfCellsOnSide.p2[faceNum/2]-nHalf+ONE);
            else
                depthHalf = float(nHalf-halfCellsOnSide.p1[faceNum/2]+ONE);
            depthFrac = depthHalf / pmlDepthHalf;
            mSigmaE[fieldDir][faceNum/2][nYee] = depthFrac;
            mKappaE[fieldDir][faceNum/2][nYee] = 1.0;
            mAlphaE[fieldDir][faceNum/2][nYee] = 0.0;
        }
        LOG << "E: field " << fieldDir << " faceNum " << faceNum << " num "
            << pmlDepthYee << "\n";
        
        // H field auxiliary constants
        pmlYee = rectHalfToYee(halfCellsOnSide, hOctantNumber(fieldDir));
        pmlDepthYee = pmlYee.size(faceNum/2)+1;
        mSigmaH[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mAlphaH[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mKappaH[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        
        nHalf0 = rectYeeToHalf(pmlYee, hOctantNumber(fieldDir)).p1[faceNum/2];
        
        for (nYee = 0, nHalf=nHalf0; nYee < pmlDepthYee; nYee++, nHalf+=2)
        {
            if (faceNum%2 == 0) // going left/down etc. (negative direction)
                depthHalf = float(halfCellsOnSide.p2[faceNum/2]-nHalf+ONE);
            else
                depthHalf = float(nHalf-halfCellsOnSide.p1[faceNum/2]+ONE);
            depthFrac = depthHalf / pmlDepthHalf;
            mSigmaH[fieldDir][faceNum/2][nYee] = depthFrac;
            mKappaH[fieldDir][faceNum/2][nYee] = 1.0;
            mAlphaH[fieldDir][faceNum/2][nYee] = 0.0;
        }
        LOG << "H: field " << fieldDir << " faceNum " << faceNum << " num "
            << pmlDepthYee << "\n";

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
        LOG << "Printing as we create runlines in field " << field << "\n";
        const std::vector<SBPMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
        {
            mRunlines[field][nn] = SimplePMLRunline(*setupRunlines[nn]);
            LOGMORE << mRunlines[field][nn] << "\n";
        }
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
            mC_JjH[fieldDir] = calcC_JH(deleg.getKappaE(fieldDir,jDir),
                deleg.getSigmaE(fieldDir,jDir), deleg.getAlphaE(fieldDir,jDir),
                dt);
            mC_PhijH[fieldDir] = calcC_PhiH(deleg.getKappaE(fieldDir,jDir),
                deleg.getSigmaE(fieldDir,jDir), deleg.getAlphaE(fieldDir,jDir),
                dt);
            mC_PhijJ[fieldDir] = calcC_PhiJ(deleg.getKappaE(fieldDir,jDir),
                deleg.getSigmaE(fieldDir,jDir), deleg.getAlphaE(fieldDir,jDir),
                dt);
            
            // the magnetic coefficients are of the same form as the electric
            // ones and can be handled with the same functions 
            mC_MjE[fieldDir] = calcC_JH(deleg.getKappaE(fieldDir,jDir),
                deleg.getSigmaE(fieldDir,jDir), deleg.getAlphaE(fieldDir,jDir),
                dt);
            mC_PsijE[fieldDir] = calcC_PhiH(deleg.getKappaE(fieldDir,jDir),
                deleg.getSigmaE(fieldDir,jDir), deleg.getAlphaE(fieldDir,jDir),
                dt);
            mC_PsijM[fieldDir] = calcC_PhiJ(deleg.getKappaE(fieldDir,jDir),
                deleg.getSigmaE(fieldDir,jDir), deleg.getAlphaE(fieldDir,jDir),
                dt);
        }
        
        if (pmlDir[kDir] != 0)
        {
            mC_JkH[fieldDir] = calcC_JH(deleg.getKappaE(fieldDir,kDir),
                deleg.getSigmaE(fieldDir,kDir), deleg.getAlphaE(fieldDir,kDir),
                dt);
            mC_PhikH[fieldDir] = calcC_PhiH(deleg.getKappaE(fieldDir,kDir),
                deleg.getSigmaE(fieldDir,kDir), deleg.getAlphaE(fieldDir,kDir),
                dt);
            mC_PhikJ[fieldDir] = calcC_PhiJ(deleg.getKappaE(fieldDir,kDir),
                deleg.getSigmaE(fieldDir,kDir), deleg.getAlphaE(fieldDir,kDir),
                dt);
            
            // the magnetic coefficients are of the same form as the electric
            // ones and can be handled with the same functions 
            mC_MkE[fieldDir] = calcC_JH(deleg.getKappaE(fieldDir,kDir),
                deleg.getSigmaE(fieldDir,kDir), deleg.getAlphaE(fieldDir,kDir),
                dt);
            mC_PsikE[fieldDir] = calcC_PhiH(deleg.getKappaE(fieldDir,kDir),
                deleg.getSigmaE(fieldDir,kDir), deleg.getAlphaE(fieldDir,kDir),
                dt);
            mC_PsikM[fieldDir] = calcC_PhiJ(deleg.getKappaE(fieldDir,kDir),
                deleg.getSigmaE(fieldDir,kDir), deleg.getAlphaE(fieldDir,kDir),
                dt);
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