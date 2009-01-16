/*
 *  Fields.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/29/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "Fields.h"

#include <cassert>
#include <cmath>
using namespace std;


Fields::
Fields(int nnx, int nny, int nnz) :
    m_nnx(nnx),
    m_nny(nny),
    m_nnz(nnz),
	m_nx((nnx+1)/2),
	m_ny((nny+1)/2),
	m_nz((nnz+1)/2),
	mEx(m_nx*m_ny*m_nz, 0.0),
	mEy(m_nx*m_ny*m_nz, 0.0),
	mEz(m_nx*m_ny*m_nz, 0.0),
	mHx(m_nx*m_ny*m_nz, 0.0),
	mHy(m_nx*m_ny*m_nz, 0.0),
	mHz(m_nx*m_ny*m_nz, 0.0)
{
    LOGF << "constructor." << endl;
    
	
	mFields[0] = 0L;
	mFields[1] = &(mEx[0]);
	mFields[2] = &(mEy[0]);
	mFields[3] = &(mHz[0]);
	mFields[4] = &(mEz[0]);
	mFields[5] = &(mHy[0]);
	mFields[6] = &(mHx[0]);
	mFields[7] = 0L;
    
	/*
	LOG << "Fields:\n";
	LOG << hex;
	
	for (int nn = 1; nn < 7; nn++)
	{
		LOGMORE << mFields[nn] << " to " << mFields[nn] + nCells << "\n";
	}
	LOG << dec;
	*/
}


Fields::
~Fields()
{
    LOGF << "destructor." << endl;
}

float* Fields::
getField(int ii, int jj, int kk)
{
    float* field = 0L;
    
    if (ii%2 == 0)
    {
        if (jj%2 == 0)
        {
            if (kk%2 == 0)  //  0 0 0: empty
                field = 0L;
            else            //  0 0 1: Ez
                field = &(mEz[0]);
        }
        else
        {
            if (kk%2 == 0)  //  0 1 0: Ey
                field = &(mEy[0]);
            else            //  0 1 1: Hx
                field = &(mHx[0]);
        }
    }
    else
    {
        if (jj%2 == 0)
        {
            if (kk%2 == 0)  //  1 0 0: Ex
                field = &(mEx[0]);
            else            //  1 0 1: Hy
                field = &(mHy[0]);
        }
        else
        {
            if (kk%2 == 0)  //  1 1 0: Hz
                field = &(mHz[0]);
            else            //  1 1 1: empty
                field = 0L;
        }
    }
    
    if (field == 0L)
    {
        cerr << "Error in getField() with ii = " << ii << ", jj = " << jj
             << ", kk = " << kk << endl;
        assert(!"Ouch.\n");
        exit(1);
    }
    
    /*
    assert(i >= 0);
    assert(i < nx);
    assert(j >= 0);
    assert(j < ny);
    assert(k >= 0);
    assert(k < nz);
    */
    
    return field + (ii/2) + m_nx*(jj/2) + m_nx*m_ny*(kk/2);
}

float* Fields::
getFieldByOffset(int ii, int jj, int kk)
{
    float* field = 0L;
    
    if (ii == 0)
    {
        if (jj == 0)
        {
            if (kk == 0)  //  0 0 0: empty
                field = 0L;
            else if (kk == 1)           //  0 0 1: Ez
                field = &(mEz[0]);
        }
        else if (jj == 1)
        {
            if (kk == 0)  //  0 1 0: Ey
                field = &(mEy[0]);
            else if (kk == 1)           //  0 1 1: Hx
                field = &(mHx[0]);
        }
    }
    else if (ii == 1)
    {
        if (jj == 0)
        {
            if (kk == 0)  //  1 0 0: Ex
                field = &(mEx[0]);
            else if (kk == 1)            //  1 0 1: Hy
                field = &(mHy[0]);
        }
        else if (jj == 1)
        {
            if (kk == 0)  //  1 1 0: Hz
                field = &(mHz[0]);
            else if (kk == 1)            //  1 1 1: empty
                field = 0L;
        }
    }
    
    if (field == 0L)
    {
        cerr << "Error in getFieldByOffset() with ii = " << ii << ", jj = "
            << jj << ", kk = " << kk << endl;
        assert(!"Ouch.\n");
        exit(1);
    }
    
    return field;
}


float* Fields::
getFieldByOffset(Vector3i pt)
{
	pt[0] %= 2;
	pt[1] %= 2;
	pt[2] %= 2;
	
	int fieldNum = pt[0] + 2*pt[1] + 4*pt[2];
	assert(fieldNum >= 0 && fieldNum <= 7);
	assert(fieldNum != 0);
	assert(fieldNum != 7);
	
	return mFields[pt[0] + 2*pt[1] + 4*pt[2]];
}

float* Fields::getEx(int index) const
{
    int nCells = m_nx*m_ny*m_nz;
    assert(index < nCells && index >= 0);
    return &(mEx[index]);
}

float* Fields::getEy(int index) const
{
    int nCells = m_nx*m_ny*m_nz;
    assert(index < nCells && index >= 0);
    return &(mEy[index]);
}

float* Fields::getEz(int index) const
{
    int nCells = m_nx*m_ny*m_nz;
    assert(index < nCells && index >= 0);
    return &(mEz[index]);
}

float* Fields::getHx(int index) const
{
    int nCells = m_nx*m_ny*m_nz;
    assert(index < nCells && index >= 0);
    return &(mHx[index]);
}

float* Fields::getHy(int index) const
{
    int nCells = m_nx*m_ny*m_nz;
    assert(index < nCells && index >= 0);
    return &(mHy[index]);
}

float* Fields::getHz(int index) const
{
    int nCells = m_nx*m_ny*m_nz;
    assert(index < nCells && index >= 0);
    return &(mHz[index]);
}


inline float& getElement(float* f, int i, int j, int k, int nx, int ny, int nz)
{
	/*
    assert(i >= 0);
    assert(i < nx);
    assert(j >= 0);
    assert(j < ny);
    assert(k >= 0);
    assert(k < nz);
	*/
    return f[i + nx*j + nx*ny*k];
}

float& Fields::
Ex(int i, int j, int k)
{
    return getElement(&(mEx[0]), i, j, k, m_nx, m_ny, m_nz);
}

float& Fields::
Ey(int i, int j, int k)
{
    return getElement(&(mEy[0]), i, j, k, m_nx, m_ny, m_nz);
}

float& Fields::
Ez(int i, int j, int k)
{
    return getElement(&(mEz[0]), i, j, k, m_nx, m_ny, m_nz);
}

float& Fields::
Hx(int i, int j, int k)
{
    return getElement(&(mHx[0]), i, j, k, m_nx, m_ny, m_nz);
}

float& Fields::
Hy(int i, int j, int k)
{
    return getElement(&(mHy[0]), i, j, k, m_nx, m_ny, m_nz);
}

float& Fields::
Hz(int i, int j, int k)
{
    return getElement(&(mHz[0]), i, j, k, m_nx, m_ny, m_nz);
}

void Fields::
sanityCheck()
{
    for (int i = 0; i < m_nx; i++)
    for (int j = 0; j < m_ny; j++)
    for (int k = 0; k < m_nz; k++)
    {
        //cerr << i << " " << j << " " << k << endl;
        assert(!isnan(Ex(i,j,k)));
        assert(!isinf(Ex(i,j,k)));
        assert(!isnan(Ey(i,j,k)));
        assert(!isinf(Ey(i,j,k)));
        assert(!isnan(Ez(i,j,k)));
        assert(!isinf(Ez(i,j,k)));
        assert(!isnan(Hx(i,j,k)));
        assert(!isinf(Hx(i,j,k)));
        assert(!isnan(Hy(i,j,k)));
        assert(!isinf(Hy(i,j,k)));
        assert(!isnan(Hz(i,j,k)));
        assert(!isinf(Hz(i,j,k)));
    }
}

void Fields::
writeCoordinates()
{
    for (int i = 0; i < m_nx; i++)
    for (int j = 0; j < m_ny; j++)
    for (int k = 0; k < m_nz; k++)
    {
        //cerr << i << " " << j << " " << k << endl;
		float position = 1e0*i + 1e3*j + 1e6*k;
		
		Ex(i,j,k) = position;
		Ey(i,j,k) = position;
		Ez(i,j,k) = position;
		Hx(i,j,k) = position;
		Hy(i,j,k) = position;
		Hz(i,j,k) = position;
    }
}

void Fields::
assertCoordinates()
{
    for (int i = 0; i < m_nx; i++)
    for (int j = 0; j < m_ny; j++)
    for (int k = 0; k < m_nz; k++)
    {
        //cerr << i << " " << j << " " << k << endl;
		float position = 1e0*i + 1e3*j + 1e6*k;
		
		assert(Ex(i,j,k) == position);
		assert(Ey(i,j,k) == position);
		assert(Ez(i,j,k) == position);
		assert(Hx(i,j,k) == position);
		assert(Hy(i,j,k) == position);
		assert(Hz(i,j,k) == position);
    }
}

void Fields::
assertZero(int fieldNum)
{
	float* field = mFields[fieldNum];
	
    for (int i = 0; i < m_nx; i++)
    for (int j = 0; j < m_ny; j++)
    for (int k = 0; k < m_nz; k++)
    {
        //cerr << i << " " << j << " " << k << endl;
		assert(field[i + j*m_nx + k*m_nx*m_ny] == 0);
    }
}





