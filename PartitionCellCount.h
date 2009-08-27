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

#include "SimulationDescription.h"
#include "Pointer.h"
#include <iostream>
#include <vector>
#include <set>
#include "geometry.h"
#include "Map.h"

class Paint;
class VoxelGrid;

class PartitionCellCount
{
public:
	PartitionCellCount(const VoxelGrid & grid, Rect3i halfCellBounds,
        int runlineDirection );
	
    GridDescPtr gridDescription() const { return mGridDescription; }
    
	long operator() (int ii, int jj, int kk) const;
	long operator() (const Vector3i & pp) const;
	
	long numCells(Paint* paint, int octant) const;
	Map<Paint*, long> allNumCells(int octant) const;
	std::set<Paint*> curlBufferParentPaints() const;
    
private:
	void calcMaterialIndices(const VoxelGrid & grid, int runlineDirection );
	void allocateAuxiliaryDataSpace(const VoxelGrid & grid);
	
    GridDescPtr mGridDescription;
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
