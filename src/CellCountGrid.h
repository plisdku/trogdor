/*
 *  CellCountGrid.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _CELLCOUNTGRID_
#define _CELLCOUNTGRID_

#include <iostream>
#include <vector>
#include "Paint.h"
#include "geometry.h"
#include "Map.h"

class VoxelGrid;

class CellCountGrid
{
public:
	CellCountGrid(const VoxelGrid & grid, Rect3i halfCellBounds);
	
	void spaceVaryingBlock();
	
	long operator() (int ii, int jj, int kk) const;
	long operator() (const Vector3i & pp) const;
	
	long getNumCells(Paint* paint, int octant) const;
	const Map<Paint*, long> getAllNumCells(int octant) const;
	
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
		const CellCountGrid & grid);
};
typedef Pointer<CellCountGrid> CellCountGridPtr;

std::ostream & operator<< (std::ostream & out, const CellCountGrid & grid);


#endif
