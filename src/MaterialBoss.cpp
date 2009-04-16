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

#include "VoxelGrid.h"
#include "CellCountGrid.h"
#include "Paint.h"
#include "Log.h"
#include "YeeUtilities.h"

#include "StaticDielectric.h"
#include "DrudeModel1.h"

using namespace std;
using namespace YeeUtilities;

MaterialDelegatePtr NewMaterialFactory::
getDelegate(const VoxelGrid & vg, const CellCountGridPtr cg, 
	const vector<CellCountGridPtr> & pmlFaces, Paint* parentPaint)
{
	assert(parentPaint != 0L);
	LOG << "Delegate for " << *parentPaint << endl;
	
	PaintType type = parentPaint->getType();
	const MaterialDescPtr bulkMaterial = parentPaint->getBulkMaterial();
	string materialClass(bulkMaterial->getClass());
	string materialName(bulkMaterial->getName());
	
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
	
	return MaterialDelegatePtr(new SimpleBulkMaterialDelegate);
}
/*
MaterialBoss::
MaterialBoss(const std::string & materialClass)
{
}

MaterialDelegatePtr MaterialBoss::
getDelegate(Paint* inPaint) const
{
}
*/

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


SimpleBulkMaterialDelegate::
SimpleBulkMaterialDelegate() :
	MaterialDelegate()
{
}

void SimpleBulkMaterialDelegate::
startRunline(const VoxelGrid & voxelGrid,
	const CellCountGrid & cellCountGrid,
	const vector<CellCountGridPtr> & pmlCellCountGrids,
	const Vector3i & startPos)
{
	mStartPoint = startPos;
	mStartPaint = voxelGrid(startPos);
	mCurLength = 1;
	// Set the start neighbor indices and check buffers
	for (int nSide = 0; nSide < 6; nSide++)
	{
		mStartNeighborIndices[nSide] = voxelGrid.linearYeeIndex(
			startPos + cardinalDirection(nSide));
		
		if (mStartPaint->hasCurlBuffer(nSide))
			mUsedNeighborIndices[nSide] = 0;
		else
			mUsedNeighborIndices[nSide] = 1;
	}
	
	// Set the mask (which directions to check)
	mStartOctant = halfCellIndex(startPos);
	switch (mStartOctant)
	{
		case 1: // Ex
		case 6: // Hx
			mUsedNeighborIndices[0] = 0;
			mUsedNeighborIndices[1] = 0;
			break;
		case 2: // Ey
		case 5: // Hy
			mUsedNeighborIndices[2] = 0;
			mUsedNeighborIndices[3] = 0;
			break;
		case 3: // Hz
		case 4: // Ez
			mUsedNeighborIndices[4] = 0;
			mUsedNeighborIndices[5] = 0;
			break;
		default:
			cerr << "Runlining in octant " << mStartOctant << "?\n";
			exit(1);
			break;
	}
}

bool SimpleBulkMaterialDelegate::
canContinueRunline(const VoxelGrid & voxelGrid,
		const CellCountGrid & cellCountGrid,
		const std::vector<CellCountGridPtr> & pmlCellCountGrids,
		const Vector3i & oldPos, const Vector3i & newPos,
	Paint* newPaint) const
{
	for (int nSide = 0; nSide < 6; nSide++)
	if (mUsedNeighborIndices[nSide])
	{
		int index = voxelGrid.linearYeeIndex(newPos + cardinalDirection(nSide));
		if (mStartNeighborIndices[nSide] + mCurLength != index)
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
	mCurLength++;
}

void SimpleBulkMaterialDelegate::
endRunline()
{
	LOG << "Ending: " << mStartPoint << " length " << mCurLength << "\n";
	
	int fieldDirection = octantFieldDirection(mStartOctant);
	int dir_j = (fieldDirection+1)%3;
	int dir_k = (fieldDirection+2)%3;
	
	/*
	
	switch (mStartOctant)
	{
		case 1: // Ex
		case 6: // Hx
			mUsedNeighborIndices[0] = 0;
			mUsedNeighborIndices[1] = 0;
			break;
		case 2: // Ey
		case 5: // Hy
			mUsedNeighborIndices[2] = 0;
			mUsedNeighborIndices[3] = 0;
			break;
		case 3: // Hz
		case 4: // Ez
			mUsedNeighborIndices[4] = 0;
			mUsedNeighborIndices[5] = 0;
			break;
		default:
			cerr << "Runlining in octant " << mStartOctant << "?\n";
			exit(1);
			break;
	}*/
	
}

SimpleBulkMaterialDelegate::SBMRunline::
SBMRunline()
{
}


/*

DefaultMaterialDelegate::
DefaultMaterialDelegate(const VoxelGrid & vg, const CellCountGrid & cg) :
	MaterialDelegate(vg, cg),
	mVoxelGrid(vg),
	mCellCountGrid(cg)
{
}

DefaultMaterialDelegate::
~DefaultMaterialDelegate()
{
}

void DefaultMaterialDelegate::
startRunline(const Vector3i & startPos, Paint* newPaint)
{
	LOG << "Starting at " << startPos << "\n";
	mStartPaint = newPaint;
	mStart = startPos;
	mEnd = startPos;
}

bool DefaultMaterialDelegate::
canContinueRunline(const Vector3i & oldPos, const Vector3i & newPos,
	Paint* newPaint) const
{
	if (mStartPaint != newPaint) // this checks for TFSF buffers etc.
		return 0;
	// now check neighbor indices
	
	
	
	return 1;
}

void DefaultMaterialDelegate::
continueRunline(const Vector3i & newPos)
{
	mEnd = newPos;
	//LOG << "Yeah.\n";
}

void DefaultMaterialDelegate::
endRunline()
{
	LOG << "Ending runline: from " << mStart << " to " << mEnd << "\n";
}

*/


