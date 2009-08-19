/*
 *  RunlineEncoder.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _GENERALRUNLINEENCODER_
#define _GENERALRUNLINEENCODER_

#include "geometry.h"
#include "MemoryUtilities.h"

class VoxelizedPartition;
class Paint;

enum NeighborFieldContinuity
{
    kNeighborFieldNone = 0,
    kNeighborFieldTransverse4,
    kNeighborFieldNearby6,
    kNeighborFieldMixed
};

enum NeighborAuxiliaryContinuity
{
    kNeighborAuxiliaryNone = 0,
    kNeighborAuxiliaryTransverse4,
    kNeighborAuxiliaryNearby6,
    kNeighborAuxiliaryMixed
};

enum MaterialContinuity
{
    kMaterialContinuityNone = 0,
    kMaterialContinuityRequired
};

enum LineContinuity
{
    kLineContinuityNone = 0,
    kLineContinuityRequired
};

class RunlineEncoder
{
public:
    /**
     *  Initialize new encoder.  The default encoder settings will always
     *  continue a runline, without respect to nearby fields and indices,
     *  change of material, or wrapping around to the next row of the grid.
     */
    RunlineEncoder();
    virtual ~RunlineEncoder();
    
    // Setup.  Set all the flags...
    void setNeighborFieldContinuity(NeighborFieldContinuity val)
        { mFieldContinuity = val; }
    void setNeighborAuxiliaryContinuity(NeighborAuxiliaryContinuity val)
        { mAuxContinuity = val; }
    void setMaterialContinuity(MaterialContinuity val)
        { mMatContinuity = val; }
    void setLineContinuity(LineContinuity val)
        { mLineContinuity = val; }
    
    // Accessors for the current runline's info
    const Vector3i & firstHalfCell() const
        { return mFirstHalfCell; }
    const Vector3i & lastHalfCell() const
        { return mLastHalfCell; }
    long length() const
        { return mLength; }
    
    // Things called by the... iterator on the grid, I guess it's called.
    void startRunline(const VoxelizedPartition & vp,
        const Vector3i & beginningHalfCell);
        
    /**
     *  Determine whether the current runline can increase by one cell to
     *  include newHalfCell.
     *
     *  @param newPaint the Paint in the voxel grid at newHalfCell; must
     *                  include all modifications such as curl buffers!
     */
    bool canContinueRunline(const VoxelizedPartition & vp,
        const Vector3i & newHalfCell,
        const Paint* newPaint) const;
    void continueRunline();
    
    // Hey user, you can implement this to make it do your bidding!
    virtual void endRunline(const VoxelizedPartition & vp);
private:
    /**
     *  Initialize mFirstNeighborFields and mUsedNeighborFields for new
     *  runline.  This is called by startRunline().
     */
    void initFirstNeighborFields(const VoxelizedPartition & vp,
        const Vector3i & beginningHalfCell);
    /**
     *  Initialize mFirstNeighborAuxIndices and mUsideNeighborAuxIndices.
     *  This is called by startRunline().
     */
    void initFirstNeighborAuxiliaryIndices(const VoxelizedPartition & vp,
        const Vector3i & beginningHalfCell);
    
    /**
     *  Returns true if the runline may include newHalfCell with regard to
     *  mFieldContinuity.
     */
    bool neighborFieldContinuityOK(const VoxelizedPartition & vp,
        const Vector3i & newHalfCell, const Paint* newPaint) const;
    /**
     *  Returns true if the runline may include newHalfCell with regard to
     *  mAuxContinuity.
     */
    bool neighborAuxContinuityOK(const VoxelizedPartition & vp,
        const Vector3i & newHalfCell, const Paint* newPaint) const;
    /**
     *  Returns true if the runline may include newHalfCell with regard to
     *  mMatContinuity.
     */
    bool materialContinuityOK(const VoxelizedPartition & vp,
        const Vector3i & newHalfCell, const Paint* newPaint) const;
    /**
     *  Returns true if the runline may include newHalfCell with regard to
     *  mLineContinuity.
     */
    bool lineContinuityOK(const VoxelizedPartition & vp,
        const Vector3i & newHalfCell, const Paint* newPaint) const;
    
    Vector3i mFirstHalfCell;
    Vector3i mLastHalfCell;
    long mLength;
    int mRunlineDirection;
    
    int mOctant;
    int mFieldDirection;
    
    // Data for various runline continuation criteria
    
    NeighborFieldContinuity mFieldContinuity;
    BufferPointer mFirstNeighborFields[6];
    bool mUsedNeighborFields[6];
    
    NeighborAuxiliaryContinuity mAuxContinuity;
    long mFirstNeighborAuxIndices[6];
    bool mUsedNeighborAuxIndices[6];
    
    MaterialContinuity mMatContinuity;
    Paint* mFirstPaint;
    
    LineContinuity mLineContinuity;
};





#endif
