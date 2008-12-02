/*
 *  SetupSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/20/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SetupSource.h"


using namespace std;

SetupSource::
SetupSource(const string & inFormula,
	const string & inFileName,
	Field inField,
	Vector3d polarization,
	Rect3i inRegion,
	const Map<string, string> & inParams) :
	mFormula(inFormula),
	mInputFileName(inFileName),
	mPolarization(polarization),
	mRegion(inRegion),
	mField(inField),
    mParams(inParams)
{
    LOGF<< "Constructor.\n";
	LOGF << "Don't forget to eventually take the params out of this call.\n";
}

SetupSource::
~SetupSource()
{
    LOGF<< "Destructor.\n";
}



