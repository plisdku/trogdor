/*
 *  DrudeModel1.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "DrudeModel1.h"
#include "SimulationDescription.h"
#include "CalculationPartition.h"
#include "Log.h"
#include "PhysicalConstants.h"

#include <sstream>
#include "YeeUtilities.h"

using namespace YeeUtilities;
using namespace std;

DrudeModel1::
DrudeModel1(
    const MaterialDescription & descrip,
    std::vector<int> numCellsE, std::vector<int> numCellsH,
    Vector3f dxyz, float dt) :
    Material(),
    mDxyz(dxyz),
    mDt(dt),
    m_epsrinf(1.0),
    m_mur(1.0),
    m_omegap(0.0),
    m_tauc(0.0)
{
    if (descrip.params().count("epsinf"))
        istringstream(descrip.params()["epsinf"]) >> m_epsrinf;
    if (descrip.params().count("mur"))
        istringstream(descrip.params()["mur"]) >> m_mur;
    if (descrip.params().count("omegap"))
        istringstream(descrip.params()["omegap"]) >> m_omegap;
    if (descrip.params().count("tauc"))
        istringstream(descrip.params()["tauc"]) >> m_tauc;
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        mCurrentBuffers[xyz] = MemoryBufferPtr(new MemoryBuffer(
            string("DrudeModel1 J")+char('x'+xyz), numCellsE[xyz]));
    }
    
    // update constants
    m_cj1 = (2*m_tauc - dt)/(2*m_tauc + dt);
    m_cj2 = 2*m_tauc*dt*m_omegap*m_omegap*m_epsrinf*Constants::eps0
        /(dt+2*m_tauc);
    m_ce = dt/m_epsrinf/Constants::eps0;
    m_ch = dt/m_mur/Constants::mu0;
}

string DrudeModel1::
modelName() const
{
    return string("DrudeModel1");
}


void DrudeModel1::
writeJ(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    assert(startingIndex >= 0);
    assert(startingIndex + length <= mCurrents[direction].size());
    //LOG << "E " << *startingField << " J " << mCurrents[direction][startingIndex] << "\n";
    //LOGMORE << "index " << startingIndex << " direction " << direction << "\n";
    //if (mCurrents[direction][startingIndex] != 0.0)
    //    int flab = 5;
    binaryStream.write((char*)&mCurrents[direction][startingIndex],
        (std::streamsize)(length*sizeof(float)) );
}

void DrudeModel1::
allocateAuxBuffers()
{
    //LOG << "Allocating currents.\n";
    for (int xyz = 0; xyz < 3; xyz++)
    {
        mCurrents[xyz].resize(mCurrentBuffers[xyz]->length());
        mCurrentBuffers[xyz]->setHeadPointer(&(mCurrents[xyz][0]));
    }
}
/*
void DrudeModel1::
calcEPhase(int direction)
{
    //LOG << "direction " << direction << " number "
    //    << eFieldNumber(direction) << "\n";
    // grab the right set of runlines (for Ex, Ey, or Ez)
    vector<SimpleAuxRunline> & rls = runlinesE(direction);
    const int STRIDE = 1;
    
    const float dj = mDxyz[(direction+1)%3];  // e.g. dy
    const float dk = mDxyz[(direction+2)%3];  // e.g. dz
    const float dt = mDt;
    const float cj1 = (2*m_tauc - dt)/(2*m_tauc + dt);
    const float cj2 = 2*m_tauc*dt*m_omegap*m_omegap*m_epsrinf*Constants::eps0
        /(dt+2*m_tauc);
    const float ce1 = dt/m_epsrinf/Constants::eps0;
    const float ce2 = ce1;
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleAuxRunline & rl(rls[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        float* Ji = &(mCurrents[direction][rl.auxIndex]);
        const int len(rl.length);
        
        //LOGMORE << rl << "\n";
        for (int mm = 0; mm < len; mm++)
        {
            *fi = *fi + ce1*(*gkHigh-*gkLow)/dj - ce1*(*gjHigh-*gjLow)/dk
                - *Ji * ce2;
            
            *Ji = *Ji * cj1 + *fi * cj2;
            
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
            Ji += 1;
        }
    }
}

void DrudeModel1::
calcHPhase(int direction)
{
    //LOG << "direction " << direction << " number "
    //    << hFieldNumber(direction) << "\n";
    // grab the right set of runlines (for Hx, Hy, or Hz)
    vector<SimpleAuxRunline> & rls = runlinesH(direction);
    const int STRIDE = 1;
    
    const float dj = mDxyz[(direction+1)%3];  // e.g. dy
    const float dk = mDxyz[(direction+2)%3];  // e.g. dz
    const float dt = mDt;
    const float ch1 = dt/m_mur/Constants::mu0;
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleAuxRunline & rl(rls.at(nRL));
        float* fi(rl.fi);               // e.g. Hx
        const float* gjLow(rl.gj[0]);   // e.g. Ey(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Ey(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Ez(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Ez(y+1/2)
        const int len(rl.length);
        
        for (int mm = 0; mm < len; mm++)
        {
            float fiOld = *fi;
            *fi = *fi - (mDt/Constants::mu0/m_mur)*
                ( (*gkHigh-*gkLow)/dj - (*gjHigh - *gjLow)/dk );
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

*/












