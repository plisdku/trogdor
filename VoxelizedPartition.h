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
#include "Map.h"
#include "VoxelGrid.h"
#include "PartitionCellCount.h"
#include "MaterialBoss.h"
#include "OutputBoss.h"
#include "SourceBoss.h"
#include "MemoryUtilities.h"

#include "SimulationDescriptionPredeclarations.h"

#include "Paint.h"

#include <vector>
#include <string>


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
    Rect3i getGridYeeCells() const;
    const Rect3i & getAllocHalfCells() const { return mFieldAllocHalfCells; }
    Rect3i getAllocYeeCells() const;
    Vector3i getOriginYee() const { return mOriginYee; }
	bool partitionHasPML(int faceNum) const;
	Rect3i getPMLHalfCellsOnFace(int faceNum) const;
	Rect3i getPartitionPMLHalfCellsOnFace(int faceNum) const;
	Rect3i getPMLHalfCells(Vector3i pmlDir) const;
	
    // v            global half-cell coordinate, possibly outside alloc region
    // returns      a vector inside the alloc region, offset from v by integer
    //              multiples of the alloc half cells in each dimension.
    Vector3i wrap(Vector3i vv) const;
    
    // ii,jj,kk     global half-cell coordinate, possibly outside alloc region
    // returns      a vector inside the alloc region, offset from v by integer
    //              multiples of the alloc half cells in each dimension.
    Vector3i wrap(int ii, int jj, int kk) const;
    
    // nb           partition's neighbor buffer
    // v            global half-cell coordinate, possibly outside alloc region
    // returns      a vector inside the alloc region, offset from v by integer
    //              multiples of the alloc half cells in each dimension.
    Vector3i wrap(const NeighborBufferDescPtr & nb, Vector3i vv) const;
    
    // nb           partition's neighbor buffer
    // ii,jj,kk     global half-cell coordinate, possibly outside alloc region
    // returns      a vector inside the alloc region, offset from v by integer
    //              multiples of the alloc half cells in each dimension.
    //Vector3i wrap(const NeighborBufferDescPtr & nb, int ii, int jj, int kk)
    //    const;
    
    // ii,jj,kk     global half-cell coordinates, inside the alloc region
    // returns      index into partition's alloc region
	//long linearYeeIndex(int ii, int jj, int kk) const;
	long linearYeeIndex(const Vector3i & halfCell) const;
	
    // nb           partition's neighbor buffer
    // ii,jj,kk     global half-cell coordinates, inside the alloc region
    // returns      index into partition's neighbor buffer
	//long linearYeeIndex(const NeighborBufferDescPtr & nb,
	//	int ii, int jj, int kk) const;
	long linearYeeIndex(const NeighborBufferDescPtr & nb,
		const Vector3i & halfCell) const;
	
    // halfCell     global half-cell coordinate, possibly outside alloc region
    // returns      linearYeeIndex into the correct field
	BufferPointer fieldPointer(Vector3i halfCell) const;
    
    // nb           partition's neighbor buffer
    // halfCell     global half-cell coordinate, possibly outside alloc region
    // returns      linearYeeIndex into partition's neighbor buffer
	BufferPointer fieldPointer(const NeighborBufferDescPtr & nb,
		Vector3i halfCell) const;
    
    // returns      amount to add to float* to go from Ex(here) to Ex(neighbor) 
    Vector3i getFieldStride() const;
	
    // returns      which material goes in which cell in this partition
	const VoxelGrid & getVoxelGrid() const { return mVoxels; }
    
    // returns      the index of each cell by material type (air #1, air #2...)
	const PartitionCellCountPtr & getIndices() const
		{ return mCentralIndices; }
    
    // returns      the allocation skeleton for fields in this partition
    const std::vector<MemoryBufferPtr> & getEHBuffers() const
        { return mEHBuffers; }
    
    // returns      the allocation skeleton for partition's neighbor buffers
    const Map<NeighborBufferDescPtr, std::vector<MemoryBufferPtr> > &
        getNBBuffers() const { return mNBBuffers; }
    
    // returns      the structures that store temp data for setting up materials
    const Map<Paint*, MaterialDelegatePtr> & getDelegates() const
        { return mMaterialDelegates; }
    
    // returns      the structures that store temp data for setting up outputs
    const std::vector<OutputDelegatePtr> & getOutputDelegates() const
        { return mOutputDelegates; }
    
    // returns      the structures that store temp data for setting up sources
    const std::vector<SourceDelegatePtr> & getSoftSourceDelegates() const
        { return mSoftSourceDelegates; }
    
    // returns      the structures that store temp data for setting up sources
    const std::vector<SourceDelegatePtr> & getHardSourceDelegates() const
        { return mHardSourceDelegates; }
    
    // returns      the structures that store temp data for setting up NBs
    const std::vector<NeighborBufferDelegatePtr> & getNeighborBufferDelegates()
        const { return mNeighborBufferDelegates; }
    
    void clearVoxelGrid();
    void clearCellCountGrid();
    
    void createHuygensSurfaceDelegates();
	
private:
	void initFieldBuffers(std::string bufferNamePrefix);
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids);
	void paintFromHuygensSurfaces(const GridDescription & gridDesc);
	void paintFromCurrentSources(const GridDescription & gridDesc);
	void paintPML();
	
	void calculateMaterialIndices();
	
	void calculateHuygensSymmetries(const GridDescription & gridDesc);
	Vector3i huygensSymmetry(const HuygensSurfaceDescription & surf);
	
	void createMaterialDelegates(const GridDescription & gridDesc);
	void loadSpaceVaryingData();
	void generateRunlines();
	void genRunlinesInOctant(int octant);
    
    void createOutputDelegates(const std::vector<OutputDescPtr> & outputs);
    void createSourceDelegates(const std::vector<SourceDescPtr> & sources);
    /*
    void createNeighborBufferDelegates(
        const std::vector<HuygensSurfaceDescPtr> & surfaces,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids);
	*/
    
	VoxelGrid mVoxels;
	PartitionCellCountPtr mCentralIndices;
	
	std::vector<MemoryBufferPtr> mEHBuffers;
	Map<NeighborBufferDescPtr, std::vector<MemoryBufferPtr> > mNBBuffers;
	
    // THIS IS WHERE GRID DENIZENS LIVE
	Map<Paint*, MaterialDelegatePtr> mMaterialDelegates;
	std::vector<OutputDelegatePtr> mOutputDelegates;
    std::vector<SourceDelegatePtr> mSoftSourceDelegates;
    std::vector<SourceDelegatePtr> mHardSourceDelegates;
    std::vector<NeighborBufferDelegatePtr> mNeighborBufferDelegates;
    
    // END OF GRID DENIZEN ZONE
    
	int m_nx, m_ny, m_nz;
    int m_nnx, m_nny, m_nnz;
    int m_nnx0, m_nny0, m_nnz0; // equal to mFieldAllocHalfCells.p1
    Vector3i mNumAllocHalfCells;
	Rect3i mGridHalfCells;
	Rect3i mFieldAllocHalfCells; // must be full Yee cells!
	Rect3i mAuxAllocRegion; // doesn't need to be full Yee cells!
	Rect3i mCalcHalfCells;
	
	Rect3i mNonPMLHalfCells;
	Vector3i mOriginYee;
		
	std::vector<Vector3i> mHuygensRegionSymmetries;
    
    //Map<Vector3i, Map<std::string, std::string> > mPMLParams;
	
	friend std::ostream & operator<< (std::ostream & out,
		const VoxelizedPartition & grid);
};

std::ostream & operator<< (std::ostream & out, const VoxelizedPartition & grid);



#endif
