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
#include "SimpleMaterialTemplates.h"

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
    if (descrip.getParams().count("epsr"))
        istringstream(descrip.getParams()["epsr"]) >> m_epsr;
    if (descrip.getParams().count("mur"))
        istringstream(descrip.getParams()["mur"]) >> m_mur;
    
    m_ce1 = dt/m_epsr/Constants::eps0;
    m_ch1 = dt/m_mur/Constants::mu0;
    
//    LOG << "Init with ce1 = " << m_ce1 << " and ch1 = " << m_ch1 << "\n";
}

string StaticDielectric::
getModelName() const
{
    return "StaticDielectric";
}


void StaticDielectric::
writeJ(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void StaticDielectric::
writeK(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    float zero = 0.0f;
    for (long nn = 0; nn < length; nn++)
        binaryStream.write((char*)&zero, (std::streamsize)sizeof(float));
}

void StaticDielectric::
allocateAuxBuffers()
{
}



