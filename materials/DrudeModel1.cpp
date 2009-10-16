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
    std::vector<long> numCellsE, std::vector<long> numCellsH,
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
    m_cj2 = 2*m_tauc*dt*m_omegap*m_omegap*Constants::eps0/(dt+2*m_tauc);
    m_ce = dt/m_epsrinf/Constants::eps0;
    m_ch = dt/m_mur/Constants::mu0;
    
    // This was the update constant for Trogdor 4, with the idiosyncratic
    // Drude model.  Note the presence of the m_epsrinf term.
    //m_cj2 = 2*m_tauc*dt*m_omegap*m_omegap*m_epsrinf*Constants::eps0
    //    /(dt+2*m_tauc);
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











