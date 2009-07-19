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
    /*
    int dir;
    for (dir = 0; dir < 6; dir++)
    {
        const std::vector<SBMRunlinePtr> & setupRunlines =
            deleg.getRunlinesE(dir);
        
        mRunlinesE[dir].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
            mRunlinesE[dir][nn] = SimpleRunline(*setupRunlines[nn]);
    }

    for (dir = 0; dir < 6; dir++)
    {
        const std::vector<SBMRunlinePtr> & setupRunlines =
            deleg.getRunlinesH(dir);
        
        mRunlinesH[dir].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
            mRunlinesH[dir][nn] = SimpleRunline(*setupRunlines[nn]);
    }
    */
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
