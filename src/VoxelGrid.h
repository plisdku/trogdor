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
	VoxelGrid(Vector3i numYeeCells, Rect3i nonPML);
	
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
	
	Paint* & operator() (int ii, int jj, int kk);
	Paint* operator() (int ii, int jj, int kk) const;
	Paint* & operator() (const Vector3i & pp);
	Paint* operator() (const Vector3i & pp) const;
	
	long linearYeeIndex(int ii, int jj, int kk) const;
	long linearYeeIndex(const Vector3i & halfCell) const;
	
	void paintPEC(Paint* paint, int iYee, int jYee, int kYee);
	void paintPMC(Paint* paint, int iYee, int jYee, int kYee);
	
private:
	std::vector<Paint*> mMaterialHalfCells;
	
	Rect3i mNonPMLRegion;
	int m_nnx;
	int m_nny;
	int m_nnz;
	int m_nx;
	int m_ny;
	int m_nz;
};

std::ostream & operator<< (std::ostream & out, const VoxelGrid & grid);





#endif
