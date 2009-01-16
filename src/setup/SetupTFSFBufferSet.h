/*
 *  SetupTFSFBufferSet.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/21/08.
 *  Copyright 2008 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _SETUPTFSFBUFFERSET_
#define _SETUPTFSFBUFFERSET_


#include "SetupConstants.h"

#include "geometry.h"
#include "Pointer.h"

#include <bitset>
#include <set>
#include <string>

// Pre-declarations are preferable to #inclusions when possible...

class SetupGrid;
typedef Pointer<SetupGrid> SetupGridPtr;

class SetupLink;
class SetupTFSFSource;

class SetupTFSFBufferSet
{
public:
	SetupTFSFBufferSet(const SetupLink& inLink, SetupGridPtr auxGrid,
		const Rect3i & activeRegion);
	
	SetupTFSFBufferSet(const SetupTFSFSource& inSource,
		const Rect3i & activeRegion);
		
	~SetupTFSFBufferSet();
	    
    std::string getDescription() const;
	
	TFSFType getTFSFType() const { return mTFSFType; }
	void omitSide(int sideNum);
	bool omits(int sideNum) const { return mOmitSideFlags[sideNum]; }
	
	bool isFileType() const { return mType == kFileType; }
	bool isLinkType() const { return mType == kLinkType; }
	const SetupGridPtr & getAuxGrid() const;
	const std::string & getAuxGridName() const;
    const Rect3i & getAuxRect() const { return mAuxRect; }
    const Rect3i & getTFRect() const { return mTFRect; }
	
	Rect3i getOuterBufferRect(int bufferNumber) const;
	Rect3i getInnerBufferRect(int bufferNumber) const;
	Rect3i getInnerAuxBufferRect(int bufferNumber) const;
	
	Rect3i getYeeBufferRect(int bufferNumber, Vector3i parity) const;
	Rect3i getYeeAuxBufferRect(int bufferNumber, Vector3i fields_pt) const;
	
	int getBufferNeighborIndex(int bufferNumber, const Vector3i & pt) const;
	
	Vector3i getBufferSize(int bufferNumber, const Vector3i & field_pt) const;
	
	TFSFType getBufferType(int bufferNumber, const Vector3i & field_pt) const; 
		
private:
	//	members used by both links and AFP sources
	TFSFBufferType mType;
	std::bitset<6> mOmitSideFlags;
	Rect3i mTFRect;
	TFSFType mTFSFType;
	
	//	members used only by links
	SetupGridPtr mAuxGrid;
	//std::string mAuxGrid;
	Rect3i mAuxRect;
	
	//	members used only by AFP sources
	Map<std::string, std::string> mParams;
	
	friend bool operator<(const SetupTFSFBufferSet & lhs,
		const SetupTFSFBufferSet & rhs);
	
};
typedef Pointer<SetupTFSFBufferSet> SetupTFSFBufferSetPtr;

bool operator<(const SetupTFSFBufferSet & lhs, const SetupTFSFBufferSet & rhs);


#endif
