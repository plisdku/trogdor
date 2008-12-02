/*
 *  SetupSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/20/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SETUPSOURCE_
#define _SETUPSOURCE_

#include "SetupConstants.h"

#include "geometry.h"
#include "Pointer.h"

#include <string>

class SetupSource
{
public:
    SetupSource(const std::string & inFormula,
		const std::string & inFileName, Field inField,
		Vector3d polarization, Rect3i inRegion,
        const Map<std::string, std::string> & inParams);
    virtual ~SetupSource();
    
	const Rect3i & getRegion() const { return mRegion; }
	const std::string & getFormula() const { return mFormula; }
	const std::string & getFile() const { return mInputFileName; }
	const Vector3d & getPolarization() const { return mPolarization; }
	Field getField() const { return mField; }
    
    const Map<std::string, std::string> & getParameters() const
        { return mParams; }
private:
	std::string mFormula;
	std::string mInputFileName;
	Vector3d mPolarization;
	Rect3i mRegion;
	Field mField;
    Map<std::string, std::string> mParams;
    
};

typedef Pointer<SetupSource> SetupSourcePtr;

#endif
