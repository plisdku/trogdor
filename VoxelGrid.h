/*
 *  VoxelGrid.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _VOXELGRID_
#define _VOXELGRID_

#include "SimulationDescriptionPredeclarations.h"
#include "Paint.h"
#include "Exception.h"

class VoxelGrid
{
public:
    // allocRegion       global coordinates within which we store fields
    // gridHalfCells     partition is responsible for calculations in here
    // nonPML            the "middle" of the simulation, not including absorbers
	VoxelGrid(Rect3i allocRegion, Rect3i gridHalfCells, Rect3i nonPML);
	
    // returns           global bounds of partition's calculation region
	const Rect3i & getGridHalfCells() { return mGridHalfCells; }
    
    // returns           global bounds of partition's allocated fields
	const Rect3i & getAllocRegion() { return mAllocRegion; }
    
    // returns           global bounds of partition's non-PML region
	const Rect3i & getNonPMLRegion() { return mNonPMLRegion; }
	
	friend std::ostream & operator<< (std::ostream & out,
		const VoxelGrid & grid);
		
	void paintBlock(const GridDescription & gridDesc,
		const Block & instruction);
	void paintKeyImage(const GridDescription & gridDesc,
		const KeyImage & instruction);
	void paintHeightMap(const GridDescription & gridDesc,
		const HeightMap & instruction);
	void paintEllipsoid(const GridDescription & gridDesc,
		const Ellipsoid & instruction);
	void paintCopyFrom(const GridDescription & gridDesc,
		const CopyFrom & instruction,
		const VoxelGrid & copyFromGrid);
	void paintExtrude(const GridDescription & gridDesc,
		const Extrude & instruction);
	
	void overlayHuygensSurface(const HuygensSurfaceDescription & surf);
	void overlayCurrentSource(const int & currentSource);
	void overlayPML();
	
    // ii,jj,kk          global coordinates; will wrap around globally
	Paint* & operator() (int ii, int jj, int kk);
	Paint* operator() (int ii, int jj, int kk) const;
    
    // pp                global coordinate; will wrap around globally
	Paint* & operator() (const Vector3i & pp);
	Paint* operator() (const Vector3i & pp) const;
    
    // paint             the material including PML, TFSF and current sources
    // ii,jj,kk          global coordinates; will wrap around globally
	void paintHalfCell(Paint* paint, int ii, int jj, int kk);
    
    // paint             the material including PML, TFSF and current sources
    // iYee,jYee,kYee    Yee cell to fill in, expanded to PEC-like boundaries
	void paintPEC(Paint* paint, int iYee, int jYee, int kYee);
    
    // paint             the material including PML, TFSF and current sources
    // iYee,jYee,kYee    Yee cell to fill in, expanded to PMC-like boundaries
	void paintPMC(Paint* paint, int iYee, int jYee, int kYee);
    
    // pmlDir            direction of increasing absorption (+/- 1 in all dirs)
    // pp                global coordinate (half cell) to modify as PML
	void paintPML(Vector3i pmlDir, Vector3i pp);
    
    // deletes the grid of material half cells; retains dimensions
    void clear();
	
private:
	std::vector<Paint*> mMaterialHalfCells;
	
	Rect3i mAllocRegion;
	Rect3i mGridHalfCells;
	Rect3i mNonPMLRegion;
	
	int m_nnx;
	int m_nny;
	int m_nnz;
	int m_nx;
	int m_ny;
	int m_nz;
};
typedef Pointer<VoxelGrid> VoxelGridPtr;

std::ostream & operator<< (std::ostream & out, const VoxelGrid & grid);





#endif
