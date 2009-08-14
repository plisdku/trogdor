/*
 *  SimpleSetupMaterial.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "SimpleSetupMaterial.h"
#include "SimulationDescription.h"

//#include "CalculationPartition.h"
#include "HuygensSurface.h"
#include "InterleavedLattice.h"
#include "VoxelizedPartition.h"
#include "VoxelGrid.h"
#include "PartitionCellCount.h"
#include "Paint.h"
#include "Log.h"
#include "YeeUtilities.h"

using namespace std;
using namespace YeeUtilities;

#pragma mark *** Simple Bulk Material ***

BulkRunlineEncoder::
BulkRunlineEncoder() :
	RunlineEncoder()
{
}

BulkRunlineEncoder::
~BulkRunlineEncoder()
{
}

void BulkRunlineEncoder::
startRunline(const VoxelizedPartition & vp, const Vector3i & startPos)
{
	int nSide;
	const VoxelGrid & voxelGrid(vp.getVoxelGrid());
	const PartitionCellCount & cellCountGrid(*vp.getIndices());
    InterleavedLatticePtr mainLattice(vp.getLattice());
	
	mStartPoint = startPos;
	mStartPaint = voxelGrid(startPos);
	
	// Set the mask (which directions to check)
	int fieldDirection = xyz(octant(startPos));
	int dir_j = (fieldDirection+1)%3;
	int dir_k = (fieldDirection+2)%3;
	
	// Set the start neighbor indices, check buffers, store pointers
	BufferPointer bp[6]; // we won't fill all of these, but it makes things easy
	for (nSide = 0; nSide < 6; nSide++)
	{
        mStartNeighborIndices[nSide] = mainLattice->wrappedLinearYeeIndex(
            startPos+cardinal(nSide));
        
		//mStartNeighborIndices[nSide] = vp.linearYeeIndex(
		//	vp.wrap(startPos + cardinal(nSide)));
		
		if (nSide/2 == fieldDirection)
			mUsedNeighborIndices[nSide] = 0;
		else if (mStartPaint->hasCurlBuffer(nSide))
		{
			mUsedNeighborIndices[nSide] = 0;
            bp[nSide] = mStartPaint->getCurlBuffer(nSide)->
                getLattice()->wrappedPointer(mStartPoint+cardinal(nSide));
		}
		else
		{
			mUsedNeighborIndices[nSide] = 1;
			bp[nSide] = mainLattice->wrappedPointer(mStartPoint+cardinal(nSide));
		}
	}
    // WATCH CAREFULLY
    // f_i will be, e.g., Ex
    // f_j will be Hy
    // f_k will be Hz
    // 
	mCurrentRunline.f_i = mainLattice->pointer(startPos);
	mCurrentRunline.f_j[0] = bp[2*dir_k];
	mCurrentRunline.f_j[1] = bp[2*dir_k+1];
	mCurrentRunline.f_k[0] = bp[2*dir_j];
	mCurrentRunline.f_k[1] = bp[2*dir_j+1];
	mCurrentRunline.auxIndex = cellCountGrid(startPos);
	mCurrentRunline.length = 1;
	
	//LOG << "Field direction " << fieldDirection << " ui " << ui << " uj "
	//	<< uj << " uk " << uk << "\n";
	
	
    /*
	LOG << "Start runline:\n";
	LOGMORE << "start " << mStartPoint << "\n";
	LOGMORE << "aux " << mCurrentRunline.auxIndex << "\n";
	LOGMORE << "f_i " << mCurrentRunline.f_i << "\n";
	LOGMORE << "f_j " << mCurrentRunline.f_j[0] << " "
		<< mCurrentRunline.f_j[1] << "\n";
	LOGMORE << "f_k " << mCurrentRunline.f_k[0] << " "
		<< mCurrentRunline.f_k[1] << "\n";
    */
}

bool BulkRunlineEncoder::
canContinueRunline(const VoxelizedPartition & vp, const Vector3i & oldPos,
	const Vector3i & newPos, Paint* newPaint, int runlineDirection) const
{
    if (newPaint != mStartPaint)
        return 0;
    InterleavedLatticePtr mainLattice(vp.getLattice());
    
	for (int nSide = 0; nSide < 6; nSide++)
	if (mUsedNeighborIndices[nSide])
	{
		int index = mainLattice->wrappedLinearYeeIndex(
            newPos + cardinal(nSide));
		if (mStartNeighborIndices[nSide] + mCurrentRunline.length != index)
			return 0;
	}
	else
	{
		if (newPaint->getCurlBuffer(nSide) != mStartPaint->getCurlBuffer(nSide))
			return 0;
	}
	return 1;
}

void BulkRunlineEncoder::
continueRunline(const Vector3i & newPos)
{
	mCurrentRunline.length++;
}

void BulkRunlineEncoder::
endRunline()
{
    int oct = octant(mStartPoint);
    if (isE(oct))
        mRunlinesE[xyz(oct)].push_back(
            SBMRunlinePtr(new SBMRunline(mCurrentRunline)));
    else if (isH(oct))
        mRunlinesH[xyz(oct)].push_back(
            SBMRunlinePtr(new SBMRunline(mCurrentRunline)));
    else
        assert(!"Bad octant.");
}

void BulkRunlineEncoder::
printRunlines(std::ostream & out) const
{
    int dir;
    unsigned int rr;
    for (dir = 0; dir < 3; dir++)
	{
		out << "E " << dir << "\n";
		for (rr = 0; rr < mRunlinesE[dir].size(); rr++)
		{
			out << rr << ": length " << mRunlinesE[dir][rr]->length <<
				" aux " << mRunlinesE[dir][rr]->auxIndex << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_i << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_j[0] << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_j[1] << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_k[0] << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_k[1] << "\n";
		}
	}
    for (dir = 0; dir < 3; dir++)
	{
		out << "H " << dir << "\n";
		for (rr = 0; rr < mRunlinesH[dir].size(); rr++)
		{
			out << rr << ": length " << mRunlinesH[dir][rr]->length <<
				" aux " << mRunlinesH[dir][rr]->auxIndex << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_i << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_j[0] << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_j[1] << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_k[0] << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_k[1] << "\n";
		}
	}
}

#pragma mark *** Simple Bulk PML Material ***

BulkPMLRunlineEncoder::
BulkPMLRunlineEncoder() :
	RunlineEncoder()
{
}

void BulkPMLRunlineEncoder::
startRunline(const VoxelizedPartition & vp, const Vector3i & startPos)
{
	int nSide;
	const VoxelGrid & voxelGrid(vp.getVoxelGrid());
	const PartitionCellCount & cellCountGrid(*vp.getIndices());
    InterleavedLatticePtr mainLattice(vp.getLattice());
	
	mStartPoint = startPos;
	mStartPaint = voxelGrid(startPos);
	
	// Set the mask (which directions to check)
	int fieldDirection = xyz(octant(startPos));
	int dir_j = (fieldDirection+1)%3;
	int dir_k = (fieldDirection+2)%3;
	
	// Set the start neighbor indices, check buffers, store pointers
	BufferPointer bp[6]; // we won't fill all of these, but it makes things easy
	for (nSide = 0; nSide < 6; nSide++)
	{
        mStartNeighborIndices[nSide] = mainLattice->wrappedLinearYeeIndex(
            startPos+cardinal(nSide));
        
		//mStartNeighborIndices[nSide] = vp.linearYeeIndex(
		//	vp.wrap(startPos + cardinal(nSide)));
		
		if (nSide/2 == fieldDirection)
			mUsedNeighborIndices[nSide] = 0;
		else if (mStartPaint->hasCurlBuffer(nSide))
		{
			mUsedNeighborIndices[nSide] = 0;
			bp[nSide] = mStartPaint->getCurlBuffer(nSide)->getLattice()->
                wrappedPointer(mStartPoint+cardinal(nSide));
		}
		else
		{
			mUsedNeighborIndices[nSide] = 1;
			bp[nSide] = mainLattice->wrappedPointer(
                mStartPoint+cardinal(nSide));
		}
	}
	mCurrentRunline.f_i = mainLattice->pointer(startPos);
	mCurrentRunline.f_j[0] = bp[2*dir_k];    // for Ex, Hy is the z neighbor.
	mCurrentRunline.f_j[1] = bp[2*dir_k+1];
	mCurrentRunline.f_k[0] = bp[2*dir_j];
	mCurrentRunline.f_k[1] = bp[2*dir_j+1];
	mCurrentRunline.auxIndex = cellCountGrid(startPos);
	mCurrentRunline.length = 1;
	
    // PML aux stuff
    // The start point of the runline *may* be outside the grid, *if* we are
    // performing calculations on ghost points!  This may happen in data-push
    // adjoint update equations.  In any case, usually the wrap does nothing.
    assert(vec_eq(vp.getGridHalfCells().p1, 0));
    Vector3i gridNumHalfCells = vp.getGridHalfCells().size() + 1;
    Vector3i wrappedStart = (mStartPoint+gridNumHalfCells) % gridNumHalfCells;
    assert(octant(wrappedStart) == octant(mStartPoint));
    Rect3i pmlYeeCells = halfToYee(
        vp.getPMLHalfCells(mStartPaint->getPMLDirections()),
        octant(wrappedStart));
    mCurrentRunline.pmlDepthIndex = halfToYee(wrappedStart) - pmlYeeCells.p1;
    
    // TODO: test that this gives the correct PML if we wrapped to the far
    // side of the grid somehow.  (What am I talking about???)
    /*
	LOGMORE << "dir " << mStartPaint->getPMLDirections() << "\n";
	LOGMORE << "start " << mStartPoint << "\n";
	LOGMORE << "depth " << mCurrentRunline.pmlDepthIndex << "\n";
    */
}

bool BulkPMLRunlineEncoder::
canContinueRunline(const VoxelizedPartition & vp, const Vector3i & oldPos,
	const Vector3i & newPos, Paint* newPaint,
    int runlineDirection) const
{
    InterleavedLatticePtr mainLattice(vp.getLattice());
    if (newPaint != mStartPaint)
        return 0;
    
	for (int nSide = 0; nSide < 6; nSide++)
	if (mUsedNeighborIndices[nSide])
	{
		int index = mainLattice->wrappedLinearYeeIndex(
            newPos + cardinal(nSide));
		if (mStartNeighborIndices[nSide] + mCurrentRunline.length != index)
			return 0;
	}
	else
	{
		if (newPaint->getCurlBuffer(nSide) != mStartPaint->getCurlBuffer(nSide))
			return 0;
	}
    
    // Notably differing from non-PML, the PML materials can't wrap to a new
    // y or z coordinate because that screws up the indexing into their
    // update constants.
    
    const int rd1 = (runlineDirection+1)%3;
    const int rd2 = (runlineDirection+2)%3;
    if (oldPos[rd1] != newPos[rd1] || oldPos[rd2] != newPos[rd2])
        return 0;
    
	return 1;
}

void BulkPMLRunlineEncoder::
continueRunline(const Vector3i & newPos)
{
	mCurrentRunline.length++;
}

void BulkPMLRunlineEncoder::
endRunline()
{
    int oct = octant(mStartPoint);
    if (isE(oct))
        mRunlinesE[xyz(oct)].push_back(
            SBPMRunlinePtr(new SBPMRunline(mCurrentRunline)));
    else if (isH(oct))
        mRunlinesH[xyz(oct)].push_back(
            SBPMRunlinePtr(new SBPMRunline(mCurrentRunline)));
    else
        assert(!"Bad octant.");
}

void BulkPMLRunlineEncoder::
printRunlines(std::ostream & out) const
{
    int dir;
    unsigned int rr;
    for (dir = 0; dir < 3; dir++)
	{
		out << "E " << dir << "\n";
		for (rr = 0; rr < mRunlinesE[dir].size(); rr++)
		{
			out << rr << ": length " << mRunlinesE[dir][rr]->length <<
				" aux " << mRunlinesE[dir][rr]->auxIndex <<
				" pml depth";
			for (int nn = 0; nn < 3; nn++)
				out << " " << mRunlinesE[dir][rr]->pmlDepthIndex[nn];
			//out << rr << ": length " << mRunlinesE[dir][rr]->length <<
			//	" aux " << mRunlinesE[dir][rr]->auxIndex << "\n";
			out << "\n\t" << mRunlinesE[dir][rr]->f_i << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_j[0] << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_j[1] << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_k[0] << "\n";
			out << "\t" << mRunlinesE[dir][rr]->f_k[1] << "\n";
		}
	}
    for (dir = 0; dir < 3; dir++)
	{
		out << "H " << dir << "\n";
		for (rr = 0; rr < mRunlinesH[dir].size(); rr++)
		{
			out << rr << ": length " << mRunlinesH[dir][rr]->length <<
				" aux " << mRunlinesH[dir][rr]->auxIndex <<
				" pml depth";
			for (int nn = 0; nn < 3; nn++)
				out << " " << mRunlinesH[dir][rr]->pmlDepthIndex[nn];
			out << "\n\t" << mRunlinesH[dir][rr]->f_i << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_j[0] << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_j[1] << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_k[0] << "\n";
			out << "\t" << mRunlinesH[dir][rr]->f_k[1] << "\n";
		}
	}
}





















