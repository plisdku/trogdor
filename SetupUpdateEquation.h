/*
 *  SetupUpdateEquation.h
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
#include "SimulationDescription.h"

class VoxelizedPartition;
class CalculationPartition;
class Paint;

// This is that rare thing—a class in Trogdor which doesn't observe RAII.
// Initializing it just has too darned many parameters.
class SetupUpdateEquation
{
public:
	SetupUpdateEquation(MaterialDescPtr description) :
        mDescription(description) {}
	virtual ~SetupUpdateEquation() {}
    
    Pointer<MaterialDescription> description() const { return mDescription; }
    
    void setID(int id) { mID = id; }
    int id() const { return mID; }
		
	virtual void printRunlines(std::ostream & out) const = 0;
    
    virtual RunlineEncoder& encoder() = 0;
    
    // Setting up the runtime materials
    virtual UpdateEquationPtr makeUpdateEquation(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const = 0;
private:
    Pointer<MaterialDescription> mDescription;
    int mID;
};
typedef Pointer<SetupUpdateEquation> SetupUpdateEquationPtr;




























#endif
