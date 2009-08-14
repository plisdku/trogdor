/*
 *  Material.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _UPDATEEQUATION_
#define _UPDATEEQUATION_

#include "Pointer.h"
#include "geometry.h"

#include <string>
#include <iostream>

class VoxelizedPartition;
class CalculationPartition;
class Paint;

class UpdateEquation
{
public:
    UpdateEquation();
    virtual ~UpdateEquation();
    
    void setSubstanceName(const std::string & name) { mSubstanceName = name; }
    const std::string & getSubstanceName() const { return mSubstanceName; }
    void setID(int id) { mID = id; }
    int id() const { return mID; }
    
    virtual void calcEPhase(int direction) = 0;
    virtual void calcHPhase(int direction) = 0;
    virtual long getNumRunlinesE() const = 0;
    virtual long getNumRunlinesH() const = 0;
    virtual long getNumHalfCellsE() const = 0;
    virtual long getNumHalfCellsH() const = 0;
    virtual std::string getModelName() const = 0;
    
    virtual void writeJ(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeP(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    
    virtual void writeK(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeM(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    
    virtual void allocateAuxBuffers();
private:
    int mID;
    std::string mSubstanceName;
};
typedef Pointer<UpdateEquation> UpdateEquationPtr;

// This is that rare thing—a class in Trogdor which doesn't observe RAII.
// Initializing it just has too darned many parameters.
class SetupUpdateEquation
{
public:
	SetupUpdateEquation();
	virtual ~SetupUpdateEquation();
    
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
typedef Pointer<SetupUpdateEquation> SetupUpdateEquationPtr;



#endif
