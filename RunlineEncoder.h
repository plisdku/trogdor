/*
 *  RunlineEncoder.h
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

class VoxelizedPartition;
class CalculationPartition;
class Paint;


// This is that rare thing—a class in Trogdor which doesn't observe RAII.
// Initializing it just has too darned many parameters.
class RunlineEncoder
{
public:
	RunlineEncoder() {}
	virtual ~RunlineEncoder() {}
    
    void setID(int id) { mID = id; }
    int id() const { return mID; }
	
	// Runline handling
	virtual void startRunline(const VoxelizedPartition & vp,
		const Vector3i & startPos) = 0;
	virtual bool canContinueRunline(const VoxelizedPartition & vp,
		const Vector3i & oldPos,
		const Vector3i & newPos, Paint* newPaint,
        int runlineDirection) const = 0;
	virtual void continueRunline(const Vector3i & newPos) = 0;
	virtual void endRunline() = 0;
	
	virtual void printRunlines(std::ostream & out) const = 0;
    
    // Setting up the runtime materials
    virtual UpdateEquationPtr makeUpdateEquation(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const = 0;
private:
    int mID;
};
typedef Pointer<RunlineEncoder> RunlineEncoderPtr;




























#endif
