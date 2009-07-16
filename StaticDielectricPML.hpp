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

SetupStaticDielectricPML::
SetupStaticDielectricPML(
    const Map<Vector3i, Map<string, string> > & pmlParams) :
    SimpleBulkPMLSetupMaterial(),
    mPML(pmlParams)
{
}

void SetupStaticDielectricPML::
setNumCellsE(int fieldDir, int numCells)
{
    mPML.setNumCellsE(fieldDir, numCells, getParentPaint());
}

void SetupStaticDielectricPML::
setNumCellsH(int fieldDir, int numCells)
{
    mPML.setNumCellsH(fieldDir, numCells, getParentPaint());
}

void SetupStaticDielectricPML::
setPMLHalfCells(int faceNum, Rect3i halfCellsOnSide,
    const GridDescription & gridDesc)
{
    mPML.setPMLHalfCells(faceNum, halfCellsOnSide, gridDesc, getParentPaint());
}

MaterialPtr SetupStaticDielectricPML::
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
StaticDielectricPML(const SetupStaticDielectricPML & deleg, Vector3f dxyz,
    float dt) :
    Material(),
    mPML(deleg.getParentPaint(), deleg.getPML()),
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
    int dir;
    for (dir = 0; dir < 3; dir++)
    {
        //LOG << "Printing as we create runlines in field " << field << "\n";
        const std::vector<SBPMRunlinePtr> & setupRunlines =
            deleg.getRunlinesE(dir);
        
        mRunlinesE[dir].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
        {
            mRunlinesE[dir][nn] = SimpleAuxPMLRunline(*setupRunlines[nn]);
            
//            LOGMORE << MemoryBuffer::identify(mRunlines[field][nn].fi)
//                << " " << MemoryBuffer::identify(mRunlines[field][nn].gj[0]) <<
//                " " << MemoryBuffer::identify(mRunlines[field][nn].gk[0]) <<
//                "\n";
            
            //LOGMORE << mRunlines[field][nn] << "\n";
        }
    }
    for (dir = 0; dir < 3; dir++)
    {
        //LOG << "Printing as we create runlines in field " << field << "\n";
        const std::vector<SBPMRunlinePtr> & setupRunlines =
            deleg.getRunlinesH(dir);
        
        mRunlinesH[dir].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
        {
            mRunlinesH[dir][nn] = SimpleAuxPMLRunline(*setupRunlines[nn]);
            
//            LOGMORE << MemoryBuffer::identify(mRunlines[field][nn].fi)
//                << " " << MemoryBuffer::identify(mRunlines[field][nn].gj[0]) <<
//                " " << MemoryBuffer::identify(mRunlines[field][nn].gk[0]) <<
//                "\n";
            
            //LOGMORE << mRunlines[field][nn] << "\n";
        }
    }
    /*
    for (int field = 0; field < 6; field++)
    {
        //LOG << "Printing as we create runlines in field " << field << "\n";
        const std::vector<SBPMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
        {
            mRunlines[field][nn] = SimpleAuxPMLRunline(*setupRunlines[nn]);
            
//            LOGMORE << MemoryBuffer::identify(mRunlines[field][nn].fi)
//                << " " << MemoryBuffer::identify(mRunlines[field][nn].gj[0]) <<
//                " " << MemoryBuffer::identify(mRunlines[field][nn].gk[0]) <<
//                "\n";
            
            //LOGMORE << mRunlines[field][nn] << "\n";
        }
    }
    */
    
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
allocateAuxBuffers()
{
    // Allocate auxiliary variables
    MemoryBufferPtr p;
    for (int fieldDir = 0; fieldDir < 3; fieldDir++)
    {
        int jDir = (fieldDir+1)%3;
        int kDir = (fieldDir+2)%3;
        
        if (mPMLDirection[jDir] != 0)
        {
            p = mBufAccumEj[fieldDir];
            mAccumEj[fieldDir].resize(p->getLength());
            p->setHeadPointer(&(mAccumEj[fieldDir][0]));
            
            p = mBufAccumHj[fieldDir];
            mAccumHj[fieldDir].resize(p->getLength());
            p->setHeadPointer(&(mAccumHj[fieldDir][0]));
        }
        
        if (mPMLDirection[kDir] != 0)
        {
            p = mBufAccumEk[fieldDir];
            mAccumEk[fieldDir].resize(p->getLength());
            p->setHeadPointer(&(mAccumEk[fieldDir][0]));
            
            p = mBufAccumHk[fieldDir];
            mAccumHk[fieldDir].resize(p->getLength());
            p->setHeadPointer(&(mAccumHk[fieldDir][0]));
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
    vector<SimpleAuxPMLRunline> & rls = mRunlinesE[DIRECTION];
    
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
    vector<SimpleAuxPMLRunline> & rls = mRunlinesE[DIRECTION];
    
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
    vector<SimpleAuxPMLRunline> & rls = mRunlinesE[DIRECTION];
    
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
    vector<SimpleAuxPMLRunline> & rls = mRunlinesH[DIRECTION];
    
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
    vector<SimpleAuxPMLRunline> & rls = mRunlinesH[DIRECTION];
    
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
    vector<SimpleAuxPMLRunline> & rls = mRunlinesH[DIRECTION];
    
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