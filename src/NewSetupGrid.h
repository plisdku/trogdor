/*
 *  NewSetupGrid.h
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

class GridDescription;
class VoxelizedGrid;
typedef Pointer<VoxelizedGrid> VoxelizedGridPtr;

class MaterialPaint
{
public:
	MaterialPaint(std::string name);
};

class VoxelizedGrid
{
public:
	VoxelizedGrid(const GridDescription & gridDesc, 
		const Map<std::string, VoxelizedGridPtr> & voxelizedGrids, 
		Mat3i orientation);
private:
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<std::string, VoxelizedGridPtr> & voxelizedGrids,
		Mat3i orientation);
	void paintFromLinks(const GridDescription & gridDesc, Mat3i orientation);
	void paintFromTFSFSources(const GridDescription & gridDesc,
		Mat3i orientation);
	void paintPML();
	
	std::vector<unsigned short> mMaterialHalfCells;
	std::vector<unsigned short> mExtraPropertyHalfCells;
	Rect3i mNonPMLRegion;
	Rect3i mCalcRegion;
	int m_nnx;
	int m_nny;
	int m_nnz;
	int m_nx;
	int m_ny;
	int m_nz;
	
	std::vector<Mat3i> mLinkSymmetries;
	std::vector<Mat3i> mTFSFSourceSymmetries;
};




#endif
