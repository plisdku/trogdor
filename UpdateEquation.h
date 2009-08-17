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
class CurrentSource;

class UpdateEquation
{
public:
    UpdateEquation();
    virtual ~UpdateEquation();
    
    void setSubstanceName(const std::string & name) { mSubstanceName = name; }
    const std::string & getSubstanceName() const { return mSubstanceName; }
    void setID(int id) { mID = id; }
    int id() const { return mID; }
    
    virtual void setCurrentSource(CurrentSource* source);
    
    virtual void allocateAuxBuffers();
    virtual void calcEPhase(int direction) = 0;
    virtual void calcHPhase(int direction) = 0;
    
    virtual long getNumRunlinesE() const = 0;
    virtual long getNumRunlinesH() const = 0;
    virtual long getNumHalfCellsE() const = 0;
    virtual long getNumHalfCellsH() const = 0;
    
    virtual void writeJ(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeP(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    
    virtual void writeK(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
    virtual void writeM(int direction, std::ostream & binaryStream,
        long startingIndex, const float* startingField, long length) const;
        
    virtual std::string getModelName() const = 0;
    
private:
    int mID;
    std::string mSubstanceName;
};
typedef Pointer<UpdateEquation> UpdateEquationPtr;




#endif
