/*
 *  DrudeMetalModel.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/21/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "DrudeMetalModel.h"
#include "DrudeMetalPML.h"

#include "PhysicalConstants.h"
#include "PECModel.h"
#include "StreamFromString.h"

using namespace std;



DrudeMetalModel::
DrudeMetalModel(const Map<string, string> & inParams) :
    MaterialModel(inParams),
	m_eps_inf(0.0),
	m_mu(0.0),
	m_omega_p(0.0),
	m_tau_c(0.0),
	m_ch1(0.0),
	m_cj1(0.0),
	m_cj2(0.0),
	m_ce1(0.0),
	m_ce2(0.0)
{
    float epsr_inf;
    
    if (inParams.count("epsinf") == 0)
    {
        cerr << "Warning: Drude model needs parameter 'epsinf'.\n";
        assert(!"Quitting.");
    }
    
    if (inParams.count("omegap") == 0)
    {
        cerr << "Warning: Drude model needs parameter 'omegap'.\n";
        assert(!"Quitting.");
    }
    
    if (inParams.count("tauc") == 0)
    {
        cerr << "Warning: Drude model needs parameter 'tauc'.\n";
        assert(!"Quitting.");
    }
    
    float mur = 1.0;
    if (inParams.count("mur"))
        inParams["mur"] >> mur;
    
    inParams["epsinf"] >> epsr_inf;
    inParams["omegap"] >> m_omega_p;
    inParams["tauc"] >> m_tau_c;
    
    m_eps_inf = Constants::eps0 * epsr_inf;
    m_mu = mur * Constants::mu0;
}

DrudeMetalModel::
~DrudeMetalModel()
{
    LOGF << "Destructor.\n";
}



void DrudeMetalModel::
allocate(float dx, float dy, float dz, float dt)
{
	//LOG << "Allocating Drude.\n";
	
    m_ce1 = dt/m_eps_inf;
    m_ce2 = dt/m_eps_inf;
    m_cj1 = (2*m_tau_c - dt)/(2*m_tau_c + dt);
    m_cj2 = 2*m_tau_c*dt*m_omega_p*m_omega_p*m_eps_inf/(dt + 2*m_tau_c);
    m_ch1 = dt/m_mu;
    
	mJ[0].resize(numCellsEx(), 0.0);
	mJ[1].resize(numCellsEy(), 0.0);
	mJ[2].resize(numCellsEz(), 0.0);
}

std::string DrudeMetalModel::
getMaterialName() const
{
    return "Drude Metal";
}


MaterialModel* DrudeMetalModel::
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
		p = new DrudeMetalPML<1,0,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,1,0))
		p = new DrudeMetalPML<0,1,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,0,1))
		p = new DrudeMetalPML<0,0,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,1,0))
		p = new DrudeMetalPML<1,1,0>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(0,1,1))
		p = new DrudeMetalPML<0,1,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,0,1))
		p = new DrudeMetalPML<1,0,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else if (posDirection == Vector3i(1,1,1))
		p = new DrudeMetalPML<1,1,1>(mParams, direction, extents, dx,
			dy, dz, dt);
	else
	{
		LOG << "PML error.\n";
		assert(0);
		exit(1);
	}
	
	return p;	
}



void DrudeMetalModel::
calcEx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEx.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE_orig_unrolled_prefetch<12>(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt, 0);
		stepE_orig(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt, 0);
	}
}

void DrudeMetalModel::
calcEy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEy.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE_orig_unrolled_prefetch<12>(mRunlinesEy[ii], dyinv, dzinv, dxinv, dt, 1);
		stepE_orig(mRunlinesEy[ii], dyinv, dzinv, dxinv, dt, 1);
	}
}

void DrudeMetalModel::
calcEz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEz.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE_orig_unrolled_prefetch<12>(mRunlinesEz[ii], dzinv, dxinv, dyinv, dt, 2);
		stepE_orig(mRunlinesEz[ii], dzinv, dxinv, dyinv, dt, 2);
	}
}

void DrudeMetalModel::
calcHx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHx.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHx[ii], dxinv, dyinv, dzinv, dt, 0);
}

void DrudeMetalModel::
calcHy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHy.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHy[ii], dyinv, dzinv, dxinv, dt, 1);
}

void DrudeMetalModel::
calcHz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHz.size();
	for (int ii = start; ii < len; ii += stride)
		stepH(mRunlinesHz[ii], dzinv, dxinv, dyinv, dt, 2);
}


void DrudeMetalModel::
stepE_orig(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    float* Ei = rl.field0();
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		/*
		assert(!isnan(*HjP));
		assert(!isnan(*HjN));
		assert(!isnan(*HkP));
		assert(!isnan(*HkN));
		assert(!isnan(*Ei));
		*/
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
		
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
}
/*

void DrudeMetalModel::
stepE_orig_unrolled(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    float* Ei = rl.field0();
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
	int ii;
    for (ii = 0; ii < rlLength%4; ii++)
    {
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
	
	for ( ; ii < rlLength; ii+=4)
	{
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
	}
}


void DrudeMetalModel::
stepE_orig_unrolled_array(const Runline& rl, float dxinv1, float dxinv2, float dxinv3,
	float dt, int dir)
{
    float* Ei = rl.field0();
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
	int ii;
    for (ii = 0; ii < rlLength%4; ii++)
    {
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
	
	for ( ; ii < rlLength; ii+=4)
	{
		Ei[0] = Ei[0] + ce1j*(HkP[0] - HkN[0]) - ce1k*(HjP[0] - HjN[0])
			- Ji[0]*ce2;
		Ji[0] = Ji[0]*cj1 + Ei[0]*cj2;
		
		Ei[1] = Ei[1] + ce1j*(HkP[1] - HkN[1]) - ce1k*(HjP[1] - HjN[1])
			- Ji[1]*ce2;
		Ji[1] = Ji[1]*cj1 + Ei[1]*cj2;
		
		Ei[2] = Ei[2] + ce1j*(HkP[2] - HkN[2]) - ce1k*(HjP[2] - HjN[2])
			- Ji[2]*ce2;
		Ji[2] = Ji[2]*cj1 + Ei[2]*cj2;
		
		Ei[3] = Ei[3] + ce1j*(HkP[3] - HkN[3]) - ce1k*(HjP[3] - HjN[3])
			- Ji[3]*ce2;
		Ji[3] = Ji[3]*cj1 + Ei[3]*cj2;
		
		Ei += 4;
		Ji += 4;
		HkN += 4;
		HkP += 4;
		HjN += 4;
		HjP += 4;
	}
}


template<int PREFETCH>
void DrudeMetalModel::
stepE_orig_prefetch(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    float* Ei = rl.field0();
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		__builtin_prefetch(Ei+PREFETCH);
		__builtin_prefetch(Ji+PREFETCH);
		__builtin_prefetch(HjN+PREFETCH);
		__builtin_prefetch(HjP+PREFETCH);
		__builtin_prefetch(HkN+PREFETCH);
		__builtin_prefetch(HkP+PREFETCH);
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
}


template<int PREFETCH>
void DrudeMetalModel::
stepE_orig_unrolled_prefetch(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    float* Ei = rl.field0();
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
	int ii;
    for (ii = 0; ii < rlLength%4; ii++)
    {
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
	
	for ( ; ii < rlLength/4; ii++)
	{
		__builtin_prefetch(Ei+PREFETCH);
		__builtin_prefetch(Ji+PREFETCH);
		__builtin_prefetch(HjN+PREFETCH);
		__builtin_prefetch(HjP+PREFETCH);
		__builtin_prefetch(HkN+PREFETCH);
		__builtin_prefetch(HkP+PREFETCH);
		
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
	}
}

void DrudeMetalModel::
stepE_split(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
	
    float* Ei = rl.field0();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        //*Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
	
	Ei = rl.field0();
    Ji = &(mJ[dir][0]) + rl.getIndex();
    for (int ii = 0; ii < rlLength; ii++)
    {
        //*Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
        //    - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
}

template <int PREFETCH>
void DrudeMetalModel::
stepE_split_prefetch(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
	
    float* Ei = rl.field0();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    for (int ii = 0; ii < rlLength; ii++)
    {
		__builtin_prefetch(Ei+PREFETCH);
		__builtin_prefetch(Ji+PREFETCH);
		__builtin_prefetch(HkP+PREFETCH);
		__builtin_prefetch(HkN+PREFETCH);
		__builtin_prefetch(HjN+PREFETCH);
		__builtin_prefetch(HjP+PREFETCH);
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        //*Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
	
	Ei = rl.field0();
    Ji = &(mJ[dir][0]) + rl.getIndex();
    for (int ii = 0; ii < rlLength; ii++)
    {
		__builtin_prefetch(Ei+PREFETCH);
		__builtin_prefetch(Ji+PREFETCH);
        //*Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
        //    - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
}

template <int PREFETCH>
void DrudeMetalModel::
stepE_split_prefetchE(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
	
    float* Ei = rl.field0();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    for (int ii = 0; ii < rlLength; ii++)
    {
		__builtin_prefetch(Ei+PREFETCH);
		__builtin_prefetch(Ji+PREFETCH);
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        //*Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
	
	Ei = rl.field0();
    Ji = &(mJ[dir][0]) + rl.getIndex();
    for (int ii = 0; ii < rlLength; ii++)
    {
		__builtin_prefetch(Ei+PREFETCH);
		__builtin_prefetch(Ji+PREFETCH);
        //*Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
        //    - *Ji*ce2;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
}

void DrudeMetalModel::
stepE_split_unrolled(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
	
	int ii;
	
    float* Ei = rl.field0();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    for (ii = 0; ii < rlLength%4; ii++)
    {
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
    }
    for ( ; ii < rlLength; ii+=4)
    {
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
    }
	
	Ei = rl.field0();
    Ji = &(mJ[dir][0]) + rl.getIndex();
    for (ii = 0; ii < rlLength%4; ii++)
    {
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
    for ( ; ii < rlLength; ii+=4)
    {
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
}

template <int PREFETCH>
void DrudeMetalModel::
stepE_split_unrolled_prefetch(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    const float* HjN = rl.field1Neg();
    const float* HjP = rl.field1Pos();
    const float* HkN = rl.field2Neg();
    const float* HkP = rl.field2Pos();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1j = m_ce1*dxinv2;
    const float ce1k = m_ce1*dxinv3;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
	
	int ii;
	
    float* Ei = rl.field0();
    float* Ji = &(mJ[dir][0]) + rl.getIndex();
    for (ii = 0; ii < rlLength%4; ii++)
    {
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
    }
    for ( ; ii < rlLength; ii+=4)
    {
		__builtin_prefetch(Ei+PREFETCH);
		__builtin_prefetch(Ji+PREFETCH);
		__builtin_prefetch(HkP+PREFETCH);
		__builtin_prefetch(HkN+PREFETCH);
		__builtin_prefetch(HjN+PREFETCH);
		__builtin_prefetch(HjP+PREFETCH);
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
        *Ei = *Ei + ce1j*(*HkP++ - *HkN++) - ce1k*(*HjP++ - *HjN++)
            - *Ji*ce2;
        
        Ei++;
        Ji++;
    }
	
	Ei = rl.field0();
    Ji = &(mJ[dir][0]) + rl.getIndex();
    for (ii = 0; ii < rlLength%4; ii++)
    {
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
    for ( ; ii < rlLength; ii+=4)
    {
		__builtin_prefetch(Ei+PREFETCH);
		__builtin_prefetch(Ji+PREFETCH);
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
        *Ji = *Ji*cj1 + *Ei*cj2;
        
        Ei++;
        Ji++;
    }
}

*/

void DrudeMetalModel::
stepH(const Runline & rl, float dxinv1, float dxinv2, float dxinv3, float dt,
	int dir)
{
    float* Hi = rl.field0();
    const float* EjP = rl.field1Pos();
    const float* EjN = rl.field1Neg();
    const float* EkP = rl.field2Pos();
    const float* EkN = rl.field2Neg();
    
    const float ch1k = m_ch1*dxinv3;
    const float ch1j = m_ch1*dxinv2;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		/*
		assert(!isnan(*EjP));
		assert(!isnan(*EjN));
		assert(!isnan(*EkP));
		assert(!isnan(*EkN));
		assert(!isnan(*Hi));
		*/
        *Hi = *Hi + ch1k*(*EjP++ - *EjN++) - ch1j*(*EkP++ - *EkN++);
		Hi++;
    }
}

/*
void DrudeMetalModel::
stepEx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ex = rl.field0();
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    float* Jx = &(mJx[0]) + rl.getIndex();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1y = m_ce1*dyinv;
    const float ce1z = m_ce1*dzinv;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Ex = *Ex + ce1y*(*HzP++ - *HzN++) - ce1z*(*HyP++ - *HyN++)
            - *Jx*ce2;
        *Jx = *Jx*cj1 + *Ex*cj2;
        
        Ex++;
        Jx++;
    }
}

void DrudeMetalModel::
stepEy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ey = rl.field0();
    const float* HzN = rl.field1Neg();
    const float* HzP = rl.field1Pos();
    const float* HxN = rl.field2Neg();
    const float* HxP = rl.field2Pos();
    float* Jy = &(mJy[0]) + rl.getIndex();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1z = m_ce1*dzinv;
    const float ce1x = m_ce1*dxinv;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Ey = *Ey + ce1z*(*HxP++ - *HxN++) - ce1x*(*HzP++ - *HzN++)
            - *Jy*ce2;
        *Jy = *Jy*cj1 + *Ey*cj2;
        
        Ey++;
        Jy++;
    }
}

void DrudeMetalModel::
stepEz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ez = rl.field0();
    const float* HxN = rl.field1Neg();
    const float* HxP = rl.field1Pos();
    const float* HyN = rl.field2Neg();
    const float* HyP = rl.field2Pos();
    float* Jz = &(mJz[0]) + rl.getIndex();
    
    const float cj1 = m_cj1;
    const float cj2 = m_cj2;
    const float ce1x = m_ce1*dxinv;
    const float ce1y = m_ce1*dyinv;
    const float ce2 = m_ce2;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Ez = *Ez + ce1x*(*HyP++ - *HyN++) - ce1y*(*HxP++ - *HxN++)
            - *Jz*ce2;
        *Jz = *Jz*cj1 + *Ez*cj2;
        
        Ez++;
        Jz++;
    }
}

void DrudeMetalModel::
stepHx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hx = rl.field0();
    const float* EyP = rl.field1Pos();
    const float* EyN = rl.field1Neg();
    const float* EzP = rl.field2Pos();
    const float* EzN = rl.field2Neg();
    
    const float ch1z = m_ch1*dzinv;
    const float ch1y = m_ch1*dyinv;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Hx = *Hx + ch1z*(*EyP++ - *EyN++) - ch1y*(*EzP++ - *EzN++);
        
        Hx++;
    }
}

void DrudeMetalModel::
stepHy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hy = rl.field0();
    const float* EzP = rl.field1Pos();
    const float* EzN = rl.field1Neg();
    const float* ExP = rl.field2Pos();
    const float* ExN = rl.field2Neg();
    
    const float ch1x = m_ch1*dxinv;
    const float ch1z = m_ch1*dzinv;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Hy = *Hy + ch1x*(*EzP++ - *EzN++) - ch1z*(*ExP++ - *ExN++);
        
        Hy++;
    }
}

void DrudeMetalModel::
stepHz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hz = rl.field0();
    const float* ExP = rl.field1Pos();
    const float* ExN = rl.field1Neg();
    const float* EyP = rl.field2Pos();
    const float* EyN = rl.field2Neg();
    
    const float ch1y = m_ch1*dyinv;
	const float ch1x = m_ch1*dxinv;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        *Hz = *Hz + ch1y*(*ExP++ - *ExN++) - ch1x*(*EyP++ - *EyN++);
        
        Hz++;
    }
}

*/
