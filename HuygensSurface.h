/*
 *  HuygensSurface.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _HUYGENSSURFACE_
#define _HUYGENSSURFACE_

#include "SimulationDescriptionPredeclarations.h"
#include "InterleavedLattice.h"
#include "Pointer.h"
#include "MemoryUtilities.h"
#include "Map.h"
#include "geometry.h"
#include <vector>

class HuygensSurface;
typedef Pointer<HuygensSurface> HuygensSurfacePtr;
class VoxelizedPartition;
typedef Pointer<VoxelizedPartition> VoxelizedPartitionPtr;

class NeighborBuffer;
typedef Pointer<NeighborBuffer> NeighborBufferPtr;

class HuygensUpdate;
typedef Pointer<HuygensUpdate> HuygensUpdatePtr;

/**
 * Factory for HuygensSurface.  Constructs a HuygensSurface and a HuygensUpdate
 * and calls huygensSurface->setUpdater(huygensUpdate).
 *
 * TODO: Refactor this all into HuygensSurface static methods?
 *
 * @see HuygensLink
 */
class HuygensSurfaceFactory
{
public:
    static HuygensSurfacePtr newHuygensSurface(
        std::string namePrefix,
        const VoxelizedPartition & vp,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
        const HuygensSurfaceDescPtr & desc);
private:
    HuygensSurfaceFactory() {}
};

/**
 * Implementation of a total-field scattered-field (TFSF) boundary.
 * A Huygens surface consists of a set of up to six NeighborBuffers, each
 * providing storage for E and H fields around the TFSF boundary, along with
 * pointers to the destination InterleavedLattice and, if applicable, the
 * source InterleavedLattice.  The operations to load E and H fields into
 * the NeighborBuffers must be provided by an implementation of HuygensUpdate.
 */
class HuygensSurface
{
public:
    /**
     * Create a Huygens surface, storing the description and dimensions and a
     * pointer to the destination InterleavedLattice (this is the lattice of EH
     * fields in which the TFSF boundary is embedded).  The description may
     * provide a pointer to a source InterleavedLattice.  After this call, the
     * HuygensSurface still lacks a HuygensUpdate; call setUpdater to complete
     * initialization.
     *
     * @param namePrefix A string prepended to the name of each field buffer, 
     *  perhaps useful for debugging.
     * @param vp Pointer to the embedding VoxelizedPartition
     * @param grids Complete list of VoxelizedPartitions, used to obtain the
     *  source InterleavedLattice if this is a HuygensLink
     * @param desc Description information for this HuygensSurface
     */
    HuygensSurface(std::string namePrefix, const VoxelizedPartition & vp,
        const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
        const HuygensSurfaceDescPtr & desc);
    
    HuygensSurfaceDescPtr description() const { return mDescription; }
    const Rect3i & halfCells() const { return mHalfCells; }
    bool hasBuffer(int side) const { return mNeighborBuffers.at(side) != 0L; }
    NeighborBufferPtr buffer(int side) const
        { return mNeighborBuffers.at(side); }
    const std::vector<NeighborBufferPtr> & neighborBuffers() const
        { return mNeighborBuffers; }
    
    InterleavedLatticePtr destLattice() const { return mDestLattice; }
    InterleavedLatticePtr sourceLattice() const { return mSourceLattice; }
    void allocate();
    
    /**
     * Complete the initialization of the HuygensSurface by providing the object
     * that puts E and H fields in the NeighborBuffers on each timestep.  Each
     * call to updateE() and updateH() will internally call update->updateE()
     * or update->updateH() with all necessary parameters.
     *
     * @param update Pointer to update object
     */
    void setUpdater(HuygensUpdatePtr update) { mUpdate = update; }
    
    /**
     * If setUpdater() has provided the object to generate E and H fields on
     * the boundary, this function calls updater->updateE().
     */
    void updateE();
    
    /**
     * If setUpdater() has provided the object to generate E and H fields on
     * the boundary, this function calls updater->updateH().
     */
    void updateH();
private:
    HuygensSurfaceDescPtr mDescription;
    Rect3i mHalfCells;
    HuygensUpdatePtr mUpdate;
    std::vector<NeighborBufferPtr> mNeighborBuffers;
    InterleavedLatticePtr mDestLattice;
    InterleavedLatticePtr mSourceLattice;
};

/**
 * Abstract base class for all objects that provide E and H fields around the
 * TFSF boundary.  The subclass HuygensLink, for instance, sums fields from the
 * destination lattice (the embedding grid, partitioned into total and scattered
 * field regions) and the source lattice (the source grid containing only
 * incident fields) with appropriate signs and stores them in the
 * HugyensSurface's six (or fewer) NeighborBuffers.
 */
class HuygensUpdate
{
public:
    HuygensUpdate() {}
    virtual ~HuygensUpdate() {}
    
    /**
     * Called once per timestep, before the H fields are updated, to provide
     * E fields to the H update equations around the total-field scattered-field
     * boundary.
     */
    virtual void updateE(HuygensSurface & hs) {}
    
    /**
     * Called once per timestep, before the E fields are updated, to provide
     * H fields to the H update equations around the total-field scattered-field
     * boundary.
     */
    virtual void updateH(HuygensSurface & hs) {}
};

/**
 * Storage for the E and H fields on one face of a total-field scattered-field
 * boundary.  Trogdor supports rectangular TFSF boundaries, and each face
 * (up to six faces in 3D) contains E and H fields.  Incident fields will be
 * added to the innermost half-cell layer of the scattered-field region and 
 * subtracted from the outermost half-cell layer of the total-field region; the
 * coefficients (+1 and -1) are stored as well.  Floquet sources may eventually
 * be supported, and for this reason the class also stores coefficients for
 * source AND destination fields.
 */
class NeighborBuffer
{
public:
    NeighborBuffer(std::string prefix,
        const Rect3i & huygensHalfCells, int sideNum,
        float incidentFieldFactor);
    NeighborBuffer(std::string prefix,
        const Rect3i & huygensHalfCells, 
        const Rect3i & sourceHalfCells,
        int sideNum,
        float incidentFieldFactor);
    
    const Rect3i & destHalfCells() const;
    const Rect3i & sourceHalfCells() const;
    float destFactorE(int fieldDirection) const;
    float destFactorH(int fieldDirection) const;
    float sourceFactorE(int fieldDirection) const;
    float sourceFactorH(int fieldDirection) const;
    
    InterleavedLatticePtr lattice() const { return mLattice; }
private:
    Rect3i edgeHalfCells(const Rect3i & halfCells, int nSide);
    void initFactors(const Rect3i & huygensHalfCells, int sideNum,
        float incidentFieldFactor);
    
    int side;
    InterleavedLatticePtr mLattice;
    
    Rect3i mSourceHalfCells; // not always used
    
    std::vector<float> mDestFactorsE;
	std::vector<float> mSourceFactorsE;
	std::vector<float> mDestFactorsH;
	std::vector<float> mSourceFactorsH;
};


#endif
