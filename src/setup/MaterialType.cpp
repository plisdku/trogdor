/*
 *  MaterialType.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/19/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "SetupTFSFBufferSet.h"
#include "MaterialType.h"

#include <sstream>
#include "StreamFromString.h"


using namespace std;

MaterialType::
MaterialType() :
    mName("Default"),
    mDirection(0,0,0),
	mBuffer(0L),
    mType(kNoType),
	mTFSFBufferIndices(6, -1),
    mIsPML(0)
{
}


MaterialType::
MaterialType(std::string inName) :
    mName(inName),
    mDirection(0,0,0),
	mBuffer(0L),
    mType(kNoType),
	mTFSFBufferIndices(6, -1),
    mIsPML(0)
{
}

MaterialType::
MaterialType(const MaterialType & parentType, Vector3i inPMLDirection) :
    mName(parentType.mName),
    mDirection(inPMLDirection),
	mBuffer(0L),
    mType(kNoType),
	mTFSFBufferIndices(6, -1),
    mIsPML(1)
{
}


MaterialType::
MaterialType(const MaterialType & parentType, SetupTFSFBufferSetPtr & inBuffer,
    TFSFType inTFSF) :
    mName(parentType.mName),
    mDirection(0,0,0),
    mBuffer(inBuffer),
    mType(inTFSF),
	mTFSFBufferIndices(6,-1),
    mIsPML(0)
{
}

MaterialType::
~MaterialType()
{
}


string MaterialType::
getDescription() const
{
    ostringstream str;
    
    if (isUnmodified())
    {
        str << mName << " unmodified\n";
    }
    else if (isPML())
    {
        str << mName << " PML, direction " << mDirection << "\n";
    }
    else if (isTFSF())
    {
        str << mName << " TF/SF";
        if (mBuffer != 0L)
            str << ", buffer " << mBuffer->getDescription();
        else
            str << " with NO LINK (this is bad)\n";
		
		str << " with buffers";
		
		for (int nn = 0; nn < 6; nn++)
			str << " " << mTFSFBufferIndices[nn];
		
    }
    else
    {
        str << mName << ", type is not PML or TFSF or unmodified ";
        str << " (this is bad)\n";
    }
    
    return str.str();
}
    

string MaterialType::
getAuxName() const
{
    if (mType != kNoType)
        return mBuffer->getAuxGridName();
    return "";
}

const SetupTFSFBufferSetPtr & MaterialType::
getBuffer() const
{
	return mBuffer;
}

void MaterialType::
setBuffer(const SetupTFSFBufferSetPtr & ptr)
{
	mBuffer = ptr;
}
                 

bool operator<(const MaterialType & lhs, const MaterialType & rhs)
{
    assert(!(lhs.isPML() && lhs.isTFSF()));
    assert(!(rhs.isPML() && rhs.isTFSF()));
    
    if (lhs.mName < rhs.mName)
        return 1;
    else if (lhs.mName > rhs.mName)
        return 0;
    else if (lhs.mIsPML < rhs.mIsPML)
        return 1;
    else if (lhs.mIsPML > rhs.mIsPML)
        return 0;
    else if (lhs.mIsPML && rhs.mIsPML)      //  if both are PML...
    {
        if (lhs.mDirection < rhs.mDirection)
            return 1;
        else if (lhs.mDirection > rhs.mDirection)
            return 0;
    }
    else if (lhs.mType < rhs.mType)         //  vs. if both are NOT PML...
        return 1;
    else if (lhs.mType > rhs.mType)
        return 0;
	else if (lhs.mTFSFBufferIndices < rhs.mTFSFBufferIndices)
		return 1;
	else if (lhs.mTFSFBufferIndices > rhs.mTFSFBufferIndices)
		return 0;
    else if (lhs.mBuffer < rhs.mBuffer) // we know by now that we're TF/SF
        return 1;
	
    return 0;
}

