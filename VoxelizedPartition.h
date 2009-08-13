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
#include "InterleavedLattice.h"

#include "Material.h"
#include "OutputBoss.h"
#include "SourceBoss.h"
#include "HuygensSurface.h"

#include "MemoryUtilities.h"

#include "SimulationDescriptionPredeclarations.h"

#include "Paint.h"

#include <vector>
#include <string>

class VoxelizedPartition
{
public:
	VoxelizedPartition(const GridDescription & gridDesc, 
		const Map<GridDescPtr, Pointer<VoxelizedPartition> > & voxelizedGrids,
		Rect3i allocRegion, Rect3i calcRegion,
        int runlineDirection );  // !
	
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
    
    // returns      which material goes in which cell in this partition
	const VoxelGrid & getVoxelGrid() const { return mVoxels; }
    
    // returns      the index of each cell by material type (air #1, air #2...)
	const Pointer<PartitionCellCount> & getIndices() const
		{ return mCentralIndices; }
    
    Pointer<InterleavedLattice> getLattice() const { return mLattice; }
    
    // returns      the structures that store temp data for setting up materials
    const Map<Paint*, Pointer<SetupUpdateEquation> > & getDelegates() const
        { return mSetupMaterials; }
    
    // returns      the structures that store temp data for setting up outputs
    const std::vector<Pointer<SetupOutput> > & getSetupOutputs() const
        { return mSetupOutputs; }
    
    // returns      the structures that store temp data for setting up sources
    const std::vector<Pointer<SetupSource> > & getSoftSetupSources() const
        { return mSoftSetupSources; }
    
    // returns      the structures that store temp data for setting up sources
    const std::vector<Pointer<SetupSource> > & getHardSetupSources() const
        { return mHardSetupSources; }
    
    // returns      the structures that store temp data for setting up NBs
    const std::vector<Pointer<HuygensSurface> > & getHuygensSurfaces()
        const { return mHuygensSurfaces; }
    
    void clearVoxelGrid();
    void clearCellCountGrid();
    
    void createHuygensSurfaces(
        const GridDescPtr & gridDescription,
        const Map<GridDescPtr, Pointer<VoxelizedPartition> > & grids);
    
    void calculateRunlines();
    
    void writeDataRequest(const HuygensSurfaceDescPtr surf,
        const GridDescPtr gridDescription) const;
	
private:
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<GridDescPtr, Pointer<VoxelizedPartition> > & voxelizedGrids);
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
    /*
    void generateOutputRunlines();
    void genOutputRunlinesInOctant(int octant);
    */
    void createSetupOutputs(const std::vector<OutputDescPtr> & outputs);
    void createSetupSources(const std::vector<SourceDescPtr> & sources);
    
	VoxelGrid mVoxels;
	Pointer<PartitionCellCount> mCentralIndices;
	
    Pointer<InterleavedLattice> mLattice;
	
    // THIS IS WHERE GRID DENIZENS LIVE
	Map<Paint*, Pointer<SetupUpdateEquation> > mSetupMaterials;
	std::vector<Pointer<SetupOutput> > mSetupOutputs;
    std::vector<Pointer<SetupSource> > mSoftSetupSources;
    std::vector<Pointer<SetupSource> > mHardSetupSources;
    
    std::vector<Pointer<HuygensSurface> > mHuygensSurfaces;
    
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
