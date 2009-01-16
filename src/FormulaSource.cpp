/*
 *  FormulaSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/3/08.
 *  Copyright 2008 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "FormulaSource.h"
#include "Log.h"
#include "StreamFromString.h"

using namespace std;
using namespace calc_load;

FormulaSource::FormulaSource(Fields & inFields, const string & inFormula,
	const Vector3d & inPolarization, Field inWhichField,
	const Rect3i & inRegion,
	const Map<string, string> & inParams) :
	mExtent(inRegion),
	mFormula(inFormula),
	mPolarization(inPolarization),
	mCalculator(),
	mFields(inFields),
	mWhichField(inWhichField)
{
	if (mWhichField == kElectric)
	{
		mFieldPtr[0] = inFields.getEx();
		mFieldPtr[1] = inFields.getEy();
		mFieldPtr[2] = inFields.getEz();
	}
	else
	{
		mFieldPtr[0] = inFields.getHx();
		mFieldPtr[1] = inFields.getHy();
		mFieldPtr[2] = inFields.getHz();
	}
	
	Map<string, string>::const_iterator itr;
	for (itr = inParams.begin(); itr != inParams.end(); itr++)
	{
		const string & nom = itr->first;
		const string & val_string = itr->second;
		
		if (nom != "polarization" && nom != "filename" && nom != "formula")
		{
			float val;
			val_string >> val;
			
			mCalculator.set(nom, val);
			
			LOGF << "Setting " << nom << " to " << mCalculator.get(nom) << "\n";
		}
	}
	
	// The calculator will eventually update "n" and "t" to be the current
	// timestep and current time; we can set them here to test the formula.
	mCalculator.set("n", 0);
	mCalculator.set("t", 0);
	
	LOGF << "Formula is " << mFormula << endl;
	
	bool err = mCalculator.parse(mFormula);
	if (err)
	{
		LOG << "Error found.";
		cerr << "Calculator cannot parse\n"
			<< mFormula << "\n"
			<< "Error message:\n";
		mCalculator.report_error(cerr);
		cerr << "Quitting.\n";
		assert(!"Assert death.");
		exit(1);
	}
}


FormulaSource::~FormulaSource()
{
	//LOG << "Doing nothing.\n";
}


void FormulaSource::
doSourceH(int timestep, float dx, float dy, float dz, float dt)
{
	if (mWhichField == kMagnetic)
		source(timestep, dx, dy, dz, dt);
}

void FormulaSource::
doSourceE(int timestep, float dx, float dy, float dz, float dt)
{
	if (mWhichField == kElectric)
		source(timestep, dx, dy, dz, dt);
}


void FormulaSource::
source(int timestep, float dx, float dy, float dz, float dt)
{
	mCalculator.set("n", timestep);
	mCalculator.set("t", dt*timestep);
	
	float val;
	mCalculator.parse(mFormula);
	
	val = mCalculator.get_value();
	
	//LOG << val << "\n";
	/*
	LOGMORE << "A = " << mCalculator.get("A") << "\n";
	LOGMORE << "sigma = " << mCalculator.get("sigma") << "\n";
	LOGMORE << "t0 = " << mCalculator.get("t0") << "\n";
	LOGMORE << "t = " << mCalculator.get("t") << "\n";
	LOGMORE << "formula = " << mFormula << "\n";
	*/
	
	int nx = mFields.get_nx();
	int ny = mFields.get_ny();
	//int nz = mFields.get_nz();
	
	//LOG << hex << mFieldPtr << dec << endl;
	
	//val = mAmplitude * exp(-(t-mCenter)*(t-mCenter)/(mSigma*mSigma));
	for (int i = mExtent.p1[0]; i <= mExtent.p2[0]; i++)
	for (int j = mExtent.p1[1]; j <= mExtent.p2[1]; j++)
	for (int k = mExtent.p1[2]; k <= mExtent.p2[2]; k++)
	{
		mFieldPtr[0][i + nx*j + nx*ny*k] = mPolarization[0]*val;
		mFieldPtr[1][i + nx*j + nx*ny*k] = mPolarization[1]*val;
		mFieldPtr[2][i + nx*j + nx*ny*k] = mPolarization[2]*val;
	}
	
}








