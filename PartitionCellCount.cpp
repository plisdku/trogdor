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
PartitionCellCount(const VoxelGrid & grid, Rect3i halfCellBounds,
    int runlineDirection ) :
    mGridDescription(grid.gridDescription()),
	mNumCells(8),
	mHalfCellBounds(halfCellBounds),
	m_nnx(halfCellBounds.size(0)+1),
	m_nny(halfCellBounds.size(1)+1),
	m_nnz(halfCellBounds.size(2)+1)
{
    long long allocSize = m_nnx*m_nny*m_nnz;
    if (mMaterialIndexHalfCells.max_size() < allocSize)
    {
        cerr << "Warning: PartitionCellCount is going to attempt to allocate a "
            << m_nnx << "x" << m_nny << "x" << m_nnz << " cell array with "
            "std::vector; the total size is " << allocSize << " and the vector"
            " maximum size is " << mMaterialIndexHalfCells.max_size()
            << ", so this will likely fail." << endl;
    }
	mMaterialIndexHalfCells.resize(m_nnx*m_nny*m_nnz);
	calcMaterialIndices(grid, runlineDirection);
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
numCells(Paint* paint, int octant) const
{
	assert(octant >= 0 && octant < 8);
	
	if (mNumCells[octant].count(paint) != 0)
		return mNumCells[octant][paint];
	return 0;
}

Map<Paint*, long> PartitionCellCount::
allNumCells(int octant) const
{
	assert(octant >= 0 && octant < 8);
	return mNumCells[octant];
}

set<Paint*> PartitionCellCount::
curlBufferParentPaints() const
{
	std::set<Paint*> paints;
	for (int octant = 0; octant < 8; octant++)
	{
		const Map<Paint*, long> & m = mNumCells[octant];
		for (map<Paint*,long>::const_iterator ii = m.begin(); ii != m.end();
			ii++)
		{
//            LOG << "paint " << *ii->first << "\n";
			paints.insert(ii->first);
		}
	}
	return paints;
}


void PartitionCellCount::
calcMaterialIndices(const VoxelGrid & grid, int runlineDirection )
{
	for (int octant = 0; octant < 8; octant++)
	{
		Vector3i rSize = mHalfCellBounds.num();
		Vector3i origin = mHalfCellBounds.p1;
		Vector3i start = yeeToHalf(halfToYee(mHalfCellBounds.p1), octant);
		
		//LOG << "Calculating from " << start << "\n";
        
        const int d0 = runlineDirection;
        const int d1 = (d0+1)%3;
        const int d2 = (d0+2)%3;
        
        Vector3i x;
        for (x[d2] = start[d2]; x[d2] <= mHalfCellBounds.p2[d2]; x[d2] += 2)
        for (x[d1] = start[d1]; x[d1] <= mHalfCellBounds.p2[d1]; x[d1] += 2)
        for (x[d0] = start[d0]; x[d0] <= mHalfCellBounds.p2[d0]; x[d0] += 2)
        {
			int linearIndex = (x[0]-origin[0]) + (x[1]-origin[1])*rSize[0] +
				(x[2]-origin[2])*rSize[0]*rSize[1];
			
			assert(linearIndex >= 0 &&
				linearIndex < mMaterialIndexHalfCells.size());
			
			Paint* p =  grid(x[0],x[1],x[2])->withoutCurlBuffers();
			
			if (mNumCells[octant].count(p) == 0)
			{
				mNumCells[octant][p] = 1;
				mMaterialIndexHalfCells[linearIndex] = 0;
				//LOG << "Starting material " << hex << p << dec << "\n";
				//LOGMORE << "at " << ii << " " << jj << " " << kk << ", "
				//	<< "linear index " << linearIndex << "\n";
			}
			else
			{
				mMaterialIndexHalfCells[linearIndex] = mNumCells[octant][p];
				mNumCells[octant][p]++;
			}
        }
	}
}


void PartitionCellCount::
allocateAuxiliaryDataSpace(const VoxelGrid & grid)
{
	LOGF << "Allocating auxiliary space (doing nothing).\n";
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

/*
void PartitionCellCount::
clear()
{
    mMaterialIndexHalfCells.clear();
    mNumCells.clear();
}
*/


std::ostream & operator<< (std::ostream & out, const PartitionCellCount & grid)
{
	int nni = grid.mHalfCellBounds.size(0)+1;
	int nnj = grid.mHalfCellBounds.size(1)+1;
	//int nnk = grid.mHalfCellBounds.size(2)+1;
	
	set<Paint*> parentPaints = grid.curlBufferParentPaints();
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

