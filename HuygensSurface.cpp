/*
 *  HuygensSurface.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "HuygensSurface.h"
#include "VoxelizedPartition.h"
#include "SimulationDescription.h"
#include "Exception.h"
#include "YeeUtilities.h"
#include "STLOutput.h"

#include "HuygensLink.h"
#include "HuygensCustomSource.h"

#include <sstream>
using namespace std;
using namespace YeeUtilities;

HuygensSurfacePtr HuygensSurfaceFactory::
newHuygensSurface(string namePrefix,
    const VoxelizedPartition & vp,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
    const HuygensSurfaceDescPtr & desc)
{
    HuygensSurfacePtr hs(new HuygensSurface(namePrefix, vp, grids, desc));
    HuygensUpdatePtr update;
    
    if (desc->type() == kLink)
    {
        update = HuygensUpdatePtr(new HuygensLink(*hs));
        hs->setUpdater(update);
    }
    else if (desc->type() == kCustomTFSFSource)
    {
        update = HuygensUpdatePtr(new HuygensCustomSource(*hs));
        hs->setUpdater(update);
    }
    else
        throw(Exception("Unknown HuygensSurface type!"));
    
    return hs;
}

HuygensSurface::
HuygensSurface(string namePrefix, const VoxelizedPartition & vp,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
    const HuygensSurfaceDescPtr & surfaceDescription) :
    mDescription(surfaceDescription),
    mHalfCells(surfaceDescription->halfCells()),
    mNeighborBuffers(6),
    mDestLattice(vp.getLattice())
{
    static const Rect3i UNUSED_SOURCE_RECT_ARGUMENT(0,0,0,0,0,0);
    Rect3i sourceHalfCells(UNUSED_SOURCE_RECT_ARGUMENT);
    
    if (surfaceDescription->type() == kLink)
    {
        assert(surfaceDescription->sourceGrid() != 0L);
        mSourceLattice =  grids[surfaceDescription->sourceGrid()]->
            getLattice();
        sourceHalfCells = surfaceDescription->fromHalfCells();
    }
    
    for (int sideNum = 0; sideNum < 6; sideNum++)
    {
        ostringstream bufPrefix;
        bufPrefix << namePrefix << " side " << sideNum;
        Vector3i sideVector(cardinal(sideNum));
        if (!surfaceDescription->omittedSides().count(sideVector))
        {
            float incidentFieldFactor =
                surfaceDescription->isTotalField() ? 1.0 : -1.0;
            NeighborBufferPtr p(new NeighborBuffer(
                bufPrefix.str(),
                surfaceDescription->halfCells(),
                sourceHalfCells,
                sideNum,
                incidentFieldFactor));
            mNeighborBuffers[sideNum] = p;
        }
    }
    
}

void HuygensSurface::
allocate()
{
    for (int nn = 0; nn < 6; nn++)
    if (hasBuffer(nn))
    {
        mNeighborBuffers.at(nn)->lattice()->allocate();
    }
}

void HuygensSurface::
updateE(CalculationPartition & cp, long timestep)
{
    assert(mUpdate != 0L);
    mUpdate->updateE(*this, cp, timestep);
}

void HuygensSurface::
updateH(CalculationPartition & cp, long timestep)
{
    assert(mUpdate != 0L);
    mUpdate->updateH(*this, cp, timestep);
}

NeighborBuffer::
NeighborBuffer(string prefix,
    const Rect3i & huygensHalfCells, int sideNum,
    float incidentFieldFactor) :
    mDestFactorsE(3),
    mSourceFactorsE(3),
    mDestFactorsH(3),
    mSourceFactorsH(3)
{
    Rect3i destHalfRect(edgeHalfCells(huygensHalfCells, sideNum));
    initFactors(huygensHalfCells, sideNum, incidentFieldFactor);
    
    mLattice = InterleavedLatticePtr(new InterleavedLattice(
        prefix, destHalfRect));
}

NeighborBuffer::
NeighborBuffer(string prefix,
    const Rect3i & huygensHalfCells, const Rect3i & sourceHalfCells,
    int sideNum,
    float incidentFieldFactor) :
    mDestFactorsE(3),
    mSourceFactorsE(3),
    mDestFactorsH(3),
    mSourceFactorsH(3)
{
    Rect3i destHalfRect(edgeHalfCells(huygensHalfCells, sideNum));
    mSourceHalfCells = edgeHalfCells(sourceHalfCells, sideNum);
    
    initFactors(huygensHalfCells, sideNum, incidentFieldFactor);
    
    mLattice = InterleavedLatticePtr(new InterleavedLattice(
        prefix, destHalfRect));
        
//    LOG << "Source " << mSourceHalfCells << " dest " << destHalfRect << "\n";
//    LOG << "Source factors: \n";
//    LOGMORE << mSourceFactorsE << "\n";
//    LOGMORE << mSourceFactorsH << "\n";
//    LOG << "Dest factors: \n";
//    LOGMORE << mDestFactorsE << "\n";
//    LOGMORE << mDestFactorsH << "\n";
}

const Rect3i & NeighborBuffer::
destHalfCells() const
{
    return mLattice->halfCells();
}

const Rect3i & NeighborBuffer::
sourceHalfCells() const
{
    return mSourceHalfCells;
}

float NeighborBuffer::
destFactorE(int fieldDirection) const
{
    return mDestFactorsE.at(fieldDirection);
}

float NeighborBuffer::
destFactorH(int fieldDirection) const
{
    return mDestFactorsH.at(fieldDirection);
}

float NeighborBuffer::
sourceFactorE(int fieldDirection) const
{
    return mSourceFactorsE.at(fieldDirection);
}

float NeighborBuffer::
sourceFactorH(int fieldDirection) const
{
    return mSourceFactorsH.at(fieldDirection);
}

Rect3i NeighborBuffer::
edgeHalfCells(const Rect3i & halfCells, int nSide)
{
	Rect3i outerHalfCells = edgeOfRect(halfCells, nSide);
	
	// the fat boundary contains the cells on BOTH sides of the TFSF boundary
	// in the destination grid
	if (nSide % 2 == 0) // if this is a low-x, low-y or low-z side
		outerHalfCells.p1 += cardinal(nSide);
	else
		outerHalfCells.p2 += cardinal(nSide);
	
    return outerHalfCells;
}

void NeighborBuffer::
initFactors(const Rect3i & huygensHalfCells, int sideNum,
    float incidentFieldFactor)
{
	Rect3i outerTotalField = edgeOfRect(huygensHalfCells, sideNum);
	unsigned int dir;
    Vector3i fieldOffset, p1Offset;
    
//    LOG << "For side number " << sideNum << " (outer " << outerTotalField
//        << "):\n";
    
    for (dir = 0; dir < 3; dir++)
    {
        bool eFieldIsWithinTotalField, hFieldIsWithinTotalField;
        
        // E fields
        fieldOffset = eFieldOffset(dir);
        p1Offset = outerTotalField.p1%2;
        
        // if we're in the total-field region, SUBTRACT the field here to
        // prevent the incident field from moving out to the scattered field
        // zone.
        
        eFieldIsWithinTotalField = (fieldOffset[sideNum/2] == p1Offset[sideNum/2]);
        
        if (eFieldIsWithinTotalField)
        {
//            LOG << "E" << char('x'+dir) << " is TOTAL FIELD\n";
            mDestFactorsE[dir] = 1.0f;
            mSourceFactorsE[dir] = -1.0f*incidentFieldFactor;
        }
        else
        {
//            LOG << "E" << char('x'+dir) << " is SCATTERED FIELD\n";
            mDestFactorsE[dir] = 1.0f;
            mSourceFactorsE[dir] = 1.0f * incidentFieldFactor;
        }
        
        // H fields
        fieldOffset = hFieldOffset(dir);
        p1Offset = outerTotalField.p1%2;
        hFieldIsWithinTotalField = (fieldOffset[sideNum/2] == p1Offset[sideNum/2]);
        
        if (hFieldIsWithinTotalField)
        {
//            LOG << "H" << char('x'+dir) << " is TOTAL FIELD\n";
            mDestFactorsH[dir] = 1.0f;
            mSourceFactorsH[dir] = -1.0f*incidentFieldFactor;
        }
        else
        {
//            LOG << "H" << char('x'+dir) << " is SCATTERED FIELD\n";
            mDestFactorsH[dir] = 1.0f;
            mSourceFactorsH[dir] = 1.0f * incidentFieldFactor;
        }
    }
}




