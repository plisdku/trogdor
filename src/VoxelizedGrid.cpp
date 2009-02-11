/*
 *  NewSetupGrid.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "NewSetupGrid.h"

#include "SimulationDescription.h"
#include "Log.h"

using namespace std;

VoxelizedGrid::
VoxelizedGrid(const GridDescription & gridDesc, 
	const Map<string, VoxelizedGridPtr> & voxelizedGrids,
	Mat3i orientation)
{
	LOG << "VoxelizedGrid()\n";
	
	Vector3i nxyz = orientation*gridDesc.getNumYeeCells();
	Vector3i nnxyz = orientation*gridDesc.getNumHalfCells();
	m_nx = nxyz[0]; m_ny = nxyz[1]; m_nz = nxyz[2];
	m_nnx = nnxyz[0]; m_nny = nnxyz[1]; m_nnz = nnxyz[2];
	mNonPMLRegion = orientation*gridDesc.getNonPMLRegion();
	mCalcRegion = orientation*gridDesc.getCalcRegion();
	
	paintFromAssembly(gridDesc, voxelizedGrids, orientation);
	paintFromLinks(gridDesc, orientation);
	paintFromTFSFSources(gridDesc, orientation);
	paintPML();
	
	// 1.  Paint in basic materials
	// 2.  Calculate symmetries of link regions and paint edges
	// 3.  Calculate symmetries of TFSF regions and paint edges
	// 4.  Paint PML
}

void VoxelizedGrid::
paintFromAssembly(const GridDescription & gridDesc,
	const Map<string, VoxelizedGridPtr> & voxelizedGrids,
	Mat3i orientation)
{
	LOG << "Painting from assembly.\n";
}

void VoxelizedGrid::
paintFromLinks(const GridDescription & gridDesc, Mat3i orientation)
{
	LOG << "Painting from links.\n";
}

void VoxelizedGrid::
paintFromTFSFSources(const GridDescription & gridDesc, Mat3i orientation)
{
	LOG << "Painting from TFSF sources.\n";
}

void VoxelizedGrid::
paintPML()
{
	LOG << "Painting PML.\n";
}







