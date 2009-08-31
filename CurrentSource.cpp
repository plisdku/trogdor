/*
 *  CurrentSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "CurrentSource.h"
#include "VoxelizedPartition.h"
#include "CalculationPartition.h"

#include "YeeUtilities.h"
using namespace std;
using namespace YeeUtilities;


typedef pair<Paint*, SetupUpdateEquationPtr> PaintMatPair;

Paint* first(PaintMatPair thePair) { return thePair.first; }
SetupUpdateEquationPtr second(PaintMatPair thePair) { return thePair.second; }

bool notThisSource(Paint* p, CurrentSourceDescPtr c)
{ return !p->hasCurrentSource() || p->currentSource() != c; }

bool comparePaintByMaterial(const Paint* lhs, const Paint* rhs)
{ return lhs->bulkMaterial()->id() < rhs->bulkMaterial()->id(); }

SetupCurrentSource::
SetupCurrentSource(const CurrentSourceDescPtr & description,
    const VoxelizedPartition & vp) :
    mDescription(description)
{
    LOG << "Constructor!\n";
    
    initInputRunlines(vp);
    initBuffers();
}

SetupCurrentSource::
~SetupCurrentSource()
{
//    LOG << "Destructor!\n";
}

void SetupCurrentSource::
initInputRunlines(const VoxelizedPartition & vp)
{
    // Make a list ("paints") of all known Paint* which have "description" as a
    // current source.
    vector<Paint*> paints;
    set<Paint*> allPaints = vp.indices().curlBufferParentPaints();
    copy(allPaints.begin(), allPaints.end(), back_inserter(paints));
    paints.erase(remove_if(paints.begin(), paints.end(),
        bind2nd(ptr_fun(notThisSource), mDescription)), paints.end());
    sort(paints.begin(), paints.end(), &comparePaintByMaterial);
    // with better data structures this would be "sort(parentPaints.values())".
    
    // Create run length encoders for all of these paints.  These encoders will
    // let us schedule the input for rapid I/O.
    mScheduledInputRegions.resize(paints.size());
    vector<Pointer<RLE> > encoders(paints.size());
    map<Paint*, RunlineEncoder*> encoderMap;
    
    for (int nn = 0; nn < paints.size(); nn++)
    {
        mScheduledInputRegions[nn].material = vp.setupMaterials()[paints[nn]];
        encoders[nn] = Pointer<RLE>(new RLE(mScheduledInputRegions[nn]));
        encoderMap[paints[nn]] = encoders[nn];
    }
    for (int oct = 1; oct < 7; oct++) // iterates over E and H octants (all six)
        vp.runLengthEncode(encoderMap, halfToYee(vp.calcHalfCells(), oct), oct);
}

void SetupCurrentSource::
initBuffers()
{
    mCurrentBuffers.resize(mScheduledInputRegions.size());
    for (int nn = 0; nn < mScheduledInputRegions.size(); nn++)
    {
        const InputRunlineList & irl = mScheduledInputRegions[nn];
        mCurrentBuffers[nn] = Pointer<CurrentBuffers>(new CurrentBuffers);
        CurrentBuffers & cb = *mCurrentBuffers[nn];
        
        cb.material = irl.material;
        LOG << "Material is " << hex << cb.material << dec << endl;
        for (int direction = 0; direction < 3; direction++)
        {
            // J and K buffers
            if (mDescription->sourceCurrents().whichJ()[direction])
            {
                if (mDescription->isSpaceVarying())
                {
                    cb.buffersJ[direction] = MemoryBuffer(string("J") +
                        char('x'+direction), irl.numCellsE(direction));
                }
                else
                {
                    cb.buffersJ[direction] = MemoryBuffer(string("J") +
                        char('x'+direction), 1);
                }
            }
            if (mDescription->sourceCurrents().whichK()[direction])
            {
                if (mDescription->isSpaceVarying())
                {
                    cb.buffersK[direction] = MemoryBuffer(string("K") +
                        char('x'+direction), irl.numCellsH(direction));
                }
                else
                {
                    cb.buffersK[direction] = MemoryBuffer(string("K") +
                        char('x'+direction), 1);
                }
            }
            
            // J and K mask buffers
            if (mDescription->hasMask())
            {
                if (mDescription->sourceCurrents().whichJ()[direction] &&
                    mDescription->hasMask())
                {
                    cb.maskJ[direction] = MemoryBuffer(string("Mask J") +
                        char('x'+direction), irl.numCellsE(direction));
                }
                if (mDescription->sourceCurrents().whichK()[direction] &&
                    mDescription->hasMask())
                {
                    cb.maskK[direction] = MemoryBuffer(string("Mask K") +
                        char('x'+direction), irl.numCellsH(direction));
                }
            }
        } // for x, y, z
    } // foreach material (essentially)
}

Pointer<CurrentSource> SetupCurrentSource::
makeCurrentSource(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
//    LOG << "Making one!\n";
    return Pointer<CurrentSource>(new CurrentSource(mDescription, cp,
        mCurrentBuffers));
}

SetupCurrentSource::InputRunlineList & SetupCurrentSource::
inputRunlineList(SetupUpdateEquation* material)
{
    for (int nn = 0; nn < mScheduledInputRegions.size(); nn++)
    if (mScheduledInputRegions[nn].material == material)
        return mScheduledInputRegions[nn];
    throw(Exception("Can't find input runline list."));
}

SetupCurrentSource::RLE::
RLE(InputRunlineList & runlines) :
    mRunlines(runlines)
{
    setLineContinuity(kLineContinuityRequired);
    setMaterialContinuity(kMaterialContinuityRequired);
}

void SetupCurrentSource::RLE::
endRunline(const VoxelizedPartition & vp, const Vector3i & lastHalfCell)
{
    Region newRunline(halfToYee(Rect3i(firstHalfCell(), lastHalfCell)));
    
    if (isE(octant(lastHalfCell)))
        mRunlines.regionsE[xyz(octant(lastHalfCell))].push_back(newRunline);
    else
        mRunlines.regionsH[xyz(octant(lastHalfCell))].push_back(newRunline);
    
    LOG << "Ending runline: " << newRunline.yeeCells() << endl;
}

long SetupCurrentSource::InputRunlineList::
numCellsE(int fieldDirection) const
{
    long numCells = 0;
    for (int nn = 0; nn < regionsE[fieldDirection].size(); nn++)
    {
        Vector3i sizeOfRegion = regionsE[fieldDirection][nn].yeeCells().num();
        numCells += sizeOfRegion[0]*sizeOfRegion[1]*sizeOfRegion[2];
    }
    return numCells;
}

long SetupCurrentSource::InputRunlineList::
numCellsH(int fieldDirection) const
{
    long numCells = 0;
    for (int nn = 0; nn < regionsH[fieldDirection].size(); nn++)
    {
        Vector3i sizeOfRegion = regionsH[fieldDirection][nn].yeeCells().num();
        numCells += sizeOfRegion[0]*sizeOfRegion[1]*sizeOfRegion[2];
    }
    return numCells;
}


CurrentSource::
CurrentSource(const CurrentSourceDescPtr & description,
    const CalculationPartition & cp,
    const vector<Pointer<CurrentBuffers> > & currentBuffers) :
    mDescription(description),
    mFieldInput(description, cp.dt()),
    mCurrentBuffers(currentBuffers)
{
//    LOG << "Constructor!\n";
    mMaterialIDs.resize(currentBuffers.size());
    for (int nn = 0; nn < currentBuffers.size(); nn++)
        mMaterialIDs[nn] = currentBuffers[nn]->material->id();
}

CurrentSource::
~CurrentSource()
{
//    LOG << "Destructor!\n";
}

void CurrentSource::
allocateAuxBuffers()
{
    /*
        Who gets a big ol' buffer?  No buffers for unused fields.  No masks
        either.  For fields with no mask, there is no mask buffer.
        
        For separable sources, T(t)X(x), the mask X(x) is allocated to its
        required size and the time buffer T(t) is just one float per direction.
        
        For nonseparable sources, T(x,t), there is no mask and the time
        buffer is as big as it needs to be.
        
        This function does not do any of the work in figuring out what's what.
        It just allocates buffers.  Furthermore it doesn't divide the buffers
        or provide pointers into them.
    */
    
    // Allocate some room
    for (int direction = 0; direction < 3; direction++)
    {
        long totalCellsJ = 0;
        long totalCellsK = 0;
        long totalMaskCellsJ = 0;
        long totalMaskCellsK = 0;
        for (int nn = 0; nn < mCurrentBuffers.size(); nn++)
        {
            totalCellsJ += mCurrentBuffers[nn]->buffersJ[direction].length();
            totalCellsK += mCurrentBuffers[nn]->buffersK[direction].length();
            totalMaskCellsJ += mCurrentBuffers[nn]->maskJ[direction].length();
            totalMaskCellsK += mCurrentBuffers[nn]->maskK[direction].length();
        }
        
        if (totalCellsJ != 0)
            mDataJ[direction].resize(totalCellsJ);
        if (totalCellsK != 0)
            mDataK[direction].resize(totalCellsK);
        if (totalMaskCellsJ != 0)
            mDataMaskJ[direction].resize(totalMaskCellsJ);
        if (totalMaskCellsK != 0)
            mDataMaskK[direction].resize(totalMaskCellsK);
    }
    
    // Fill in the MemoryBuffers' head pointers
    for (int direction = 0; direction < 3; direction++)
    {
        long offsetJ = 0;
        long offsetK = 0;
        long offsetMaskJ = 0;
        long offsetMaskK = 0;
        
        for (int nn = 0; nn < mCurrentBuffers.size(); nn++)
        {
            assert(mCurrentBuffers[nn] != 0L);
            CurrentBuffers & cb = *mCurrentBuffers[nn];
            
            if (cb.buffersJ[direction].length() != 0)
            {
                cb.buffersJ[direction].setHeadPointer(
                    &(mDataJ[direction][0]) + offsetJ);
                offsetJ += cb.buffersJ[direction].length();
            }
            if (cb.buffersK[direction].length() != 0)
            {
                cb.buffersK[direction].setHeadPointer(
                    &(mDataK[direction][0]) + offsetK);
                offsetK += cb.buffersK[direction].length();
            }
            if (cb.maskJ[direction].length() != 0)
            {
                cb.maskJ[direction].setHeadPointer(
                    &(mDataMaskJ[direction][0]) + offsetMaskJ);
                offsetMaskJ += cb.maskJ[direction].length();
            }
            if (cb.maskK[direction].length() != 0)
            {
                cb.maskK[direction].setHeadPointer(
                    &(mDataMaskK[direction][0]) + offsetMaskK);
                offsetMaskK += cb.maskK[direction].length();
            }   
        }
    }
}



BufferPointer CurrentSource::
pointerJ(int direction, int materialID)
{
    for (int nn = 0; nn < mMaterialIDs.size(); nn++)
    if (mMaterialIDs[nn] == materialID)
        return BufferPointer(mCurrentBuffers[nn]->buffersJ[direction]);
    throw(Exception("Cannot find buffer."));
}

BufferPointer CurrentSource::
pointerK(int direction, int materialID)
{
    for (int nn = 0; nn < mMaterialIDs.size(); nn++)
    if (mMaterialIDs[nn] == materialID)
        return BufferPointer(mCurrentBuffers[nn]->buffersK[direction]);
    throw(Exception("Cannot find buffer."));
}

BufferPointer CurrentSource::
pointerMaskJ(int direction, int materialID)
{
    for (int nn = 0; nn < mMaterialIDs.size(); nn++)
    if (mMaterialIDs[nn] == materialID)
        return BufferPointer(mCurrentBuffers[nn]->maskJ[direction]);
    throw(Exception("Cannot find buffer."));
}

BufferPointer CurrentSource::
pointerMaskK(int direction, int materialID)
{
    for (int nn = 0; nn < mMaterialIDs.size(); nn++)
    if (mMaterialIDs[nn] == materialID)
        return BufferPointer(mCurrentBuffers[nn]->maskK[direction]);
    throw(Exception("Cannot find buffer."));
}


void CurrentSource::
prepareJ(long timestep)
{
//    LOG << "Somehow loading J into all the right BufferedCurrents.\n";
    
}

void CurrentSource::
prepareK(long timestep)
{
//    LOG << "Somehow loading K into all the right BufferedCurrents.\n";
}



float CurrentSource::
getJ(int direction) const
{
    assert(direction >= 0 && direction < 3);
    return 0.0f;
}

float CurrentSource::
getK(int direction) const
{
    assert(direction >= 0 && direction < 3);
    return 0.0f;
}
