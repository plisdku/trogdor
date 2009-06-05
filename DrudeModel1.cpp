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
#include "Paint.h"
#include "Log.h"

#include <sstream>
#include "YeeUtilities.h"
using namespace YeeUtilities;
using namespace std;

DrudeModel1Delegate::
DrudeModel1Delegate(const MaterialDescPtr & material) :
	SimpleBulkMaterialDelegate(),
    mDesc(material)
{
}


void DrudeModel1Delegate::
setNumCellsE(int fieldDir, int number)
{
    const int STRIDE = 1;
    
    ostringstream bufName;
    bufName << mDesc->getName() << " J" << fieldDir;
    mCurrents[fieldDir] = MemoryBufferPtr(new MemoryBuffer(
        bufName.str(), number, STRIDE));
}


/*
void DrudeModel1Delegate::
setNumCells(int octant, int number)
{
	const int STRIDE = 1;
	int eIndex = octantENumber(octant);
	if (eIndex == -1)  // if this isn't an E-field octant
		return;
        
    LOG << "Setting number of cells.\n";
    
    ostringstream bufName;
    bufName << mDesc->getName() << " J" << eIndex;
	mCurrents[eIndex] = MemoryBufferPtr(new MemoryBuffer(
        bufName.str(), number, STRIDE));
}
*/

MaterialPtr DrudeModel1Delegate::
makeCalcMaterial(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    return MaterialPtr(new DrudeModel1(*this,
        *(mStartPaint->getBulkMaterial()),
        cp.getDxyz(),
        cp.getDt()
        ));
}



DrudeModel1::
DrudeModel1(const DrudeModel1Delegate & deleg,
    const MaterialDescription & descrip,
    Vector3f dxyz, float dt) :
    Material(),
    m_epsrinf(1.0),
    m_mur(1.0),
    m_omegap(0.0),
    m_tauc(0.0)
{
    if (descrip.getParams().count("epsinf"))
        istringstream(descrip.getParams()["epsinf"]) >> m_epsrinf;
    if (descrip.getParams().count("mur"))
        istringstream(descrip.getParams()["mur"]) >> m_mur;
    if (descrip.getParams().count("omegap"))
        istringstream(descrip.getParams()["omegap"]) >> m_omegap;
    if (descrip.getParams().count("tauc"))
        istringstream(descrip.getParams()["tauc"]) >> m_tauc;
    
    for (int field = 0; field < 6; field++)
    {
        const std::vector<SBMRunlinePtr> & setupRunlines =
            deleg.getRunlines(field);
        
        mRunlines[field].resize(setupRunlines.size());
        
        for (unsigned int nn = 0; nn < setupRunlines.size(); nn++)
            mRunlines[field][nn] = SimpleAuxRunline(*setupRunlines[nn]);
    }
    LOG << "Created all runlines.\n";
    
    for (int nn = 0; nn < 3; nn++)
        mCurrentBuffers[nn] = deleg.mCurrents[nn];
}

void DrudeModel1::
allocateAuxBuffers()
{
    LOG << "Allocating currents.\n";
    for (int nn = 0; nn < 3; nn++)
    {
        mCurrents[nn].resize(mCurrentBuffers[nn]->getLength());
        mCurrentBuffers[nn]->setHeadPointer(&(mCurrents[nn][0]));
    }
}

void DrudeModel1::
calcEPhase(int direction)
{
    //LOG << "Calculating E.\n";
}

void DrudeModel1::
calcHPhase(int direction)
{
    //LOG << "Calculating H.\n";
}














