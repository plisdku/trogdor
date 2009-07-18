/*
 *  Material.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _MATERIAL_
#define _MATERIAL_

#include "Pointer.h"
#include "geometry.h"

class VoxelizedPartition;
class CalculationPartition;
class Paint;

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
typedef Pointer<SetupMaterial> SetupMaterialPtr;



#endif