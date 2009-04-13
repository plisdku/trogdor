/*
 *  MaterialBoss.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "MaterialBoss.h"

#include "VoxelGrid.h"
#include "CellCountGrid.h"
#include "Paint.h"
#include "Log.h"

using namespace std;

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
		
		return MaterialDelegatePtr(new DefaultMaterialDelegate(vg, cg));
	}
	else if (materialClass == "DrudeMetalModel"
	{
	}
	else if (materialClass == "PEC")
	{
	}
	
	return MaterialDelegatePtr(new DefaultMaterialDelegate(vg, cg));
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
MaterialDelegate(const VoxelGrid & vg, const CellCountGrid & cg)
{
}

MaterialDelegate::
~MaterialDelegate()
{
}

void MaterialDelegate::
setNumCells(int octant, int number)
{
}

void MaterialDelegate::
setNumCellsOnPMLFace(int octant, int faceNum, int number)
{
}

void MaterialDelegate::
setPMLDepth(int octant, int faceNum, int number)
{
}

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




