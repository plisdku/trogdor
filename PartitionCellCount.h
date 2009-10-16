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
    /**
     * Construct a new cell count for the given VoxelGrid.  After construction
     * all the cells in the grid have been counted with respect to the update
     * equation they represent.  Specifically, materials with and without PML
     * are counted separately, different directions of PML are counted
     * separately, and different current sources or cells without current
     * sources are counted separately; however, cells that obtain neighboring
     * field data from curl buffers instead of the adjacent cell (as for TFSF
     * boundaries) are all counted together, ignoring the boundary.
     *
     * On return, the materials have been counted and each cell has been
     * assigned a unique index among its update type, which can be used as an
     * index into any auxiliary arrays for dispersive update equations. 
     */
	PartitionCellCount(const VoxelGrid & grid, Rect3i halfCellBounds,
        int runlineDirection );
	
    GridDescPtr gridDescription() const { return mGridDescription; }
    
    /**
     * Return the index of the material in the given cell.  A return value of
     * 5 may represent the fifth cell of air located at an Ex position, or
     * the fifth cell of gold located at an Hz position, et cetera.  The
     * indices increase monotonically along the runline direction, then
     * along the other axes; in a grid with N total Yee cells, the return value
     * of this function will range from 0 to N-1.
     */
	long operator() (int ii, int jj, int kk) const;
	long operator() (const Vector3i & pp) const;
	
    /**
     * Return the number of cells in the partition with a particular update
     * equation.  PML and non-PML are counted separately, materials with and
     * without current sources are counted separately, but materials with curl
     * buffers (used to implement total-field scattered-field boundaries) are 
     * all counted together.
     *
     * @param paint     A Paint stripped of curl buffers with
     *                  withoutCurlBuffers()
     * @param octant    octant number (0-7) to count in
     * @see Paint
     */
	long numCells(Paint* paint, int octant) const;
    
    /**
     * Return a map containing all cell counts for the given octant.  This
     * information can be grabbed piecemeal from calls to numCells() over all
     * Paints without curl buffers (obtained from any Paint by a call to
     * withoutCurlBuffers(), or all together from curlBufferParentPaints()).
     *
     * @param octant    octant number (0-7) to count in
     */
	Map<Paint*, long> allNumCells(int octant) const;
    
    /**
     * Return a set of all Paints which are counted.  This includes all
     * materials, separately by PML and by current sources, but does not include
     * any Paints with curl buffers.
     */
	std::set<Paint*> curlBufferParentPaints() const;
    
private:
	void calcMaterialIndices(const VoxelGrid & grid, int runlineDirection );
	void allocateAuxiliaryDataSpace(const VoxelGrid & grid);
	
    GridDescPtr mGridDescription;
	std::vector<Map<Paint*, long> > mNumCells;
	std::vector<long> mMaterialIndexHalfCells;
	Rect3i mHalfCellBounds;
	long m_nnx;
	long m_nny;
	long m_nnz;
	
	friend std::ostream & operator<< (std::ostream & out,
		const PartitionCellCount & grid);
};
typedef Pointer<PartitionCellCount> PartitionCellCountPtr;

std::ostream & operator<< (std::ostream & out, const PartitionCellCount & grid);


#endif
