/*
 *  SetupTFSFSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/23/08.
 *  Copyright 2008 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "SetupTFSFSource.h"

#include "StreamFromString.h"

using namespace std;

SetupTFSFSource::
SetupTFSFSource(const SetupTFSFSource & copyMe) :
	mClass(copyMe.mClass),
	mParams(copyMe.mParams),
	mTFRect(copyMe.mTFRect),
	mDirection(copyMe.mDirection),
	mType(copyMe.mType),
	mOmitSideFlags(copyMe.mOmitSideFlags),
	mCachedGridSymmetries()
{
	LOGF << "Constructor.\n";
}
	
SetupTFSFSource::
SetupTFSFSource(const string & inClass,
	const Rect3i & inTFRect,
	const Vector3d & inDirection,
	const Map<string, string> & inParams,
	TFSFType inType):
	mClass(inClass),
	mParams(inParams),
	mTFRect(inTFRect),
	mDirection(inDirection),
	mType(inType),
	mOmitSideFlags(),
	mCachedGridSymmetries()
{
	LOGF << "Constructor.\n";
}

SetupTFSFSource::
~SetupTFSFSource()
{
	LOGF << "Destructor.\n";
}

void SetupTFSFSource::
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
bool SetupTFSFSource::
omits(const Vector3i & inSide) const
{
    return (mOmittedSides.count(inSide) != 0);
}
*/

string SetupTFSFSource::
getFormula() const
{
	if (mParams.count("formula") != 0)
	{
		return mParams["formula"];
	}
	else
		return "";
}

string SetupTFSFSource::
getInputFile() const
{
	if (mParams.count("filename") != 0)
	{
		return mParams["filename"];
	}
	else
		return "";
}

Vector3b SetupTFSFSource::
getSymmetries() const
{
	Vector3b symmetries(0,0,0);
	
	//LOG << mDirection << "\n";
	if (mClass == "PlaneWave")
	{
		for (int nn = 0; nn < 3; nn++)
			if (mDirection[nn] == 0)
				symmetries[nn] = 1;
	}
	else if (mClass == "Waveguide")
	{
		
	}
	else
	{
		LOG << "Unsupported source class " << mClass << ".\n";
	}
	
	//LOG << "mDirection " << mDirection << " symmetries " << symmetries << "\n";
	return symmetries;
}

Vector3i SetupTFSFSource::
getAxialDirection () const
{
	Vector3i dir(0,0,0);
	
	if (mClass == "PlaneWave")
	{
		if (!(mDirection == dominantComponent(mDirection)))
		{
			cerr << "Plane wave is not axially oriented!\n";
			LOG << "Quitting from vector " << mDirection << endl;
			assert(0);
		}
		for (int nn = 0; nn < 3; nn++)
		{
			if (mDirection[nn] > 0)
				dir[nn] = 1;
			else if (mDirection[nn] < 0)
				dir[nn] = -1;
		}
	}
	else
	{
		LOG << "Unsupported source class " << mClass << ".\n";
	}
	
	return dir;
}


char SetupTFSFSource::
getField() const
{
	if (mClass == "PlaneWave")
	{
		string fieldString;
		if (mParams.count("field"))
			fieldString = mParams["field"];
		else
			fieldString = "e";
		
		if (fieldString == "e" || fieldString == "electric")
			return 'e';
		else if (fieldString == "h" || fieldString == "magnetic")
			return 'h';
		else
		{
			LOG << "Unsupported source field " << fieldString << ".\n";
			assert(!"Halp.");
		}
	}
	else
	{
		LOG << "Can't call this except for plane waves.\n";
		assert(!"Halp.");
	}
	return 'x';
}

Vector3d SetupTFSFSource::
getPolarization() const
{
	Vector3d polVec;
	if (mClass == "PlaneWave")
	{
		string polString = mParams["polarization"];
		polString >> polVec;
	}
	else
	{
		LOG << "Can't call this except for plane waves.\n";
		assert(!"Halp.");
	}
	return polVec;
}



