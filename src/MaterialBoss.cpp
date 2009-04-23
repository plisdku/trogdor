/*
 *  MaterialBoss.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "MaterialBoss.h"
#include "SimulationDescription.h"

#include "VoxelizedPartition.h"
#include "VoxelGrid.h"
#include "PartitionCellCount.h"
#include "Paint.h"
#include "Log.h"
#include "YeeUtilities.h"

#include "StaticDielectric.h"
#include "DrudeModel1.h"

using namespace std;
using namespace YeeUtilities;

MaterialDelegatePtr NewMaterialFactory::
getDelegate(const VoxelGrid & vg, const PartitionCellCountPtr cg, 
	Paint* parentPaint)
{
	assert(parentPaint != 0L);
	LOG << "Delegate for " << *parentPaint << endl;
	
	PaintType type = parentPaint->getType();
	const MaterialDescPtr bulkMaterial = parentPaint->getBulkMaterial();
	string materialClass(bulkMaterial->getClass());
	string materialName(bulkMaterial->getName());
	
	/*
	if (materialClass == "StaticDielectricModel")
	{
		return MaterialDelegatePtr(new StaticDielectricDelegate);
	}
	else if (materialClass == "DrudeMetalModel")
	{
		return MaterialDelegatePtr(new DrudeModel1Delegate);
	}
	else if (materialClass == "PEC")
	{
	}
	*/
	
	if (parentPaint->isPML())
		return MaterialDelegatePtr(new SimpleBulkPMLMaterialDelegate);
	return MaterialDelegatePtr(new SimpleBulkMaterialDelegate);
}

MaterialDelegate::
MaterialDelegate()
{
}

MaterialDelegate::
~MaterialDelegate()
{
}

void MaterialDelegate::
setNumCells(int octant, int number)
{
	//LOG << "Default: octant " << octant << ", num " << number << "\n";
}

void MaterialDelegate::
setNumCellsOnPMLFace(int octant, int faceNum, int number)
{
	//LOG << "Default: octant " << octant << ", face " << faceNum << ", num "
	//	<< number << "\n";
}

void MaterialDelegate::
setPMLDepth(int octant, int faceNum, int number)
{
	//LOG << "Default: octant " << octant << ", face " << faceNum << ", num "
	//	<< number << "\n";
}


#pragma mark *** Simple Bulk Material ***

SimpleBulkMaterialDelegate::
SimpleBulkMaterialDelegate() :
	MaterialDelegate()
{
}

void SimpleBulkMaterialDelegate::
startRunline(const VoxelizedPartition & vp, const Vector3i & startPos)
{
	int nSide;
	const VoxelGrid & voxelGrid(vp.getVoxelGrid());
	const PartitionCellCount & cellCountGrid(*vp.getIndices());
	
	mStartPoint = startPos;
	mStartPaint = voxelGrid(startPos);
	
	// Set the mask (which directions to check)
	int fieldDirection = octantFieldDirection(startPos);
	int dir_j = (fieldDirection+1)%3;
	int dir_k = (fieldDirection+2)%3;
	
	// Set the start neighbor indices, check buffers, store pointers
	BufferPointer bp[6]; // we won't fill all of these, but it makes things easy
	for (nSide = 0; nSide < 6; nSide++)
	{
		mStartNeighborIndices[nSide] = vp.linearYeeIndex(
			startPos + cardinalDirection(nSide));
		
		if (nSide/2 == fieldDirection)
			mUsedNeighborIndices[nSide] = 0;
		else if (mStartPaint->hasCurlBuffer(nSide))
		{
			mUsedNeighborIndices[nSide] = 0;
			bp[nSide] = vp.fieldPointer(mStartPaint->getCurlBuffer(nSide),
				mStartPoint+cardinalDirection(nSide));
		}
		else
		{
			mUsedNeighborIndices[nSide] = 1;
			bp[nSide] = vp.fieldPointer(mStartPoint+cardinalDirection(nSide));
		}
	}
	mCurrentRunline.f_i = vp.fieldPointer(startPos);
	mCurrentRunline.f_j[0] = bp[2*dir_j];
	mCurrentRunline.f_j[1] = bp[2*dir_j+1];
	mCurrentRunline.f_k[0] = bp[2*dir_k];
	mCurrentRunline.f_k[1] = bp[2*dir_k+1];
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

bool SimpleBulkMaterialDelegate::
canContinueRunline(const VoxelizedPartition & vp, const Vector3i & oldPos,
	const Vector3i & newPos, Paint* newPaint) const
{
	for (int nSide = 0; nSide < 6; nSide++)
	if (mUsedNeighborIndices[nSide])
	{
		int index = vp.linearYeeIndex(newPos + cardinalDirection(nSide));
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

void SimpleBulkMaterialDelegate::
continueRunline(const Vector3i & newPos)
{
	mCurrentRunline.length++;
}

void SimpleBulkMaterialDelegate::
endRunline()
{
	int field = octantFieldNumber(mStartPoint);
	mRunlines[field].push_back(SBMRunlinePtr(new SBMRunline(mCurrentRunline)));
}

void SimpleBulkMaterialDelegate::
printRunlines(std::ostream & out) const
{
	for (int field = 0; field < 6; field++)
	{
		out << "Field " << field << "\n";
		for (unsigned int rr = 0; rr < mRunlines[field].size(); rr++)
		{
			out << rr << ": length " << mRunlines[field][rr]->length <<
				" aux " << mRunlines[field][rr]->auxIndex << "\n";
			out << "\t" << mRunlines[field][rr]->f_i << "\n";
			out << "\t" << mRunlines[field][rr]->f_j[0] << "\n";
			out << "\t" << mRunlines[field][rr]->f_j[1] << "\n";
			out << "\t" << mRunlines[field][rr]->f_k[0] << "\n";
			out << "\t" << mRunlines[field][rr]->f_k[1] << "\n";
		}
	}
}

#pragma mark *** Simple Bulk PML Material ***

SimpleBulkPMLMaterialDelegate::
SimpleBulkPMLMaterialDelegate() :
	MaterialDelegate()
{
}

void SimpleBulkPMLMaterialDelegate::
startRunline(const VoxelizedPartition & vp, const Vector3i & startPos)
{
	int nSide;
	const VoxelGrid & voxelGrid(vp.getVoxelGrid());
	const PartitionCellCount & cellCountGrid(*vp.getIndices());
	
	mStartPoint = startPos;
	mStartPaint = voxelGrid(startPos);
	
	// Set the mask (which directions to check)
	int fieldDirection = octantFieldDirection(startPos);
	int dir_j = (fieldDirection+1)%3;
	int dir_k = (fieldDirection+2)%3;
	
	// Set the start neighbor indices, check buffers, store pointers
	BufferPointer bp[6]; // we won't fill all of these, but it makes things easy
	for (nSide = 0; nSide < 6; nSide++)
	{
		mStartNeighborIndices[nSide] = vp.linearYeeIndex(
			startPos + cardinalDirection(nSide));
		
		if (nSide/2 == fieldDirection)
			mUsedNeighborIndices[nSide] = 0;
		else if (mStartPaint->hasCurlBuffer(nSide))
		{
			mUsedNeighborIndices[nSide] = 0;
			bp[nSide] = vp.fieldPointer(mStartPaint->getCurlBuffer(nSide),
				mStartPoint+cardinalDirection(nSide));
		}
		else
		{
			mUsedNeighborIndices[nSide] = 1;
			bp[nSide] = vp.fieldPointer(mStartPoint+cardinalDirection(nSide));
		}
	}
	mCurrentRunline.f_i = vp.fieldPointer(startPos);
	mCurrentRunline.f_j[0] = bp[2*dir_j];
	mCurrentRunline.f_j[1] = bp[2*dir_j+1];
	mCurrentRunline.f_k[0] = bp[2*dir_k];
	mCurrentRunline.f_k[1] = bp[2*dir_k+1];
	mCurrentRunline.auxIndex = cellCountGrid(startPos);
	mCurrentRunline.length = 1;
	
	// PML aux stuff
	Rect3i gridHalfCells = vp.getGridHalfCells();
	Vector3i gridSize = gridHalfCells.size() + Vector3i(1,1,1);
	Vector3i pmlDir = mStartPaint->getPMLDirections();
	mPMLRect = vp.getPMLRegion(pmlDir);
	
	Vector3i wrappedStartPoint = Vector3i(mStartPoint + gridSize) % gridSize;
	mCurrentRunline.pmlDepthIndex = wrappedStartPoint/2 - mPMLRect.p1/2;
	/*
	LOG << "runline for PML in " << mPMLRect << "\n";
	LOGMORE << "dir " << pmlDir << "\n";
	LOGMORE << "start " << mStartPoint << "\n";
	LOGMORE << "grid size " << gridSize << "\n";
	LOGMORE << "wrap start " << wrappedStartPoint << "\n";
	LOGMORE << "depth " << mCurrentRunline.pmlDepthIndex << "\n";
	*/
}

bool SimpleBulkPMLMaterialDelegate::
canContinueRunline(const VoxelizedPartition & vp, const Vector3i & oldPos,
	const Vector3i & newPos, Paint* newPaint) const
{
	for (int nSide = 0; nSide < 6; nSide++)
	if (mUsedNeighborIndices[nSide])
	{
		int index = vp.linearYeeIndex(newPos + cardinalDirection(nSide));
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

void SimpleBulkPMLMaterialDelegate::
continueRunline(const Vector3i & newPos)
{
	mCurrentRunline.length++;
}

void SimpleBulkPMLMaterialDelegate::
endRunline()
{
	int field = octantFieldNumber(mStartPoint);
	mRunlines[field].push_back(SBPMRunlinePtr(
		new SBPMRunline(mCurrentRunline)));
}

void SimpleBulkPMLMaterialDelegate::
printRunlines(std::ostream & out) const
{
	for (int field = 0; field < 6; field++)
	{
		out << "Field " << field << "\n";
		for (unsigned int rr = 0; rr < mRunlines[field].size(); rr++)
		{
			out << rr << ": length " << mRunlines[field][rr]->length <<
				" aux " << mRunlines[field][rr]->auxIndex <<
				" pml depth";
			for (int nn = 0; nn < 3; nn++)
				out << " " << mRunlines[field][rr]->pmlDepthIndex[nn];
			out << "\n";
			out << "\t" << mRunlines[field][rr]->f_i << "\n";
			out << "\t" << mRunlines[field][rr]->f_j[0] << "\n";
			out << "\t" << mRunlines[field][rr]->f_j[1] << "\n";
			out << "\t" << mRunlines[field][rr]->f_k[0] << "\n";
			out << "\t" << mRunlines[field][rr]->f_k[1] << "\n";
		}
	}
}


