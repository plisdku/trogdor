/*
 *  StaticLossyDielectric.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "StaticLossyDielectric.h"
#include "CalculationPartition.h"
#include "Paint.h"
#include "Log.h"

#include <sstream>

using namespace std;

StaticLossyDielectric::
StaticLossyDielectric(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt) :
    m_epsr(1.0),
    m_mur(1.0),
    m_sigma(0.0)
{
    if (descrip.getParams().count("epsr"))
        istringstream(descrip.getParams()["epsr"]) >> m_epsr;
    if (descrip.getParams().count("mur"))
        istringstream(descrip.getParams()["mur"]) >> m_mur;
    if (descrip.getParams().count("sigma"))
        istringstream(descrip.getParams()["sigma"]) >> m_sigma;
    
    m_ce1 = (2*m_epsr*Constants::eps0 - m_sigma*dt) /
        (2*m_epsr*Constants::eps0 + m_sigma*dt);
    m_ce2 = 2*dt/(2*m_epsr*Constants::eps0 + m_sigma*dt);
    m_ch1 = dt/m_mur/Constants::mu0;
}

string StaticLossyDielectric::
getModelName() const
{
    return string("StaticLossyDielectric");
}

void StaticLossyDielectric::
writeJ(int direction, std::ostream & binaryStream, long startingIndex,
    const float* startingField, long length) const
{
    for (int rr = 0; rr < length; rr++)
    {
        float value = m_sigma*startingField[rr];
        binaryStream.write((char*) &value,
            (std::streamsize)(sizeof(float)) );
    }
}

