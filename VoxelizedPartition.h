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
//#include "HuygensSurface.h"
#include "MemoryUtilities.h"

#include "SimulationDescriptionPredeclarations.h"

#include "Paint.h"

#include <vector>
#include <string>

class InterleavedLattice;
typedef Pointer<InterleavedLattice> InterleavedLatticePtr;

class VoxelizedPartition;
typedef Pointer<VoxelizedPartition> VoxelizedPartitionPtr;

class HuygensSurface;
typedef Pointer<HuygensSurface> HuygensSurfacePtr;

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
	
    // halfCell     global half-cell coordinate, possibly outside alloc region
    // returns      linearYeeIndex into the correct field
	BufferPointer fieldPointer(Vector3i halfCell) const;
    
    // nb           partition's neighbor buffer
    // halfCell     global half-cell coordinate, possibly outside alloc region
    // returns      linearYeeIndex into partition's neighbor buffer
	BufferPointer fieldPointer(const NeighborBufferDescPtr & nb,
		Vector3i halfCell) const;
    
    // direction      0,1,2 for x,y,z
    // xi,xj,xk       global Yee cell coordinates
    // returns        BufferPointer to EM field
    //BufferPointer getE(int direction, int xi, int xj, int xk) const;
    //BufferPointer getH(int direction, int xi, int xj, int xk) const;
    BufferPointer getE(int direction, Vector3i yeeCell) const;
    BufferPointer getH(int direction, Vector3i yeeCell) const;
        
    // returns      amount to add to float* to go from Ex(here) to Ex(neighbor) 
    Vector3i getFieldStride() const;
	
    // returns      which material goes in which cell in this partition
	const VoxelGrid & getVoxelGrid() const { return mVoxels; }
    
    // returns      the index of each cell by material type (air #1, air #2...)
	const PartitionCellCountPtr & getIndices() const
		{ return mCentralIndices; }
    
    InterleavedLatticePtr getLattice() const { return mLattice; }
    
    // returns      the structures that store temp data for setting up materials
    const Map<Paint*, SetupMaterialPtr> & getDelegates() const
        { return mSetupMaterials; }
    
    // returns      the structures that store temp data for setting up outputs
    const std::vector<SetupOutputPtr> & getSetupOutputs() const
        { return mSetupOutputs; }
    
    // returns      the structures that store temp data for setting up sources
    const std::vector<SetupSourcePtr> & getSoftSetupSources() const
        { return mSoftSetupSources; }
    
    // returns      the structures that store temp data for setting up sources
    const std::vector<SetupSourcePtr> & getHardSetupSources() const
        { return mHardSetupSources; }
    
    // returns      the structures that store temp data for setting up NBs
    const std::vector<HuygensSurfacePtr> & getHuygensSurfaces()
        const { return mHuygensSurfaces; }
    
    void clearVoxelGrid();
    void clearCellCountGrid();
    
    void createHuygensSurfaces(
        const GridDescPtr & gridDescription,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids);
    
    void calculateRunlines();
	
private:
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids);
	void paintFromHuygensSurfaces(const GridDescription & gridDesc);
	void paintFromCurrentSources(const GridDescription & gridDesc);
	void paintPML();
	
	void calculateMaterialIndices();
	
	void calculateHuygensSymmetries(const GridDescription & gridDesc);
	Vector3i huygensSymmetry(const HuygensSurfaceDescription & surf);
	
	void createSetupMaterials(const GridDescription & gridDesc);
	void loadSpaceVaryingData();
	void generateRunlines();
	void genRunlinesInOctant(int octant);
    
    void createSetupOutputs(const std::vector<OutputDescPtr> & outputs);
    void createSetupSources(const std::vector<SourceDescPtr> & sources);
    
	VoxelGrid mVoxels;
	PartitionCellCountPtr mCentralIndices;
	
    InterleavedLatticePtr mLattice;
	
    // THIS IS WHERE GRID DENIZENS LIVE
	Map<Paint*, SetupMaterialPtr> mSetupMaterials;
	std::vector<SetupOutputPtr> mSetupOutputs;
    std::vector<SetupSourcePtr> mSoftSetupSources;
    std::vector<SetupSourcePtr> mHardSetupSources;
    
    std::vector<HuygensSurfacePtr> mHuygensSurfaces;
    
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
	
	friend std::ostream & operator<< (std::ostream & out,
		const VoxelizedPartition & grid);
};

std::ostream & operator<< (std::ostream & out, const VoxelizedPartition & grid);



#endif
