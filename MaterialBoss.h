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

class SetupMaterial;
typedef Pointer<SetupMaterial> SetupMaterialPtr;

class GridDescription;

class MaterialFactory
{
public:
	static SetupMaterialPtr newSetupMaterial(const VoxelGrid & vg,
		const PartitionCellCountPtr cg,
        const GridDescription & gridDesc,
		Paint* parentPaint);
    
    static Map<Vector3i, Map<std::string, std::string> > defaultPMLParams();
};

class Material
{
public:
    Material();
    virtual ~Material();
    
    virtual void calcEPhase(int direction) = 0;
    virtual void calcHPhase(int direction) = 0;
    
    virtual void allocateAuxBuffers();
private:
    
};
typedef Pointer<Material> MaterialPtr;

// This is that rare thing—a class in Trogdor which doesn't observe RAII.
// Initializing it just has too darned many parameters.
class SetupMaterial
{
public:
	SetupMaterial();
	virtual ~SetupMaterial();
	
	// Auxiliary variables
    virtual void setNumCellsE(int fieldDir, int numCells);
    virtual void setNumCellsH(int fieldDir, int numCells);
    virtual void setPMLHalfCells(int pmlDirection, Rect3i halfCellsOnSide,
        const GridDescription & gridDesc);
    virtual void setNumCellsOnPMLFaceE(int fieldDir, int faceNum, int numCells);
    virtual void setNumCellsOnPMLFaceH(int fieldDir, int faceNum, int numCells);
	
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
    
    // Accessor for paint with all the goodies
    void setParentPaint(Paint* paint) { mParentPaint = paint; }
    Paint* getParentPaint() const { return mParentPaint; }
private:
    Paint* mParentPaint;
};
typedef Pointer<SetupMaterial> SetupMaterialPtr;

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
	
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
    
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
	
    virtual MaterialPtr makeCalcMaterial(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
        
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
    
    float* fi;
    float* gj[2];
    float* gk[2];
    unsigned long length;
};

struct SimplePMLRunline
{
    SimplePMLRunline() {}
    SimplePMLRunline(const SBPMRunline & setupRunline);
    
    float* fi;
    float* gj[2];
    float* gk[2];
    unsigned long pmlIndex[3];
    unsigned long length;
};

struct SimpleAuxRunline
{
    SimpleAuxRunline() {}
    SimpleAuxRunline(const SBMRunline & setupRunline);
    
    float* fi;
    float* gj[2];
    float* gk[2];
    unsigned long length;
    unsigned long auxIndex;
};

struct SimpleAuxPMLRunline
{
    SimpleAuxPMLRunline() {}
    SimpleAuxPMLRunline(const SBPMRunline & setupRunline);
    
    float* fi;
    float* gj[2];
    float* gk[2];
    unsigned long length;
    unsigned long auxIndex;
    unsigned long pmlIndex[3];
};

std::ostream & operator<<(std::ostream & str, const SimpleRunline & rl);
std::ostream & operator<<(std::ostream & str, const SimplePMLRunline & rl);
std::ostream & operator<<(std::ostream & str, const SimpleAuxRunline & rl);
std::ostream & operator<<(std::ostream & str, const SimpleAuxPMLRunline & rl);












#endif
