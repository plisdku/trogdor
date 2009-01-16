/*
 *  MaterialType.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/19/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#ifndef _MATERIALTYPE_
#define _MATERIALTYPE_

#include "SetupConstants.h"

#include <string>
#include <vector>

#include "geometry.h"
#include "Pointer.h"

class SetupTFSFBufferSet;
typedef Pointer<SetupTFSFBufferSet> SetupTFSFBufferSetPtr;

class MaterialType
{
public:
    MaterialType();
    MaterialType(std::string inName);
    MaterialType(const MaterialType & parentType, Vector3i inPMLDirection);
    MaterialType(const MaterialType & parentType,
		SetupTFSFBufferSetPtr & inBuffer, TFSFType inTFSF);
	~MaterialType();
    
    std::string getDescription() const;
    
    const std::string & getName() const { return mName; }
    std::string getAuxName() const;
    const Vector3i & getDirection() const { return mDirection; }
    const SetupTFSFBufferSetPtr & getBuffer() const;
	void setBuffer(const SetupTFSFBufferSetPtr & ptr);
    TFSFType getLinkType() const { return mType; }
	TFSFType & linkType() { return mType; }
    bool isPML() const { return mIsPML; }
    bool isTFSF() const { return mType != kNoType; }
    bool isUnmodified() const
		{ return (mIsPML == 0 && mType == kNoType); }
	
	void setBufferIndex(int side, int bufferNumber)
		{ mTFSFBufferIndices[side] = bufferNumber; }
	int getBufferIndex(int side) const { return mTFSFBufferIndices[side]; }
	const std::vector<int> & getTFSFBufferIndices() const
		{ return mTFSFBufferIndices; }
    
    friend bool operator<(const MaterialType & lhs, const MaterialType & rhs);
private:
    std::string mName;
    Vector3i mDirection;
    SetupTFSFBufferSetPtr mBuffer;
    TFSFType mType;
	std::vector<int> mTFSFBufferIndices;
    bool mIsPML;
};
typedef Pointer<MaterialType> MaterialTypePtr;

bool operator<(const MaterialType & lhs, const MaterialType & rhs);


#endif
