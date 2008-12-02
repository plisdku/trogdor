/*
 *  DrudeMetalPML.hh
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/16/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef _DRUDEMETALPML_

#include "DrudeMetalPML.h"

#include "PhysicalConstants.h"
#include "StreamFromString.h"

using namespace std;

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
DrudeMetalPML(const Map<std::string, std::string> & inParams, 
    Vector3i inDirection, const Rect3i & inPMLExtent,
    float dx, float dy, float dz, float dt) :
    MaterialModel(inParams),
	mAttenuation(X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION),
    mExtents(inPMLExtent)
{
    //  Some of these arrays will be a tad too long (by one cell), but that's
    //  a fair price to pay for not going nuts about indices.
    LOGF << "constructor" << endl;
    
	float dxs[3] = {dx, dy, dz};
	
    float m = 3.5;
    float sigmaCoeff = 0.8;  //  0.8 from Taflove pg. 307
    float kappaMax = 5.0; //  seems good from Roden & Gedney 2000
    float etar = 1.0;
    float alphaMax = 0.24; // Gedney 2005
    float mAlpha = 1.0;  // Gedney 2005
    
	
    float epsr_inf = 1.0;
    float mur = 1.0;
	
    if (inParams.count("omegap"))
        inParams["omegap"] >> m_omega_p;
    if (inParams.count("tauc"))
        inParams["tauc"] >> m_tau_c;
    if (inParams.count("epsinf"))
        inParams["epsinf"] >> epsr_inf;
    if (inParams.count("mur"))
        inParams["mur"] >> mur;
    if (inParams.count("PMLetar"))
        inParams["PMLetar"] >> etar;
    if (inParams.count("PMLm"))
        inParams["PMLm"] >> m;
    if (inParams.count("PMLkappaMax"))
        inParams["PMLkappaMax"] >> kappaMax;
    if (inParams.count("PMLsigmaCoeff"))
        inParams["PMLsigmaCoeff"] >> sigmaCoeff;
    if (inParams.count("PMLalphaMax"))
        inParams["PMLalphaMax"] >> alphaMax;
    if (inParams.count("PMLmAlpha"))
        inParams["PMLmAlpha"] >> mAlpha;
	/*
    LOG << "PML params: epsr = " << m_epsr << " mur = " << m_mur << " etar = "
        << etar << " m = " << m << " kappaMax = " << kappaMax <<
        " sigmaCoeff = " << sigmaCoeff << " alphaMax = " << alphaMax <<
        " mAlpha = " << mAlpha << endl;
    */
	
    m_eps_inf = Constants::eps0 * epsr_inf;
    m_mu = Constants::mu0 * mur;
	
    float eta = etar*sqrt(Constants::mu0 / Constants::eps0);
    float sigmaMax;
    
    int halfCells;
    int ii;
    
    //  X conductivity
    //LOGMORE << "sigma X... " << flush;
	
	
	for (int dir = 0; dir < 3; dir++)
	if (mAttenuation[dir])
	{
		sigmaMax = sigmaCoeff*(m+1.0)/(eta*dxs[dir]);
		halfCells = inPMLExtent.p2[dir] - inPMLExtent.p1[dir] + 1;
		
		mAEj[dir].resize((halfCells+1)/2, 0.0);
		mAEk[dir].resize((halfCells+1)/2, 0.0);
		mAHj[dir].resize((halfCells+1)/2, 0.0);
		mAHk[dir].resize((halfCells+1)/2, 0.0);
		
		mBEj[dir].resize((halfCells+1)/2, 0.0);
		mBEk[dir].resize((halfCells+1)/2, 0.0);
		mBHj[dir].resize((halfCells+1)/2, 0.0);
		mBHk[dir].resize((halfCells+1)/2, 0.0);
		
		mKEj[dir].resize((halfCells+1)/2, 1.0);
		mKEk[dir].resize((halfCells+1)/2, 1.0);
		mKHj[dir].resize((halfCells+1)/2, 1.0);
		mKHk[dir].resize((halfCells+1)/2, 1.0);
		
        vector<float> sigma(halfCells, 0.0);
        vector<float> kappa(halfCells, 1.0);
        vector<float> alpha(halfCells, 0.0);
		
		//if (dir == 1)
		//LOG << "Printing: \n";
        if (inDirection[dir] < 0)
        for (ii = 0; ii < halfCells; ii++)
        {
            sigma[ii] = sigmaMax*pow(float(halfCells-ii)/halfCells, m);
            kappa[ii] = 1.0 +
                (kappaMax-1.0)*pow(float(halfCells-ii)/halfCells, m);
            alpha[ii] = alphaMax*pow(float(ii)/halfCells, mAlpha);
			
			//if (dir == 1)
			//LOGMORE << sigma[ii] << " " << kappa[ii] << " " << alpha[ii] << "\n";
        }
        else
        for (ii = 1; ii <= halfCells; ii++)
        {
            sigma[ii-1] = sigmaMax*pow(float(ii)/halfCells, m);
            kappa[ii-1] = 1.0 + (kappaMax-1.0)*pow(float(ii)/halfCells, m);
            alpha[ii-1] = alphaMax*pow(float(halfCells-ii)/halfCells, mAlpha);
        }
        //LOG << "Printing:\n";
        for (ii = 0; ii < halfCells; ii++)
        {
            float aa = (sigma[ii]/(sigma[ii]*kappa[ii]
                + kappa[ii]*kappa[ii]*alpha[ii]))
                * (exp(-(dt/Constants::eps0)
                * (sigma[ii]/kappa[ii] + alpha[ii])) - 1);
            
            float bb = exp(-(dt/Constants::eps0)
                *(sigma[ii]/kappa[ii] + alpha[ii]));
            
			
            if ( (inPMLExtent.p1[dir] + ii)%2 == 0)
            {
                mAEj[dir][ii/2] = aa;
                mAEk[dir][ii/2] = aa;
                mBEj[dir][ii/2] = bb;
                mBEk[dir][ii/2] = bb;
                mKEj[dir][ii/2] = 1.0/kappa[ii];
                mKEk[dir][ii/2] = 1.0/kappa[ii];
				
				//LOGMORE << "E fields: " << aa << "\n";
            }
            else
            {
                mAHj[dir][ii/2] = aa;
                mAHk[dir][ii/2] = aa;
                mBHj[dir][ii/2] = bb;
                mBHk[dir][ii/2] = bb;
                mKHj[dir][ii/2] = 1.0/kappa[ii];
                mKHk[dir][ii/2] = 1.0/kappa[ii];
				
				//LOGMORE << "H fields: " << aa << "\n";
            }
        }
	}
    
    //LOGMORE << "done." << endl;
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
~DrudeMetalPML()
{
    LOGF << "Destructor.\n";
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
onAddRunline(Runline & rl, int ii, int jj, int kk)
{
    if (X_ATTENUATION)
        rl.setAuxIndex1( (ii - mExtents.p1[0])  / 2);
    if (Y_ATTENUATION)
        rl.setAuxIndex2( (jj - mExtents.p1[1]) / 2);
    if (Z_ATTENUATION)
        rl.setAuxIndex3( (kk - mExtents.p1[2]) / 2);
    /*
    LOG << rl.getAuxIndex1() << " " << rl.getAuxIndex2()
                         << " at " << ii << " " << jj << " " << kk
                         << " with flags " << mHasSigY << " " << mHasSigZ
                         << "\n";
    */
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
allocate(float dx, float dy, float dz, float dt)
{
    //LOG << "allocate... " << endl;
	
    if (X_ATTENUATION)
    {
        mAccumEj[0].resize(numCellsEy());
        mAccumEk[0].resize(numCellsEz());
        mAccumHj[0].resize(numCellsHy());
        mAccumHk[0].resize(numCellsHz());
    }
    
    if (Y_ATTENUATION)
    {
        mAccumEj[1].resize(numCellsEz());
        mAccumEk[1].resize(numCellsEx());
        mAccumHj[1].resize(numCellsHz());
        mAccumHk[1].resize(numCellsHx());
    }
    
    if (Z_ATTENUATION)
    {
        mAccumEj[2].resize(numCellsEx());
        mAccumEk[2].resize(numCellsEy());
        mAccumHj[2].resize(numCellsHx());
        mAccumHk[2].resize(numCellsHy());
    }
    
    mJx.resize(numCellsEx());
    mJy.resize(numCellsEy());
    mJz.resize(numCellsEz());
    
    //  I derive the Drude model with Gamma = 1/tau.
    m_j1 = (2*m_tau_c - dt)/(2*m_tau_c + dt);
    m_j2 = (2*Constants::eps0*m_omega_p*m_omega_p*dt*m_tau_c)/(2*m_tau_c + dt);
    m_e2 = 1.0/m_eps_inf;
    m_e3 = dt/m_eps_inf;
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
string DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
getMaterialName() const
{
	ostringstream str;
	str << "Drude Metal PML " << X_ATTENUATION << " "
		<< Y_ATTENUATION << " " << Z_ATTENUATION;
	return str.str();
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcEx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEx.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt);
		stepEx_split(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt);
	}
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcEy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEy.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE(mRunlinesEy[ii], dyinv, dzinv, dxinv, dt);
		stepEy_split(mRunlinesEy[ii], dxinv, dyinv, dzinv, dt);
	}
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcEz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEz.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE(mRunlinesEz[ii], dzinv, dxinv, dyinv, dt);
		stepEz_split(mRunlinesEz[ii], dxinv, dyinv, dzinv, dt);
	}
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcHx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHx.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepH(mRunlinesHx[ii], dxinv, dyinv, dzinv, dt);
		stepHx(mRunlinesHx[ii], dxinv, dyinv, dzinv, dt);
	}
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcHy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHy.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepH(mRunlinesHy[ii], dyinv, dzinv, dxinv, dt);
		stepHy(mRunlinesHy[ii], dxinv, dyinv, dzinv, dt);
	}
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcHz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesHz.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepH(mRunlinesHz[ii], dzinv, dxinv, dyinv, dt);
		stepHz(mRunlinesHz[ii], dxinv, dyinv, dzinv, dt);
	}
}



template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
template <int PREFETCH>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEx_prefetch(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ex = rl.field0();
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    
    float* Jx = &mJx[rl.getIndex()];
	
    float* psiY;
    float* psiZ;
    
    float ay;
    float az;
    float by;
    float bz;
    float kzinv = 1/1.0;
    float kyinv = 1/1.0;
	
    
    if (Y_ATTENUATION)
    {   
        psiY = &mAccumEk[1][rl.getIndex()];
        ay = mAEk[1][rl.getAuxIndex2()];
        by = mBEk[1][rl.getAuxIndex2()];
        kyinv = mKEk[1][rl.getAuxIndex2()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEj[2][rl.getIndex()];
        az = mAEj[2][rl.getAuxIndex3()];
        bz = mBEj[2][rl.getAuxIndex3()];
        kzinv =  mKEj[2][rl.getAuxIndex3()];
    }
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
    //LOG << ii < rlLength << "\n";
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		__builtin_prefetch(HzP+PREFETCH);
		__builtin_prefetch(HzN+PREFETCH);
		__builtin_prefetch(HyP+PREFETCH);
		__builtin_prefetch(HyN+PREFETCH);
		__builtin_prefetch(Ex+PREFETCH);
		__builtin_prefetch(Jx+PREFETCH);
        float dHz = (*HzP++ - *HzN++)*dyinv;
        float dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
			__builtin_prefetch(psiY+PREFETCH);
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
			__builtin_prefetch(psiZ+PREFETCH);
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jx = *Jx*j1 + *Ex*j2;
        
        Ex++;
        Jx++;
    }
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
template <int PREFETCH>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEx_prefetchE(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ex = rl.field0();
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    
    float* Jx = &mJx[rl.getIndex()];
	
    float* psiY;
    float* psiZ;
    
    float ay;
    float az;
    float by;
    float bz;
    float kzinv = 1/1.0;
    float kyinv = 1/1.0;
	
    
    if (Y_ATTENUATION)
    {   
        psiY = &mAccumEk[1][rl.getIndex()];
        ay = mAEk[1][rl.getAuxIndex2()];
        by = mBEk[1][rl.getAuxIndex2()];
        kyinv = mKEk[1][rl.getAuxIndex2()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEj[2][rl.getIndex()];
        az = mAEj[2][rl.getAuxIndex3()];
        bz = mBEj[2][rl.getAuxIndex3()];
        kzinv =  mKEj[2][rl.getAuxIndex3()];
    }
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
    //LOG << ii < rlLength << "\n";
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		__builtin_prefetch(Ex+PREFETCH);
		__builtin_prefetch(Jx+PREFETCH);
        float dHz = (*HzP++ - *HzN++)*dyinv;
        float dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
			__builtin_prefetch(psiY+PREFETCH);
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
			__builtin_prefetch(psiZ+PREFETCH);
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jx = *Jx*j1 + *Ex*j2;
        
        Ex++;
        Jx++;
    }
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEx_split(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
	float* Ex;
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    
    float* Jx;
	
    float* psiY;
    float* psiZ;
    
    float ay;
    float az;
    float by;
    float bz;
    float kzinv = 1/1.0;
    float kyinv = 1/1.0;
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
    //LOG << ii < rlLength << "\n";
	int rlLength = rl.getLength();
	int ii;
	
	// everything depending on dHz and dHy
	
    if (Y_ATTENUATION)
    {   
        psiY = &mAccumEk[1][rl.getIndex()];
        ay = mAEk[1][rl.getAuxIndex2()];
        by = mBEk[1][rl.getAuxIndex2()];
        kyinv = mKEk[1][rl.getAuxIndex2()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEj[2][rl.getIndex()];
        az = mAEj[2][rl.getAuxIndex3()];
        bz = mBEj[2][rl.getAuxIndex3()];
        kzinv =  mKEj[2][rl.getAuxIndex3()];
    }
    Ex = rl.field0();
    Jx = &mJx[rl.getIndex()];
    for (ii = 0; ii < rlLength; ii++)
    {
		/*
		assert(!isnan(*HyP));
		assert(!isnan(*HyN));
		assert(!isnan(*HzP));
		assert(!isnan(*HzN));
		*/
        float dHz = (*HzP++ - *HzN++)*dyinv;
        float dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            psiZ++;
        }
        
        Ex++;
        Jx++;
    }

	// everything not depending on dHz and dHy
	if (Y_ATTENUATION)
        psiY = &mAccumEk[1][rl.getIndex()];
    if (Z_ATTENUATION)
        psiZ = &mAccumEj[2][rl.getIndex()];
    Ex = rl.field0();
    Jx = &mJx[rl.getIndex()];
    for (ii = 0; ii < rlLength; ii++)
    {
        if (Y_ATTENUATION)
        {
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        *Jx = *Jx*j1 + *Ex*j2;
        Jx++;
		Ex++;
    }
	/*
    Ex = rl.field0();
    Jx = &mJx[rl.getIndex()];
    for (ii = 0; ii < rlLength; ii++)
    {
        *Jx = *Jx*j1 + *Ex*j2;
        
		Jx++;
		Ex++;
    }
	*/
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEx_unrolled(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ex = rl.field0();
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    
    float* Jx = &mJx[rl.getIndex()];
	
    float* psiY;
    float* psiZ;
    
    float ay;
    float az;
    float by;
    float bz;
    float kzinv = 1/1.0;
    float kyinv = 1/1.0;
	
    
    if (Y_ATTENUATION)
    {   
        psiY = &mAccumEk[1][rl.getIndex()];
        ay = mAEk[1][rl.getAuxIndex2()];
        by = mBEk[1][rl.getAuxIndex2()];
        kyinv = mKEk[1][rl.getAuxIndex2()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEj[2][rl.getIndex()];
        az = mAEj[2][rl.getAuxIndex3()];
        bz = mBEj[2][rl.getAuxIndex3()];
        kzinv =  mKEj[2][rl.getAuxIndex3()];
    }
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
    //LOG << ii < rlLength << "\n";
	int rlLength = rl.getLength();
	int ii;
	
    for (ii = 0; ii < rlLength%4; ii++)
    {
        float dHz = (*HzP++ - *HzN++)*dyinv;
        float dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jx = *Jx*j1 + *Ex*j2;
        
        Ex++;
        Jx++;
    }
	
    for ( ; ii < rlLength; ii+=4)
    {
		float dHz, dHy;
		
        dHz = (*HzP++ - *HzN++)*dyinv;
        dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jx = *Jx*j1 + *Ex*j2;
        
        Ex++;
        Jx++;

        dHz = (*HzP++ - *HzN++)*dyinv;
        dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jx = *Jx*j1 + *Ex*j2;
        
        Ex++;
        Jx++;

        dHz = (*HzP++ - *HzN++)*dyinv;
        dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jx = *Jx*j1 + *Ex*j2;
        
        Ex++;
        Jx++;

        dHz = (*HzP++ - *HzN++)*dyinv;
        dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jx = *Jx*j1 + *Ex*j2;
        
        Ex++;
        Jx++;
    }
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEx_split_unrolled(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
	float* Ex;
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    
    float* Jx;
	
    float* psiY;
    float* psiZ;
    
    float ay;
    float az;
    float by;
    float bz;
    float kzinv = 1/1.0;
    float kyinv = 1/1.0;
	
    
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
    //LOG << ii < rlLength << "\n";
	int rlLength = rl.getLength();
	int ii;
	
	// everything depending on dHz and dHy
	
    if (Y_ATTENUATION)
    {   
        psiY = &mAccumEk[1][rl.getIndex()];
        ay = mAEk[1][rl.getAuxIndex2()];
        by = mBEk[1][rl.getAuxIndex2()];
        kyinv = mKEk[1][rl.getAuxIndex2()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEj[2][rl.getIndex()];
        az = mAEj[2][rl.getAuxIndex3()];
        bz = mBEj[2][rl.getAuxIndex3()];
        kzinv =  mKEj[2][rl.getAuxIndex3()];
    }
    Ex = rl.field0();
    Jx = &mJx[rl.getIndex()];
    for (ii = 0; ii < rlLength%4; ii++)
    {
        float dHz = (*HzP++ - *HzN++)*dyinv;
        float dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            psiZ++;
        }
        
        Ex++;
        Jx++;
    }
    for ( ; ii < rlLength; ii+=4)
    {
		float dHz, dHy;
		
        dHz = (*HzP++ - *HzN++)*dyinv;
        dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            psiZ++;
        }
        
        Ex++;
        Jx++;
        dHz = (*HzP++ - *HzN++)*dyinv;
        dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            psiZ++;
        }
        
        Ex++;
        Jx++;
        dHz = (*HzP++ - *HzN++)*dyinv;
        dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            psiZ++;
        }
        
        Ex++;
        Jx++;
        dHz = (*HzP++ - *HzN++)*dyinv;
        dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            psiZ++;
        }
        
        Ex++;
        Jx++;
    }

	// everything not depending on dHz and dHy
	if (Y_ATTENUATION)
        psiY = &mAccumEk[1][rl.getIndex()];
    if (Z_ATTENUATION)
        psiZ = &mAccumEj[2][rl.getIndex()];
    Ex = rl.field0();
    for (ii = 0; ii < rlLength%4; ii++)
    {
        if (Y_ATTENUATION)
        {
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
		Ex++;
    }

    for ( ; ii < rlLength; ii+=4)
    {
        if (Y_ATTENUATION)
        {
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
		Ex++;
        if (Y_ATTENUATION)
        {
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
		Ex++;
        if (Y_ATTENUATION)
        {
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
		Ex++;
        if (Y_ATTENUATION)
        {
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
		Ex++;
    }
}





template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ex = rl.field0();
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    
    float* Jx = &mJx[rl.getIndex()];
	
    float* psiY;
    float* psiZ;
    
    float ay;
    float az;
    float by;
    float bz;
    float kzinv = 1/1.0;
    float kyinv = 1/1.0;
	
    
    if (Y_ATTENUATION)
    {   
        psiY = &mAccumEk[1][rl.getIndex()];
        ay = mAEk[1][rl.getAuxIndex2()];
        by = mBEk[1][rl.getAuxIndex2()];
        kyinv = mKEk[1][rl.getAuxIndex2()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEj[2][rl.getIndex()];
        az = mAEj[2][rl.getAuxIndex3()];
        bz = mBEj[2][rl.getAuxIndex3()];
        kzinv =  mKEj[2][rl.getAuxIndex3()];
    }
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
    //LOG << ii < rlLength << "\n";
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        float dHz = (*HzP++ - *HzN++)*dyinv;
        float dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex - *Jx*dt_epsinf + dHz*dt_epsinf*kyinv - dHy*dt_epsinf*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jx = *Jx*j1 + *Ex*j2;
        
        Ex++;
        Jx++;
    }
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEy_split(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ey = rl.field0();
    const float* HzN = rl.field1Neg();
    const float* HzP = rl.field1Pos();
    const float* HxN = rl.field2Neg();
    const float* HxP = rl.field2Pos();
    
    float*  Jy = &mJy[rl.getIndex()];
	
    float* psiX;
    float* psiZ;
    
    float* ax;
    float* bx;
    float* pkxinv;
    float az;
    float bz;
    float kxinv = 1/1.0;
    float kzinv = 1/1.0;
	
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
	int rlLength = rl.getLength();
	
    if (X_ATTENUATION)
    {
        psiX = &mAccumEj[0][rl.getIndex()];
        
        ax = &mAEj[0][rl.getAuxIndex1()];
        bx = &mBEj[0][rl.getAuxIndex1()];
        pkxinv = &mKEj[0][rl.getAuxIndex1()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEk[2][rl.getIndex()];
        az = mAEk[2][rl.getAuxIndex3()];
        bz = mBEk[2][rl.getAuxIndex3()];
        kzinv =  mKEk[2][rl.getAuxIndex3()];
    }
    for (int ii = 0; ii < rlLength; ii++)
    {
		/*
		assert(!isnan(*HxP));
		assert(!isnan(*HxN));
		assert(!isnan(*HzP));
		assert(!isnan(*HzN));
		*/
        float dHx = (*HxP++ - *HxN++)*dzinv;
        float dHz = (*HzP++ - *HzN++)*dxinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        
        *Ey = *Ey - *Jy*dt_epsinf + dHx*dt_epsinf*kzinv - dHz*dt_epsinf*kxinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHz;            psiX++;
            bx++;
            ax++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHx; // 1/2 timestep earlier
			psiZ++;
        }
        
        Ey++;
        Jy++;
    }
	
	
    if (X_ATTENUATION)
        psiX = &mAccumEj[0][rl.getIndex()];
    if (Z_ATTENUATION)
		psiZ = &mAccumEk[2][rl.getIndex()];
	
	Ey = rl.field0();
	Jy = &mJy[rl.getIndex()];

    for (int ii = 0; ii < rlLength; ii++)
    {
        if (X_ATTENUATION)
        {
            *Ey -= *psiX * dt_epsinf;
            psiX++;
		}
        
        if (Z_ATTENUATION)
        {
            *Ey += dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jy = *Jy*j1 + *Ey*j2;
        
        Ey++;
        Jy++;

    }
	/*
	Ey = rl.field0();
	Jy = &mJy[rl.getIndex()];
    for (int ii = 0; ii < rlLength; ii++)
    {
        //  takes place a half step later
        *Jy = *Jy*j1 + *Ey*j2;
        
        Ey++;
        Jy++;
    }
	*/
}




template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEy_unrolled(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ey = rl.field0();
    const float* HzN = rl.field1Neg();
    const float* HzP = rl.field1Pos();
    const float* HxN = rl.field2Neg();
    const float* HxP = rl.field2Pos();
    
    float*  Jy = &mJy[rl.getIndex()];
	
    float* psiX;
    float* psiZ;
    
    float* ax;
    float* bx;
    float* pkxinv;
    float az;
    float bz;
    float kxinv = 1/1.0;
    float kzinv = 1/1.0;
	
    
    if (X_ATTENUATION)
    {
        psiX = &mAccumEj[0][rl.getIndex()];
        
        ax = &mAEj[0][rl.getAuxIndex1()];
        bx = &mBEj[0][rl.getAuxIndex1()];
        pkxinv = &mKEj[0][rl.getAuxIndex1()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEk[2][rl.getIndex()];
        az = mAEk[2][rl.getAuxIndex3()];
        bz = mBEk[2][rl.getAuxIndex3()];
        kzinv =  mKEk[2][rl.getAuxIndex3()];
    }

    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
	int rlLength = rl.getLength();
	int ii;
	
    for (ii = 0; ii < rlLength%4; ii++)
    {
        float dHx = (*HxP++ - *HxN++)*dzinv;
        float dHz = (*HzP++ - *HzN++)*dxinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        
        *Ey = *Ey - *Jy*dt_epsinf + dHx*dt_epsinf*kzinv - dHz*dt_epsinf*kxinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHz;
            *Ey -= *psiX * dt_epsinf;
            psiX++;
            bx++;
            ax++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHx; // 1/2 timestep earlier
            *Ey += dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jy = *Jy*j1 + *Ey*j2;
        
        Ey++;
        Jy++;
    }

    for ( ; ii < rlLength; ii+=4)
    {
		float dHx, dHz;
		
        dHx = (*HxP++ - *HxN++)*dzinv;
        dHz = (*HzP++ - *HzN++)*dxinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        
        *Ey = *Ey - *Jy*dt_epsinf + dHx*dt_epsinf*kzinv - dHz*dt_epsinf*kxinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHz;
            *Ey -= *psiX * dt_epsinf;
            psiX++;
            bx++;
            ax++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHx; // 1/2 timestep earlier
            *Ey += dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jy = *Jy*j1 + *Ey*j2;
        
        Ey++;
        Jy++;
        dHx = (*HxP++ - *HxN++)*dzinv;
        dHz = (*HzP++ - *HzN++)*dxinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        
        *Ey = *Ey - *Jy*dt_epsinf + dHx*dt_epsinf*kzinv - dHz*dt_epsinf*kxinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHz;
            *Ey -= *psiX * dt_epsinf;
            psiX++;
            bx++;
            ax++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHx; // 1/2 timestep earlier
            *Ey += dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jy = *Jy*j1 + *Ey*j2;
        
        Ey++;
        Jy++;
        dHx = (*HxP++ - *HxN++)*dzinv;
        dHz = (*HzP++ - *HzN++)*dxinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        
        *Ey = *Ey - *Jy*dt_epsinf + dHx*dt_epsinf*kzinv - dHz*dt_epsinf*kxinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHz;
            *Ey -= *psiX * dt_epsinf;
            psiX++;
            bx++;
            ax++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHx; // 1/2 timestep earlier
            *Ey += dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jy = *Jy*j1 + *Ey*j2;
        
        Ey++;
        Jy++;
        dHx = (*HxP++ - *HxN++)*dzinv;
        dHz = (*HzP++ - *HzN++)*dxinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        
        *Ey = *Ey - *Jy*dt_epsinf + dHx*dt_epsinf*kzinv - dHz*dt_epsinf*kxinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHz;
            *Ey -= *psiX * dt_epsinf;
            psiX++;
            bx++;
            ax++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHx; // 1/2 timestep earlier
            *Ey += dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jy = *Jy*j1 + *Ey*j2;
        
        Ey++;
        Jy++;
    }
}




template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ey = rl.field0();
    const float* HzN = rl.field1Neg();
    const float* HzP = rl.field1Pos();
    const float* HxN = rl.field2Neg();
    const float* HxP = rl.field2Pos();
    
    float*  Jy = &mJy[rl.getIndex()];
	
    float* psiX;
    float* psiZ;
    
    float* ax;
    float* bx;
    float* pkxinv;
    float az;
    float bz;
    float kxinv = 1/1.0;
    float kzinv = 1/1.0;
	
    
    if (X_ATTENUATION)
    {
        psiX = &mAccumEj[0][rl.getIndex()];
        
        ax = &mAEj[0][rl.getAuxIndex1()];
        bx = &mBEj[0][rl.getAuxIndex1()];
        pkxinv = &mKEj[0][rl.getAuxIndex1()];
    }
    
    if (Z_ATTENUATION)
    {
        psiZ = &mAccumEk[2][rl.getIndex()];
        az = mAEk[2][rl.getAuxIndex3()];
        bz = mBEk[2][rl.getAuxIndex3()];
        kzinv =  mKEk[2][rl.getAuxIndex3()];
    }

    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        float dHx = (*HxP++ - *HxN++)*dzinv;
        float dHz = (*HzP++ - *HzN++)*dxinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        
        *Ey = *Ey - *Jy*dt_epsinf + dHx*dt_epsinf*kzinv - dHz*dt_epsinf*kxinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHz;
            *Ey -= *psiX * dt_epsinf;
            psiX++;
            bx++;
            ax++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHx; // 1/2 timestep earlier
            *Ey += dt_epsinf*(*psiZ); // current timestep
            psiZ++;
        }
        
        //  takes place a half step later
        *Jy = *Jy*j1 + *Ey*j2;
        
        Ey++;
        Jy++;
    }
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEz_split(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ez = rl.field0();
    const float* HxN = rl.field1Neg();
    const float* HxP = rl.field1Pos();
    const float* HyN = rl.field2Neg();
    const float* HyP = rl.field2Pos();
    
    float* Jz = &mJz[rl.getIndex()];
	
    float* psiX;
    float* psiY;
    
    float* ax;
    float* bx;
    float* pkxinv;
    float ay;
    float by;
    float kxinv = 1/1.0;
    float kyinv = 1/1.0;
	
    
    if (X_ATTENUATION)
    {
        psiX = &mAccumEk[0][rl.getIndex()];
        ax = &mAEk[0][rl.getAuxIndex1()];
        bx = &mBEk[0][rl.getAuxIndex1()];
        pkxinv = &mKEk[0][rl.getAuxIndex1()];
    }
    
    if (Y_ATTENUATION)
    {
        psiY = &mAccumEj[1][rl.getIndex()];
        ay = mAEj[1][rl.getAuxIndex2()];
        by = mBEj[1][rl.getAuxIndex2()];
        kyinv = mKEj[1][rl.getAuxIndex2()];
    }
    
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		/*
		assert(!isnan(*HyP));
		assert(!isnan(*HyN));
		assert(!isnan(*HxP));
		assert(!isnan(*HxN));
		*/
        float dHy = (*HyP++ - *HyN++)*dxinv;
        float dHx = (*HxP++ - *HxN++)*dyinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        *Ez = *Ez - *Jz*dt_epsinf + dHy*dt_epsinf*kxinv - dHx*dt_epsinf*kyinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHy;
            psiX++;
            ax++;
            bx++;
        }
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHx; // 1/2 timestep earlier
            psiY++;
        }
        
        Ez++;
        Jz++;
    }
	
    if (X_ATTENUATION)
        psiX = &mAccumEk[0][rl.getIndex()];
	if (Y_ATTENUATION)
        psiY = &mAccumEj[1][rl.getIndex()];
	Ez = rl.field0();
    Jz = &mJz[rl.getIndex()];
    for (int ii = 0; ii < rlLength; ii++)
    {
		if (X_ATTENUATION)
        {
            *Ez += dt_epsinf*(*psiX);
            psiX++;
        }
        
        if (Y_ATTENUATION)
		{
            *Ez -= dt_epsinf*(*psiY); // current timestep
			psiY++;
		}
        
        
        //  takes place a half step later
        *Jz = *Jz*j1 + *Ez*j2;
		
		Jz++;
        Ez++;
    }
/*
	Ez = rl.field0();
    Jz = &mJz[rl.getIndex()];
    for (int ii = 0; ii < rlLength; ii++)
    {
        
        Ez++;
        Jz++;
    }
*/
}



template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ez = rl.field0();
    const float* HxN = rl.field1Neg();
    const float* HxP = rl.field1Pos();
    const float* HyN = rl.field2Neg();
    const float* HyP = rl.field2Pos();
    
    float* Jz = &mJz[rl.getIndex()];
	
    float* psiX;
    float* psiY;
    
    float* ax;
    float* bx;
    float* pkxinv;
    float ay;
    float by;
    float kxinv = 1/1.0;
    float kyinv = 1/1.0;
	
    
    if (X_ATTENUATION)
    {
        psiX = &mAccumEk[0][rl.getIndex()];
        ax = &mAEk[0][rl.getAuxIndex1()];
        bx = &mBEk[0][rl.getAuxIndex1()];
        pkxinv = &mKEk[0][rl.getAuxIndex1()];
    }
    
    if (Y_ATTENUATION)
    {
        psiY = &mAccumEj[1][rl.getIndex()];
        ay = mAEj[1][rl.getAuxIndex2()];
        by = mBEj[1][rl.getAuxIndex2()];
        kyinv = mKEj[1][rl.getAuxIndex2()];
    }
    
    
    float j1 = m_j1;
    float j2 = m_j2;
    
    float dt_epsinf = dt / m_eps_inf;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        float dHy = (*HyP++ - *HyN++)*dxinv;
        float dHx = (*HxP++ - *HxN++)*dyinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        *Ez = *Ez - *Jz*dt_epsinf + dHy*dt_epsinf*kxinv - dHx*dt_epsinf*kyinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHy;
            *Ez += dt_epsinf*(*psiX);
            psiX++;
            ax++;
            bx++;
        }
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHx; // 1/2 timestep earlier
            
            *Ez -= dt_epsinf*(*psiY); // current timestep
            psiY++;
        }
        
        //  takes place a half step later
        *Jz = *Jz*j1 + *Ez*j2;
        
        Ez++;
        Jz++;
    }

}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepHx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hx = rl.field0();
    const float* EyP = rl.field1Pos();
    const float* EyN = rl.field1Neg();
    const float* EzP = rl.field2Pos();
    const float* EzN = rl.field2Neg();
    
    float* phiY;
    float* phiZ;
    
    float ay;
    float az;
    float by;
    float bz;
    float kzinv = 1/1.0;
    float kyinv = 1/1.0;
	
    
    if (Y_ATTENUATION)
    {   
        phiY = &mAccumHk[1][rl.getIndex()];
        ay = mAHk[1][rl.getAuxIndex2()];
        by = mBHk[1][rl.getAuxIndex2()];
        kyinv = mKHk[1][rl.getAuxIndex2()];
    }
    
    if (Z_ATTENUATION)
    {
        phiZ = &mAccumHj[2][rl.getIndex()];
        az = mAHj[2][rl.getAuxIndex3()];
        bz = mBHj[2][rl.getAuxIndex3()];
        kzinv =  mKHj[2][rl.getAuxIndex3()];
    }
    
    float dt_mu = dt / m_mu;
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		/*
		assert(!isnan(*EyP));
		assert(!isnan(*EyN));
		assert(!isnan(*EzP));
		assert(!isnan(*EzN));
		*/
        float dEz = (*EzP++ - *EzN++)*dyinv;
        float dEy = (*EyP++ - *EyN++)*dzinv;
    
        // half-timestep earlier
        
        // current timestep
        
        *Hx = *Hx - dEz*dt_mu*kyinv + dEy*dt_mu*kzinv;
        
        if (Y_ATTENUATION)
        {
            *phiY = by * *phiY + ay * dEz;  // 1/2 timestep earlier
            
            //LOG << "-phiY = " << -*phiY << " hx = " << *Hx << "\n";
            
            *Hx -= dt_mu*(*phiY); // current timestep
            phiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *phiZ = bz * *phiZ + az * dEy; // 1/2 timestep earlier
            *Hx += dt_mu*(*phiZ); // current timestep
            phiZ++;
        }
        
        Hx++;
    }
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepHy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hy = rl.field0();
    const float* EzP = rl.field1Pos();
    const float* EzN = rl.field1Neg();
    const float* ExP = rl.field2Pos();
    const float* ExN = rl.field2Neg();
    
    float* phiX;
    float* phiZ;
    
    float* ax;
    float* bx;
    float* pkxinv;
    float az;
    float bz;
    float kxinv = 1/1.0;
    float kzinv = 1/1.0;
	
    
    if (X_ATTENUATION)
    {
        phiX = &mAccumHj[0][rl.getIndex()];
        
        ax = &mAHj[0][rl.getAuxIndex1()];
        bx = &mBHj[0][rl.getAuxIndex1()];
        pkxinv = &mKHj[0][rl.getAuxIndex1()];
    }
    
    if (Z_ATTENUATION)
    {   
        phiZ = &mAccumHk[2][rl.getIndex()];
        az = mAHk[2][rl.getAuxIndex3()];
        bz = mBHk[2][rl.getAuxIndex3()];
        kzinv =  mKHk[2][rl.getAuxIndex3()];
    }
    
        
    float dt_mu = dt / m_mu;    
    
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		/*
		assert(!isnan(*ExP));
		assert(!isnan(*ExN));
		assert(!isnan(*EzP));
		assert(!isnan(*EzN));
		*/
		
        float dEx = (*ExP++ - *ExN++)*dzinv;
        float dEz = (*EzP++ - *EzN++)*dxinv;
    
        // half-timestep earlier
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        *Hy = *Hy - dEx*dt_mu*kzinv + dEz*dt_mu*kxinv;
        
        if (X_ATTENUATION)
        {
            *phiX = *bx * *phiX + *ax * dEz;
            *Hy += *phiX*dt_mu;
            phiX++;
            ax++;
            bx++;
        }
        
        if (Z_ATTENUATION)
        {
            *phiZ = bz * *phiZ + az * dEx;  // 1/2 timestep earlier
            *Hy -= dt_mu*(*phiZ); // current timestep
            phiZ++;
        }
        
        Hy++;
    }
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void DrudeMetalPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepHz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Hz = rl.field0();
    const float* ExP = rl.field1Pos();
    const float* ExN = rl.field1Neg();
    const float* EyP = rl.field2Pos();
    const float* EyN = rl.field2Neg();
    
    float* phiX;
    float* phiY;
    
    float* ax;
    float* bx;
    float* pkxinv;
    float ay;
    float by;
    float kyinv = 1/1.0;
    float kxinv = 1/1.0;
	
    
    if (X_ATTENUATION)
    {
        phiX = &mAccumHk[0][rl.getIndex()];
        ax = &mAHk[0][rl.getAuxIndex1()];
        bx = &mBHk[0][rl.getAuxIndex1()];
        pkxinv = &mKHk[0][rl.getAuxIndex1()];
    }
    
    if (Y_ATTENUATION)
    {
        phiY = &mAccumHj[1][rl.getIndex()];
        ay = mAHj[1][rl.getAuxIndex2()];
        by = mBHj[1][rl.getAuxIndex2()];
        kyinv = mKHj[1][rl.getAuxIndex2()];
    }
    
    float dt_mu = dt / m_mu;

    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
		/*
		assert(!isnan(*EyP));
		assert(!isnan(*EyN));
		assert(!isnan(*ExP));
		assert(!isnan(*ExN));
		*/
		
        float dEy = (*EyP++ - *EyN++)*dxinv;
        float dEx = (*ExP++ - *ExN++)*dyinv;
    
        // half-timestep earlier
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        *Hz = *Hz - dEy*dt_mu*kxinv + dEx*dt_mu*kyinv;
        
        if (X_ATTENUATION)
        {
            *phiX = *bx * *phiX + *ax * dEy;
            *Hz -= *phiX*dt_mu;
            phiX++;
            ax++;
            bx++;
        }
        
        if (Y_ATTENUATION)
        {
            *phiY = by * *phiY + ay * dEx; // 1/2 timestep earlier
            
            //LOG << "phiY = " << *phiY << " hz = " << *Hz << "\n";
            
            *Hz += dt_mu*(*phiY); // current timestep
            phiY++;
        }
        
        Hz++;
    }

}


#endif

