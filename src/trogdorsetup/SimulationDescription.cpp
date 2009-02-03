/*
 *  SimulationDescription.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 1/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "SimulationDescription.h"
#include "XMLParameterFile.h"

#include <iostream>

using namespace std;

SimulationDescription::
SimulationDescription(const XMLParameterFile & file) throw(Exception)
{
	file.load(*this);
}

#pragma mark *** Grid ***

GridDescription::
GridDescription(string name, Vector3i numYeeCells, Vector3i numHalfCells,
	Rect3i calcRegionHalf, Rect3i nonPMLHalf) throw(Exception) :
	mName(name),
	mNumYeeCells(numYeeCells),
	mNumHalfCells(numHalfCells),
	mCalcRegionHalf(calcRegionHalf),
	mNonPMLHalf(nonPMLHalf)
{
	assert(mNumYeeCells == Vector3i(2*mNumHalfCells)); // caller's job...
	
	if (!vec_ge(mCalcRegionHalf.size(), 0))
		throw(Exception("Calc region has some negative dimensions"));
	if (!vec_ge(mNonPMLHalf.size(), 0))
		throw(Exception("Non-PML region has some negative dimensions"));
}

#pragma mark *** InputEH ***

InputEHDescription::
InputEHDescription(string fileName, string inClass, 
	const Map<string, string> & inParameters) throw(Exception) :
	mFileName(fileName),
	mClass(inClass),
	mParams(inParameters)
{
}

#pragma mark *** Output ***

OutputDescription::
OutputDescription(string fileName, string inClass, int inPeriod,
	const Map<string, string> & inParameters) throw(Exception) :
	mFileName(fileName),
	mClass(inClass),
	mPeriod(inPeriod),
	mParams(inParameters)
{
	cerr << "Warning: OutputDescription does not validate inClass.\n";
}

#pragma mark *** Source ***

SourceDescription::
SourceDescription(string formula, string inFileName,
	Vector3f polarization, Rect3i region, string field,
	const Map<string, string> & inParameters) throw(Exception) :
	mFormula(formula),
	mInputFileName(inFileName),
	mPolarization(polarization),
	mRegion(region),
	mField(field),
	mParams(inParameters)
{
	cerr << "Warning: SourceDescription does not validate formula.\n";
	cerr << "Warning: SourceDescription does not validate field.\n";
	if (!vec_ge(mRegion.size(), 0))
		throw(Exception("Source region has some negative dimensions"));
}

#pragma mark *** TFSFSource ***

TFSFSourceDescription::
TFSFSourceDescription(string inClass, Rect3i inTFRect, Vector3f direction,
	string inTFSFType, const Map<string,string> & inParameters)
	throw(Exception) :
	mClass(inClass),
	mTypeStr(inTFSFType),
	mDirection(direction),
	mRegion(inTFRect),
	mParams(inParameters)
{
	cerr << "Warning: TFSFSourceDescription does not validate class.\n";
	cerr << "Warning: TFSFSourceDescription does not validate TFSF type.\n";
	if (!vec_ge(mRegion.size(), 0))
		throw(Exception("TFSFSource TF rect has some negative dimensions"));
}

void TFSFSourceDescription::
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
}

#pragma mark *** Link ***

LinkDescription::
LinkDescription(string typeString, std::string sourceGridName,  Rect3i sourceRect,
	Rect3i destRect) throw(Exception) :
	mTypeString(typeString),
	mSourceGridName(sourceGridName),
	mSourceHalfRect(sourceRect),
	mDestHalfRect(destRect)
{
	cerr << "Warning: LinkDescription does not validate source grid name.\n";
	cerr << "Warning: LinkDescription does not validate link type string.\n";
	if (!vec_ge(mSourceHalfRect.size(), 0))
		throw(Exception("Link source rect has some negative dimensions"));
	if (!vec_ge(mDestHalfRect.size(), 0))
		throw(Exception("Link dest rect has some negative dimensions"));
}

void LinkDescription::
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
}

#pragma mark *** Material ***

MaterialDescription::
MaterialDescription(string name, string inClass,
	const Map<string,string> & inParams) throw(Exception) :
	mName(name),
	mClass(inClass),
	mParams(inParams)
{
	cerr << "Warning: MaterialDescription does not validate class.\n";
}


#pragma mark *** Assembly ***

AssemblyDescription::
AssemblyDescription() throw(Exception)
{
}


