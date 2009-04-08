/*
 *  VoxelizedGrid.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 *  $Rev:: 12                            $:  Revision of last commit
 *  $Author:: pch                        $:  Author of last commit
 *
 *  $Date: 2009-01-15 18:44:23 -0800 (Thu, 15 Jan 2009) $:
 *  $Id: MaterialType.h 12 2009-01-16 02:44:23Z pch $:
 *
 */

#ifndef _VOXELIZEDGRID_
#define _VOXELIZEDGRID_

#include "Pointer.h"
#include "geometry.h"
#include <vector>
#include <string>
#include "Map.h"
#include "VoxelGrid.h"
#include "CellCountGrid.h"

#include "SimulationDescriptionPredeclarations.h"

#include "Paint.h"

class VoxelizedGrid
{
public:
	VoxelizedGrid(const GridDescription & gridDesc, 
		const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids);  // !
	
	const std::vector<Vector3i> & getHuygensRegionSymmetries() const {
		return mHuygensRegionSymmetries; } // !
	
	bool hasPML(int faceNum) const;
private:
	//! ?
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids);
	void paintFromHuygensSurfaces(const GridDescription & gridDesc);
	void paintFromCurrentSources(const GridDescription & gridDesc);
	void paintPML();
	
	void calculateMaterialIndices();
	
	//!
	void calculateHuygensSymmetries(const GridDescription & gridDesc);
	Vector3i huygensSymmetry(const HuygensSurfaceDescription & surf);
	
	void loadSpaceVaryingData();
	
	//!
	void generateRunlines();
	void genRunlinesInOctant(int octant);
	
	VoxelGrid mVoxels;
	CellCountGridPtr mCentralIndices;
	std::vector<CellCountGridPtr> mPMLFaceIndices;
	
	Rect3i mNonPMLRegion;
	Rect3i mCalcRegion;
	Vector3i mOriginYee;
	Vector3i mNumYeeCells;
	Vector3i mNumHalfCells;
	int m_nnx;
	int m_nny;
	int m_nnz;
	int m_nx;
	int m_ny;
	int m_nz;
	
	std::vector<Vector3i> mHuygensRegionSymmetries;
	
	friend std::ostream & operator<< (std::ostream & out,
		const VoxelizedGrid & grid);
};

std::ostream & operator<< (std::ostream & out, const VoxelizedGrid & grid);



#endif
