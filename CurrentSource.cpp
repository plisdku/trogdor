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

#include "YeeUtilities.h"
using namespace std;
using namespace YeeUtilities;

SetupCurrentSource::
SetupCurrentSource(const CurrentSourceDescPtr & description,
    const VoxelizedPartition & vp) :
    mDescription(description)
{
    LOG << "Constructor!\n";
    
    // Find all Paints that point to this current source.
    // Next, run-length encode in order.
    /*
    map<Paint*, SetupMaterialPtr>::const_iterator itr;
    for (itr = vp.setupMaterials().begin(); itr != vp.setupMaterials.end();
        itr++)
    {
        
    }
    */
}

SetupCurrentSource::
~SetupCurrentSource()
{
//    LOG << "Destructor!\n";
}

Pointer<CurrentSource> SetupCurrentSource::
makeCurrentSource(const VoxelizedPartition & vp,
    const CalculationPartition & cp) const
{
//    LOG << "Making one!\n";
    
    return Pointer<CurrentSource>(new CurrentSource);
}

SetupCurrentSource::InputRunlineList & SetupCurrentSource::
inputRunlineList(SetupMaterial* material)
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
CurrentSource()
{
//    LOG << "Constructor!\n";
}

CurrentSource::
~CurrentSource()
{
//    LOG << "Destructor!\n";
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
