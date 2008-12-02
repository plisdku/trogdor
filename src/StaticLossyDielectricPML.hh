/*
 *  StaticLossyDielectricPML.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/16/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef _STATICLOSSYDIELECTRICPML_

#include "StaticLossyDielectricPML.h"

#include "PhysicalConstants.h"
#include "StreamFromString.h"

using namespace std;

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
StaticLossyDielectricPML(const Map<std::string, std::string> & inParams, 
    Vector3i inDirection, const Rect3i & inPMLExtent,
    float dx, float dy, float dz, float dt) :
    MaterialModel(inParams),
	mAttenuation(X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION),
    mExtents(inPMLExtent),
    m_epsr(1.0),
    m_mur(1.0)
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
    
    if (inParams.count("epsr"))
        inParams["epsr"] >> m_epsr;
    if (inParams.count("mur"))
        inParams["mur"] >> m_mur;
	if (inParams.count("sigma"))
		inParams["sigma"] >> m_sigma;
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
		
		mAEj[dir].resize((halfCells+1)/2);
		mAEk[dir].resize((halfCells+1)/2);
		mAHj[dir].resize((halfCells+1)/2);
		mAHk[dir].resize((halfCells+1)/2);
		
		mBEj[dir].resize((halfCells+1)/2);
		mBEk[dir].resize((halfCells+1)/2);
		mBHj[dir].resize((halfCells+1)/2);
		mBHk[dir].resize((halfCells+1)/2);
		
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
    
	/*
    if (X_ATTENUATION) // i think this will always be true
    {
        sigmaMax = sigmaCoeff*(m+1.0)/(eta*dx);
        halfCells = inPMLExtent.p2[0] - inPMLExtent.p1[0] + 1;
        
        mAEy.resize((halfCells+1)/2);
        mAEz.resize((halfCells+1)/2);
        mAHy.resize((halfCells+1)/2);
        mAHz.resize((halfCells+1)/2);
        
        mBEy.resize((halfCells+1)/2);
        mBEz.resize((halfCells+1)/2);
        mBHy.resize((halfCells+1)/2);
        mBHz.resize((halfCells+1)/2);

        mKEy.resize((halfCells+1)/2, 1.0);
        mKEz.resize((halfCells+1)/2, 1.0);
        mKHy.resize((halfCells+1)/2, 1.0);
        mKHz.resize((halfCells+1)/2, 1.0);
        
        vector<float> sigma(halfCells, 0.0);
        vector<float> kappa(halfCells, 1.0);
        vector<float> alpha(halfCells, 0.0);
        
        if (inDirection[0] < 0)
        for (ii = 0; ii < halfCells; ii++)
        {
            sigma[ii] = sigmaMax*pow(float(halfCells-ii)/halfCells, m);
            kappa[ii] = 1.0 +
                (kappaMax-1.0)*pow(float(halfCells-ii)/halfCells, m);
            alpha[ii] = alphaMax*pow(float(ii)/halfCells, mAlpha);
        }
        else
        for (ii = 1; ii <= halfCells; ii++)
        {
            sigma[ii-1] = sigmaMax*pow(float(ii)/halfCells, m);
            kappa[ii-1] = 1.0 + (kappaMax-1.0)*pow(float(ii)/halfCells, m);
            alpha[ii-1] = alphaMax*pow(float(halfCells-ii)/halfCells, mAlpha);
        }
        
        for (ii = 0; ii < halfCells; ii++)
        {
            float ax = (sigma[ii]/(sigma[ii]*kappa[ii]
                + kappa[ii]*kappa[ii]*alpha[ii]))
                * (exp(-(dt/Constants::eps0)
                * (sigma[ii]/kappa[ii] + alpha[ii])) - 1);
            
            float bx = exp(-(dt/Constants::eps0)
                *(sigma[ii]/kappa[ii] + alpha[ii]));
            
            
            if ( (inPMLExtent.p1[0] + ii)%2 == 0)
            {
                mAEy[ii/2] = ax;
                mAEz[ii/2] = ax;
                mBEy[ii/2] = bx;
                mBEz[ii/2] = bx;
                mKEy[ii/2] = 1.0/kappa[ii];
                mKEz[ii/2] = 1.0/kappa[ii];
            }
            else
            {
                mAHy[ii/2] = ax;
                mAHz[ii/2] = ax;
                mBHy[ii/2] = bx;
                mBHz[ii/2] = bx;
                mKHy[ii/2] = 1.0/kappa[ii];
                mKHz[ii/2] = 1.0/kappa[ii];
            }
        }
        
    }
    
    //   Y conductivity
    //LOGMORE << "sigma Y... " << flush;
    
    if (Y_ATTENUATION)
    {
        sigmaMax = sigmaCoeff*(m+1.0)/(eta*dy);
        halfCells = inPMLExtent.p2[1] - inPMLExtent.p1[1] + 1;
        
        mAEx.resize((halfCells+1)/2);
        mAEz.resize((halfCells+1)/2);
        mAHx.resize((halfCells+1)/2);
        mAHz.resize((halfCells+1)/2);
        
        mBEx.resize((halfCells+1)/2);
        mBEz.resize((halfCells+1)/2);
        mBHx.resize((halfCells+1)/2);
        mBHz.resize((halfCells+1)/2);
        
        mKEx.resize((halfCells+1)/2);
        mKEz.resize((halfCells+1)/2);
        mKHx.resize((halfCells+1)/2);
        mKHz.resize((halfCells+1)/2);
        
        vector<float> sigma(halfCells, 0.0);
        vector<float> kappa(halfCells, 1.0);
        vector<float> alpha(halfCells, 0.0);
        
        if (inDirection[1] < 0)
        for (ii = 0; ii < halfCells; ii++)
        {
            sigma[ii] = sigmaMax*pow(float(halfCells-ii)/halfCells, m);
            kappa[ii] = 1.0 +
                (kappaMax-1.0)*pow(float(halfCells-ii)/halfCells, m);
            alpha[ii] = alphaMax*pow(float(ii)/halfCells, mAlpha);
        }
        else
        for (ii = 1; ii <= halfCells; ii++)
        {
            sigma[ii-1] = sigmaMax*pow(float(ii)/halfCells, m);
            kappa[ii-1] = 1.0 + (kappaMax-1.0)*pow(float(ii)/halfCells, m);
            alpha[ii-1] = alphaMax*pow(float(halfCells-ii)/halfCells, mAlpha);
        }
        
        //  How the conductivity values are loaded depends on which side of the
        //  Yee cell the PML begins on.
        
        for (ii = 0; ii < halfCells; ii++)
        {
            float aa = (sigma[ii]/(sigma[ii]*kappa[ii]
                + kappa[ii]*kappa[ii]*alpha[ii]))
                * (exp(-(dt/Constants::eps0)
                * (sigma[ii]/kappa[ii] + alpha[ii])) - 1);
            
            float bb = exp(-(dt/Constants::eps0)
                *(sigma[ii]/kappa[ii] + alpha[ii]));
            
            
            if ( (inPMLExtent.p1[1] + ii)%2 == 0)
            {
                mAEx[ii/2] = aa;
                mBEx[ii/2] = bb;
                mKEx[ii/2] = 1.0/kappa[ii];
                
                mAEz[ii/2] = aa;
                mBEz[ii/2] = bb;
                mKEz[ii/2] = 1.0/kappa[ii];
            }
            else
            {
                mAHx[ii/2] = aa;
                mBHx[ii/2] = bb;
                mKHx[ii/2] = 1.0/kappa[ii];
                
                mAHz[ii/2] = aa;
                mBHz[ii/2] = bb;
                mKHz[ii/2] = 1.0/kappa[ii];
            }
        }
    }
    
    //  Z conductivity
    //LOGMORE << "sigma Z... " << flush;
    if (Z_ATTENUATION)
    {
        sigmaMax = sigmaCoeff*(m+1.0)/(eta*dz);
        halfCells = inPMLExtent.p2[2] - inPMLExtent.p1[2] + 1;
        
        mAEy.resize((halfCells+1)/2);
        mAEx.resize((halfCells+1)/2);
        mAHy.resize((halfCells+1)/2);
        mAHx.resize((halfCells+1)/2);
        
        mBEy.resize((halfCells+1)/2);
        mBEx.resize((halfCells+1)/2);
        mBHy.resize((halfCells+1)/2);
        mBHx.resize((halfCells+1)/2);
        
        mKEy.resize((halfCells+1)/2);
        mKEx.resize((halfCells+1)/2);
        mKHy.resize((halfCells+1)/2);
        mKHx.resize((halfCells+1)/2);
        
        vector<float> sigma(halfCells, 0.0);
        vector<float> kappa(halfCells, 1.0);
        vector<float> alpha(halfCells, 0.0);
        
        if (inDirection[2] < 0)
        for (ii = 0; ii < halfCells; ii++)
        {
            sigma[ii] = sigmaMax*pow(float(halfCells-ii)/halfCells, m);
            kappa[ii] = 1.0 +
                (kappaMax-1.0)*pow(float(halfCells-ii)/halfCells, m);
            alpha[ii] = alphaMax*pow(float(ii)/halfCells, mAlpha);
        }
        else
        for (ii = 1; ii <= halfCells; ii++)
        {
            sigma[ii-1] = sigmaMax*pow(float(ii)/halfCells, m);
            kappa[ii-1] = 1.0 + (kappaMax-1.0)*pow(float(ii)/halfCells, m);
            alpha[ii-1] = alphaMax*pow(float(halfCells-ii)/halfCells, mAlpha);
        }
        
        //  How the conductivity values are loaded depends on which side of the
        //  Yee cell the PML begins on.
        
        for (ii = 0; ii < halfCells; ii++)
        {
            float aa = (sigma[ii]/(sigma[ii]*kappa[ii]
                + kappa[ii]*kappa[ii]*alpha[ii]))
                * (exp(-(dt/Constants::eps0)
                * (sigma[ii]/kappa[ii] + alpha[ii])) - 1);
            
            float bb = exp(-(dt/Constants::eps0)
                *(sigma[ii]/kappa[ii] + alpha[ii]));
            
            //LOG << "aa[" << ii << "] = " << aa << " and bb[" << ii << "] = "
            //    << bb << "\n";
            
            
            if ( (inPMLExtent.p1[2] + ii)%2 == 0)
            {
                mAEx[ii/2] = aa;
                mBEx[ii/2] = bb;
                mKEx[ii/2] = 1.0/kappa[ii];
                
                mAEy[ii/2] = aa;
                mBEy[ii/2] = bb;
                mKEy[ii/2] = 1.0/kappa[ii];
            }
            else
            {
                mAHx[ii/2] = aa;
                mBHx[ii/2] = bb;
                mKHx[ii/2] = 1.0/kappa[ii];
                
                mAHy[ii/2] = aa;
                mBHy[ii/2] = bb;
                mKHy[ii/2] = 1.0/kappa[ii];
            }
        }
    }
	*/
    
    //LOGMORE << "done." << endl;
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
~StaticLossyDielectricPML()
{
    LOGF << "Destructor.\n";
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
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
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
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
    //LOGMORE << "done.\n";
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
string StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
getMaterialName() const
{
	ostringstream str;
	str << "Static Lossy Dielectric PML " << X_ATTENUATION << " "
		<< Y_ATTENUATION << " " << Z_ATTENUATION;
	return str.str();
}


template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcEx(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEx.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt);
		stepEx(mRunlinesEx[ii], dxinv, dyinv, dzinv, dt);
	}
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcEy(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEy.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE(mRunlinesEy[ii], dyinv, dzinv, dxinv, dt);
		stepEy(mRunlinesEy[ii], dxinv, dyinv, dzinv, dt);
	}
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
calcEz(float dxinv, float dyinv, float dzinv, float dt,
	int start, int stride)
{
	int len = mRunlinesEz.size();
	for (int ii = start; ii < len; ii += stride)
	{
		//stepE(mRunlinesEz[ii], dzinv, dxinv, dyinv, dt);
		stepEz(mRunlinesEz[ii], dxinv, dyinv, dzinv, dt);
	}
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
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
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
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
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
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
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEx(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ex = rl.field0();
    const float* HyN = rl.field1Neg();
    const float* HyP = rl.field1Pos();
    const float* HzN = rl.field2Neg();
    const float* HzP = rl.field2Pos();
    
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
    
    float ce = (2*m_epsr*Constants::eps0 - m_sigma*dt) /
        (2*m_epsr*Constants::eps0 + m_sigma*dt);
    float ch = 2*dt/(2*m_epsr*Constants::eps0 + m_sigma*dt);
    
    //LOG << ii < rlLength << "\n";
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        float dHz = (*HzP++ - *HzN++)*dyinv;
        float dHy = (*HyP++ - *HyN++)*dzinv;
        
        // current timestep
        *Ex = *Ex + dHz*ch*kyinv - dHy*ch*kzinv;
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHz; // 1/2 timestep earlier
            
            *Ex += ch*(*psiY); // current timestep
            psiY++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHy; // 1/2 timestep earlier
            *Ex -= ch*(*psiZ); // current timestep
            psiZ++;
        }
        
        Ex++;
    }
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEy(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ey = rl.field0();
    const float* HzN = rl.field1Neg();
    const float* HzP = rl.field1Pos();
    const float* HxN = rl.field2Neg();
    const float* HxP = rl.field2Pos();
    
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

    float ce = (2*m_epsr*Constants::eps0 - m_sigma*dt) /
        (2*m_epsr*Constants::eps0 + m_sigma*dt);
    float ch = 2*dt/(2*m_epsr*Constants::eps0 + m_sigma*dt);
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        float dHx = (*HxP++ - *HxN++)*dzinv;
        float dHz = (*HzP++ - *HzN++)*dxinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
        
        *Ey = *Ey*ce + dHx*ch*kzinv - dHz*ch*kxinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHz;
            *Ey -= *psiX * ch;
            psiX++;
            bx++;
            ax++;
        }
        
        if (Z_ATTENUATION)
        {
            *psiZ = bz * *psiZ + az * dHx; // 1/2 timestep earlier
            *Ey += ch*(*psiZ); // current timestep
            psiZ++;
        }
        
        Ey++;
    }
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
stepEz(const Runline & rl, float dxinv, float dyinv, float dzinv, float dt)
{
    float* Ez = rl.field0();
    const float* HxN = rl.field1Neg();
    const float* HxP = rl.field1Pos();
    const float* HyN = rl.field2Neg();
    const float* HyP = rl.field2Pos();
    
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
    
    float ce = (2*m_epsr*Constants::eps0 - m_sigma*dt) /
        (2*m_epsr*Constants::eps0 + m_sigma*dt);
    float ch = 2*dt/(2*m_epsr*Constants::eps0 + m_sigma*dt);
	
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        float dHy = (*HyP++ - *HyN++)*dxinv;
        float dHx = (*HxP++ - *HxN++)*dyinv;
        
        // current timestep
        
        if (X_ATTENUATION)
            kxinv = *pkxinv++;
		
        *Ez = *Ez*ce + dHy*ch*kxinv - dHx*ch*kyinv;
        
        if (X_ATTENUATION)
        {
            *psiX = *bx * *psiX + *ax * dHy;
            *Ez += ch*(*psiX);
            psiX++;
            ax++;
            bx++;
        }
        
        if (Y_ATTENUATION)
        {
            *psiY = by * *psiY + ay * dHx; // 1/2 timestep earlier
            *Ez -= ch*(*psiY); // current timestep            
            psiY++;
        }
        
        Ez++;
    }
}

template <bool X_ATTENUATION, bool Y_ATTENUATION, bool Z_ATTENUATION>
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
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
    
    float dt_mu = dt / (Constants::mu0 * m_mur);
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
        float dEz = (*EzP++ - *EzN++)*dyinv;
        float dEy = (*EyP++ - *EyN++)*dzinv;
    
		// current timestep
        
        *Hx = *Hx - dEz*dt_mu*kyinv + dEy*dt_mu*kzinv;
        
        if (Y_ATTENUATION)
        {
            *phiY = by * *phiY + ay * dEz;  // 1/2 timestep earlier
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
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
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
    
    float dt_mu = dt / (Constants::mu0 * m_mur);
    
    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
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
void StaticLossyDielectricPML<X_ATTENUATION, Y_ATTENUATION, Z_ATTENUATION>::
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
    
    float dt_mu = dt / (Constants::mu0 * m_mur);

    
	int rlLength = rl.getLength();
    for (int ii = 0; ii < rlLength; ii++)
    {
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

