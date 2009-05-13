/*
 *  StaticDielectricPML.hh
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifdef _STATICDIELECTRICPML_

#include "StaticDielectricPML.h"
#include "CalculationPartition.h"
#include "Paint.h"
#include <sstream>

using namespace std;

StaticDielectricPMLDelegate::
StaticDielectricPMLDelegate() :
    SimpleBulkPMLMaterialDelegate()
{
}

MaterialPtr StaticDielectricPMLDelegate::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    return MaterialPtr(new StaticDielectricPML(*this,
        *mStartPaint->getBulkMaterial(),
        cp.getDxyz(),
        cp.getDt()
        ));
}


StaticDielectricPML::
StaticDielectricPML(const StaticDielectricPMLDelegate & deleg,
    const MaterialDescription & descrip, Vector3f dxyz, float dt) :
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
        const std::vector<SBPMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
            mRunlines[field][nn] = SimplePMLRunline(*setupRunlines[nn]);
    }
    LOG << "Created all runlines.\n";
}

void StaticDielectricPML::
calcEPhase(int direction)
{
    //LOG << "Calculating E.\n";
}

void StaticDielectricPML::
calcHPhase(int direction)
{
    //LOG << "Calculating H.\n";
}


#endif