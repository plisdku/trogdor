/*
 *  SetupMaterial.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/13/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _RUNLINEENCODER_
#define _RUNLINEENCODER_

#include "Pointer.h"
#include "geometry.h"
#include "UpdateEquation.h"
#include "RunlineEncoder.h"

class VoxelizedPartition;
class CalculationPartition;
class Paint;


// This is that rare thing—a class in Trogdor which doesn't observe RAII.
// Initializing it just has too darned many parameters.
class SetupMaterial
{
public:
	SetupMaterial() {}
	virtual ~SetupMaterial() {}
    
    void setID(int id) { mID = id; }
    int id() const { return mID; }
		
	virtual void printRunlines(std::ostream & out) const = 0;
    
    virtual RunlineEncoder* encoder();
    
    // Setting up the runtime materials
    virtual UpdateEquationPtr makeUpdateEquation(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const = 0;
private:
    int mID;
};
typedef Pointer<SetupMaterial> SetupMaterialPtr;




























#endif
