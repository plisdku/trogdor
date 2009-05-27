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

StaticLossyDielectricDelegate::
StaticLossyDielectricDelegate() :
	SimpleBulkMaterialDelegate()
{
    
}


MaterialPtr StaticLossyDielectricDelegate::
makeCalcMaterial(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    return MaterialPtr(new StaticLossyDielectric(*this,
        *(mStartPaint->getBulkMaterial()),
        cp.getDxyz(),
        cp.getDt()
        ));
}


StaticLossyDielectric::
StaticLossyDielectric(const StaticLossyDielectricDelegate & deleg,
    const MaterialDescription & descrip,
    Vector3f dxyz, float dt) :
    Material(),
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
    
    for (int field = 0; field < 6; field++)
    {
        const std::vector<SBMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
            mRunlines[field][nn] = SimpleRunline(*setupRunlines[nn]);
    }
    //LOG << "Created all runlines.\n";
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
