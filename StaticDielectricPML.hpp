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
#include "calc.hh"
#include "STLOutput.h"
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
StaticDielectricPMLDelegate(
    const Map<Vector3i, Map<string, string> > & pmlParams) :
    SimpleBulkPMLMaterialDelegate(),
    mPMLParams(pmlParams)
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
setPMLHalfCells(int faceNum, Rect3i halfCellsOnSide,
    const GridDescription & gridDesc)
{
    Vector3i pmlDir = getParentPaint()->getPMLDirections();
    
    if (dot(pmlDir, cardinalDirection(faceNum)) < 0) // check which side it is
        return;
    
    Rect3i pmlYee;
    int pmlDepthYee;
    int nYee, nHalf, nHalf0;
    int fieldDir;
    float depthHalf, depthFrac;
    float pmlDepthHalf = halfCellsOnSide.size(faceNum/2)+1;
    string alphaStr, kappaStr, sigmaStr;
    
    const int ONE = 1; // if I set it to 0, the PML layer includes depth==0.
    
    //LOG << "------------ " << getParentPaint()->getPMLDirections() << "\n";
    
    calc_defs::Calculator<float> calculator;
    calculator.set("eps0", Constants::eps0);
    calculator.set("mu0", Constants::mu0);
    calculator.set("L", pmlDepthHalf*gridDesc.getDxyz()[faceNum/2]);
    calculator.set("dx", gridDesc.getDxyz()[faceNum/2]);
    
    for (fieldDir = 0; fieldDir < 3; fieldDir++)
    if (fieldDir != faceNum/2)
    {
        // E field auxiliary constants
        pmlYee = rectHalfToYee(halfCellsOnSide, eOctantNumber(fieldDir));
        pmlDepthYee = pmlYee.size(faceNum/2)+1;
        mSigmaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mAlphaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        mKappaE[fieldDir][faceNum/2].resize(pmlDepthYee, 0.0);
        
        // the first half cell of the current octant.  we jump through hoops
        // to make sure that it has the right half cell offset.
        nHalf0 = rectYeeToHalf(pmlYee, eOctantNumber(fieldDir)).p1[faceNum/2];
        
        alphaStr = mPMLParams[cardinalDirection(faceNum)]["alpha"];
        kappaStr = mPMLParams[cardinalDirection(faceNum)]["kappa"];
        sigmaStr = mPMLParams[cardinalDirection(faceNum)]["sigma"];
        
        for (nYee = 0, nHalf=nHalf0; nYee < pmlDepthYee; nYee++, nHalf+=2)
        {
            if (faceNum%2 == 0) // going left/down etc. (negative direction)
                depthHalf = float(halfCellsOnSide.p2[faceNum/2]-nHalf+ONE);
            else
                depthHalf = float(nHalf-halfCellsOnSide.p1[faceNum/2]+ONE);
            depthFrac = depthHalf / pmlDepthHalf;
            calculator.set("d", depthFrac);
            /*
            LOG << "nYee " << nYee << " nHalf " << nHalf << " within "
                << halfCellsOnSide << " depth " << depthFrac << "\n";
            */
            bool parseError = calculator.parse(sigmaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mSigmaE[fieldDir][faceNum/2][nYee] = calculator.get_value();
            parseError = calculator.parse(kappaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mKappaE[fieldDir][faceNum/2][nYee] = calculator.get_value();
            parseError = calculator.parse(alphaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mAlphaE[fieldDir][faceNum/2][nYee] = calculator.get_value();
            
            /*
            mSigmaE[fieldDir][faceNum/2][nYee] = depthFrac;
            mKappaE[fieldDir][faceNum/2][nYee] = 1.0;
            mAlphaE[fieldDir][faceNum/2][nYee] = 0.0;
            */
        }
        /*
        LOG << "E: field " << fieldDir << " faceNum " << faceNum << " num "
            << pmlDepthYee << "\n";
        LOGMORE << "Sigma " << sigmaStr << "\n" <<
            mSigmaE[fieldDir][faceNum/2] << endl;
        LOGMORE << "Alpha " << alphaStr << "\n" <<
            mAlphaE[fieldDir][faceNum/2] << endl;
        LOGMORE << "Kappa " << kappaStr << "\n" <<
            mKappaE[fieldDir][faceNum/2] << endl;
        */
        
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
            calculator.set("d", depthFrac);
            /*
            LOG << "nYee " << nYee << " nHalf " << nHalf << " within "
                << halfCellsOnSide << "\n";
            */
            bool parseError = calculator.parse(sigmaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mSigmaH[fieldDir][faceNum/2][nYee] = calculator.get_value();
            parseError = calculator.parse(kappaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mKappaH[fieldDir][faceNum/2][nYee] = calculator.get_value();
            parseError = calculator.parse(alphaStr);
            if (parseError)
            {
                calculator.report_error(cerr);
                exit(1);
            }
            mAlphaH[fieldDir][faceNum/2][nYee] = calculator.get_value();
            
            /*
            mSigmaH[fieldDir][faceNum/2][nYee] = depthFrac;
            mKappaH[fieldDir][faceNum/2][nYee] = 1.0;
            mAlphaH[fieldDir][faceNum/2][nYee] = 0.0;
            */
        }
        /*
        LOG << "H: field " << fieldDir << " faceNum " << faceNum << " num "
            << pmlDepthYee << "\n";
        LOGMORE << "Sigma " << sigmaStr << "\n" <<
            mSigmaH[fieldDir][faceNum/2] << endl;
        LOGMORE << "Alpha " << alphaStr << "\n" <<
            mAlphaH[fieldDir][faceNum/2] << endl;
        LOGMORE << "Kappa " << kappaStr << "\n" <<
            mKappaH[fieldDir][faceNum/2] << endl;
        */
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
        //LOG << "Printing as we create runlines in field " << field << "\n";
        const std::vector<SBPMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
        {
            mRunlines[field][nn] = SimpleAuxPMLRunline(*setupRunlines[nn]);
            /*
            LOGMORE << MemoryBuffer::identify(mRunlines[field][nn].fi)
                << " " << MemoryBuffer::identify(mRunlines[field][nn].gj[0]) <<
                " " << MemoryBuffer::identify(mRunlines[field][nn].gk[0]) <<
                "\n";
            */
            //LOGMORE << mRunlines[field][nn] << "\n";
        }
    }
    
    // PML STUFF HERE
    Vector3i pmlDir = deleg.getParentPaint()->getPMLDirections();
    //LOG << "------- " << pmlDir << "\n";
    
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
            mC_MjE[fieldDir] = calcC_JH(deleg.getKappaH(fieldDir,jDir),
                deleg.getSigmaH(fieldDir,jDir), deleg.getAlphaH(fieldDir,jDir),
                dt);
            mC_PsijE[fieldDir] = calcC_PhiH(deleg.getKappaH(fieldDir,jDir),
                deleg.getSigmaH(fieldDir,jDir), deleg.getAlphaH(fieldDir,jDir),
                dt);
            mC_PsijM[fieldDir] = calcC_PhiJ(deleg.getKappaH(fieldDir,jDir),
                deleg.getSigmaH(fieldDir,jDir), deleg.getAlphaH(fieldDir,jDir),
                dt);
            /*
            LOG << "SigmaE " << fieldDir << jDir << ":\n";
            LOGMORE << deleg.getSigmaE(fieldDir,jDir) << "\n";
            LOG << "KappaE " << fieldDir << jDir << ":\n";
            LOGMORE << deleg.getKappaE(fieldDir,jDir) << "\n";
            LOG << "AlphaE " << fieldDir << jDir << ":\n";
            LOGMORE << deleg.getAlphaE(fieldDir,jDir) << "\n";
            
            
            LOG << "SigmaH " << fieldDir << jDir << ":\n";
            LOGMORE << deleg.getSigmaH(fieldDir,jDir) << "\n";
            LOG << "KappaH " << fieldDir << jDir << ":\n";
            LOGMORE << deleg.getKappaH(fieldDir,jDir) << "\n";
            LOG << "AlphaH " << fieldDir << jDir << ":\n";
            LOGMORE << deleg.getAlphaH(fieldDir,jDir) << "\n";
            */
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
            mC_MkE[fieldDir] = calcC_JH(deleg.getKappaH(fieldDir,kDir),
                deleg.getSigmaH(fieldDir,kDir), deleg.getAlphaH(fieldDir,kDir),
                dt);
            mC_PsikE[fieldDir] = calcC_PhiH(deleg.getKappaH(fieldDir,kDir),
                deleg.getSigmaH(fieldDir,kDir), deleg.getAlphaH(fieldDir,kDir),
                dt);
            mC_PsikM[fieldDir] = calcC_PhiJ(deleg.getKappaH(fieldDir,kDir),
                deleg.getSigmaH(fieldDir,kDir), deleg.getAlphaH(fieldDir,kDir),
                dt);
            /*
            LOG << "SigmaE " << fieldDir << kDir << ":\n";
            LOGMORE << deleg.getSigmaE(fieldDir,kDir) << "\n";
            LOG << "KappaE " << fieldDir << kDir << ":\n";
            LOGMORE << deleg.getKappaE(fieldDir,kDir) << "\n";
            LOG << "AlphaE " << fieldDir << kDir << ":\n";
            LOGMORE << deleg.getAlphaE(fieldDir,kDir) << "\n";
            
            LOG << "SigmaH " << fieldDir << kDir << ":\n";
            LOGMORE << deleg.getSigmaH(fieldDir,kDir) << "\n";
            LOG << "KappaH " << fieldDir << kDir << ":\n";
            LOGMORE << deleg.getKappaH(fieldDir,kDir) << "\n";
            LOG << "AlphaH " << fieldDir << kDir << ":\n";
            LOGMORE << deleg.getAlphaH(fieldDir,kDir) << "\n";
            */
        }
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcEPhase(int direction)
{
    if (direction == 0)
        calcEx();
    else if (direction == 1)
        calcEy();
    else
        calcEz();
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcHPhase(int direction)
{
    if (direction == 0)
        calcHx();
    else if (direction == 1)
        calcHy();
    else
        calcHz();
}


template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcEx()
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 0;
    const int STRIDE = 1;
    vector<SimpleAuxPMLRunline> & rls = mRunlines[eFieldNumber(DIRECTION)];
    
    float dj = mDxyz[(DIRECTION+1)%3];  // e.g. dy
    float dk = mDxyz[(DIRECTION+2)%3];  // e.g. dz
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleAuxPMLRunline & rl(rls[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        //float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
        float* Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
        float c_JijH, c_JikH;           // constants for Ex update!  yay!
        float c_Phi_ijH, c_Phi_ikH;     // also constant, hoorah!
        float c_Phi_ijJ, c_Phi_ikJ;     // and still constant!
        
        if (Y_ATTEN)
        {
            Phi_ij = &(mAccumEj[DIRECTION][rl.auxIndex]);
            c_JijH = mC_JjH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Phi_ijH = mC_PhijH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Phi_ijJ = mC_PhijJ[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        }
        
        if (Z_ATTEN)
        {
            Phi_ik = &mAccumEk[DIRECTION][rl.auxIndex];
            c_JikH = mC_JkH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Phi_ikH = mC_PhikH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Phi_ikJ = mC_PhikJ[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        }
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
            float fiOld = *fi;
            float dHj = (*gjHigh - *gjLow)/dk;
            float dHk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi + (mDt/Constants::eps0/m_epsr)*
                ( dHk - dHj );
            
            if (Y_ATTEN)
            {
                Jij = c_JijH*dHk + *Phi_ij;
                *fi = *fi + (mDt/Constants::eps0/m_epsr)*Jij;
                *Phi_ij += (c_Phi_ijH*dHk - c_Phi_ijJ*Jij);
                /*
                LOG << MemoryBuffer::identify(Phi_ij) << "\n";
                LOG << MemoryBuffer::identify(gjLow) << "\n";
                LOG << MemoryBuffer::identify(gjHigh) << "\n";
                LOG << MemoryBuffer::identify(gkLow) << "\n";
                LOG << MemoryBuffer::identify(gkHigh) << "\n";
                */
                Phi_ij++;
            }
            
            if (Z_ATTEN)
            {
                Jik = c_JikH*dHj + *Phi_ik;
                *fi = *fi - (mDt/Constants::eps0/m_epsr)*Jik;
                *Phi_ik += (c_Phi_ikH*dHj - c_Phi_ikJ*Jik);
                Phi_ik++;
            }
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcEy()
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 1;
    const int STRIDE = 1;
    vector<SimpleAuxPMLRunline> & rls = mRunlines[eFieldNumber(DIRECTION)];
    
    float dj = mDxyz[(DIRECTION+1)%3];  // e.g. dy
    float dk = mDxyz[(DIRECTION+2)%3];  // e.g. dz
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleAuxPMLRunline & rl(rls[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        //float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
        float *Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
        float c_JijH, *c_JikH;           // constants for Ex update!  yay!
        float c_Phi_ijH, *c_Phi_ikH;
        float c_Phi_ijJ, *c_Phi_ikJ;
        
        if (Z_ATTEN)
        {
            Phi_ij = &mAccumEj[DIRECTION][rl.auxIndex];
            c_JijH = mC_JjH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Phi_ijH = mC_PhijH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Phi_ijJ = mC_PhijJ[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        }
        
        if (X_ATTEN)
        {
            Phi_ik = &mAccumEk[DIRECTION][rl.auxIndex];
            c_JikH = &mC_JkH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Phi_ikH = &mC_PhikH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Phi_ikJ = &mC_PhikJ[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        }
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
            float fiOld = *fi;
            float dHj = (*gjHigh - *gjLow)/dk;
            float dHk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi + (mDt/Constants::eps0/m_epsr)*
                ( dHk - dHj );
            
            if (Z_ATTEN)
            {
                Jij = c_JijH*dHk + *Phi_ij;
                *fi = *fi + (mDt/Constants::eps0/m_epsr)*Jij;
                *Phi_ij += (c_Phi_ijH*dHk - c_Phi_ijJ*Jij);
                Phi_ij++;
            }
            
            if (X_ATTEN)
            {
                Jik = *c_JikH*dHj + *Phi_ik;
                *fi = *fi - (mDt/Constants::eps0/m_epsr)*Jik;
                *Phi_ik += (*c_Phi_ikH*dHj - *c_Phi_ikJ*Jik);
                Phi_ik++;
                c_JikH++;
                c_Phi_ikH++;
                c_Phi_ikJ++;
            }
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcEz()
{
    // grab the right set of runlines (for Ex, Ey, or Ez)
    const int DIRECTION = 2;
    const int STRIDE = 1;
    vector<SimpleAuxPMLRunline> & rls = mRunlines[eFieldNumber(DIRECTION)];
    
    float dj = mDxyz[(DIRECTION+1)%3];  // e.g. dy
    float dk = mDxyz[(DIRECTION+2)%3];  // e.g. dz
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleAuxPMLRunline & rl(rls[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        //float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
        float* Phi_ij, *Phi_ik;         // e.g. Phi_xy, Phi_xz
        float *c_JijH, c_JikH;
        float *c_Phi_ijH, c_Phi_ikH;
        float *c_Phi_ijJ, c_Phi_ikJ;
        
        if (X_ATTEN)
        {
            Phi_ij = &mAccumEj[DIRECTION][rl.auxIndex];
            c_JijH = &mC_JjH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Phi_ijH = &mC_PhijH[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Phi_ijJ = &mC_PhijJ[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        }
        
        if (Y_ATTEN)
        {
            Phi_ik = &mAccumEk[DIRECTION][rl.auxIndex];
            c_JikH = mC_JkH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Phi_ikH = mC_PhikH[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Phi_ikJ = mC_PhikJ[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        }
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
            float fiOld = *fi;
            float dHj = (*gjHigh - *gjLow)/dk;
            float dHk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi + (mDt/Constants::eps0/m_epsr)*
                ( dHk - dHj );
            
            
            if (X_ATTEN)
            {
                Jij = *c_JijH*dHk + *Phi_ij;
                *fi = *fi + (mDt/Constants::eps0/m_epsr)*Jij;
                *Phi_ij += (*c_Phi_ijH*dHk - *c_Phi_ijJ*Jij);
                Phi_ij++;
                c_JijH++;
                c_Phi_ijH++;
                c_Phi_ijJ++;
            }
            
            if (Y_ATTEN)
            {
                Jik = c_JikH*dHj + *Phi_ik;
                *fi = *fi - (mDt/Constants::eps0/m_epsr)*Jik;
                *Phi_ik += (c_Phi_ikH*dHj - c_Phi_ikJ*Jik);
                Phi_ik++;
            }
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcHx()
{
    // grab the right set of runlines (for Hx, Hy, or Hz)
    const int DIRECTION = 0;
    const int STRIDE = 1;
    vector<SimpleAuxPMLRunline> & rls = mRunlines[hFieldNumber(DIRECTION)];
    
    float dj = mDxyz[(DIRECTION+1)%3];  // e.g. dy
    float dk = mDxyz[(DIRECTION+2)%3];  // e.g. dz
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleAuxPMLRunline & rl(rls[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        //float Mij, Mik;                 // e.g. Jxy, Jxz (temp vars)
        float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
        float c_MijE, c_MikE;           // constants for Ex update!  yay!
        float c_Psi_ijE, c_Psi_ikE;     // also constant, hoorah!
        float c_Psi_ijM, c_Psi_ikM;     // and still constant!
        
        if (Y_ATTEN)
        {
            Psi_ij = &mAccumHj[DIRECTION][rl.auxIndex];
            c_MijE = mC_MjE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Psi_ijE = mC_PsijE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Psi_ijM = mC_PsijM[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        }
        
        if (Z_ATTEN)
        {
            Psi_ik = &mAccumHk[DIRECTION][rl.auxIndex];
            c_MikE = mC_MkE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Psi_ikE = mC_PsikE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Psi_ikM = mC_PsikM[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        }
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Mij, Mik;                 // e.g. Mxy, Mxz (temp vars)
            float fiOld = *fi;
            float dEj = (*gjHigh - *gjLow)/dk;
            float dEk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi - (mDt/Constants::mu0/m_mur)*
                ( dEk - dEj );
            
            if (Y_ATTEN)
            {
                Mij = c_MijE*dEk + *Psi_ij;
                *fi = *fi - (mDt/Constants::mu0/m_mur)*Mij;
                *Psi_ij += (c_Psi_ijE*dEk - c_Psi_ijM*Mij);
                Psi_ij++;
            }
            
            if (Z_ATTEN)
            {
                Mik = c_MikE*dEj + *Psi_ik;
                *fi = *fi + (mDt/Constants::mu0/m_mur)*Mik;
                *Psi_ik += (c_Psi_ikE*dEj - c_Psi_ikM*Mik);
                Psi_ik++;
            }
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcHy()
{
    // grab the right set of runlines (for Hx, Hy, or Hz)
    const int DIRECTION = 1;
    const int STRIDE = 1;
    vector<SimpleAuxPMLRunline> & rls = mRunlines[hFieldNumber(DIRECTION)];
    
    float dj = mDxyz[(DIRECTION+1)%3];  // e.g. dy
    float dk = mDxyz[(DIRECTION+2)%3];  // e.g. dz
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleAuxPMLRunline & rl(rls[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        //float Mij, Mik;                 // e.g. Jxy, Jxz (temp vars)
        float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
        float c_MijE, *c_MikE;           // constants for Ex update!  yay!
        float c_Psi_ijE, *c_Psi_ikE;     // also constant, hoorah!
        float c_Psi_ijM, *c_Psi_ikM;     // and still constant!
        
        if (Z_ATTEN)
        {
            Psi_ij = &mAccumHj[DIRECTION][rl.auxIndex];
            c_MijE = mC_MjE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Psi_ijE = mC_PsijE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Psi_ijM = mC_PsijM[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        }
        
        if (X_ATTEN)
        {
            Psi_ik = &mAccumHk[DIRECTION][rl.auxIndex];
            c_MikE = &mC_MkE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Psi_ikE = &mC_PsikE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Psi_ikM = &mC_PsikM[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        }
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Mij, Mik;                 // e.g. Mxy, Mxz (temp vars)
            float fiOld = *fi;
            float dEj = (*gjHigh - *gjLow)/dk;
            float dEk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi - (mDt/Constants::mu0/m_mur)*
                ( dEk - dEj );
            
            if (Z_ATTEN)
            {
                Mij = c_MijE*dEk + *Psi_ij;
                *fi = *fi - (mDt/Constants::mu0/m_mur)*Mij;
                *Psi_ij += (c_Psi_ijE*dEk - c_Psi_ijM*Mij);
                Psi_ij++;
            }
            
            if (X_ATTEN)
            {
                Mik = *c_MikE*dEj + *Psi_ik;
                *fi = *fi + (mDt/Constants::mu0/m_mur)*Mik;
                *Psi_ik += (*c_Psi_ikE*dEj - *c_Psi_ikM*Mik);
                c_MikE++;
                c_Psi_ikE++;
                c_Psi_ikM++;
                Psi_ik++;
            }
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcHz()
{
    // grab the right set of runlines (for Hx, Hy, or Hz)
    const int DIRECTION = 2;
    const int STRIDE = 1;
    vector<SimpleAuxPMLRunline> & rls = mRunlines[hFieldNumber(DIRECTION)];
    
    float dj = mDxyz[(DIRECTION+1)%3];  // e.g. dy
    float dk = mDxyz[(DIRECTION+2)%3];  // e.g. dz
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleAuxPMLRunline & rl(rls[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        
        //float Mij, Mik;                 // e.g. Jxy, Jxz (temp vars)
        float* Psi_ij, *Psi_ik;         // e.g. Phi_xy, Phi_xz
        float *c_MijE, c_MikE;           // constants for Ex update!  yay!
        float *c_Psi_ijE, c_Psi_ikE;     // also constant, hoorah!
        float *c_Psi_ijM, c_Psi_ikM;     // and still constant!
        
        if (X_ATTEN)
        {
            Psi_ij = &mAccumHj[DIRECTION][rl.auxIndex];
            c_MijE = &mC_MjE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Psi_ijE = &mC_PsijE[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
            c_Psi_ijM = &mC_PsijM[DIRECTION][rl.pmlIndex[(DIRECTION+1)%3]];
        }
        
        if (Y_ATTEN)
        {
            Psi_ik = &mAccumHk[DIRECTION][rl.auxIndex];
            c_MikE = mC_MkE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Psi_ikE = mC_PsikE[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
            c_Psi_ikM = mC_PsikM[DIRECTION][rl.pmlIndex[(DIRECTION+2)%3]];
        }
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Mij, Mik;                 // e.g. Mxy, Mxz (temp vars)
            float fiOld = *fi;
            float dEj = (*gjHigh - *gjLow)/dk;
            float dEk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi - (mDt/Constants::mu0/m_mur)*
                ( dEk - dEj );
            
            if (X_ATTEN)
            {
                Mij = *c_MijE*dEk + *Psi_ij;
                *fi = *fi - (mDt/Constants::mu0/m_mur)*Mij;
                *Psi_ij += (*c_Psi_ijE*dEk - *c_Psi_ijM*Mij);
                c_MijE++;
                c_Psi_ijE++;
                c_Psi_ijM++;
                Psi_ij++;
            }
            
            if (Y_ATTEN)
            {
                Mik = c_MikE*dEj + *Psi_ik;
                *fi = *fi + (mDt/Constants::mu0/m_mur)*Mik;
                *Psi_ik += (c_Psi_ikE*dEj - c_Psi_ikM*Mik);
                Psi_ik++;
            }
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}


#endif