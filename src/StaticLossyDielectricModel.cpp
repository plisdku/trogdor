/*
 *  StaticLossyDielectricModel.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/10/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */


#include "StaticLossyDielectricModel.h"
#include "StaticLossyDielectricPML.h"

#include "StreamFromString.h"

#include "PhysicalConstants.h"

#include <cmath>


using namespace std;



#pragma mark *** Unmodified version ***

StaticLossyDielectricModel::
StaticLossyDielectricModel(const Map<std::string, std::string> & inParams) :
    MaterialModel(inParams),
	m_epsr(0.0),
	m_mur(0.0),
	m_sigma(0.0)
{
    LOGF<< "Constructor.\n";
    inParams["epsr"] >> m_epsr;
    inParams["mur"] >> m_mur;
    inParams["sigma"] >> m_sigma;
}

void StaticLossyDielectricModel::
allocate(float dx, float dy, float dz, float dt)
{
    LOGF<< "allocate()\n";
}

string StaticLossyDielectricModel::
getMaterialName() const
{
    return string("Static Lossy Dielectric");
}

MaterialModel* StaticLossyDielectricModel::
createPML(Vector3i direction, Rect3i activeRect, Rect3i regionOfInterest,
          float dx, float dy, float dz, float dt) const
{
    Rect3i extents(regionOfInterest);
    
    if (direction[0] > 0) {
        extents.p1[0] = regionOfInterest.p2[0]+1;
        extents.p2[0] = activeRect.p2[0];
    } else if (direction[0] < 0) {
        extents.p1[0] = activeRect.p1[0];
        extents.p2[0] = regionOfInterest.p1[0]-1;
    }
    
    if (direction[1] > 0) {
        extents.p1[1] = regionOfInterest.p2[1]+1;
        extents.p2[1] = activeRect.p2[1];
    } else if (direction[1] < 0) {
        extents.p1[1] = activeRect.p1[1];
        extents.p2[1] = regionOfInterest.p1[1]-1;
    }
    
    if (direction[2] > 0) {
        extents.p1[2] = regionOfInterest.p2[2]+1;
        extents.p2[2] = activeRect.p2[2];
    } else if (direction[2] < 0) {
        extents.p1[2] = activeRect.p1[2];
        extents.p2[2] = regionOfInterest.p1[2]-1;
    }
    
	Vector3i posDirection(abs(direction[0]), abs(direction[1]),
		abs(direction[2]));
	
	MaterialModel* p;
	
	if (posDirection == Vector3i(1,0,0))
		p = new StaticLossyDielectricPML<1,0,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,1,0))
		p = new StaticLossyDielectricPML<0,1,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,0,1))
		p = new StaticLossyDielectricPML<0,0,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,1,0))
		p = new StaticLossyDielectricPML<1,1,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,1,1))
		p = new StaticLossyDielectricPML<0,1,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,0,1))
		p = new StaticLossyDielectricPML<1,0,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,1,1))
		p = new StaticLossyDielectricPML<1,1,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else
	{
		LOG << "PML error.\n";
		assert(0);
		exit(1);
	}
	
	return p;
}


void StaticLossyDielectricModel::
calcEx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEx.size();
	for (int ii = start; ii < len; ii += stride)
	{
		stepE(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt);
	}
}

void StaticLossyDielectricModel::
calcEy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEy.size();
	for (int ii = start; ii < len; ii += stride)
	{
		stepE(mRunlinesEy[ii], dyinv, dzinv, dxinv, dt);
	}
}

void StaticLossyDielectricModel::
calcEz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEz.size();
	for (int ii = start; ii < len; ii += stride)
	{
		stepE(mRunlinesEz[ii], dzinv, dxinv, dyinv, dt);
	}
}

void StaticLossyDielectricModel::
calcHx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHx.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHx[ii], dxinv, dyinv, dzinv, dt);
}

void StaticLossyDielectricModel::
calcHy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHy.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHy[ii], dyinv, dzinv, dxinv, dt);
}

void StaticLossyDielectricModel::
calcHz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHz.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHz[ii], dzinv, dxinv, dyinv, dt);
}



void StaticLossyDielectricModel::
stepE(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt)
{
    //  Access Hy with field1
    //  Access Hz with field2
    float* Ei = rl.field0();
    
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    
    float ce = (2*m_epsr*Constants::eps0 - m_sigma*dt) /
        (2*m_epsr*Constants::eps0 + m_sigma*dt);
    float ch = 2*dt/(2*m_epsr*Constants::eps0 + m_sigma*dt);
	
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {        
        *Ei = *Ei*ce +
			ch*( (*HkP++ - *HkN++)*dxinv2 -(*HjP++ - *HjN++)*dxinv3 );
        
        Ei++;        
    }
}


void StaticLossyDielectricModel::
stepH(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt)
{
    float* Hi = rl.field0();
    const float* EjP = rl.field1Pos();
    const float* EjN = rl.field1Neg();
    const float* EkP = rl.field2Pos();
    const float* EkN = rl.field2Neg();
    
    float mu_inv = 1.0/(Constants::mu0 * m_mur);
    
	
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Hi += mu_inv*dt*( (*EjP++ - *EjN++)*dxinv3 - (*EkP++ - *EkN++)*dxinv2 );
        
        Hi++;
    }
}

/*
void StaticLossyDielectricModel::
stepEx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    //  Access Hy with field1
    //  Access Hz with field2
    float* Ex = rl.field0();
    
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    
    float ce = (2*m_epsr*Constants::eps0 - m_sigma*dt) /
        (2*m_epsr*Constants::eps0 + m_sigma*dt);
    float ch = 2*dt/(2*m_epsr*Constants::eps0 + m_sigma*dt);
	
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {        
        *Ex = *Ex*ce + ch*( (*HzP++ - *HzN++)*dyinv - (*HyP++ - *HyN++)*dzinv );
        
        Ex++;        
    }
}

void StaticLossyDielectricModel::
stepEy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ey = rl.field0();
    const float* HzN = rl.field1Neg();
    const float* HzP = rl.field1Pos();
    const float* HxN = rl.field2Neg();
    const float* HxP = rl.field2Pos();
    
    float ce = (2*m_epsr*Constants::eps0 - m_sigma*dt) /
        (2*m_epsr*Constants::eps0 + m_sigma*dt);
    float ch = 2*dt/(2*m_epsr*Constants::eps0 + m_sigma*dt);
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Ey = *Ey*ce + ch*( (*HxP++ - *HxN++)*dzinv - (*HzP++ - *HzN++)*dxinv );
        
        //assert(!isnan(*Ey));
        Ey++;
    }
}

void StaticLossyDielectricModel::
stepEz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ez = rl.field0();
    const float* HxN = rl.field1Neg();
    const float* HxP = rl.field1Pos();
    const float* HyN = rl.field2Neg();
    const float* HyP = rl.field2Pos();
    
    float ce = (2*m_epsr*Constants::eps0 - m_sigma*dt) /
        (2*m_epsr*Constants::eps0 + m_sigma*dt);
    float ch = 2*dt/(2*m_epsr*Constants::eps0 + m_sigma*dt);
	
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        //LOG << *Ez << endl;
        
        *Ez = *Ez*ce + ch*( (*HyP++ - *HyN++)*dxinv - (*HxP++ - *HxN++)*dyinv );
        
        Ez++;
    }
}

void StaticLossyDielectricModel::
stepHx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hx = rl.field0();
    const float* EyP = rl.field1Pos();
    const float* EyN = rl.field1Neg();
    const float* EzP = rl.field2Pos();
    const float* EzN = rl.field2Neg();
    
    float mu_inv = 1.0/(Constants::mu0 * m_mur);
    
	
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Hx += mu_inv*dt*( (*EyP++ - *EyN++)*dzinv - (*EzP++ - *EzN++)*dyinv );
        
        Hx++;
        //assert(!isnan(*Hx));
        
    }
}

void StaticLossyDielectricModel::
stepHy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hy = rl.field0();
    const float* EzP = rl.field1Pos();
    const float* EzN = rl.field1Neg();
    const float* ExP = rl.field2Pos();
    const float* ExN = rl.field2Neg();
    
    float mu_inv = 1.0/(Constants::mu0 * m_mur);
	
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Hy += mu_inv*dt*( (*EzP++ - *EzN++)*dxinv - (*ExP++ - *ExN++)*dzinv );
                
        //assert(!isnan(*Hy));
        Hy++;
    }
}

void StaticLossyDielectricModel::
stepHz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hz = rl.field0();
    const float* ExP = rl.field1Pos();
    const float* ExN = rl.field1Neg();
    const float* EyP = rl.field2Pos();
    const float* EyN = rl.field2Neg();
    
    float mu_inv = 1.0/(Constants::mu0 * m_mur);
	
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Hz += mu_inv*dt*( (*ExP++ - *ExN++)*dyinv - (*EyP++ - *EyN++)*dxinv );
        Hz++;
    }
}
*/


