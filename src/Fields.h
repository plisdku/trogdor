/*
 *  Fields.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/29/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _FIELDS_
#define _FIELDS_

#include "Pointer.h"
#include "geometry.h"
#include <vector>

class Fields
{
public:
    Fields(int nnx, int nny, int nnz);
    virtual ~Fields();
    
    int get_nx() const { return m_nx; }
    int get_ny() const { return m_ny; }
    int get_nz() const { return m_nz; }
    
    int get_nnx() const { return m_nnx; }
    int get_nny() const { return m_nny; }
    int get_nnz() const { return m_nnz; }
    
    float* getEx() const { return &(mEx[0]); }
    float* getEy() const { return &(mEy[0]); }
    float* getEz() const { return &(mEz[0]); }
    float* getHx() const { return &(mHx[0]); }
    float* getHy() const { return &(mHy[0]); }
    float* getHz() const { return &(mHz[0]); }
    float* getField(int ii, int jj, int kk);
    
    /**
     *  Get the E or H field beginning at a point in the Yee cell
     *
     *  @param ii   half-cell index in x (must be 1 or 0)
     *  @param jj   half-cell index in y (must be 1 or 0)
     *  @param kk   half-cell index in z (must be 1 or 0)
     *  @returns    the beginning of the E or H field array
     */
    float* getFieldByOffset(int ii, int jj, int kk);
	float* getFieldByOffset(Vector3i pt);
    
    //  Range-checked getters
    float* getEx(int index) const;
    float* getEy(int index) const;
    float* getEz(int index) const;
    float* getHx(int index) const;
    float* getHy(int index) const;
    float* getHz(int index) const;
    
    float& Ex(int i, int j, int k);
    float& Ey(int i, int j, int k);
    float& Ez(int i, int j, int k);
    float& Hx(int i, int j, int k);
    float& Hy(int i, int j, int k);
    float& Hz(int i, int j, int k);
    
    void sanityCheck();   //  look for Inf, NaN, etc.
	void writeCoordinates();
	void assertCoordinates();
	
	void assertZero(int fieldNum);
    
private:
    int m_nnx;
    int m_nny;
    int m_nnz;
    int m_nx;
    int m_ny;
    int m_nz;
    
    //  Fields
	mutable std::vector<float> mEx;
	mutable std::vector<float> mEy;
	mutable std::vector<float> mEz;
	mutable std::vector<float> mHx;
	mutable std::vector<float> mHy;
	mutable std::vector<float> mHz;
	
	// Field LUT
	float* mFields[8];
};

typedef Pointer<Fields> FieldsPtr;



#endif
