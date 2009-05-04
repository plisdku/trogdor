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
class PartitionCellCount;
typedef Pointer<PartitionCellCount> PartitionCellCountPtr;

class CalculationPartition;
typedef Pointer<CalculationPartition> CalculationPartitionPtr;

class VoxelizedPartition;

class MaterialDelegate;
typedef Pointer<MaterialDelegate> MaterialDelegatePtr;

class NewMaterialFactory
{
public:
	static MaterialDelegatePtr getDelegate(const VoxelGrid & vg,
		const PartitionCellCountPtr cg,
		/*const std::vector<PartitionCellCountPtr> & pmlFaces,*/
		Paint* parentPaint);
};

class Material
{
public:
    Material();
    virtual ~Material();
    
    virtual void calcEPhase(int phasePart = 0) = 0; // subphase, cache friendly?
    virtual void calcHPhase(int phasePart = 0) = 0;
private:
    
};
typedef Pointer<Material> MaterialPtr;

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
	virtual void startRunline(const VoxelizedPartition & vp,
		const Vector3i & startPos) = 0;
	virtual bool canContinueRunline(const VoxelizedPartition & vp,
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint) const = 0;
	virtual void continueRunline(const Vector3i & newPos) = 0;
	virtual void endRunline() = 0;
	
	virtual void printRunlines(std::ostream & out) const = 0;
    
    // Setting up the runtime materials
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const = 0;
};
typedef Pointer<MaterialDelegate> MaterialDelegatePtr;


class SimpleBulkMaterialDelegate : public MaterialDelegate
{
public:
	SimpleBulkMaterialDelegate();
	
	// Runline handling
	virtual void startRunline(const VoxelizedPartition & vp,
		const Vector3i & startPos);
	virtual bool canContinueRunline(const VoxelizedPartition & vp,
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint) const;
	virtual void continueRunline(const Vector3i & newPos);
	virtual void endRunline();
	
	virtual void printRunlines(std::ostream & out) const;
	
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
protected:
	struct SBMRunline
	{
		long length;
		long auxIndex;
		BufferPointer f_i;
		BufferPointer f_j[2];
		BufferPointer f_k[2];
	};
	typedef Pointer<SBMRunline> SBMRunlinePtr;
	
	std::vector<SBMRunlinePtr> mRunlines[6];
	
	// Runline generation storage
	SBMRunline mCurrentRunline;
	Paint* mStartPaint;
	Vector3i mStartPoint;
	int mStartOctant;
	long mStartNeighborIndices[6];
	bool mUsedNeighborIndices[6];
};

class SimpleBulkPMLMaterialDelegate : public MaterialDelegate
{
public:
	SimpleBulkPMLMaterialDelegate();
	
	// Runline handling
	virtual void startRunline(const VoxelizedPartition & vp,
		const Vector3i & startPos);
	virtual bool canContinueRunline(const VoxelizedPartition & vp,
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint) const;
	virtual void continueRunline(const Vector3i & newPos);
	virtual void endRunline();
	
	virtual void printRunlines(std::ostream & out) const;
	
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
protected:
	struct SBPMRunline
	{
		long length;
		long auxIndex;
		Vector3i pmlDepthIndex;
		BufferPointer f_i;
		BufferPointer f_j[2];
		BufferPointer f_k[2];
	};
	typedef Pointer<SBPMRunline> SBPMRunlinePtr;
	
	std::vector<SBPMRunlinePtr> mRunlines[6];
	
	// Runline generation storage
	SBPMRunline mCurrentRunline;
	Paint* mStartPaint;
	Vector3i mStartPoint;
	int mStartOctant;
	long mStartNeighborIndices[6];
	bool mUsedNeighborIndices[6];
	Rect3i mPMLRect;
};

#endif
