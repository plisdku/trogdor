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

#include "SetupUpdateEquation.h"
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
	VoxelizedPartition(SimulationDescPtr simDesc,
        GridDescPtr gridDesc, 
		const Map<GridDescPtr, Pointer<VoxelizedPartition> > & voxelizedGrids,
		Rect3i allocRegion, Rect3i calcRegion,
        int runlineDirection );  // !
	
    SimulationDescPtr simulationDescription() const
        { return mSimulationDescription; }
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
    const Map<Paint*, Pointer<SetupUpdateEquation> > & setupMaterials() const
        { return mSetupUpdateEquations; }
    
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
    
    //void writeDataRequest(const HuygensSurfaceDescPtr surf,
    //    const GridDescPtr gridDescription) const;
    
    void writeDataRequests() const;
	
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
    
    /**
     *  Determine the symmetry of the materials in the grid within the Huygens
     *  surface (the total-field region).  The TF region is assumed to be
     *  symmetrical; symmetry in x is broken if the material at (x0, y, z) is
     *  not the same as the material at (x1, y, z) for any y or z on the +/- x
     *  faces, or if the material is not constant along (x, y_side, z_side), 
     *  the +/- y and +/- z faces of the TF box.  However, symmetry is only
     *  determined for non-omitted sides (so if the TF region is not symmetrical
     *  it may be possible to omit some of the sides of the Huygens surface to
     *  obtain a "symmetrical" TF region).
     */
	Vector3i huygensSymmetry(const HuygensSurfaceDescription & surf);
	
	void createSetupUpdateEquations(const GridDescription & gridDesc);
	void loadSpaceVaryingData();
    
    void createSetupOutputs(const std::vector<OutputDescPtr> & outputs);
    void createSetupSources(const std::vector<SourceDescPtr> & sources);
    void createSetupCurrentSources(
        const std::vector<CurrentSourceDescPtr> & currentSources);
    
    SimulationDescPtr mSimulationDescription;
    GridDescPtr mGridDescription;
    
	VoxelGrid mVoxels;
	Pointer<PartitionCellCount> mCentralIndices;
	
    Pointer<InterleavedLattice> mLattice;
	
    // THIS IS WHERE GRID DENIZENS LIVE
	Map<Paint*, Pointer<SetupUpdateEquation> > mSetupUpdateEquations;
	std::vector<Pointer<SetupOutput> > mSetupOutputs;
    std::vector<Pointer<SetupSource> > mSoftSetupSources;
    std::vector<Pointer<SetupSource> > mHardSetupSources;
    std::vector<Pointer<SetupCurrentSource> > mSetupCurrentSources;
    
    std::vector<Pointer<HuygensSurface> > mHuygensSurfaces;
    
    // END OF GRID DENIZEN ZONE
    
	long m_nx, m_ny, m_nz;
    long m_nnx, m_nny, m_nnz;
    long m_nnx0, m_nny0, m_nnz0; // equal to mFieldAllocHalfCells.p1
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
