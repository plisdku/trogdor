/*
 *  StaticDielectricModel.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/10/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */


#include "StaticDielectricModel.h"
#include "StaticDielectricPML.h"

#include "StreamFromString.h"

#include "PhysicalConstants.h"

#include <cmath>


using namespace std;

#pragma mark *** Unmodified version ***

StaticDielectricModel::
StaticDielectricModel(const Map<std::string, std::string> & inParams) :
	m_epsr(0.0),
	m_mur(0.0),
    MaterialModel(inParams)
{
    LOGF << "Constructor.\n";
    inParams["epsr"] >> m_epsr;
    inParams["mur"] >> m_mur;
}

void StaticDielectricModel::
onAddRunline(Runline & rl, int ii, int jj, int kk)
{
	/*
    LOG << "add runline from " << ii << " " << jj << " "
                         << kk << " length " << rl.getLength() << "\n";
	*/
}

void StaticDielectricModel::
allocate(float dx, float dy, float dz, float dt)
{
    //LOG << "allocate()\n";
}

string StaticDielectricModel::
getMaterialName() const
{
    return string("Static Dielectric");
}

MaterialModel* StaticDielectricModel::
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
		p = new StaticDielectricPML<1,0,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,1,0))
		p = new StaticDielectricPML<0,1,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,0,1))
		p = new StaticDielectricPML<0,0,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,1,0))
		p = new StaticDielectricPML<1,1,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,1,1))
		p = new StaticDielectricPML<0,1,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,0,1))
		p = new StaticDielectricPML<1,0,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,1,1))
		p = new StaticDielectricPML<1,1,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else
	{
		LOG << "PML error.\n";
		assert(0);
		exit(1);
	}
	
	return p;
}

#pragma mark *** New-style updates ***


void StaticDielectricModel::
calcEx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEx.size();
	for (int ii = start; ii < len; ii += stride)
	{
		stepE(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt);
	}
}

void StaticDielectricModel::
calcEy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEy.size();
	for (int ii = start; ii < len; ii += stride)
	{
		stepE(mRunlinesEy[ii], dyinv, dzinv, dxinv, dt);
	}
}

void StaticDielectricModel::
calcEz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEz.size();
	for (int ii = start; ii < len; ii += stride)
	{
		stepE(mRunlinesEz[ii], dzinv, dxinv, dyinv, dt);
	}
}

void StaticDielectricModel::
calcHx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHx.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHx[ii], dxinv, dyinv, dzinv, dt);
}

void StaticDielectricModel::
calcHy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHy.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHy[ii], dyinv, dzinv, dxinv, dt);
}

void StaticDielectricModel::
calcHz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHz.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHz[ii], dzinv, dxinv, dyinv, dt);
}


void StaticDielectricModel::
stepE(const Runline& rl, float dxinv1, float dxinv2, float dxinv3, float dt)
{
    float* Ei = rl.field0();
	
	//cerr << "Runline: " << rl.m_ii0 << " " << rl.m_jj0 << " " <<  rl.m_kk0
	//	<< endl;
    
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    const float eps_inv_dt_j = dxinv2*dt/(Constants::eps0 * m_epsr);
    const float eps_inv_dt_k = dxinv3*dt/(Constants::eps0 * m_epsr);
	
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		*Ei += eps_inv_dt_j*(*HkP++ - *HkN++) - eps_inv_dt_k*(*HjP++ - *HjN++);		
        Ei++;        
    }
}

void StaticDielectricModel::
stepH(const Runline& rl, float dxinv1, float dxinv2, float dxinv3, float dt)
{
    float* Hi = rl.field0();
    const float* EjP = rl.field1Pos();
    const float* EjN = rl.field1Neg();
    const float* EkP = rl.field2Pos();
    const float* EkN = rl.field2Neg();
    
	const float mu_inv_dt_k = dxinv3*dt/(Constants::mu0 * m_mur);
	const float mu_inv_dt_j = dxinv2*dt/(Constants::mu0 * m_mur);
    
	int rlLength = rl.getLength();
	
    for (int ii = 0; ii < rlLength; ii++)
    {
		*Hi += mu_inv_dt_k*(*EjP++ - *EjN++) - mu_inv_dt_j*(*EkP++ - *EkN++);		
        Hi++;
    }
}


#pragma mark *** Old-style updates ***

/*
void StaticDielectricModel::
stepEx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    //  Access Hy with field1
    //  Access Hz with field2
    float* Ex = rl.field0();
    
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    const float eps_inv_dt_y = dyinv*dt/(Constants::eps0 * m_epsr);
    const float eps_inv_dt_z = dzinv*dt/(Constants::eps0 * m_epsr);
	
	int rlLength = rl.getLength();
    //LOG << rl.m_ii0 << " " << rlLength << "\n";
    for (int ii = 0; ii < rlLength; ii++)
    {        
        //*Ex += eps_inv_dt*( (*HzP++ - *HzN++)*dyinv - (*HyP++ - *HyN++)*dzinv );
        
		*Ex += eps_inv_dt_y*(*HzP++ - *HzN++) - eps_inv_dt_z*(*HyP++ - *HyN++);
		assert(!isnan(*Ex));
        Ex++;        
    }
}

void StaticDielectricModel::
stepEy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ey = rl.field0();
    const float* HzN = rl.field1Neg();
    const float* HzP = rl.field1Pos();
    const float* HxN = rl.field2Neg();
    const float* HxP = rl.field2Pos();
    
    const float eps_inv_dt_z = dzinv*dt/(Constants::eps0 * m_epsr);
    const float eps_inv_dt_x = dxinv*dt/(Constants::eps0 * m_epsr);
    
	int rlLength = rl.getLength();
	
    //LOG << rl.m_ii0 << " " << rlLength << "\n";
    for (int ii = 0; ii < rlLength; ii++)
    {
        //*Ey += eps_inv_dt*( (*HxP++ - *HxN++)*dzinv - (*HzP++ - *HzN++)*dxinv );
		assert(!isnan(*Ey));
        *Ey += eps_inv_dt_z*(*HxP++ - *HxN++) - eps_inv_dt_x*(*HzP++ - *HzN++);
		assert(!isnan(*Ey));
		
        Ey++;
    }
}

void StaticDielectricModel::
stepEz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ez = rl.field0();
    const float* HxN = rl.field1Neg();
    const float* HxP = rl.field1Pos();
    const float* HyN = rl.field2Neg();
    const float* HyP = rl.field2Pos();
    
    const float eps_inv_dt_x = dxinv*dt/(Constants::eps0 * m_epsr);
    const float eps_inv_dt_y = dyinv*dt/(Constants::eps0 * m_epsr);
    
	
	
	int rlLength = rl.getLength();
    //LOG << rl.m_ii0 << " " << rlLength << "\n";
	
	
    for (int ii = 0; ii < rlLength; ii++)
    {
		//cout << rl.m_ii0/2+ii << " " << rl.m_jj0/2 << " " << rl.m_kk0/2 <<
		//	": " << *Ez << " " << *HxN << " " << *HxP << " " << *HyN << " "
		//	<< *HyP<< "\n";
        //*Ez += eps_inv_dt*( (*HyP++ - *HyN++)*dxinv - (*HxP++ - *HxN++)*dyinv );
		
        *Ez += eps_inv_dt_x*(*HyP++ - *HyN++) - eps_inv_dt_y*(*HxP++ - *HxN++);
		assert(!isnan(*Ez));
		
        Ez++;
    }
}

void StaticDielectricModel::
stepHx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hx = rl.field0();
    const float* EyP = rl.field1Pos();
    const float* EyN = rl.field1Neg();
    const float* EzP = rl.field2Pos();
    const float* EzN = rl.field2Neg();
    
	const float mu_inv_dt_z = dzinv*dt/(Constants::mu0 * m_mur);
	const float mu_inv_dt_y = dyinv*dt/(Constants::mu0 * m_mur);
    
	int rlLength = rl.getLength();
    //LOG << rl.m_ii0 << " " << rlLength << "\n";
    for (int ii = 0; ii < rlLength; ii++)
    {		
        //*Hx += mu_inv_dt*( (*EyP++ - *EyN++)*dzinv - (*EzP++ - *EzN++)*dyinv );
		*Hx += mu_inv_dt_z*(*EyP++ - *EyN++) - mu_inv_dt_y*(*EzP++ - *EzN++);
		assert(!isnan(*Hx));
		
        Hx++;
        
    }
}

void StaticDielectricModel::
stepHy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hy = rl.field0();
    const float* EzP = rl.field1Pos();
    const float* EzN = rl.field1Neg();
    const float* ExP = rl.field2Pos();
    const float* ExN = rl.field2Neg();
	
	const float mu_inv_dt_x = dxinv*dt/(Constants::mu0 * m_mur);
	const float mu_inv_dt_z = dzinv*dt/(Constants::mu0 * m_mur);
	    
	int rlLength = rl.getLength();
    //LOG << rl.m_ii0 << " " << rlLength << "\n";
    for (int ii = 0; ii < rlLength; ii++)
    {
		assert(!isnan(*Hy));
		assert(!isnan(*ExP));
		assert(!isnan(*ExN));
		assert(!isnan(*EzP));
		assert(!isnan(*EzN));
		*Hy += mu_inv_dt_x*(*EzP++ - *EzN++) - mu_inv_dt_z*(*ExP++ - *ExN++);
		assert(!isnan(*Hy));
		
        Hy++;
    }
}

void StaticDielectricModel::
stepHz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hz = rl.field0();
    const float* ExP = rl.field1Pos();
    const float* ExN = rl.field1Neg();
    const float* EyP = rl.field2Pos();
    const float* EyN = rl.field2Neg();
    
	const float mu_inv_dt_y = dyinv*dt/(Constants::mu0 * m_mur);
	const float mu_inv_dt_x = dxinv*dt/(Constants::mu0 * m_mur);
    
	int rlLength = rl.getLength();
    //LOG << rl.m_ii0 << " " << rlLength << "\n";
    for (int ii = 0; ii < rlLength; ii++)
    {
        //*Hz += mu_inv_dt*( (*ExP++ - *ExN++)*dyinv - (*EyP++ - *EyN++)*dxinv );
		assert(!isnan(*Hz));
		assert(!isnan(*EyP));
		assert(!isnan(*EyN));
		assert(!isnan(*ExP));
		assert(!isnan(*ExN));
		*Hz += mu_inv_dt_y*(*ExP++ - *ExN++) - mu_inv_dt_x*(*EyP++ - *EyN++);
		assert(!isnan(*Hz));
		
        Hz++;
    }
}
*/
