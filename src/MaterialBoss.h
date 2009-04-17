/*
 *  MaterialBoss.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MATERIALBOSS_
#define _MATERIALBOSS_

#include "Pointer.h"
#include "geometry.h"
#include "MemoryUtilities.h"
#include <vector>

class Paint;
class VoxelGrid;
class CellCountGrid;
typedef Pointer<CellCountGrid> CellCountGridPtr;

class MaterialDelegate;
typedef Pointer<MaterialDelegate> MaterialDelegatePtr;

class NewMaterialFactory
{
public:
	static MaterialDelegatePtr getDelegate(const VoxelGrid & vg,
		const CellCountGridPtr cg,
		/*const std::vector<CellCountGridPtr> & pmlFaces,*/
		Paint* parentPaint);
};

/*
class MaterialBoss
{
public:
	MaterialBoss(const std::string & materialClass);
	MaterialDelegatePtr getDelegate(Paint* inPaint) const;
	
};
*/

// This is that rare thing—a class in Trogdor which doesn't observe RAII.
// Initializing it just has too darned many parameters.
class MaterialDelegate
{
public:
	MaterialDelegate();
	virtual ~MaterialDelegate();
	
	// Auxiliary variables
	virtual void setNumCells(int octant, int number);
	virtual void setNumCellsOnPMLFace(int octant, int faceNum, int number);
	virtual void setPMLDepth(int octant, int faceNum, int depthCells);
	
	// Runline handling
	virtual void startRunline(const VoxelGrid & voxelGrid,
		const CellCountGrid & cellCountGrid,
		/*const std::vector<CellCountGridPtr> & pmlCellCountGrids,*/
		const Vector3i & startPos) = 0;
	virtual bool canContinueRunline(const VoxelGrid & voxelGrid,
		const CellCountGrid & cellCountGrid,
		/*const std::vector<CellCountGridPtr> & pmlCellCountGrids,*/
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint) const = 0;
	virtual void continueRunline(const Vector3i & newPos) = 0;
	virtual void endRunline() = 0;
};
typedef Pointer<MaterialDelegate> MaterialDelegatePtr;


class SimpleBulkMaterialDelegate : public MaterialDelegate
{
public:
	SimpleBulkMaterialDelegate();
	
	// Runline handling
	virtual void startRunline(const VoxelGrid & voxelGrid,
		const CellCountGrid & cellCountGrid,
		/*const std::vector<CellCountGridPtr> & pmlCellCountGrids,*/
		const Vector3i & startPos);
	virtual bool canContinueRunline(const VoxelGrid & voxelGrid,
		const CellCountGrid & cellCountGrid,
		/*const std::vector<CellCountGridPtr> & pmlCellCountGrids,*/
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint) const;
	virtual void continueRunline(const Vector3i & newPos);
	virtual void endRunline();
	
protected:
	class SBMRunline
	{
	public:
		SBMRunline();
		long length;
		long auxIndex;
		//BufferPointer f_i;
		//BufferPointer f_j[2];
		//BufferPointer f_k[2];
	};
	typedef Pointer<SBMRunline> SBMRunlinePtr;
	
	std::vector<SBMRunlinePtr> mRunlines[6];
	
	// Runline generation storage
	SBMRunline mCurrentRunline;
	Paint* mStartPaint;
	Vector3i mStartPoint;
	int mStartOctant;
	int mCurLength;
	long mStartNeighborIndices[6];
	bool mUsedNeighborIndices[6];
	
};

/*
class DefaultMaterialDelegate : public MaterialDelegate
{
public:
	DefaultMaterialDelegate(const VoxelGrid & vg, const CellCountGrid & cg);
	virtual ~DefaultMaterialDelegate();
	virtual void startRunline(const Vector3i & startPos, Paint* newPaint);
	virtual bool canContinueRunline(const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint) const;
	virtual void continueRunline(const Vector3i & newPos);
	virtual void endRunline();
	
protected:
	Map<std::string, MemoryBuffer> m
	
private:
	const VoxelGrid & mVoxelGrid;
	const CellCountGrid & mCellCountGrid;
	Paint* mStartPaint;
	Vector3i mStart;
	Vector3i mEnd;
};
*/


#endif
