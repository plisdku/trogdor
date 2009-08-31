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
#include "FieldInput.h"
#include "RunlineEncoder.h"
#include "MemoryUtilities.h"
#include "SimulationDescription.h"

class VoxelizedPartition;
class CalculationPartition;
class CurrentSource;
class SetupUpdateEquation;

    
struct CurrentBuffers
{
    SetupUpdateEquation* material;
    MemoryBuffer buffersJ[3];
    MemoryBuffer buffersK[3];
    MemoryBuffer maskJ[3];
    MemoryBuffer maskK[3];
};

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
    
    
    struct InputRunlineList
    {
        SetupUpdateEquation* material;
        std::vector<Region> regionsE[3];
        std::vector<Region> regionsH[3];
        long numCellsE(int fieldDirection) const;
        long numCellsH(int fieldDirection) const;
    };
    
    InputRunlineList & inputRunlineList(SetupUpdateEquation* material);
    
    class RLE : public RunlineEncoder
    {
    public:
        RLE(InputRunlineList & runlines);
        virtual void endRunline(const VoxelizedPartition & vp,
            const Vector3i & lastHalfCell);
    private:
        InputRunlineList & mRunlines;
    };
    
private:
    void initInputRunlines(const VoxelizedPartition & vp);
    void initBuffers();
    CurrentSourceDescPtr mDescription;
    
    std::vector<Pointer<CurrentBuffers> > mCurrentBuffers;
    std::vector<InputRunlineList> mScheduledInputRegions;
};
typedef Pointer<SetupCurrentSource> SetupCurrentSourcePtr;

class CurrentSource
{
public:
    CurrentSource(const CurrentSourceDescPtr & mDescription,
        const CalculationPartition & cp,
        const std::vector<Pointer<CurrentBuffers> > & currentBuffers);
    virtual ~CurrentSource();
    
    void allocateAuxBuffers();
    
    CurrentSourceDescPtr description() const { return mDescription; }
    BufferPointer pointerJ(int direction, int materialID);
    BufferPointer pointerK(int direction, int materialID);
    BufferPointer pointerMaskJ(int direction, int materialID);
    BufferPointer pointerMaskK(int direction, int materialID);
    
    void prepareJ(long timestep);
    void prepareK(long timestep);
    
    float getJ(int direction) const;
    float getK(int direction) const;
private:
    CurrentSourceDescPtr mDescription;
    FieldInput mFieldInput;
    
    std::vector<Pointer<CurrentBuffers> > mCurrentBuffers;
    
    std::vector<int> mMaterialIDs;
    std::vector<float> mDataJ[3];
    std::vector<float> mDataK[3];
    std::vector<float> mDataMaskJ[3];
    std::vector<float> mDataMaskK[3];
};
typedef Pointer<CurrentSource> CurrentSourcePtr;

#endif
