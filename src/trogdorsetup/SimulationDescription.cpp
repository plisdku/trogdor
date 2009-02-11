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

// This is included for the function that converts hex colors to RGB.
#include <Magick++.h>

static Vector3i sConvertColor(const Magick::Color & inColor);

using namespace std;

SimulationDescription::
SimulationDescription(const XMLParameterFile & file) throw(Exception) :
	m_dt(0.0f),
	m_dxyz(Vector3f(0.0f, 0.0f, 0.0f)),
	mNumTimesteps(0)
{
	file.load(*this);
	
	// This error checking is redundant provided that the loading mechanism
	// remembers to check the timestep.  But it's the constructor's job to
	// make sure it returns a *valid* data structure, so let's double-check
	// these silly conditions here.
	if (m_dt <= 0)
		throw(Exception("Nonpositive timestep"));
	if (!vec_gt(m_dxyz, 0.0f))
		throw(Exception("Nonpositive cell dimensions"));
	if (mNumTimesteps < 0)
		throw(Exception("Negative number of iterations"));
}

void SimulationDescription::
setDiscretization(Vector3f dxyz, float dt)
{
	if (!vec_gt(dxyz, 0.0f))
		throw(Exception("Nonpositive cell dimensions"));
	if (dt <= 0)
		throw(Exception("Nonpositive timestep"));
	
	m_dxyz = dxyz;
	m_dt = dt;
}

void SimulationDescription::
setDuration(int numT)
{
	if (numT < 0)
		throw(Exception("Number of timesteps must be nonnegative"));
	mNumTimesteps = numT;
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
	assert(Vector3i(2*mNumYeeCells) == mNumHalfCells); // caller's job...
	
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
	if (inOmitSide != dominantComponent(inOmitSide) ||
		norm1(inOmitSide) != 1)
		throw(Exception("Omitted sides must be axis-oriented unit vectors"));
	mOmittedSides.insert(inOmitSide);
	/*
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
	*/
}

#pragma mark *** Link ***

LinkDescription::
LinkDescription(string typeString, string sourceGridName,  Rect3i sourceRect,
	Rect3i destRect)
	throw(Exception) :
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
	if (inOmitSide != dominantComponent(inOmitSide) ||
		norm1(inOmitSide) != 1)
		throw(Exception("Omitted sides must be axis-oriented unit vectors"));
	mOmittedSides.insert(inOmitSide);
	/*
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
	*/
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
AssemblyDescription(const vector<AssemblyDescription::InstructionPtr> & recipe)
	throw(Exception) :
	mInstructions(recipe)
{
	
}

AssemblyDescription::ColorKey::
ColorKey(string hexColor, string materialName, FillStyle style)
	throw(Exception) :
	mMaterial(materialName),
	mFillStyle(style)
{
	try {
		mColor = sConvertColor(Magick::Color(hexColor));
	}
	catch (const Magick::Exception & ex) {
		throw(Exception(ex.what()));
	}
}


AssemblyDescription::ColorKey::
ColorKey(Vector3i rgbColor, string materialName, FillStyle style)
	throw(Exception) :
	mColor(rgbColor),
	mMaterial(materialName),
	mFillStyle(style)
{
	assert(mFillStyle != kHalfCellStyle); // KeyImage uses one pixel per cell
}



AssemblyDescription::Instruction::
Instruction(AssemblyDescription::InstructionType inType) :
	mType(inType)
{
}

AssemblyDescription::Block::
Block(Rect3i halfCellRect, string material) throw(Exception) :
	AssemblyDescription::Instruction(kBlockType),
	mFillRect(halfCellRect),
	mMaterial(material),
	mStyle(kHalfCellStyle)
{
	cerr << "Warning: minimal validation done for Block().\n";
	if (!vec_ge(mFillRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative."));
}

AssemblyDescription::Block::
Block(Rect3i yeeCellRect, FillStyle style, string material) throw(Exception) :
	AssemblyDescription::Instruction(kBlockType),
	mFillRect(yeeCellRect),
	mMaterial(material),
	mStyle(style)
{
	cerr << "Warning: minimal validation done for Block().\n";
	assert(mStyle != kHalfCellStyle);
	
	if (!vec_ge(mFillRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative."));
}

const Rect3i & AssemblyDescription::Block::
getYeeRect() const
{
	assert(mStyle != kHalfCellStyle);
	return mFillRect;
}

const Rect3i & AssemblyDescription::Block::
getHalfRect() const
{
	assert(mStyle == kHalfCellStyle);
	return mFillRect;
}

AssemblyDescription::KeyImage::
KeyImage(Rect3i yeeCellRect, string imageFileName, Vector3i rowDirection,
	Vector3i colDirection, vector<ColorKey> keys) throw(Exception) :
	AssemblyDescription::Instruction(kKeyImageType),
	mYeeRect(yeeCellRect),
	mRow(rowDirection),
	mCol(colDirection),
	mImageFileName(imageFileName),
	mKeys(keys)
{
	cerr << "Warning: minimal validation done for KeyImage().\n";
	
	if (!vec_ge(mYeeRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative."));
	
	if (mRow != dominantComponent(mRow) ||
		norm1(mRow) != 1)
		throw(Exception("Image row must be axis-oriented unit vector"));
	if (mCol != dominantComponent(mCol) ||
		norm1(mCol) != 1)
		throw(Exception("Image column must be axis-oriented unit vector"));
	if (dot(mRow, mCol) != 0)
		throw(Exception("Image row and column must be orthogonal"));
}

AssemblyDescription::HeightMap::
HeightMap(Rect3i yeeCellRect, FillStyle style, string material,
	string imageFileName, Vector3i rowDirection, Vector3i colDirection,
	Vector3i upDirection) throw(Exception) :
	AssemblyDescription::Instruction(kHeightMapType),
	mYeeRect(yeeCellRect),
	mStyle(style),
	mMaterial(material),
	mImageFileName(imageFileName),
	mRow(rowDirection),
	mCol(colDirection),
	mUp(upDirection)
{
	cerr << "Warning: minimal validation done for HeightMap().\n";
	
	assert(style != kHalfCellStyle); // HeightMap uses one pixel per cell
	
	if (!vec_ge(mYeeRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative."));
	
	if (mRow != dominantComponent(mRow) ||
		norm1(mRow) != 1)
		throw(Exception("Image row must be axis-oriented unit vector"));
	if (mCol != dominantComponent(mCol) ||
		norm1(mCol) != 1)
		throw(Exception("Image column must be axis-oriented unit vector"));
	if (mUp != dominantComponent(mUp) ||
		norm1(mUp) != 1)
		throw(Exception("Image up vector must be axis-oriented unit vector"));
	if (abs(dot(mUp, cross(mRow, mCol))) != 1)
		throw(Exception("Image row, column and up vector must be orthogonal"));
}

AssemblyDescription::Ellipsoid::
Ellipsoid(Rect3i halfCellRect, string material) throw(Exception) :
	AssemblyDescription::Instruction(kEllipsoidType),
	mFillRect(halfCellRect),
	mStyle(kHalfCellStyle),
	mMaterial(material)
{
	cerr << "Warning: minimal validation done for Ellipsoid().\n";
	if (!vec_ge(mFillRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative"));
}

AssemblyDescription::Ellipsoid::
Ellipsoid(Rect3i yeeCellRect, FillStyle style, string material)
	throw(Exception) :
	AssemblyDescription::Instruction(kEllipsoidType),
	mFillRect(yeeCellRect),
	mStyle(style),
	mMaterial(material)
{
	cerr << "Warning: minimal validation done for Ellipsoid().\n";
	assert(mStyle != kHalfCellStyle);
	if (!vec_ge(mFillRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative"));
}

const Rect3i & AssemblyDescription::Ellipsoid::
getYeeRect() const
{
	assert(mStyle != kHalfCellStyle);
	return mFillRect;
}

const Rect3i & AssemblyDescription::Ellipsoid::
getHalfRect() const
{
	assert(mStyle == kHalfCellStyle);
	return mFillRect;
}

AssemblyDescription::CopyFrom::
CopyFrom(Rect3i halfCellSourceRegion, Rect3i halfCellDestRegion,
	string gridName) throw(Exception) :
	AssemblyDescription::Instruction(kCopyFromType),
	mSourceRect(halfCellSourceRegion),
	mDestRect(halfCellDestRegion),
	mGridName(gridName)
{
	cerr << "Warning: minimal validation done for CopyFrom().\n";
	if (!vec_ge(mSourceRect.size(), 0))
		throw(Exception("Some source rect dimensions are negative"));
	if (!vec_ge(mDestRect.size(), 0))
		throw(Exception("Some dest rect dimensions are negative"));
}


#pragma mark *** Static methods ***

static Vector3i sConvertColor(const Magick::Color & inColor)
{
    Vector3i outColor;
    
#if (MagickLibVersion == 0x618)
    outColor[0] = Magick::ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = Magick::ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = Magick::ScaleQuantumToChar(inColor.greenQuantum());
#else
    outColor[0] = MagickLib::ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = MagickLib::ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = MagickLib::ScaleQuantumToChar(inColor.greenQuantum());
#endif

    return outColor;
}



