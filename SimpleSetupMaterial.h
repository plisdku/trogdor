/*
 *  SimpleSetupMaterial.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SIMPLESETUPMATERIAL_
#define _SIMPLESETUPMATERIAL_

#include "Material.h"     // we inherit Material and SetupMaterial
#include "Pointer.h"
#include "geometry.h"
#include "MemoryUtilities.h"
#include <vector>

class Paint;
class VoxelGrid;
class PartitionCellCount;
typedef Pointer<PartitionCellCount> PartitionCellCountPtr;

class VoxelizedPartition;

class SetupMaterial;
typedef Pointer<SetupMaterial> SetupMaterialPtr;

class GridDescription;

struct SBMRunline
{
    long length;
    long auxIndex;
    BufferPointer f_i;
    BufferPointer f_j[2];
    BufferPointer f_k[2];
};
typedef Pointer<SBMRunline> SBMRunlinePtr;

class SimpleBulkSetupMaterial : public SetupMaterial
{
public:
	SimpleBulkSetupMaterial();
	
	// Runline handling
	virtual void startRunline(const VoxelizedPartition & vp,
		const Vector3i & startPos);
	virtual bool canContinueRunline(const VoxelizedPartition & vp,
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint) const;
	virtual void continueRunline(const Vector3i & newPos);
	virtual void endRunline();
	
	virtual void printRunlines(std::ostream & out) const;
	
//    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
//        const CalculationPartition & cp) const;
    
    const std::vector<SBMRunlinePtr> & getRunlinesE(int dir) const
        { return mRunlinesE[dir]; }
    const std::vector<SBMRunlinePtr> & getRunlinesH(int dir) const
        { return mRunlinesH[dir]; }
    Paint* getPaint() const { return mStartPaint; }
protected:
	std::vector<SBMRunlinePtr> mRunlinesE[3];
	std::vector<SBMRunlinePtr> mRunlinesH[3];
	
	// Runline generation storage
	SBMRunline mCurrentRunline;
	Paint* mStartPaint;
	Vector3i mStartPoint;
	int mStartOctant;
	long mStartNeighborIndices[6];
	bool mUsedNeighborIndices[6];
};

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

class SimpleBulkPMLSetupMaterial : public SetupMaterial
{
public:
	SimpleBulkPMLSetupMaterial();
	
	// Runline handling
	virtual void startRunline(const VoxelizedPartition & vp,
		const Vector3i & startPos);
	virtual bool canContinueRunline(const VoxelizedPartition & vp,
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint) const;
	virtual void continueRunline(const Vector3i & newPos);
	virtual void endRunline();
	
	virtual void printRunlines(std::ostream & out) const;
	    
    const std::vector<SBPMRunlinePtr> & getRunlinesE(int dir) const
        { return mRunlinesE[dir]; }
    const std::vector<SBPMRunlinePtr> & getRunlinesH(int dir) const
        { return mRunlinesH[dir]; }
    
protected:
	std::vector<SBPMRunlinePtr> mRunlinesE[3];
	std::vector<SBPMRunlinePtr> mRunlinesH[3];
	
	// Runline generation storage
	SBPMRunline mCurrentRunline;
	Paint* mStartPaint;
	Vector3i mStartPoint;
	int mStartOctant;
	long mStartNeighborIndices[6];
	bool mUsedNeighborIndices[6];
	Rect3i mPMLRect;
};

struct SimpleRunline
{
    SimpleRunline() {}
    SimpleRunline(const SBMRunline & setupRunline);
    SimpleRunline(const SBPMRunline & setupRunline);
    
    float* fi;
    float* gj[2];
    float* gk[2];
    unsigned long length;
};

struct SimpleAuxRunline
{
    SimpleAuxRunline() {}
    SimpleAuxRunline(const SBMRunline & setupRunline);
    SimpleAuxRunline(const SBPMRunline & setupRunline);
    
    float* fi;
    float* gj[2];
    float* gk[2];
    unsigned long length;
    unsigned long auxIndex;
};

std::ostream & operator<<(std::ostream & str, const SimpleRunline & rl);
std::ostream & operator<<(std::ostream & str, const SimpleAuxRunline & rl);












#endif
