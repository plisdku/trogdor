/*
 *  MaterialSetupMaterial.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/19/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "MaterialRunlineEncoder.h"
#include "VoxelizedPartition.h"
#include "YeeUtilities.h"

#include <iostream>
using namespace std;
using namespace YeeUtilities;

BulkMaterialRLE::
BulkMaterialRLE()
{
    setNeighborFieldContinuity(kNeighborFieldTransverse4);
}

void BulkMaterialRLE::
endRunline(const VoxelizedPartition & vp, const Vector3i & lastHalfCell)
{
//    LOG << "Ending runline from " << firstHalfCell() << " length " << length()
//        << endl;
    
    int oct = octant(firstHalfCell());
    int dir0 = xyz(oct);    // this is the field direction, "i"
    int dir1 = (dir0+1)%3;   // first transverse direction, "j"
    int dir2 = (dir1+1)%3;  // second transverse direction, "k"
    
    SBMRunline* rl = new SBMRunline;
    rl->length = length();
    rl->auxIndex = vp.indices()(firstHalfCell());
    rl->f_i = vp.lattice().pointer(firstHalfCell());
    
    // WATCH CAREFULLY
    //
    // f_i will be, e.g., Ex
    // f_j will be Hy
    // f_k will be Hz
    //
    // The non-trivial piece here is that the neighbor in direction j is a
    // field that points along k.  I've screwed this up several times!
    
    rl->f_j[0] = vp.lattice().wrappedPointer(firstHalfCell()+cardinal(2*dir2));
    rl->f_j[1] = vp.lattice().wrappedPointer(firstHalfCell()+cardinal(2*dir2+1));
    rl->f_k[0] = vp.lattice().wrappedPointer(firstHalfCell()+cardinal(2*dir1));
    rl->f_k[1] = vp.lattice().wrappedPointer(firstHalfCell()+cardinal(2*dir1+1));
    
    if (isE(oct))
        mRunlinesE[dir0].push_back(SBMRunlinePtr(rl));
    else
        mRunlinesH[dir0].push_back(SBMRunlinePtr(rl));
}

BulkPMLRLE::
BulkPMLRLE()
{
    setNeighborFieldContinuity(kNeighborFieldTransverse4);
    setLineContinuity(kLineContinuityRequired);
}

void BulkPMLRLE::
endRunline(const VoxelizedPartition & vp, const Vector3i & lastHalfCell)
{
    int oct = octant(firstHalfCell());
    int dir0 = xyz(oct);    // this is the field direction, "i"
    int dir1 = (dir0+1)%3;   // first transverse direction, "j"
    int dir2 = (dir1+1)%3;  // second transverse direction, "k"
    
    SBPMRunline* rl = new SBPMRunline;
    rl->length = length();
    rl->auxIndex = vp.indices()(firstHalfCell());
    rl->f_i = vp.lattice().pointer(firstHalfCell());
    
    // WATCH CAREFULLY
    //
    // f_i will be, e.g., Ex
    // f_j will be Hy
    // f_k will be Hz
    //
    // The non-trivial piece here is that the neighbor in direction j is a
    // field that points along k.  I've screwed this up several times!
    
    rl->f_j[0] = vp.lattice().wrappedPointer(firstHalfCell()+cardinal(2*dir2));
    rl->f_j[1] = vp.lattice().wrappedPointer(firstHalfCell()+cardinal(2*dir2+1));
    rl->f_k[0] = vp.lattice().wrappedPointer(firstHalfCell()+cardinal(2*dir1));
    rl->f_k[1] = vp.lattice().wrappedPointer(firstHalfCell()+cardinal(2*dir1+1));
    
    // PML aux stuff
    // The start point of the runline *may* be outside the grid, *if* we are
    // performing calculations on ghost points!  This may happen in data-push
    // adjoint update equations.  In any case, usually the wrap does nothing.
    
    Vector3i pmlDirections = vp.voxels()(firstHalfCell())->pmlDirections();
    assert(vec_eq(vp.gridHalfCells().p1, 0));
    Vector3i gridNumHalfCells = vp.gridHalfCells().size() + 1;
    Vector3i wrappedStart = (firstHalfCell()+gridNumHalfCells)
        % gridNumHalfCells;
    assert(octant(wrappedStart) == octant(firstHalfCell()));
    Rect3i pmlYeeCells = halfToYee(vp.pmlHalfCells(pmlDirections), oct);
    rl->pmlDepthIndex = halfToYee(wrappedStart) - pmlYeeCells.p1;
    
    if (isE(oct))
        mRunlinesE[dir0].push_back(SBPMRunlinePtr(rl));
    else
        mRunlinesH[dir0].push_back(SBPMRunlinePtr(rl));
}
