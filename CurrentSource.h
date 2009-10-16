/*
 *  CurrentSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _CURRENTSOURCE_
#define _CURRENTSOURCE_

#include <vector>
#include "BufferedFieldInput.h"
#include "RunlineEncoder.h"
#include "MemoryUtilities.h"
#include "SimulationDescription.h"

class VoxelizedPartition;
class CalculationPartition;
class CurrentSource;
class SetupUpdateEquation;

// What this stores:
//  a list of material IDs and their sizes in Ex, Ey, Ez, Hx, Hy and Hz
// What this delivers on request:
//  an ordered list of regions (Ex, ..., Hz) for data requests
// What this uses on calls to makeCurentSource():
//  an ordered list of material IDs and the sizes of each buffer (Ex, ..., Hz)
class SetupCurrentSource
{
public:
    SetupCurrentSource(const CurrentSourceDescPtr & description,
        const VoxelizedPartition & vp);
    virtual ~SetupCurrentSource();
    
    CurrentSourceDescPtr description() const { return mDescription; }
    
    virtual Pointer<CurrentSource> makeCurrentSource(
        const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
    
    /**
     *  Return an ordered list of regions to evaluate Jx, Jy and Jz in.  The
     *  rects are all 1D strips oriented along the runline direction, so there
     *  is no ambiguity about ordering of cells.
     *
     *  @returns    a vector indexed by direction: v[xyz][nn] for xyz = [0-3]
     *              and nn from 0 to some total number N of rectangles.
     */
    std::vector<std::vector<Rect3i> > getRectsJ() const
        { return mRectsJ; }
        
    /**
     *  Return an ordered list of regions to evaluate Kx, Ky and Kz in.  The
     *  rects are all 1D strips oriented along the runline direction, so there
     *  is no ambiguity about ordering of cells.
     *
     *  @returns    a vector indexed by direction: v[xyz][nn] for xyz = [0-3]
     *              and nn from 0 to some total number N of rectangles.
     */
    std::vector<std::vector<Rect3i> > getRectsK() const
        { return mRectsK; }
    
    /**
     *  Return a list of all material IDs that update using this source.  The
     *  ordering of the list is the internal ordering of the current buffer.
     */
    const std::vector<long> & materialIDs() const { return mMaterialIDs; }
    
    /**
     *  Return the number of buffered current components for each material.
     *  The material IDs can be obtained from materialIDs() in the same order as
     *  used here.
     *
     *  @returns    a vector sorted by the internal ordering of the current
     *              buffer; each element of the vector contains the number of
     *              buffered Jx, Jy and Jz components respectively.
     */
    const std::vector<Vector3i> & numCellsJ() const { return mNumCellsJ; }
    
    /**
     *  Return the number of buffered current components for each material.
     *  The material IDs can be obtained from materialIDs() in the same order as
     *  used here.
     *
     *  @returns    a vector sorted by the internal ordering of the current
     *              buffer; each element of the vector contains the number of
     *              buffered Kx, Ky and Kz components respectively.
     */
    const std::vector<Vector3i> & numCellsK() const { return mNumCellsK; }
    
private:
    void initInputRunlines(const VoxelizedPartition & vp);
    void initBuffers();
    CurrentSourceDescPtr mDescription;
    
    std::vector<std::vector<Rect3i> > mRectsJ;
    std::vector<std::vector<Rect3i> > mRectsK;
    
    std::vector<long> mMaterialIDs;
    std::vector<Vector3i> mNumCellsJ;
    std::vector<Vector3i> mNumCellsK;
};
typedef Pointer<SetupCurrentSource> SetupCurrentSourcePtr;


class CurrentSource
{
public:
    /**
     *  Create a current source object to manage loading/generation of J and K
     *  for a particular CurrentSourceDescription.  This object is essentially a
     *  big floating-point buffer with some information about which material
     *  should read what parts, so the constructor input contains an ordered
     *  list of id numbers, then corresponding lists of how many cells go into
     *  the buffers for J and K for each material.
     *
     *  @param description  information from the parameter file
     *  @param materialIDs  the list of which material reads in which order
     *  @param numCellsJ    the amount of the buffer for each material for Jx,
     *                      Jy and Jz; must be the same length as materialIDs
     *  @param numCellsK    the amount of the buffer for each material for Kx,
     *                      Ky and Kz; must be the same length as materialIDs
     */
    CurrentSource(const CurrentSourceDescPtr & description,
        const std::vector<long> & materialIDs,
        const std::vector<Vector3i> & numCellsJ,
        const std::vector<Vector3i> & numCellsK);
    virtual ~CurrentSource();
    
    /**
     *  Calls mFieldInput.allocate(), which makes room for the current buffers.
     *  Do note that a BufferedFieldInput will actually allocate and load any
     *  mask data on construction, and the mask data takes up as much room as
     *  the current buffers.
     */
    void allocateAuxBuffers();
    
    CurrentSourceDescPtr description() const { return mDescription; }
    BufferPointer pointerJ(int direction, long materialID);
    BufferPointer pointerK(int direction, long materialID);
    BufferPointer pointerMaskJ(int direction, long materialID);
    BufferPointer pointerMaskK(int direction, long materialID);
    
    void prepareJ(long timestep, float time);
    void prepareK(long timestep, float time);
private:
    CurrentSourceDescPtr mDescription;
    BufferedFieldInput mFieldInput;
    
    std::vector<long> mMaterialIDs;
    
    long mCurrentSampleInterval;
    
    Map<long, long> mOffsetsJ[3]; // from material ID to offset
    Map<long, long> mOffsetsK[3]; // from material ID to offset
};
typedef Pointer<CurrentSource> CurrentSourcePtr;

#endif
