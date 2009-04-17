/*
 *  VoxelizedPartition.h
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
#include "MaterialBoss.h"
#include "MemoryUtilities.h"

#include "SimulationDescriptionPredeclarations.h"

#include "Paint.h"

class VoxelizedPartition
{
public:
	VoxelizedPartition(const GridDescription & gridDesc, 
		const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
		Rect3i partitionBounds, Rect3i calcRegion);  // !
	
	const std::vector<Vector3i> & getHuygensRegionSymmetries() const {
		return mHuygensRegionSymmetries; }
	
	bool partitionHasPML(int faceNum) const;
	Rect3i getPMLRegionOnFace(int faceNum) const;
private:
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids);
	void paintFromHuygensSurfaces(const GridDescription & gridDesc);
	void paintFromCurrentSources(const GridDescription & gridDesc);
	void paintPML();
	
	void calculateMaterialIndices();
	
	void calculateHuygensSymmetries(const GridDescription & gridDesc);
	Vector3i huygensSymmetry(const HuygensSurfaceDescription & surf);
	
	void createMaterialDelegates();
	void loadSpaceVaryingData();
	void generateRunlines();
	void genRunlinesInOctant(int octant);
	
	VoxelGrid mVoxels;
	CellCountGridPtr mCentralIndices;
	//std::vector<CellCountGridPtr> mPMLFaceIndices;
	
	class EHBufferSet
	{
	public:
		EHBufferSet() : buffers(6) {}
		std::vector<MemoryBuffer> buffers;
	};
	
	EHBufferSet mEHBuffers;
	Map<NeighborBufferDescPtr, EHBufferSet> mNBBuffers;
	Map<Paint*, MaterialDelegatePtr> mDelegates;
	
	Rect3i mPartitionBounds;
	Rect3i mCalcRegion;
	
	Rect3i mNonPMLRegion;
	Vector3i mOriginYee;
		
	std::vector<Vector3i> mHuygensRegionSymmetries;
	
	friend std::ostream & operator<< (std::ostream & out,
		const VoxelizedPartition & grid);
};

std::ostream & operator<< (std::ostream & out, const VoxelizedPartition & grid);



#endif
