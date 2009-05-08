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

#include <sstream>

using namespace std;

StaticDielectricDelegate::
StaticDielectricDelegate() :
	SimpleBulkMaterialDelegate()
{
    
}


MaterialPtr StaticDielectricDelegate::
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
StaticDielectric(const StaticDielectricDelegate & deleg,
    const MaterialDescription & descrip,
    Vector3f dxyz, float dt) :
    Material(),
    m_epsr(1.0),
    m_mur(1.0)
{
    if (descrip.getParams().count("epsr"))
        istringstream(descrip.getParams()["epsr"]) >> m_epsr;
    if (descrip.getParams().count("mur"))
        istringstream(descrip.getParams()["mur"]) >> m_mur;
    
    for (int field = 0; field < 6; field++)
    {
        const std::vector<SBMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
            mRunlines[field][nn] = SimpleRunline(*setupRunlines[nn]);
    }
    LOG << "Created all runlines.\n";
}

void StaticDielectric::
calcEPhase(int direction)
{
    //LOG << "Calculating E.\n";
}

void StaticDielectric::
calcHPhase(int direction)
{
    //LOG << "Calculating H.\n";
}


