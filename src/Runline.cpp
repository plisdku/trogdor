/*
 *  Runline.cpp
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

#include "Runline.h"

#include "TFSFBufferSet.h"

#include <cstdlib>
#include <cassert>
#include <vector>

using namespace std;


Runline::
Runline(const RunlineType & inRunline, Fields & inFields) :
    mLength(inRunline.getLength()),
    mIndex(inRunline.materialIndex()),
	mField0(0L)
{
    int ii = inRunline.get_ii0();
    int jj = inRunline.get_jj0();
    int kk = inRunline.get_kk0();
    const IndexStencil & neighbors = inRunline.getNeighbors();
	
    int total_cells = inFields.get_nx()*inFields.get_ny()*inFields.get_nz();
    
    // Debugging chunk
    m_ii0 = ii;
    m_jj0 = jj;
    m_kk0 = kk;
	
    
    assert(inRunline.materialIndex() >= 0);
	assert(inRunline.getType().getName() != "Default");
    
	/*
    LOG << "Runline from " << ii << " " << jj << " " << kk << " length ";
    LOGMORE << inRunline.getLength() << ": first index "
        << inRunline.getSelfIndex() << "\n";
    neighbors.print(cout);
    */
	
    //  end chunk
    
    mField0 = inFields.getField(ii,jj,kk);
    
    //LOG << *mField0 << endl;
    
    if (inRunline.isTFSF())
        LOG << "Error: using the wrong Runline constructor for TFSF.\n";
    
	vector<Vector3i> cardinals;
	cardinals.push_back(Vector3i(1,0,0));
	cardinals.push_back(Vector3i(0,1,0));
	cardinals.push_back(Vector3i(0,0,1));
	
	Vector3i here(ii,jj,kk);
	int fieldDir;  // 0 = x, 1 = y, 2 = z   (this indicates Ex/Hx, Ey/Hy, etc.)
	if (jj % 2 == kk % 2)
		fieldDir = 0;
	else if (kk % 2 == ii % 2)
		fieldDir = 1;
	else if (ii % 2 == jj % 2)
		fieldDir = 2;
	else
		assert(!"Trubble.");
	
	int fieldDir_j = (fieldDir+1)%3;
	int fieldDir_k = (fieldDir+2)%3;
	
	// WATCH CAREFULLY
	// if fi is Ex, then fj is Hy, which is offset along fieldDir_k
	// because of Maxwell's equations.
	float* fi = inFields.getFieldByOffset(here);
	float* fj = inFields.getFieldByOffset(here+cardinals[fieldDir_k]);
	float* fk = inFields.getFieldByOffset(here+cardinals[fieldDir_j]);
	assert(fi != 0L);
	assert(fj != 0L);
	assert(fk != 0L);
	
	mNeighbors_j[0] = fj + neighbors.getIndex(2*fieldDir_k);
	mNeighbors_j[1] = fj + neighbors.getIndex(2*fieldDir_k+1);
	mNeighbors_k[0] = fk + neighbors.getIndex(2*fieldDir_j);
	mNeighbors_k[1] = fk + neighbors.getIndex(2*fieldDir_j+1);
	
	assert(mField0 >= fi);
	assert(mField0 + inRunline.getLength() <= fi + total_cells);
	
	assert(mNeighbors_j[0] + inRunline.getLength() <= fj + total_cells);
	assert(mNeighbors_j[1] + inRunline.getLength() <= fj + total_cells);
	assert(mNeighbors_k[0] + inRunline.getLength() <= fk + total_cells);
	assert(mNeighbors_k[1] + inRunline.getLength() <= fk + total_cells);
	
	assert(mNeighbors_j[0] >= fj);
	assert(mNeighbors_j[1] >= fj);
	assert(mNeighbors_k[0] >= fk);
	assert(mNeighbors_k[1] >= fk);
	
	// Debugging chunk
	/*
	vector<float*> ptrs; 
	ptrs.push_back(fi);
	ptrs.push_back(fj);
	ptrs.push_back(fk);
	LOG << "Fields: \n";
	for (int ll = 0; ll < 3; ll++)
	{
		float* ppp = ptrs[ll];
		if (ppp == inFields.getEx())
			LOGMORE << "Ex\n";
		else if (ppp == inFields.getEy())
			LOGMORE << "Ey\n";
		else if (ppp == inFields.getEz())
			LOGMORE << "Ez\n";
		else if (ppp == inFields.getHx())
			LOGMORE << "Hx\n";
		else if (ppp == inFields.getHy())
			LOGMORE << "Hy\n";
		else if (ppp == inFields.getHz())
			LOGMORE << "Hz\n";
		else
			LOGMORE << "UNKNOWN\n";
	}
	LOG << "done\n";
	*/
	
	/*
	LOG << "Pointers: \n";
	LOGMORE << mField0;
	for (int nn = 0; nn < 2; nn++)
	{
		LOGMORE << " " << mNeighbors_j[nn] << " " << mNeighbors_k[nn] << " ";
	}
	LOGMORE << "\n";
	*/
	
	//LOG << "index " << mIndex << "\n";
}

Runline::
Runline(const RunlineType & inRunline, Fields & inFields,
	TFSFBufferSet & inBuffer):
    mLength(inRunline.getLength()),
    mIndex(inRunline.materialIndex())
{
    int ii = inRunline.get_ii0();
    int jj = inRunline.get_jj0();
    int kk = inRunline.get_kk0();
    const IndexStencil & neighbors = inRunline.getNeighbors();
	const IndexStencil & bufferNeighbors = inRunline.getBufferNeighbors();
	const MaterialType & matType = inRunline.getType();
    
    int total_cells = inFields.get_nx()*inFields.get_ny()*inFields.get_nz();
    
    // Debugging chunk
    m_ii0 = ii;
    m_jj0 = jj;
    m_kk0 = kk;
	
    assert(inRunline.materialIndex() >= 0);
	assert(inRunline.getType().getName() != "Default");
    
    //LOG << "Runline from " << ii << " " << jj << " " << kk << " length ";
    //LOGMORE << inRunline.getLength() << ": first index "
    //    << inRunline.getSelfIndex() << "\n";
    //neighbors.print(cout);
	//bufferNeighbors.print(cout);
    //
    
    mField0 = inFields.getField(ii,jj,kk);
    
    //LOG << *mField0 << endl;
    
    if (!inRunline.isTFSF())
        LOG << "Error: using the wrong Runline constructor for non-TFSF.\n";
    
	vector<Vector3i> cardinals;
	cardinals.push_back(Vector3i(1,0,0));
	cardinals.push_back(Vector3i(0,1,0));
	cardinals.push_back(Vector3i(0,0,1));
	
	int fieldDir;  // 0 = x, 1 = y, 2 = z   (this indicates Ex/Hx, Ey/Hy, etc.)
	if (jj % 2 == kk % 2)
		fieldDir = 0;
	else if (kk % 2 == ii % 2)
		fieldDir = 1;
	else if (ii % 2 == jj % 2)
		fieldDir = 2;
	else
		assert(!"Trubble.");
		
	int fieldDir_j = (fieldDir+1)%3;
	int fieldDir_k = (fieldDir+2)%3;
		
	Vector3i here(ii,jj,kk);
	Vector3i here_j = here + cardinals[fieldDir_j];
	Vector3i here_k = here + cardinals[fieldDir_k];
	
	int low_j = 2*fieldDir_j;
	int high_j = low_j+1;
	int low_k = 2*fieldDir_k;
	int high_k = low_k+1;
	
	// CAREFUL: fj = field at here_k.
	float* fi = inFields.getFieldByOffset(here);
	float* fj = inFields.getFieldByOffset(here_k);
	float* fk = inFields.getFieldByOffset(here_j);
	assert(fi != 0L);
	assert(fj != 0L);
	assert(fk != 0L);
	
	float* buffer_ptr;
	
	// Remember the pattern: field j in neighboring direction k.
	
	if (bufferNeighbors.getIndex(low_j) == -1)
	{
		mNeighbors_k[0] = fk + neighbors.getIndex(low_j);
		assert(mNeighbors_k[0] + inRunline.getLength() <= fk + total_cells);
		assert(mNeighbors_k[0] >= fk);
	}
	else
	{
		buffer_ptr = inBuffer.getBufferPointer(
			matType.getBufferIndex(low_j), here_j);
		//LOG << "Low j Buffer pointer " << hex << buffer_ptr << dec << "\n";
		mNeighbors_k[0] = buffer_ptr + bufferNeighbors.getIndex(low_j);
		assert(mNeighbors_k[0] + inRunline.getLength() <= buffer_ptr +
			inBuffer.getNumBufferCells(matType.getBufferIndex(low_j), here_j));
		assert(mNeighbors_k[0] >= buffer_ptr);
	}
	if (bufferNeighbors.getIndex(high_j) == -1)
	{
		mNeighbors_k[1] = fk + neighbors.getIndex(high_j);
		assert(mNeighbors_k[1] + inRunline.getLength() <= fk + total_cells);
		assert(mNeighbors_k[1] >= fk);
	}
	else
	{
		buffer_ptr = inBuffer.getBufferPointer(
			matType.getBufferIndex(high_j), here_j);
		//LOG << "High j Buffer pointer " << hex << buffer_ptr << dec << "\n";
		mNeighbors_k[1] = buffer_ptr + bufferNeighbors.getIndex(high_j);
		assert(mNeighbors_k[1] + inRunline.getLength() <= buffer_ptr +
			inBuffer.getNumBufferCells(matType.getBufferIndex(high_j), here_j));
		assert(mNeighbors_k[1] >= buffer_ptr);
	}
	if (bufferNeighbors.getIndex(low_k) == -1)
	{
		mNeighbors_j[0] = fj + neighbors.getIndex(low_k);
		assert(mNeighbors_j[0] + inRunline.getLength() <= fj + total_cells);
		assert(mNeighbors_j[0] >= fj);
	}
	else
	{
		buffer_ptr = inBuffer.getBufferPointer(
			matType.getBufferIndex(low_k), here_k);
		//LOG << "Low k Buffer pointer " << hex << buffer_ptr << dec << "\n";
		mNeighbors_j[0] = buffer_ptr + bufferNeighbors.getIndex(low_k);
		assert(mNeighbors_j[0] + inRunline.getLength() <= buffer_ptr +
			inBuffer.getNumBufferCells(matType.getBufferIndex(low_k), here_k));
		assert(mNeighbors_j[0] >= buffer_ptr);
	}
	if (bufferNeighbors.getIndex(high_k) == -1)
	{
		mNeighbors_j[1] = fj + neighbors.getIndex(high_k);
		assert(mNeighbors_j[1] + inRunline.getLength() <= fj + total_cells);
		assert(mNeighbors_j[1] >= fj);
	}
	else
	{
		buffer_ptr = inBuffer.getBufferPointer(
			matType.getBufferIndex(high_k), here_k);
		//LOG << "High k Buffer pointer " << hex << buffer_ptr << dec << "\n";
		mNeighbors_j[1] = buffer_ptr + bufferNeighbors.getIndex(high_k);
		assert(mNeighbors_j[1] + inRunline.getLength() <= buffer_ptr +
			inBuffer.getNumBufferCells(matType.getBufferIndex(high_k), here_k));
		assert(mNeighbors_j[1] >= buffer_ptr);
	}
	
	assert(mField0 >= fi);
	assert(mField0 + inRunline.getLength() <= fi + total_cells);
	
	/*
	LOG << "Printing initial values (" << m_ii0 << " " << m_jj0 << " "
		<< m_kk0 << ")\n";
	LOGMORE << *mField0 << " " << *mNeighbors_j[0] << " " << *mNeighbors_j[1]
		<< " " << *mNeighbors_k[0] << " " << *mNeighbors_k[1] << "\n";
	*/
	
	//LOG << "printing: \n";
	//printNeighbors();
	
	//LOG << "index " << mIndex << "\n";
}

Runline::
Runline(const Runline & copyMe)
{
	assert(&copyMe != this); 
    *this = copyMe;
}

const Runline & Runline::
operator=(const Runline & rhs)
{
    if (&rhs == this)
        return *this;
    
    mLength = rhs.mLength;
    mIndex = rhs.mIndex;
	
	for (int nn = 0; nn < 3; nn++)
		mAuxIndex[nn] = rhs.mAuxIndex[nn];
    
    mField0 = rhs.mField0;
    m_ii0 = rhs.m_ii0;
    m_jj0 = rhs.m_jj0;
    m_kk0 = rhs.m_kk0;
    
    for (int n = 0; n < 2; n++)
    {
        mNeighbors_j[n] = rhs.mNeighbors_j[n];
        mNeighbors_k[n] = rhs.mNeighbors_k[n];
    }
    
	for (int n = 0; n < 4; n++)
		mAuxNeighbors[n] = rhs.mAuxNeighbors[n];
			
    return *this;
}

Runline::
~Runline()
{
    //LOG << "destructor.\n" << flush;
    //LOG << m_ii0 << " " << m_jj0 << " " << m_kk0 << " " << flush << endl;
    //if (mAuxNeighbors != 0L)
    //    delete [] mAuxNeighbors;
    //LOGMORE << "and done.\n" << flush;
}


void Runline::
printNeighbors() const
{
	cout << hex;
	cout << mNeighbors_j[0] << " ";
	cout << mNeighbors_j[1] << " ";
	cout << mNeighbors_k[0] << " ";
	cout << mNeighbors_k[1];
    cout << dec;
}


void Runline::
assertZero() const
{
	for (int nn = 0; nn < mLength; nn++)
	{
		assert(mField0[nn] == 0);
	}
}





