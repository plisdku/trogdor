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

#include "SetupMaterial.h"
#include "UpdateEquation.h"
#include "Source.h"
#include "Output.h"
#include "CurrentSource.h"
#include "HuygensSurface.h"

#include "MemoryUtilities.h"

#include "SimulationDescriptionPredeclarations.h"

#include "Paint.h"

#include <vector>
#include <string>

class RunlineEncoder;

class VoxelizedPartition
{
public:
	VoxelizedPartition(GridDescPtr gridDesc, 
		const Map<GridDescPtr, Pointer<VoxelizedPartition> > & voxelizedGrids,
		Rect3i allocRegion, Rect3i calcRegion,
        int runlineDirection );  // !
	
    GridDescPtr gridDescription() const { return mGridDescription; }
    
	const std::vector<Vector3i> & huygensRegionSymmetries() const {
		return mHuygensRegionSymmetries; }
	
	const Rect3i & gridHalfCells() const { return mGridHalfCells; }
    Rect3i gridYeeCells() const;
    const Rect3i & allocHalfCells() const { return mFieldAllocHalfCells; }
    Rect3i allocYeeCells() const;
    const Rect3i & calcHalfCells() const { return mCalcHalfCells; }
    Vector3i originYee() const { return mOriginYee; }
	bool partitionHasPML(int faceNum) const;
	Rect3i pmlHalfCellsOnFace(int faceNum) const;
	Rect3i partitionPMLHalfCellsOnFace(int faceNum) const;
	Rect3i pmlHalfCells(Vector3i pmlDir) const;
    
    // returns      which material goes in which cell in this partition
	const VoxelGrid & voxels() const { return mVoxels; }
    
    // returns      the index of each cell by material type (air #1, air #2...)
	const PartitionCellCount & indices() const
		{ assert(mCentralIndices != 0L); return *mCentralIndices; }
    
    const InterleavedLattice & lattice() const
        { assert(mLattice != 0L); return *mLattice; }
    
    InterleavedLatticePtr getLattice() const { return mLattice; }
    
    // returns      the structures that store temp data for setting up materials
    const Map<Paint*, Pointer<SetupMaterial> > & setupMaterials() const
        { return mSetupMaterials; }
    
    // returns      the structures that store temp data for setting up outputs
    const std::vector<Pointer<SetupOutput> > & setupOutputs() const
        { return mSetupOutputs; }
    
    // returns      the structures that store temp data for setting up sources
    const std::vector<Pointer<SetupSource> > & softSetupSources() const
        { return mSoftSetupSources; }
    
    // returns      the structures that store temp data for setting up sources
    const std::vector<Pointer<SetupSource> > & hardSetupSources() const
        { return mHardSetupSources; }
    
    const std::vector<Pointer<SetupCurrentSource> > & setupCurrentSources()
        const { return mSetupCurrentSources; }
    
    // returns      the structures that store temp data for setting up NBs
    const std::vector<Pointer<HuygensSurface> > & huygensSurfaces()
        const { return mHuygensSurfaces; }
    
    void clearVoxelGrid();
    void clearCellCountGrid();
    
    void createHuygensSurfaces(
        const GridDescPtr & gridDescription,
        const Map<GridDescPtr, Pointer<VoxelizedPartition> > & grids);
    
    /**
     *  Run length encode the update equations on the grid for speedy
     *  calculation of electromagnetic fields (FDTD!).
     *
     *  This step also makes current sources, because they depend on the
     *  voxel grid and partition cell count (they need Paints).
     */
    void calculateRunlines();
    
    void writeDataRequest(const HuygensSurfaceDescPtr surf,
        const GridDescPtr gridDescription) const;
	
    /**
     *  Scan a run length encoder over the given rect and octant of the grid.
     *  The entire rect will be processed (so the final runlines will fill
     *  the rect).  This is suitable for use with RLE outputs.
     */
    void runLengthEncode(RunlineEncoder & encoder,
        Rect3i yeeCells, int octant) const;
    
    /**
     *  Scan a set of run length encoders over the given rect and octant of
     *  the grid.  Each encoder will be responsible for one Paint.
     *  (The paints will be compared without curl buffers, i.e. they represent
     *  unique update equations, regardless of where the field data comes from.)
     *  Paints that do not have encoders will be passed over, so not every cell
     *  will be included in a runline.
     */
    void runLengthEncode(std::map<Paint*, RunlineEncoder*>& encoders,
        Rect3i yeeCells, int octant) const;
    
private:
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<GridDescPtr, Pointer<VoxelizedPartition> > & voxelizedGrids);
	void paintFromHuygensSurfaces(const GridDescription & gridDesc);
	void paintFromCurrentSources(const GridDescription & gridDesc);
	void paintPML();
	
	void calculateMaterialIndices();
	
	void calculateHuygensSurfaceSymmetries(const GridDescription & gridDesc);
	Vector3i huygensSymmetry(const HuygensSurfaceDescription & surf);
	
	void createSetupMaterials(const GridDescription & gridDesc);
	void loadSpaceVaryingData();
    
    void createSetupOutputs(const std::vector<OutputDescPtr> & outputs);
    void createSetupSources(const std::vector<SourceDescPtr> & sources);
    void createSetupCurrentSources(
        const std::vector<CurrentSourceDescPtr> & currentSources);
    
    GridDescPtr mGridDescription;
    
	VoxelGrid mVoxels;
	Pointer<PartitionCellCount> mCentralIndices;
	
    Pointer<InterleavedLattice> mLattice;
	
    // THIS IS WHERE GRID DENIZENS LIVE
	Map<Paint*, Pointer<SetupMaterial> > mSetupMaterials;
	std::vector<Pointer<SetupOutput> > mSetupOutputs;
    std::vector<Pointer<SetupSource> > mSoftSetupSources;
    std::vector<Pointer<SetupSource> > mHardSetupSources;
    std::vector<Pointer<SetupCurrentSource> > mSetupCurrentSources;
    
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
