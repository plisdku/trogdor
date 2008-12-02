/*
 *  SetupTFSFSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/23/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SETUPTFSFSOURCE_
#define _SETUPTFSFSOURCE_

#include "Pointer.h"
#include "SetupConstants.h"
#include "geometry.h"
#include <string>
#include <set>
#include <bitset>

class SetupTFSFSource
{
public:
	SetupTFSFSource(const SetupTFSFSource & copyMe);
	
	SetupTFSFSource(const std::string & inClass,
		const Rect3i & inTFRect, const Vector3d & inDirection,
		const Map<std::string, std::string> & inParams,	
		TFSFType inType = kTFType);
	virtual ~SetupTFSFSource();
	
	const std::string & getClass() const { return mClass; }
	const Map<std::string, std::string> & getParameters() const
		{ return mParams; }
	
	TFSFType getType() const { return mType; }
    void omitSide(Vector3i inOmitSide);
    //bool omits(const Vector3i & inSide) const;
	bool omits(int sideNum) const { return mOmitSideFlags[sideNum]; }
    //const std::set<Vector3i> & getOmittedSides() const { return mOmittedSides; }
	const std::bitset<6> getOmitSideFlags() const { return mOmitSideFlags; }
	
	std::string getFormula() const;
	std::string getInputFile() const;
	
	const Rect3i & getTFRect() const { return mTFRect; }
	
	char getField() const;
	Vector3d getPolarization() const;
	
	Vector3b getSymmetries() const;
	Vector3i getAxialDirection () const;
	Vector3d getDirection () const { return mDirection; }
	
	void cacheGridSymmetries(const vmlib::SMat<3,bool> & symm)
		{ mCachedGridSymmetries = symm; }
	
	const vmlib::SMat<3,bool> & getCachedGridSymmetries() const
		{ return mCachedGridSymmetries; }
	
private:
	std::string mClass;
	Map<std::string, std::string> mParams;
	Rect3i mTFRect;
	Vector3d mDirection;
	
	TFSFType mType;
	
    //std::set<Vector3i> mOmittedSides;
	std::bitset<6> mOmitSideFlags;
	
	// cache
	vmlib::SMat<3,bool> mCachedGridSymmetries;
};

typedef Pointer<SetupTFSFSource> SetupTFSFSourcePtr;





#endif

