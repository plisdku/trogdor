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

class HuygensSurface;
class SetupCurrentSource;

class VoxelGrid
{
public:
    // allocRegion       global coordinates within which we store fields
    // gridHalfCells     partition is responsible for calculations in here
    // nonPML            the "middle" of the simulation, not including absorbers
	VoxelGrid(GridDescPtr gridDescription, Rect3i allocRegion);
	
    GridDescPtr gridDescription() const { return mGridDescription; }
    
    // returns           global bounds of partition's calculation region
	const Rect3i & gridHalfCells() { return mGridHalfCells; }
    
    // returns           global bounds of partition's allocated fields
	const Rect3i & allocRegion() { return mAllocRegion; }
    
    // returns           global bounds of partition's non-PML region
	const Rect3i & nonPMLRegion() { return mNonPMLRegion; }
	
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
	
    void overlayHuygensSurface(const HuygensSurface & surf);
	void overlayCurrentSource(const CurrentSourceDescPtr & current);
	void overlayPML();
	
    // ii,jj,kk          global coordinates; will wrap around in the partition
	Paint* & operator() (int ii, int jj, int kk);
	Paint* operator() (int ii, int jj, int kk) const;
    
    // pp                global coordinate; will wrap around in the partition
	Paint* & operator() (const Vector3i & pp);
	Paint* operator() (const Vector3i & pp) const;
    
    // paint             the material including PML, TFSF and current sources
    // ii,jj,kk          global coordinates; will wrap around globally
	void paintHalfCell(Paint* paint, int ii, int jj, int kk);
    void paintHalfCell(Paint* paint, const Vector3i & pp);
    
    // paint             the material including PML, TFSF and current sources
    // iYee,jYee,kYee    Yee cell to fill in
    void paintYeeCell(Paint* paint, int iYee, int jYee, int kYee);
    
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
    GridDescPtr mGridDescription;
	std::vector<Paint*> mMaterialHalfCells;
	
	Rect3i mAllocRegion;
	Rect3i mGridHalfCells;
	Rect3i mNonPMLRegion;
	
	long m_nnx;
	long m_nny;
	long m_nnz;
	long m_nx;
	long m_ny;
	long m_nz;
};
typedef Pointer<VoxelGrid> VoxelGridPtr;

std::ostream & operator<< (std::ostream & out, const VoxelGrid & grid);





#endif
