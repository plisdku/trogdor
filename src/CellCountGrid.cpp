/*
 *  CellCountGrid.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "CellCountGrid.h"

#include <iostream>
using namespace std;

#include "VoxelGrid.h"
#include "YeeUtilities.h"
using namespace YeeUtilities;

CellCountGrid::
CellCountGrid(const VoxelGrid & grid, Rect3i halfCellBounds) :
	//mNumCells(),
	mHalfCellBounds(halfCellBounds),
	m_nnx(halfCellBounds.size(0)+1),
	m_nny(halfCellBounds.size(1)+1),
	m_nnz(halfCellBounds.size(2)+1)
{
	mMaterialIndexHalfCells.resize(m_nnx*m_nny*m_nnz);
	calcMaterialIndices(grid);
	allocateAuxiliaryDataSpace(grid);
}


long CellCountGrid::
operator() (int ii, int jj, int kk) const
{
	return mMaterialIndexHalfCells[(ii+m_nnx)%m_nnx + 
		((jj+m_nny)%m_nny)*m_nnx +
		((kk+m_nnz)%m_nnz)*m_nnx*m_nny];
}

long CellCountGrid::
operator() (const Vector3i & pp) const
{
	return mMaterialIndexHalfCells[(pp[0]+m_nnx)%m_nnx + 
		((pp[1]+m_nny)%m_nny)*m_nnx +
		((pp[2]+m_nnz)%m_nnz)*m_nnx*m_nny];
}


void CellCountGrid::
calcMaterialIndices(const VoxelGrid & grid)
{
	for (int nn = 0; nn < 8; nn++)
	{
		//LOG << "Starting side " << nn << "\n";
		Map<Paint*, long> materialIndices;
		Vector3i offset = halfCellOffset(nn);
		//LOG << "Offset is " << offset << "\n";
		//LOG << "Size is " << m_nnx << " " << m_nny << " " << m_nnz << "\n";
		
		Vector3i rSize = mHalfCellBounds.size() + Vector3i(1,1,1);
		Vector3i o = mHalfCellBounds.p1;
		
		for (int kk = offset[2]; kk < rSize[2]; kk += 2)
		for (int jj = offset[1]; jj < rSize[1]; jj += 2)
		for (int ii = offset[0]; ii < rSize[0]; ii += 2)
		{
			int linearIndex = (ii) + (jj)*rSize[0] +
				(kk)*rSize[0]*rSize[1];
			assert(linearIndex >= 0 &&
				linearIndex < mMaterialIndexHalfCells.size());
			
			Paint* p = grid(ii+o[0],jj+o[1],kk+o[2]);
			if (materialIndices.count(p) == 0)
			{
				materialIndices[p] = 1;
				mMaterialIndexHalfCells[linearIndex] = 0;
				//LOG << "Starting material " << hex << p << dec << "\n";
			}
			else
			{
				mMaterialIndexHalfCells[linearIndex] = materialIndices[p];
				materialIndices[p]++;
			}
		}
	}
}


void CellCountGrid::
allocateAuxiliaryDataSpace(const VoxelGrid & grid)
{
	LOG << "Allocating auxiliary space.\n";
}


std::ostream & operator<< (std::ostream & out, const CellCountGrid & grid)
{
	int nni = grid.mHalfCellBounds.size(0)+1;
	int nnj = grid.mHalfCellBounds.size(1)+1;
	int nnk = grid.mHalfCellBounds.size(2)+1;
	
	for (int kk = grid.mHalfCellBounds.p1[2];
		kk < grid.mHalfCellBounds.p2[2]; kk++)
	{
		out << "z = " << kk << "\n";
		for (int jj = grid.mHalfCellBounds.p2[1];
			jj >= grid.mHalfCellBounds.p1[1]; jj--)
		{
			for (int ii = grid.mHalfCellBounds.p1[0];
				ii <= grid.mHalfCellBounds.p2[0]; ii++)
			{
				int linearIndex = (ii-grid.mHalfCellBounds.p1[0]) +
					(jj-grid.mHalfCellBounds.p1[1])*nni +
					(kk-grid.mHalfCellBounds.p1[2])*nni*nnj;
				out << setw(6) << grid.mMaterialIndexHalfCells[linearIndex];
			}
			out << "\n";
		}
	}
	return out;
}

