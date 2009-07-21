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
/*
StaticLossyDielectric::
StaticLossyDielectric(
        const MaterialDescription & descrip,
        std::vector<int> numCellsE, std::vector<int> numCellsH,
        Vector3f dxyz, float dt) :
    SimpleMaterial<SimpleRunline>(),
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
        
    //LOG << "Created all runlines.\n";
}

string StaticLossyDielectric::
getModelName() const
{
    return string("StaticLossyDielectric");
}

void StaticLossyDielectric::
calcEPhase(int direction)
{
    //LOG << "Calculating E.\n";
}

void StaticLossyDielectric::
calcHPhase(int direction)
{
    //LOG << "Calculating H.\n";
}
*/
