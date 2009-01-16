/*
 *  SetupLink.cpp
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

//#include "SetupConstants.h"
#include "SetupLink.h"
#include "StreamFromString.h"
#include <sstream>

using namespace std;

SetupLink::
SetupLink() :
    mLinkType(kNoType),
	mSourceGridName(),
	mSourceRect(),
	mDestRect(),
	mOmitSideFlags()
{
}

SetupLink::
SetupLink(TFSFType inLinkType,
    const string & inSourceGridName,
    const Rect3i & inSourceRect, const Rect3i & inDestRect,
	bitset<6> inOmitSideFlags) :
    mLinkType(inLinkType),
    mSourceGridName(inSourceGridName),
    mSourceRect(inSourceRect),
    mDestRect(inDestRect),
	mOmitSideFlags(inOmitSideFlags)
{
}

string SetupLink::
getDescription() const
{
    ostringstream str;
    str << "link type = ";
    if (mLinkType == kNoType)
        str << "no type,";
    else if (mLinkType == kTFType)
        str << "TF type,";
    else if (mLinkType == kSFType)
        str << "SF type,";
    else
        str << "unknown (this is bad),";
    
    str << " source name = " << mSourceGridName;
    str << " sourceRect = " << mSourceRect;
    str << " destRect = " << mDestRect;
    
    if (mOmitSideFlags.count())
    {
        str << "omitting sides";
		for (int nn = 0; nn < 6; nn++)
		if (mOmitSideFlags[nn])
			str << " " << nn;
		/*
        for (set<Vector3i>::iterator itr = mOmittedSides.begin();
            itr != mOmittedSides.end(); itr++)
            str << " " << *itr;
		*/
    }
    str << "\n";
    
    return str.str();
}

void SetupLink::
omitSide(Vector3i inOmitSide)
{
	if (inOmitSide == Vector3i(-1,0,0))
		mOmitSideFlags[0] = 1;
	else if (inOmitSide == Vector3i(1,0,0))
		mOmitSideFlags[1] = 1;
	else if (inOmitSide == Vector3i(0,-1,0))
		mOmitSideFlags[2] = 1;
	else if (inOmitSide == Vector3i(0,1,0))
		mOmitSideFlags[3] = 1;
	else if (inOmitSide == Vector3i(0,0,-1))
		mOmitSideFlags[4] = 1;
	else if (inOmitSide == Vector3i(0,0,1))
		mOmitSideFlags[5] = 1;
	
    //mOmittedSides.insert(inOmitSide);
}

/*
bool SetupLink::
omits(const Vector3i & inSide) const
{
    return (mOmittedSides.count(inSide) != 0);
}
*/


TFSFType SetupLink::
getLinkType() const
{
    return mLinkType;
}

const Rect3i & SetupLink::
getSourceRect() const
{
    return mSourceRect;
}

const Rect3i & SetupLink::
getDestRect() const
{
    return mDestRect;
}
/*
const set<Vector3i> & SetupLink::
getOmittedSides() const
{
    return mOmittedSides;
}
*/
const string & SetupLink::
getSourceGridName() const
{
    return mSourceGridName;
}

bool operator<(const SetupLink & lhs, const SetupLink & rhs)
{
    if (lhs.mSourceRect < rhs.mSourceRect)
        return 1;
    else if (lhs.mSourceRect > rhs.mSourceRect)
        return 0;
    else if (lhs.mDestRect < rhs.mDestRect)
        return 1;
    else if (lhs.mDestRect > rhs.mDestRect)
        return 0;
    else if (lhs.mSourceGridName < rhs.mSourceGridName)
        return 1;
    else if (lhs.mSourceGridName > rhs.mSourceGridName)
        return 0;
    else if (lhs.mLinkType < rhs.mLinkType)
        return 1;
    else if (lhs.mLinkType > rhs.mLinkType)
        return 0;
    return 0;
}


