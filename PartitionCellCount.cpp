/*
 *  PartitionCellCount.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "PartitionCellCount.h"

#include <iostream>
using namespace std;

#include "VoxelGrid.h"
#include "YeeUtilities.h"
using namespace YeeUtilities;

PartitionCellCount::
PartitionCellCount(const VoxelGrid & grid, Rect3i halfCellBounds) :
	mNumCells(8),
	mHalfCellBounds(halfCellBounds),
	m_nnx(halfCellBounds.size(0)+1),
	m_nny(halfCellBounds.size(1)+1),
	m_nnz(halfCellBounds.size(2)+1)
{
	mMaterialIndexHalfCells.resize(m_nnx*m_nny*m_nnz);
	calcMaterialIndices(grid);
	allocateAuxiliaryDataSpace(grid);
}


long PartitionCellCount::
operator() (int ii, int jj, int kk) const
{
	ii = ii - mHalfCellBounds.p1[0];
	jj = jj - mHalfCellBounds.p1[1];
	kk = kk - mHalfCellBounds.p1[2];
	return mMaterialIndexHalfCells[(ii+m_nnx)%m_nnx + 
		((jj+m_nny)%m_nny)*m_nnx +
		((kk+m_nnz)%m_nnz)*m_nnx*m_nny];
}

long PartitionCellCount::
operator() (const Vector3i & pp) const
{
	Vector3i qq(pp - mHalfCellBounds.p1);
	return mMaterialIndexHalfCells[(qq[0]+m_nnx)%m_nnx + 
		((qq[1]+m_nny)%m_nny)*m_nnx +
		((qq[2]+m_nnz)%m_nnz)*m_nnx*m_nny];
}

long PartitionCellCount::
getNumCells(Paint* paint, int octant) const
{
	assert(octant >= 0 && octant < 8);
	
	if (mNumCells[octant].count(paint) != 0)
		return mNumCells[octant][paint];
	return 0;
}

Map<Paint*, long> PartitionCellCount::
getAllNumCells(int octant) const
{
	assert(octant >= 0 && octant < 8);
	return mNumCells[octant];
}

set<Paint*> PartitionCellCount::
getCurlBufferParentPaints() const
{
	std::set<Paint*> paints;
	for (int octant = 0; octant < 8; octant++)
	{
		const Map<Paint*, long> & m = mNumCells[octant];
		for (map<Paint*,long>::const_iterator ii = m.begin(); ii != m.end();
			ii++)
		{
			paints.insert(ii->first);
		}
	}
	return paints;
}


void PartitionCellCount::
calcMaterialIndices(const VoxelGrid & grid)
{
	for (int nn = 0; nn < 8; nn++)
	{
		Vector3i offset = halfCellOffset(nn);
		Vector3i rSize = mHalfCellBounds.size() + Vector3i(1,1,1);
		Vector3i o = mHalfCellBounds.p1;
		
		Vector3i start = mHalfCellBounds.p1;
		for (int ss = 0; ss < 3; ss++)
		if (start[ss]%2 != offset[ss]%2)
			start[ss] += 1;
		
		//LOG << "Calculating from " << start << "\n";
		
		for (int kk = start[2]; kk <= mHalfCellBounds.p2[2]; kk += 2)
		for (int jj = start[1]; jj <= mHalfCellBounds.p2[1]; jj += 2)
		for (int ii = start[0]; ii <= mHalfCellBounds.p2[0]; ii += 2)
		{
			int linearIndex = (ii-o[0]) + (jj-o[1])*rSize[0] +
				(kk-o[2])*rSize[0]*rSize[1];
			
			assert(linearIndex >= 0 &&
				linearIndex < mMaterialIndexHalfCells.size());
			
			Paint* p =  grid(ii,jj,kk)->withoutCurlBuffers();
			
			if (mNumCells[nn].count(p) == 0)
			{
				mNumCells[nn][p] = 1;
				mMaterialIndexHalfCells[linearIndex] = 0;
				//LOG << "Starting material " << hex << p << dec << "\n";
				//LOGMORE << "at " << ii << " " << jj << " " << kk << ", "
				//	<< "linear index " << linearIndex << "\n";
			}
			else
			{
				mMaterialIndexHalfCells[linearIndex] = mNumCells[nn][p];
				mNumCells[nn][p]++;
			}
		}
	}
}


void PartitionCellCount::
allocateAuxiliaryDataSpace(const VoxelGrid & grid)
{
	LOG << "Allocating auxiliary space.\n";
	// So what functionality goes here?
	// Well, my current thought is that the *material model* is what is aware
	// of any space-varying data.  This baokuo both the boundary-type materials
	// (which need to know about local surface normals and such) and the simple
	// graded-index type materials.
	//
	// So based on the Paints in the VoxelGrid, and I imagine also on the basis
	// of the internal material count here, and lastly on the basis of the
	// setup material (whatever it is that informs us here of how much space
	// will be needed for a material), we can allocate data arrays.  This is not
	// the same as setting up MemoryBuffers, which is the job of the parent
	// VoxelizedPartition (or whatever it will be called).
	// 
	// It's a little weird of me to include the auxiliary data allocation here. 
}


std::ostream & operator<< (std::ostream & out, const PartitionCellCount & grid)
{
	int nni = grid.mHalfCellBounds.size(0)+1;
	int nnj = grid.mHalfCellBounds.size(1)+1;
	int nnk = grid.mHalfCellBounds.size(2)+1;
	
	set<Paint*> parentPaints = grid.getCurlBufferParentPaints();
	for (set<Paint*>::iterator itr = parentPaints.begin();
		itr != parentPaints.end(); itr++)
	{
		out << hex << *itr << dec << ":\n" << *(*itr) << "\n";
	}
	
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

