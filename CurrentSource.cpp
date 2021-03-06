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

#include <algorithm>
#include <numeric>
using namespace std;
using namespace YeeUtilities;

SetupCurrentSource::
SetupCurrentSource(const CurrentSourceDescPtr & description,
    const VoxelizedPartition & vp) :
    mDescription(description),
    mRectsJ(3),
    mRectsK(3)
{
    //LOG << "Constructor!\n";
    
    initInputRunlines(vp);
}

SetupCurrentSource::
~SetupCurrentSource()
{
}

/**
 *  This helper class is used to help set up the ordered list locations of
 *  evaluation of Jx, Jy, Jz, Kx, Ky and Kz.  It breaks on new rows and new
 *  materials, and saves runlines for each update equation that uses the given
 *  current source.
 */
class CurrentSourceEncoder : public RunlineEncoder
{
public:
    CurrentSourceEncoder(CurrentSourceDescPtr thisSrc) :
        mSrc(thisSrc)
    {
        setLineContinuity(kLineContinuityRequired);
        setMaterialContinuity(kMaterialContinuityRequired);
    }
    
    virtual void endRunline(const VoxelizedPartition & vp,
        const Vector3i & lastHalfCell)
    {
        if (vp.voxels()(lastHalfCell)->currentSource() == mSrc)
        {
            Rect3i newRect(halfToYee(Rect3i(firstHalfCell(), lastHalfCell)));
            Paint* p = vp.voxels()(lastHalfCell)->withoutCurlBuffers();
            mRects[p].push_back(newRect);
            //mPaints.insert(p);
            
            // slow but preserves ordering as on the grid
            if (mPaints.end() == find(mPaints.begin(), mPaints.end(), p))
                mPaints.push_back(p);
            
            //LOG << "Encoded mat " << p->fullName() << " rect " << newRect
            //    << "\n";
        }
    }
    
    const Map<Paint*, vector<Rect3i> > & rects() const { return mRects; }
    //const set<Paint*> & paints() const { return mPaints; }
    const vector<Paint*> & paints() const { return mPaints; }
private:
    CurrentSourceDescPtr mSrc;
    Map<Paint*, vector<Rect3i> > mRects;
    vector<Paint*> mPaints;
    //set<Paint*> mPaints;
};


// Called by the constructor.
void SetupCurrentSource::
initInputRunlines(const VoxelizedPartition & vp)
{
    //set<Paint*> allPaints;
    vector<Paint*> allPaints;
    Map<Paint*, vector<Rect3i> > rectsJ[3]; // bulk material ID -> runlines
    Map<Paint*, vector<Rect3i> > rectsK[3];
    
    // Run length encode the current source.
    for (int oct = 1; oct < 7; oct++)
    {
        CurrentSourceEncoder encoder(mDescription);
        vp.runLengthEncode(encoder, halfToYee(vp.calcHalfCells(), oct), oct);
        
        if (isE(oct))
        {
            rectsJ[xyz(oct)] = encoder.rects();
            /*
            map<Paint*, vector<Rect3i> >::iterator itr;
            for (itr = rectsJ[xyz(oct)].begin(); itr != rectsJ[xyz(oct)].end();
                itr++)
            {
                LOG << "dir " << xyz(oct) << " paint " << itr->first->fullName()
                    << " num rects " << itr->second.size() << "\n";
            }
            */
        }
        else
            rectsK[xyz(oct)] = encoder.rects();
        
        const vector<Paint*> & s = encoder.paints();
        
        for (int nn = 0; nn < s.size(); nn++)
        if (allPaints.end() == find(allPaints.begin(), allPaints.end(), s[nn]))
            allPaints.push_back(s[nn]);
        //allPaints.insert(s.begin(), s.end());
    }
    
    // Iterate over allPaints, which establishes the order in the file
    // and the order in the big buffer.
    //
    // Three bits o' member data to write:
    //  1.  mMaterialIDs, the list (in order!) of which material reads from
    //      the buffer in which order.
    //  2.  mRectsJ and mRectsK, which follow the same ordering but don't 
    //      need to reveal which material is in which rect.  This data is only
    //      needed if Trogdor writes a data request, which is an ordered list
    //      of positions to provide field values at.
    //  3.  mNumCellsJ and mNumCellsK, which are ordered the same way as the
    //      material IDs, and just say how much of the current buffer belongs
    //      to each material.
    // I write all of these data in one loop since they all need to be in the
    // same order (thus a float in an external file is guaranteed to be read
    // into the correct update equation). The order is established by iteration
    // over the set of Paints, so it could be anything.
    mMaterialIDs.clear();
    int nn = 0;
    
    mNumCellsJ.resize(allPaints.size());
    mNumCellsK.resize(allPaints.size());
    //for (set<Paint*>::const_iterator paint = allPaints.begin();
    for (vector<Paint*>::const_iterator paint = allPaints.begin();
        paint != allPaints.end();
        paint++)
    {
        // (1) above: set the ordering of materials in the current buffer.
        mMaterialIDs.push_back(vp.setupMaterials()[*paint]->id());
        
        //LOG << "ID " << mMaterialIDs[mMaterialIDs.size()-1] << "\n";
        for (int xyz = 0; xyz < 3; xyz++)
        {
            //LOG << "dir " << xyz << " paint " << (*paint)->fullName()
            //    << " num rects " << rectsJ[xyz][*paint].size() << "\n";
            // (2) above: specify order of current storage in memory
            copy(rectsJ[xyz][*paint].begin(), rectsJ[xyz][*paint].end(),
                back_inserter(mRectsJ[xyz]));
            copy(rectsK[xyz][*paint].begin(), rectsK[xyz][*paint].end(),
                back_inserter(mRectsK[xyz]));
            
            // (3) above: calculate size of buffer for each update equation
            vector<long> numJ, numK;
            transform(rectsJ[xyz][*paint].begin(), rectsJ[xyz][*paint].end(),
                back_inserter(numJ), mem_fun_ref(&Rect3i::count));
            transform(rectsK[xyz][*paint].begin(), rectsK[xyz][*paint].end(),
                back_inserter(numK), mem_fun_ref(&Rect3i::count));
            
            mNumCellsJ.at(nn)[xyz] = accumulate(numJ.begin(), numJ.end(), 0);
            mNumCellsK.at(nn)[xyz] = accumulate(numK.begin(), numK.end(), 0);
            
            LOGF << "Mat " << mMaterialIDs[nn] << " J" << char('x'+xyz) <<
                " num " << mNumCellsJ.at(nn)[xyz] << "\n";
            LOGF << "Mat " << mMaterialIDs[nn] << " K" << char('x'+xyz) <<
                " num " << mNumCellsK.at(nn)[xyz] << "\n";
        }
        nn++;
    }
}

Pointer<CurrentSource> SetupCurrentSource::
makeCurrentSource(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
    return Pointer<CurrentSource>(new CurrentSource(mDescription,
        mMaterialIDs, mNumCellsJ, mNumCellsK));
}

CurrentSource::
CurrentSource(const CurrentSourceDescPtr & description,
    const vector<long> & materialIDs, const vector<Vector3i> & numCellsJ,
    const vector<Vector3i> & numCellsK) :
    mDescription(description),
    mFieldInput(description),
    mMaterialIDs(materialIDs),
    mCurrentSampleInterval(0)
{
    // We'll allocate things later.  For now, let's make maps of offsets
    // for each material.
    
    for (int mm = 0; mm < mMaterialIDs.size(); mm++)
        LOGF << mMaterialIDs[mm] << endl;
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        long offset = 0;
        
        for (int nn = 0; nn < numCellsJ.size(); nn++)
        {
            mOffsetsJ[xyz][mMaterialIDs[nn]] = offset;
            LOGF << "Mat " << mMaterialIDs[nn] << " J" << char('x'+xyz) <<
                " offset " << offset << " size " << numCellsJ[nn][xyz] << "\n";
            offset += numCellsJ[nn][xyz];
        }
        offset = 0;
        for (int nn = 0; nn < numCellsK.size(); nn++)
        {
            mOffsetsK[xyz][mMaterialIDs[nn]] = offset;
            LOGF << "Mat " << mMaterialIDs[nn] << " K" << char('x'+xyz) <<
                " offset " << offset << " size " << numCellsJ[nn][xyz] << "\n";
            offset += numCellsK[nn][xyz];
        }
    }
}

CurrentSource::
~CurrentSource()
{
//    LOG << "Destructor!\n";
}

void CurrentSource::
allocateAuxBuffers()
{
    mFieldInput.allocate();
}

BufferPointer CurrentSource::
pointerJ(int xyz, long materialID)
{
    return mFieldInput.pointerE(xyz, mOffsetsJ[xyz][materialID]);
}

BufferPointer CurrentSource::
pointerK(int xyz, long materialID)
{
    return mFieldInput.pointerH(xyz, mOffsetsK[xyz][materialID]);
}

BufferPointer CurrentSource::
pointerMaskJ(int xyz, long materialID)
{
    return mFieldInput.pointerMaskE(xyz, mOffsetsJ[xyz][materialID]);
}

BufferPointer CurrentSource::
pointerMaskK(int xyz, long materialID)
{
    return mFieldInput.pointerMaskH(xyz, mOffsetsK[xyz][materialID]);
}

void CurrentSource::
prepareJ(long timestep, float time)
{
    if (norm2(mDescription->sourceCurrents().whichJ()) == 0)
        return;
    
    if (mCurrentSampleInterval >= mDescription->durations().size())
    {
        mFieldInput.zeroBuffersE();
        return;
    }
    while (timestep > mDescription->durations()[mCurrentSampleInterval].last())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDescription->durations().size())
        {
            mFieldInput.zeroBuffersE();
            return;
        }
    }
    
    /*
    LOGF << "Timestep " << timestep << " in duration " <<
        mDescription->durations()[mCurrentSampleInterval].first() << " to "
        << mDescription->durations()[mCurrentSampleInterval].last() << ".\n";
    */
    if (timestep >= mDescription->durations()[mCurrentSampleInterval].first()
        && timestep <= mDescription->durations()[mCurrentSampleInterval].last())
        mFieldInput.startHalfTimestepE(timestep, time);
    else
        mFieldInput.zeroBuffersE();
}

void CurrentSource::
prepareK(long timestep, float time)
{
    if (norm2(mDescription->sourceCurrents().whichK()) == 0)
        return;
    
    if (mCurrentSampleInterval >= mDescription->durations().size())
    {
        mFieldInput.zeroBuffersH();
        return;
    }
    while (timestep > mDescription->durations()[mCurrentSampleInterval].last())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDescription->durations().size())
        {
            mFieldInput.zeroBuffersH();
            return;
        }
    }
    /*
    LOGF << "Timestep " << timestep << " in duration " <<
        mDescription->durations()[mCurrentSampleInterval].first() << " to "
        << mDescription->durations()[mCurrentSampleInterval].last() << ".\n";
    */
    if (timestep >= mDescription->durations()[mCurrentSampleInterval].first()
        && timestep <= mDescription->durations()[mCurrentSampleInterval].last())
        mFieldInput.startHalfTimestepH(timestep, time);
    else
        mFieldInput.zeroBuffersH();
}
