/*
 *  StaticDielectric.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "StaticDielectric.h"
#include "CalculationPartition.h"
#include "Paint.h"
#include "Log.h"
#include "PhysicalConstants.h"
#include "YeeUtilities.h"

#include <sstream>

using namespace std;
using namespace YeeUtilities;

SetupStaticDielectric::
SetupStaticDielectric() :
	SimpleBulkSetupMaterial()
{
    
}

MaterialPtr SetupStaticDielectric::
makeCalcMaterial(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    return MaterialPtr(new StaticDielectric(*this,
        *(mStartPaint->getBulkMaterial()),
        cp.getDxyz(),
        cp.getDt()
        ));
}

StaticDielectric::
StaticDielectric(const SetupStaticDielectric & deleg,
    const MaterialDescription & descrip,
    Vector3f dxyz, float dt) :
    Material(),
    mDxyz(dxyz),
    mDt(dt),
    m_epsr(1.0),
    m_mur(1.0)
{
    if (descrip.getParams().count("epsr"))
        istringstream(descrip.getParams()["epsr"]) >> m_epsr;
    if (descrip.getParams().count("mur"))
        istringstream(descrip.getParams()["mur"]) >> m_mur;
    
    int dir;
    for (dir = 0; dir < 3; dir++)
    {
        const std::vector<SBMRunlinePtr> & setupRunlines =
            deleg.getRunlinesE(dir);
        
        mRunlinesE[dir].resize(setupRunlines.size());
        
        LOG << "Printing as we create E " << dir << " runlines\n";
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
        {
            mRunlinesE[dir][nn] = SimpleRunline(*setupRunlines[nn]);
            LOGMORE << mRunlinesE[dir][nn] << "\n";
        }
    }
    
    for (dir = 0; dir < 3; dir++)
    {
        const std::vector<SBMRunlinePtr> & setupRunlines =
            deleg.getRunlinesH(dir);
        
        mRunlinesH[dir].resize(setupRunlines.size());
        
        LOG << "Printing as we create H " << dir << " runlines\n";
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
        {
            mRunlinesH[dir][nn] = SimpleRunline(*setupRunlines[nn]);
            LOGMORE << mRunlinesH[dir][nn] << "\n";
        }
    }
    LOG << "Created all runlines.\n";
}

void StaticDielectric::
calcEPhase(int direction)
{
    //LOG << "direction " << direction << " number "
    //    << eFieldNumber(direction) << "\n";
    // grab the right set of runlines (for Ex, Ey, or Ez)
    vector<SimpleRunline> & rls = mRunlinesE[direction];
    const int STRIDE = 1;
    
    float dj = mDxyz[(direction+1)%3];  // e.g. dy
    float dk = mDxyz[(direction+2)%3];  // e.g. dz
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleRunline & rl(rls[nRL]);
        float* fi(rl.fi);               // e.g. Ex
        const float* gjLow(rl.gj[0]);   // e.g. Hy(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Hy(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Hz(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Hz(y+1/2)
        const int len(rl.length);
        
        //LOGMORE << rl << "\n";
        for (int mm = 0; mm < len; mm++)
        {
            //float fiOld = *fi;
            *fi = *fi + (mDt/Constants::eps0/m_epsr)*
                ( (*gkHigh-*gkLow)/dj - (*gjHigh - *gjLow)/dk );
            /*
            if (mm == 0)
            {
                LOG << "**" << MemoryBuffer::identify(fi) << " is " << *fi << "\n";
                LOG << MemoryBuffer::identify(gjLow) << " is " << *gjLow << "\n";
                LOG << MemoryBuffer::identify(gjHigh) << " is " << *gjHigh << "\n";
                LOG << MemoryBuffer::identify(gkLow) << " is " << *gkLow << " \n";
                LOG << MemoryBuffer::identify(gkHigh) << " is " << *gkHigh << "\n";
            }
            */
            
            /*
            //if (*fi != fiOld)
            //    LOG << mm << " diff " << *fi - fiOld << "\n";
            if (*fi != 0)
                LOG << "**" << MemoryBuffer::identify(fi) << " is " << *fi << "\n";
            if (*gjLow != 0)
                LOG << MemoryBuffer::identify(gjLow) << " is " << *gjLow << "\n";
            if (*gjHigh != 0)
                LOG << MemoryBuffer::identify(gjHigh) << " is " << *gjHigh << "\n";
            if (*gkLow != 0)
                LOG << MemoryBuffer::identify(gkLow) << " is " << *gkLow << " \n";
            if (*gkHigh != 0)
                LOG << MemoryBuffer::identify(gkHigh) << " is " << *gkHigh << "\n";
            */
            
            //*fi = *fi * 0.9 + 0.025*(*gjLow + *gjHigh + *gkLow + *gkHigh);
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}

void StaticDielectric::
calcHPhase(int direction)
{
    //LOG << "direction " << direction << " number "
    //    << hFieldNumber(direction) << "\n";
    // grab the right set of runlines (for Hx, Hy, or Hz)
    vector<SimpleRunline> & rls = mRunlinesH[direction];
    const int STRIDE = 1;
    
    float dj = mDxyz[(direction+1)%3];  // e.g. dy
    float dk = mDxyz[(direction+2)%3];  // e.g. dz
    
    for (int nRL = 0; nRL < rls.size(); nRL++)
    {
        SimpleRunline & rl(rls.at(nRL));
        float* fi(rl.fi);               // e.g. Hx
        const float* gjLow(rl.gj[0]);   // e.g. Ey(z-1/2)
        const float* gjHigh(rl.gj[1]);  // e.g. Ey(z+1/2)
        const float* gkLow(rl.gk[0]);   // e.g. Ez(y-1/2)
        const float* gkHigh(rl.gk[1]);  // e.g. Ez(y+1/2)
        const int len(rl.length);
        
        for (int mm = 0; mm < len; mm++)
        {
            //float fiOld = *fi;
            *fi = *fi - (mDt/Constants::mu0/m_mur)*
                ( (*gkHigh-*gkLow)/dj - (*gjHigh - *gjLow)/dk );
            //LOG << "neighbors " << *gkLow << " " << *gkHigh << "\n";
            //*fi = *fi * 0.9 + 0.025*(*gjLow + *gjHigh + *gkLow + *gkHigh);
            /*
            if (mm == 0)
            {
                LOG << "**" << MemoryBuffer::identify(fi) << " is " << *fi << "\n";
                LOG << MemoryBuffer::identify(gjLow) << " is " << *gjLow << "\n";
                LOG << MemoryBuffer::identify(gjHigh) << " is " << *gjHigh << "\n";
                LOG << MemoryBuffer::identify(gkLow) << " is " << *gkLow << " \n";
                LOG << MemoryBuffer::identify(gkHigh) << " is " << *gkHigh << "\n";
            }
            */
            
            /*
            if (*fi != fiOld)
                LOG << mm << " diff " << *fi - fiOld << "\n";
            
            if (*gjLow != 0)
                LOG << MemoryBuffer::identify(gjLow) << "\n";
            if (*gjHigh != 0)
                LOG << MemoryBuffer::identify(gjHigh) << "\n";
            if (*gkLow != 0)
                LOG << MemoryBuffer::identify(gkLow) << "\n";
            if (*gkHigh != 0)
                LOG << MemoryBuffer::identify(gkHigh) << "\n";
            */
            fi += STRIDE;
            gkLow += STRIDE;
            gkHigh += STRIDE;
            gjLow += STRIDE;
            gjHigh += STRIDE;
        }
    }
}



