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

#include "YeeUtilities.h"
using namespace YeeUtilities;

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
setAllPointers()
{
	unsigned int nGrid, nMaterial;
	
	// 1.  Make maps of names to pointers
	Map<string, GridDescPtr> gridMap;
	Map<string, MaterialDescPtr> materialMap;
	
	for (nGrid = 0; nGrid < mGrids.size(); nGrid++)
		gridMap[mGrids[nGrid]->getName()] = mGrids[nGrid];
	for (nMaterial = 0; nMaterial < mMaterials.size(); nMaterial++)
		materialMap[mMaterials[nMaterial]->getName()] = mMaterials[nMaterial];
	
	
	// 2.  Point Huygens surfaces to appropriate grids
	for (nGrid = 0; nGrid < mGrids.size(); nGrid++)
	{
		mGrids[nGrid]->setPointers(materialMap, gridMap);
		/*
		std::vector<HuygensSurfaceDescPtr> & huygens = mGrids[nGrid]->
			getHuygensSurfaces();
		
		for (nHuygens = 0; nHuygens < huygens.size(); nHuygens++)
		if (huygens[nHuygens]->getType() == kLink)
			huygens[nHuygens]->setLinkSourceGrid(gridMap[huygens[nHuygens]->
				getLinkSourceGridName()]);
		*/
	}
	/*
	// 3.  Point various instructions to appropriate materials
	for (nGrid = 0; nGrid < mGrids.size(); nGrid++)
	{
		AssemblyDescPtr assem = mGrids[nGrid]->getAssembly();
		assert(assem != 0L);
		assem->setPointers(materialMap, gridMap);
	}
	*/
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

void GridDescription::
setPointers(const Map<string, MaterialDescPtr> & materialMap,
	const Map<string, GridDescPtr> & gridMap)
{
	for (unsigned int nn = 0; nn < mHuygensSurfaces.size(); nn++)
	if (mHuygensSurfaces[nn]->getType() == kLink)
		mHuygensSurfaces[nn]->setPointers(gridMap);
	
	if (mAssembly != 0L)
		mAssembly->setPointers(materialMap, gridMap);
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

/*
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
}
*/
#pragma mark *** HuygensSurface ***

// constructor for LINKS
HuygensSurfaceDescription::
HuygensSurfaceDescription(string typeString, string sourceGridName, 
	Rect3i sourceHalfRect, Rect3i destHalfRect,
	const std::set<Vector3i> & omittedSides)
	throw(Exception) :
	mType(kLink),
	mDestHalfRect(destHalfRect),
	mOmittedSides(omittedSides),
	mBuffers(),
	mLinkSourceHalfRect(sourceHalfRect),
	mLinkSourceGridName(sourceGridName),
	mTFSFSourceClass(),
	mTFSFSourceParams(),
	mTFSFSourceSymmetries()
{
	if (!vec_ge(mDestHalfRect.size(), 0))
		throw(Exception("Link dest rect has some negative dimensions"));
	if (!vec_ge(mLinkSourceHalfRect.size(), 0))
		throw(Exception("Link source rect has some negative dimensions"));
	
	for (set<Vector3i>::iterator ii = mOmittedSides.begin();
		ii != mOmittedSides.end(); ii++)
	{
		if (*ii != dominantComponent(*ii) ||
			norm1(*ii) != 1)
			throw(Exception("Omitted sides must be axis-oriented unit "
				"vectors"));
	}
	
	if (typeString == "TF")
		initTFSFBuffers(1.0);
	else if (typeString == "SF")
		initTFSFBuffers(-1.0);
	else
		throw(Exception("Link type must be 'TF' or 'SF'"));
}

// constructor for TFSFSOURCES
HuygensSurfaceDescription::
HuygensSurfaceDescription(string inClass, Rect3i inTFRect, Vector3i symmetries,
	string inTFSFType, const Map<string,string> & inParameters,
	const std::set<Vector3i> & omittedSides)
	throw(Exception) :
	mType(kTFSFSource),
	mDestHalfRect(inTFRect),
	mOmittedSides(omittedSides),
	mBuffers(),
	mLinkSourceHalfRect(),
	mLinkSourceGridName(),
	mTFSFSourceClass(inClass),
	mTFSFSourceParams(inParameters),
	mTFSFSourceSymmetries(symmetries)
{
	cerr << "Warning: HuygensSurfaceDescription does not validate class.\n";
	if (!vec_ge(mDestHalfRect.size(), 0))
		throw(Exception("TFSFSource TF rect has some negative dimensions"));
	for (unsigned int nn = 0; nn < 3; nn++)
	if (mTFSFSourceSymmetries[nn] != 1 && mTFSFSourceSymmetries[nn] != 0)
		throw(Exception("TFSFSource symmetry vector must have "
			"1s and 0s only"));
	
	for (set<Vector3i>::iterator ii = mOmittedSides.begin();
		ii != mOmittedSides.end(); ii++)
	{
		if (*ii != dominantComponent(*ii) ||
			norm1(*ii) != 1)
			throw(Exception("Omitted sides must be axis-oriented unit "
				"vectors"));
	}
	if (inTFSFType == "TF")
		initTFSFBuffers(1.0);
	else if (inTFSFType == "SF")
		initTFSFBuffers(-1.0);
	else
		throw(Exception("Link type must be 'TF' or 'SF'"));
}

HuygensSurfaceDescription::
HuygensSurfaceDescription(string requestyArgs) throw(Exception)
{
	throw(Exception("Data request constructor not implemented yet."));
}

HuygensSurfaceDescription* HuygensSurfaceDescription::
newLink(string typeString, string sourceGridName, Rect3i sourceHalfRect,
	Rect3i destHalfRect, const set<Vector3i> & omittedSides)
	throw(Exception)
{
	return new HuygensSurfaceDescription(typeString, sourceGridName,
		sourceHalfRect, destHalfRect, omittedSides);
}
HuygensSurfaceDescription* HuygensSurfaceDescription::
newTFSFSource(string inClass, Rect3i inHalfRect, Vector3i symmetries,
	string inTFSFType, const Map<string, string> & inParameters,
	const set<Vector3i> & omittedSides) throw(Exception)
{
	return new HuygensSurfaceDescription(inClass, inHalfRect, symmetries,
		inTFSFType, inParameters, omittedSides);
}

HuygensSurfaceDescription* HuygensSurfaceDescription::
newDataRequest() throw(Exception)
{
	return new HuygensSurfaceDescription("this is a data request");
}

/*
void HuygensSurfaceDescription::
omitSide(Vector3i inOmitSide)
{
	if (inOmitSide != dominantComponent(inOmitSide) ||
		norm1(inOmitSide) != 1)
		throw(Exception("Omitted sides must be axis-oriented unit vectors"));
	mOmittedSides.insert(inOmitSide);
}
*/

void HuygensSurfaceDescription::
initTFSFBuffers(float srcFactor)
{
	// The function works as:
	//		for each side of each link
	//			for each field (Ex, Ey, Hz, Ez, Hy, Hx)
	//				add a TF or SF buffer appropriately
	
	// For all sides not "omitted" explicitly...
	for (unsigned int nDir = 0; nDir < 6; nDir++)
	if (mOmittedSides.count(cardinalDirection(nDir)) == 0)
	{
		Rect3i outerTotalField = edgeOfRect(mDestHalfRect, nDir);
		
		for (unsigned int fieldNum = 0; fieldNum < 6; fieldNum++) // on E, H
		if ( outerTotalField.encloses( outerTotalField.p1 +
			 halfCellFieldOffset(fieldNum) ) )
		{
			// total-field buffer
			Rect3i destYeeRect = rectHalfToYee(outerTotalField,
				halfCellFieldIndex(fieldNum));
			NeighborBufferDescPtr nb(new NeighborBufferDescription(
				destYeeRect, cardinalDirection(nDir),
				halfCellFieldOffset(fieldNum), 1.0, 1.0*srcFactor));
			mBuffers[halfCellFieldOffset(fieldNum)].push_back(nb);
		}
		else
		{
			// scattered-field buffer
			Rect3i destYeeRect = rectHalfToYee(outerTotalField +
				cardinalDirection(nDir), halfCellFieldIndex(fieldNum));
			NeighborBufferDescPtr nb(new NeighborBufferDescription(
				destYeeRect, -1*cardinalDirection(nDir),
				halfCellFieldOffset(fieldNum), 1.0, -1.0*srcFactor));
			mBuffers[halfCellFieldOffset(fieldNum)].push_back(nb);
		}
	}
}

void HuygensSurfaceDescription::
initFloquetBuffers()
{
	LOG << "Not doing anything for Floquet buffers.\n";
}

void HuygensSurfaceDescription::
setPointers(const Map<string, GridDescPtr> & gridMap)
{
	assert(mType == kLink);
	mLinkSourceGrid = gridMap[mLinkSourceGridName];
}

#pragma mark *** NeighborBuffer ***

NeighborBufferDescription::
NeighborBufferDescription(const Rect3i & applicationYeeRect,
	Vector3i neighborDirection, Vector3i yeeOctant, float applicationFactor,
	float addendFactor) :
	mDestYeeRect(applicationYeeRect),
	mNeighborDirection(neighborDirection),
	mFieldOffset(yeeOctant),
	mDestFactor(applicationFactor),
	mSrcFactor(addendFactor)
{
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
AssemblyDescription(const vector<InstructionPtr> & recipe)
	throw(Exception) :
	mInstructions(recipe)
{
	
}

void AssemblyDescription::
setPointers(const Map<string, MaterialDescPtr> & materialMap,
	const Map<string, GridDescPtr> & gridMap)
{
	for (unsigned int nn = 0; nn < mInstructions.size(); nn++)
	{
		InstructionPtr ii = mInstructions[nn];
		switch (ii->getType())
		{
			case kBlockType:
				((Block &)*ii).setPointers(materialMap);
				break;
			case kKeyImageType:
				((KeyImage&)*ii).setPointers(materialMap);
				break;
			case kHeightMapType:
				((HeightMap&)*ii).setPointers(materialMap);
				break;
			case kEllipsoidType:
				((Ellipsoid&)*ii).setPointers(materialMap);
				break;
			case kCopyFromType:
				((CopyFrom&)*ii).setPointers(gridMap);
				break;
		}
	}
}

ColorKey::
ColorKey(string hexColor, string materialName, FillStyle style)
	throw(Exception) :
	mMaterialName(materialName),
	mFillStyle(style)
{
	try {
		mColor = sConvertColor(Magick::Color(hexColor));
	}
	catch (const Magick::Exception & ex) {
		throw(Exception(ex.what()));
	}
}


ColorKey::
ColorKey(Vector3i rgbColor, string materialName, FillStyle style)
	throw(Exception) :
	mColor(rgbColor),
	mMaterialName(materialName),
	mFillStyle(style)
{
	assert(mFillStyle != kHalfCellStyle); // KeyImage uses one pixel per cell
}

void ColorKey::
setPointers(const Map<string, MaterialDescPtr> & materialMap)
{
	mMaterial = materialMap[mMaterialName];
}


Instruction::
Instruction(InstructionType inType) :
	mType(inType)
{
}

Block::
Block(Rect3i halfCellRect, string material) throw(Exception) :
	Instruction(kBlockType),
	mFillRect(halfCellRect),
	mMaterialName(material),
	mStyle(kHalfCellStyle)
{
	cerr << "Warning: minimal validation done for Block().\n";
	if (!vec_ge(mFillRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative."));
}

Block::
Block(Rect3i yeeCellRect, FillStyle style, string material) throw(Exception) :
	Instruction(kBlockType),
	mFillRect(yeeCellRect),
	mMaterialName(material),
	mStyle(style)
{
	cerr << "Warning: minimal validation done for Block().\n";
	assert(mStyle != kHalfCellStyle);
	
	if (!vec_ge(mFillRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative."));
}

const Rect3i & Block::
getYeeRect() const
{
	assert(mStyle != kHalfCellStyle);
	return mFillRect;
}

const Rect3i & Block::
getHalfRect() const
{
	assert(mStyle == kHalfCellStyle);
	return mFillRect;
}

void Block::
setPointers(const Map<string, MaterialDescPtr> & materialMap)
{
	mMaterial = materialMap[mMaterialName];
}

KeyImage::
KeyImage(Rect3i yeeCellRect, string imageFileName, Vector3i rowDirection,
	Vector3i colDirection, vector<ColorKey> keys) throw(Exception) :
	Instruction(kKeyImageType),
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

void KeyImage::
setPointers(const Map<string, MaterialDescPtr> & materialMap)
{
	for (unsigned int nn = 0; nn < mKeys.size(); nn++)
		mKeys[nn].setPointers(materialMap);
}

HeightMap::
HeightMap(Rect3i yeeCellRect, FillStyle style, string material,
	string imageFileName, Vector3i rowDirection, Vector3i colDirection,
	Vector3i upDirection) throw(Exception) :
	Instruction(kHeightMapType),
	mYeeRect(yeeCellRect),
	mStyle(style),
	mMaterialName(material),
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

void HeightMap::
setPointers(const Map<string, MaterialDescPtr> & materialMap)
{
	mMaterial = materialMap[mMaterialName];
}

Ellipsoid::
Ellipsoid(Rect3i halfCellRect, string material) throw(Exception) :
	Instruction(kEllipsoidType),
	mFillRect(halfCellRect),
	mStyle(kHalfCellStyle),
	mMaterialName(material)
{
	cerr << "Warning: minimal validation done for Ellipsoid().\n";
	if (!vec_ge(mFillRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative"));
}

Ellipsoid::
Ellipsoid(Rect3i yeeCellRect, FillStyle style, string material)
	throw(Exception) :
	Instruction(kEllipsoidType),
	mFillRect(yeeCellRect),
	mStyle(style),
	mMaterialName(material)
{
	cerr << "Warning: minimal validation done for Ellipsoid().\n";
	assert(mStyle != kHalfCellStyle);
	if (!vec_ge(mFillRect.size(), 0))
		throw(Exception("Some fill rect dimensions are negative"));
}

void Ellipsoid::
setPointers(const Map<string, MaterialDescPtr> & materialMap)
{
	mMaterial = materialMap[mMaterialName];
}

const Rect3i & Ellipsoid::
getYeeRect() const
{
	assert(mStyle != kHalfCellStyle);
	return mFillRect;
}

const Rect3i & Ellipsoid::
getHalfRect() const
{
	assert(mStyle == kHalfCellStyle);
	return mFillRect;
}

CopyFrom::
CopyFrom(Rect3i halfCellSourceRegion, Rect3i halfCellDestRegion,
	string gridName) throw(Exception) :
	Instruction(kCopyFromType),
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

void CopyFrom::
setPointers(const Map<string, GridDescPtr> & gridMap)
{
	mGrid = gridMap[mGridName];
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



