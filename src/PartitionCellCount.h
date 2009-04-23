/*
 *  PartitionCellCount.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _PARTITIONCELLCOUNT_
#define _PARTITIONCELLCOUNT_

#include <iostream>
#include <vector>
#include <set>
#include "Paint.h"
#include "geometry.h"
#include "Map.h"

class VoxelGrid;

class PartitionCellCount
{
public:
	PartitionCellCount(const VoxelGrid & grid, Rect3i halfCellBounds);
	
	long operator() (int ii, int jj, int kk) const;
	long operator() (const Vector3i & pp) const;
	
	long getNumCells(Paint* paint, int octant) const;
	Map<Paint*, long> getAllNumCells(int octant) const;
	std::set<Paint*> getCurlBufferParentPaints() const;
	
private:
	void calcMaterialIndices(const VoxelGrid & grid);
	void allocateAuxiliaryDataSpace(const VoxelGrid & grid);
	
	std::vector<Map<Paint*, long> > mNumCells;
	std::vector<long> mMaterialIndexHalfCells;
	Rect3i mHalfCellBounds;
	int m_nnx;
	int m_nny;
	int m_nnz;
	
	friend std::ostream & operator<< (std::ostream & out,
		const PartitionCellCount & grid);
};
typedef Pointer<PartitionCellCount> PartitionCellCountPtr;

std::ostream & operator<< (std::ostream & out, const PartitionCellCount & grid);


#endif
