/*
 *  FileSource.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "FileSource.h"

using namespace std;

FileSource::
FileSource( Fields & inFields,
	const string & inFilename,
	const Vector3d & inPolarization,
	Field inWhichField,
	const Rect3i & inRegion) :
	
	mExtent(inRegion),
	mFile(),
	mPolarization(inPolarization),
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
	
	mFile.open(inFilename.c_str(), ios::binary);
	if (mFile.good())
		LOGF << "Successfully opened binary file " << inFilename << ".\n";
	else
	{
		cerr << "Error: could not open binary file " << inFilename << ".\n";
		exit(1);
	}
}


FileSource::
~FileSource()
{
	//LOG << "Doing nothing.\n";
}


void FileSource::
doSourceH(int timestep, float dx, float dy, float dz, float dt)
{
	if (mWhichField == kMagnetic)
		source(timestep, dx, dy, dz, dt);
}

void FileSource::
doSourceE(int timestep, float dx, float dy, float dz, float dt)
{
	if (mWhichField == kElectric)
		source(timestep, dx, dy, dz, dt);
}


void FileSource::
source(int timestep, float dx, float dy, float dz, float dt)
{	
	float val;
	
	int nx = mFields.get_nx();
	int ny = mFields.get_ny();
	
	if (mFile.good())
		mFile.read((char*)&val, (std::streamsize)sizeof(float));
	/*
	LOG << "Screwing up the source.\n";
	
	val = 0.0;
	if (timestep == 5)
		val = 1.0;
	*/
	for (int i = mExtent.p1[0]; i <= mExtent.p2[0]; i++)
	for (int j = mExtent.p1[1]; j <= mExtent.p2[1]; j++)
	for (int k = mExtent.p1[2]; k <= mExtent.p2[2]; k++)
	{
		mFieldPtr[0][i + nx*j + nx*ny*k] = mPolarization[0]*val;
		mFieldPtr[1][i + nx*j + nx*ny*k] = mPolarization[1]*val;
		mFieldPtr[2][i + nx*j + nx*ny*k] = mPolarization[2]*val;
	}
	
}




