/*
 *  GridWalker.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/23/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "GridWalker.h"

#include "SetupTFSFBufferSet.h"
#include "SetupGrid.h"

using namespace std;



GridWalker::
GridWalker(StructureGridPtr grid, int ii0, int jj0, int kk0) :
    mGrid(grid),
	mIsTFSF(0),
	m_aux_nx(0), m_aux_ny(0), m_aux_nz(0),
	mTagForTFSFTag(),
	mMaterialIndexForTag(),
	mHasRunline(0),
	mRunline(0L),
	mMaterial(-1),
	mMatTFSFFlags(),
	mUseXYZFlags(),
	mComparisonXYZFlags(),
	m_ii(-1), m_jj(-1), m_kk(-1),
	mNeighborIndices(),
	mCardinals()
{
    //  Ex runlines only use neighbors along Y and Z for curl calculations.
    //  Determine which neighbors this runline will use.
	if ( (jj0 + kk0) % 2 )
	{
		mUseXYZFlags[0] = 1;
		mUseXYZFlags[1] = 1;
	}
	if ( (kk0 + ii0) % 2 )
	{
		mUseXYZFlags[2] = 1;
		mUseXYZFlags[3] = 1;
	}
	if ( (ii0 + jj0) % 2 )
	{
		mUseXYZFlags[4] = 1;
		mUseXYZFlags[5] = 1;
	}
	
	mCardinals.push_back(Vector3i(-1,0,0));
	mCardinals.push_back(Vector3i(1,0,0));
	mCardinals.push_back(Vector3i(0,-1,0));
	mCardinals.push_back(Vector3i(0,1,0));
	mCardinals.push_back(Vector3i(0,0,-1));
	mCardinals.push_back(Vector3i(0,0,1));
	
}

GridWalker::
~GridWalker()
{
}

void GridWalker::
beginRow()
{
    mHasRunline = 0;
}

void GridWalker::
startNewRunline(const MaterialType & inType, int ii, int jj, int kk)
{
    mHasRunline = 1;
    mMaterial = mGrid->material(ii,jj,kk);
    mIsTFSF = inType.isTFSF();
	
	for (int nn = 0; nn < 6; nn++)
		mMatTFSFFlags[nn] = (inType.getBufferIndex(nn) != -1);
	
	mComparisonXYZFlags = ~mMatTFSFFlags & mUseXYZFlags;
	
	IndexStencil bufferIndices;
    
    //LOG << "Starting runline at " << ii << " " << jj << " " << kk << endl;
	
    if (mIsTFSF)
    {
		Vector3i pt(ii,jj,kk);
		
		for (int nSide = 0; nSide < 6; nSide++)
		{
			if (mMatTFSFFlags[nSide]) // if corrected in this direction
			{
				int bufferNumber = inType.getBufferIndex(nSide);
				//LOG << "** bufferNumber = " << bufferNumber << "\n";
				
				Vector3i neighbor(pt + mCardinals[nSide]);
				
				Rect3i bufferRect = inType.getBuffer()->getYeeBufferRect(
					bufferNumber, neighbor % 2);
				
				Vector3i dp = neighbor/2 - bufferRect.p1;
				
				int nnx = bufferRect.p2[0] - bufferRect.p1[0] + 1;
				int nny = bufferRect.p2[1] - bufferRect.p1[1] + 1;
				int index = dp[0] + dp[1]*nnx + dp[2]*nnx*nny;
				
				bufferIndices.setIndex(nSide, index);
				
				//LOGMORE << "side " << nSide << " index " << index << "\n";
				//LOGMORE << " ( Rect " << bufferRect << " )\n";
			}
			else
				bufferIndices.setIndex(nSide, -1);
		}
		//LOG << "Buffer indices: ";
		//bufferIndices.print(cout);
    }
    
    getStencil(mNeighborIndices, ii, jj, kk);
    
    //  Create and return the runline.
    
	// assert(mMaterialIndexForTag[mTagForTFSFTag[mMaterial]]-startLength >= 0);
    
    Pointer<RunlineType> newRunline(new RunlineType(inType,
        mMaterialIndexForTag[mTagForTFSFTag[mMaterial]],
		mGrid->yeeIndex(ii,jj,kk), ii, jj, kk, mNeighborIndices,
		bufferIndices));
	
	//Pointer<RunlineType> foo(new RunlineType());
    mRunline = newRunline;
	
}

void GridWalker::
incrementLength()
{
    mRunline->setLength(mRunline->getLength() + 1);
}


void GridWalker::
step(int ii, int jj, int kk)
{
    int mat = mGrid->material(ii,jj,kk);
	
	Map<int,int>::iterator iter = mTagForTFSFTag.find(mat);
	if (iter == mTagForTFSFTag.end())
	{
		const MaterialType & matType = mGrid->getMaterialType(mat);
		if (matType.isTFSF())
		{
			//	Obtain the index of the parent type for this material.
			mTagForTFSFTag[mat] = mGrid->getMaterialIndex(matType.getName());
		}
		else
			mTagForTFSFTag[mat] = mat;
	}
	
	int theTag = mTagForTFSFTag[mat];
	
    if (mMaterialIndexForTag.count(theTag) == 0)
        mMaterialIndexForTag[theTag] = 0;
    else
        mMaterialIndexForTag[theTag]++;
    
    m_ii = ii;
    m_jj = jj;
    m_kk = kk;
}

bool GridWalker::
needNewRunline()
{
    //  End of runline criteria:
    //      1.  Have we crossed a material interface?
    //      2.  Have any of the neighbor indices crossed to a new row?
    
    if (!mHasRunline)
        return 1;
    
    if (mGrid->material(m_ii, m_jj, m_kk) != mMaterial)
        return 1;
	
	IndexStencil hereIndices;
	bool indicesAreContiguous;
    getStencil(hereIndices, m_ii, m_jj, m_kk);
	indicesAreContiguous = mNeighborIndices.compareToIncremented(
		hereIndices, mRunline->getLength(), mComparisonXYZFlags);
	
	return (0 == indicesAreContiguous);
}

RunlineType& GridWalker::
getRunline()
{
	//mRunline->print(cout);
	//LOGMORE << mRunline->getType().getDescription() << "\n";
    validateRunline();
    return *mRunline;
}


void GridWalker::
getStencil(IndexStencil & stencil, int ii, int jj, int kk) const
{   
    int nnx = mGrid->nnx();
    int nny = mGrid->nny();
    int nnz = mGrid->nnz();
    int iii, jjj, kkk;
    
    for (int n = 0; n < 2; n++)
    {
        iii = (ii + 2*nnx - 1 + 2*n) % nnx;
        jjj = (jj + 2*nny - 1 + 2*n) % nny;
        kkk = (kk + 2*nnz - 1 + 2*n) % nnz;
        
		stencil.setIndex(0+n, mGrid->yeeIndex(iii,jj,kk));
		stencil.setIndex(2+n, mGrid->yeeIndex(ii,jjj,kk));
		stencil.setIndex(4+n, mGrid->yeeIndex(ii,jj,kkk));
	}
}

void GridWalker::
validateRunline()
{
    //  1.  Main grid
    int mainCells = mGrid->nx()*mGrid->ny()*mGrid->nz();
    int length = mRunline->getLength();
    int n;
    assert(mRunline->materialIndex() >= 0);
    
	for (n = 0; n < 6; n++)
	if (mUseXYZFlags[n])
	if (mNeighborIndices.getIndex(n) + length > mainCells)
		assert(mNeighborIndices.getIndex(n) + length <= mainCells);
	
	for (n = 0; n < 6; n++)
	if (mMatTFSFFlags[n])
	{
		Vector3i neighbor(
			Vector3i(mRunline->get_ii0(), mRunline->get_jj0(), 
			mRunline->get_kk0()) + mCardinals[n]);
		
		Rect3i bufferRect = mRunline->getType().getBuffer()->getYeeBufferRect(
			n, neighbor % 2);
		
		int nnx = bufferRect.p2[0] - bufferRect.p1[0] + 1;
		int nny = bufferRect.p2[1] - bufferRect.p1[1] + 1;
		int nnz = bufferRect.p2[2] - bufferRect.p1[2] + 1;
		if (mRunline->getBufferNeighbors().getIndex(n) + length
			> nnx*nny*nnz)
			assert(mRunline->getBufferNeighbors().getIndex(n) + length
				<= nnx*nny*nnz);
	}
}



