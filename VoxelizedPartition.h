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
#include "PartitionCellCount.h"
#include "MaterialBoss.h"
#include "OutputBoss.h"
#include "SourceBoss.h"
#include "MemoryUtilities.h"

#include "SimulationDescriptionPredeclarations.h"

#include "Paint.h"

class VoxelizedPartition;
typedef Pointer<VoxelizedPartition> VoxelizedPartitionPtr;

class VoxelizedPartition
{
public:
	VoxelizedPartition(const GridDescription & gridDesc, 
		const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
		Rect3i allocRegion, Rect3i calcRegion);  // !
	
	const std::vector<Vector3i> & getHuygensRegionSymmetries() const {
		return mHuygensRegionSymmetries; }
	
	const Rect3i & getGridHalfCells() const { return mGridHalfCells; }
	bool partitionHasPML(int faceNum) const;
	Rect3i getPMLRegionOnFace(int faceNum) const;
	Rect3i getPartitionPMLRegionOnFace(int faceNum) const;
	Rect3i getPMLRegion(Vector3i pmlDir) const;
	
	long linearYeeIndex(int ii, int jj, int kk) const;
	long linearYeeIndex(const Vector3i & halfCell) const;
	
	long linearYeeIndex(const NeighborBufferDescPtr & nb,
		int ii, int jj, int kk) const;
	long linearYeeIndex(const NeighborBufferDescPtr & nb,
		const Vector3i & halfCell) const;
	
	BufferPointer fieldPointer(Vector3i halfCell) const;
	BufferPointer fieldPointer(const NeighborBufferDescPtr & nb,
		Vector3i halfCell) const;
	
	const VoxelGrid & getVoxelGrid() const { return mVoxels; }
	const PartitionCellCountPtr & getIndices() const
		{ return mCentralIndices; }
    
    EHBufferSetPtr getEHBuffers() const { return mEHBuffers; }
    const Map<Paint*, MaterialDelegatePtr> & getDelegates() const
        { return mDelegates; }
    
    const std::vector<OutputDelegatePtr> & getOutputDelegates() const
        { return mOutputDelegates; }
    
    void clearVoxelGrid();
    void clearCellCountGrid();
	
private:
	void initFieldBuffers();
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
    
    void createOutputDelegates(const std::vector<OutputDescPtr> & outputs);
    void createSourceDelegates(const std::vector<SourceDescPtr> & sources);
	
	VoxelGrid mVoxels;
	PartitionCellCountPtr mCentralIndices;
	
	EHBufferSetPtr mEHBuffers;
	Map<NeighborBufferDescPtr, EHBufferSet> mNBBuffers;
	
    // THIS IS WHERE GRID DENIZENS LIVE
	Map<Paint*, MaterialDelegatePtr> mDelegates;
	std::vector<OutputDelegatePtr> mOutputDelegates;
    std::vector<SourceDelegatePtr> mSourceDelegates;
    int mSources;
    int mInputs;
    
    // END OF GRID DENIZEN ZONE
    
	int m_nx, m_ny, m_nz;
	Rect3i mGridHalfCells;
	Rect3i mFieldAllocRegion; // must be full Yee cells!
	Rect3i mAuxAllocRegion; // doesn't need to be full Yee cells!
	Rect3i mCalcRegion;
	
	Rect3i mNonPMLRegion;
	Vector3i mOriginYee;
		
	std::vector<Vector3i> mHuygensRegionSymmetries;
	
	friend std::ostream & operator<< (std::ostream & out,
		const VoxelizedPartition & grid);
};

std::ostream & operator<< (std::ostream & out, const VoxelizedPartition & grid);



#endif
