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
/*
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
    mDxyz(dxyz),
    mDt(dt),
    m_epsr(1.0),
    m_mur(1.0)
{
    int fieldDir, jDir, kDir;
    
    mPML = new StandardPML<X_ATTEN, Y_ATTEN, Z_ATTEN>(
        deleg.getPML(), deleg.getParentPaint(), dxyz, dt);
    
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
    
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
allocateAuxBuffers()
{
    mPML->allocateAuxBuffers();
}


template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcEPhase(int direction)
{
    if (direction == 0)
    {
        calcEx();
        mPML->calcEx(mRunlinesE[0]);
    }
    else if (direction == 1)
    {
        calcEy();
        mPML->calcEy(mRunlinesE[1]);
    }
    else
    {
        calcEz();
        mPML->calcEz(mRunlinesE[2]);
    }
}

template <bool X_ATTEN, bool Y_ATTEN, bool Z_ATTEN>
void StaticDielectricPML<X_ATTEN, Y_ATTEN, Z_ATTEN>::
calcHPhase(int direction)
{
    if (direction == 0)
    {
        calcHx();
        mPML->calcHx(mRunlinesH[0]);
    }
    else if (direction == 1)
    {
        calcHy();
        mPML->calcHy(mRunlinesH[1]);
    }
    else
    {
        calcHz();
        mPML->calcHz(mRunlinesH[2]);
    }
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
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
            float fiOld = *fi;
            float dHj = (*gjHigh - *gjLow)/dk;
            float dHk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi + (mDt/Constants::eps0/m_epsr)*
                ( dHk - dHj );
            
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
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
            float fiOld = *fi;
            float dHj = (*gjHigh - *gjLow)/dk;
            float dHk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi + (mDt/Constants::eps0/m_epsr)*
                ( dHk - dHj );
            
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
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Jij, Jik;                 // e.g. Jxy, Jxz (temp vars)
            float fiOld = *fi;
            float dHj = (*gjHigh - *gjLow)/dk;
            float dHk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi + (mDt/Constants::eps0/m_epsr)*
                ( dHk - dHj );
            
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
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Mij, Mik;                 // e.g. Mxy, Mxz (temp vars)
            float fiOld = *fi;
            float dEj = (*gjHigh - *gjLow)/dk;
            float dEk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi - (mDt/Constants::mu0/m_mur)*
                ( dEk - dEj );
            
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
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Mij, Mik;                 // e.g. Mxy, Mxz (temp vars)
            float fiOld = *fi;
            float dEj = (*gjHigh - *gjLow)/dk;
            float dEk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi - (mDt/Constants::mu0/m_mur)*
                ( dEk - dEj );
            
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
        
        const int len(rl.length);
        for (int mm = 0; mm < len; mm++)
        {
            float Mij, Mik;                 // e.g. Mxy, Mxz (temp vars)
            float fiOld = *fi;
            float dEj = (*gjHigh - *gjLow)/dk;
            float dEk = (*gkHigh - *gkLow)/dj;
            
            *fi = *fi - (mDt/Constants::mu0/m_mur)*
                ( dEk - dEj );
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}
*/

#endif