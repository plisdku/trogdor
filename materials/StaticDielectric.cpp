/*
 *  StaticDielectric.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "StaticDielectric.h"
#include "SimulationDescription.h"
#include "CalculationPartition.h"
#include "Paint.h"
#include "Log.h"
#include "PhysicalConstants.h"
#include "YeeUtilities.h"
#include "SetupModularUpdateEquation.h"

#include <sstream>

using namespace std;
using namespace YeeUtilities;

StaticDielectric::
StaticDielectric(
    const MaterialDescription & descrip,
    std::vector<int> numCellsE, std::vector<int> numCellsH,
    Vector3f dxyz, float dt) :
    Material(),
    mDxyz(dxyz),
    mDt(dt),
    m_epsr(1.0),
    m_mur(1.0)
{
    if (descrip.params().count("epsr"))
        istringstream(descrip.params()["epsr"]) >> m_epsr;
    if (descrip.params().count("mur"))
        istringstream(descrip.params()["mur"]) >> m_mur;
    
    m_ce1 = dt/m_epsr/Constants::eps0;
    m_ch1 = dt/m_mur/Constants::mu0;
    
//    LOG << "Init with ce1 = " << m_ce1 << " and ch1 = " << m_ch1 << "\n";
}

string StaticDielectric::
modelName() const
{
    return "StaticDielectric";
}

void StaticDielectric::
allocateAuxBuffers()
{
}



