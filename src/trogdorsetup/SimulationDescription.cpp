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
		mGrids[nGrid]->setPointers(materialMap, gridMap);
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

void SimulationDescription::
cycleCoordinates()
{
	m_dxyz = Vector3f(m_dxyz[2], m_dxyz[0], m_dxyz[1]);
	int nn;
	for (nn = 0; nn < mGrids.size(); nn++)
		mGrids[nn]->cycleCoordinates();
	for (nn = 0; nn < mMaterials.size(); nn++)
		mMaterials[nn]->cycleCoordinates();
}

#pragma mark *** Grid ***

GridDescription::
GridDescription(string name, Vector3i numYeeCells, Vector3i numHalfCells,
	Rect3i calcRegionHalf, Rect3i nonPMLHalf, Vector3i originYee)
	throw(Exception) :
	mName(name),
	mNumYeeCells(numYeeCells),
	mNumHalfCells(numHalfCells),
	mCalcRegionHalf(calcRegionHalf),
	mNonPMLHalf(nonPMLHalf),
	mOriginYee(originYee)
{
	assert(Vector3i(2*mNumYeeCells) == mNumHalfCells); // caller's job...
	
	if (!vec_ge(mCalcRegionHalf.size(), 0))
		throw(Exception("Calc region has some negative dimensions"));
	if (!vec_ge(mNonPMLHalf.size(), 0))
		throw(Exception("Non-PML region has some negative dimensions"));
}


void GridDescription::
setHuygensSurfaces(const vector<HuygensSurfaceDescPtr> & surfaces)
{
	mHuygensSurfaces = surfaces;
	// We need to make sure that Huygens surfaces in repeating grids will
	// omit any sides that are adjacent due to periodicity.  That means, if the
	// Huygens surface extends the full dimension of the grid, don't make a
	// correction.
	for (unsigned int nn = 0; nn < mHuygensSurfaces.size(); nn++)
	{
		Rect3i destHalfRect = mHuygensSurfaces[nn]->getDestHalfRect();
		
		for (int xyz = 0; xyz < 3; xyz++)
		{
			if (destHalfRect.p1[xyz] <= 0 &&
				destHalfRect.p2[xyz] >= mNumHalfCells[xyz]-1)
			{
				mHuygensSurfaces[nn]->omitSide(2*xyz); // the -x, -y or -z side
				mHuygensSurfaces[nn]->omitSide(2*xyz+1); // the +x, +y or +z side
			}
		}
	}
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

void GridDescription::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mNumYeeCells = permuteForward*mNumYeeCells;
	mNumHalfCells = permuteForward*mNumHalfCells;
	mCalcRegionHalf = permuteForward*mCalcRegionHalf;
	mNonPMLHalf = permuteForward*mNonPMLHalf;
	mOriginYee = permuteForward*mOriginYee;
	
	unsigned int nn;
	
	for (nn = 0; nn < mOutputs.size(); nn++)
		mOutputs[nn]->cycleCoordinates();
	for (nn = 0; nn < mInputs.size(); nn++)
		mInputs[nn]->cycleCoordinates();
	for (nn = 0; nn < mSources.size(); nn++)
		mSources[nn]->cycleCoordinates();
	for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
		mHuygensSurfaces[nn]->cycleCoordinates();
	mAssembly->cycleCoordinates();
}

Rect3i GridDescription::
getYeeBounds() const
{
	return Rect3i(Vector3i(0,0,0), mNumYeeCells-Vector3i(1,1,1));
}

Rect3i GridDescription::
getHalfCellBounds() const
{
	return Rect3i(Vector3i(0,0,0), mNumHalfCells-Vector3i(1,1,1));
}

int GridDescription::
getNumDimensions() const
{
	int nDim = 0;
	for (int nn = 0; nn < 3; nn++)
	if (mNumYeeCells[nn] > 1)
		nDim += 1;
	return nDim;
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


void InputEHDescription::
cycleCoordinates()
{
	mCoordinatePermutationNumber += 1;
	mCoordinatePermutationNumber %= 3;
}

#pragma mark *** Output ***

OutputDescription::
OutputDescription(string fileName, string inClass, int inPeriod,
    Rect3i inRegion, Vector3b whichE, Vector3b whichH, Vector3i inStride,
	const Map<string, string> & inParameters) throw(Exception) :
	mFileName(fileName),
	mClass(inClass),
	mPeriod(inPeriod),
    mWhichE(whichE),
    mWhichH(whichH),
    mYeeRects(1,inRegion),
    mStrides(1,inStride),
	mParams(inParameters)
{
	cerr << "Warning: OutputDescription does not validate inClass.\n";
}

void OutputDescription::
cycleCoordinates()
{
    unsigned int nn;
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mCoordinatePermutationNumber += 1;
	mCoordinatePermutationNumber %= 3;
    
    mWhichE = Vector3b(mWhichE[2], mWhichE[0], mWhichE[1]);
    mWhichH = Vector3b(mWhichH[2], mWhichH[0], mWhichH[1]);
    
    for (nn = 0; nn < mYeeRects.size(); nn++)
        mYeeRects[nn] = permuteForward * mYeeRects[nn];
    for (nn = 0; nn < mStrides.size(); nn++)
        mStrides[nn] = permuteForward * mStrides[nn];
}

#pragma mark *** Source ***

SourceDescription::
SourceDescription(string formula, string inFileName,
	Vector3f polarization, Rect3i region, Vector3b whichE, Vector3b whichH,
	const Map<string, string> & inParameters) throw(Exception) :
	mFormula(formula),
	mInputFileName(inFileName),
	mPolarization(polarization),
    mWhichE(whichE),
    mWhichH(whichH),
    mIsSpaceVarying(0),
    mYeeRects(1, region),
    mFirstTimestep(),
    mLastTimestep(),
	mParams(inParameters)
{
	cerr << "Warning: SourceDescription does not validate formula.\n";
    
    for (unsigned int nn = 0; nn < mYeeRects.size(); nn++)
        if (!vec_ge(mYeeRects[nn].size(), 0))
            throw(Exception("Source region has some negative dimensions"));
}

void SourceDescription::
cycleCoordinates()
{
    unsigned int nn;
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mCoordinatePermutationNumber += 1;
	mCoordinatePermutationNumber %= 3;
	
	mPolarization = Vector3f(
		mPolarization[2], mPolarization[0], mPolarization[1]);
    
    mWhichE = Vector3b(mWhichE[2], mWhichE[0], mWhichE[1]);
    mWhichH = Vector3b(mWhichH[2], mWhichH[0], mWhichH[1]);
	
    for (nn = 0; nn < mYeeRects.size(); nn++)
        mYeeRects[nn] = permuteForward * mYeeRects[nn];
}


#pragma mark *** HuygensSurface ***

// constructor for LINKS, using source grid name
HuygensSurfaceDescription::
HuygensSurfaceDescription(string typeString, string sourceGridName, 
	Rect3i sourceHalfRect, Rect3i destHalfRect,
	const std::set<Vector3i> & omittedSides)
	throw(Exception) :
	mType(kLink),
	mDestHalfRect(destHalfRect),
	mOmittedSides(omittedSides),
	mBuffers(6),
	mLinkSourceHalfRect(sourceHalfRect),
	mLinkSourceGridName(sourceGridName),
	mLinkSourceGrid(),
	mTFSFSourceSymmetries(0,0,0),
	mTFSFType(),
	mTFSFSourceParams(),
	mTFSFSourceDirection(0,0,0),
	mTFSFFormula(),
	mTFSFPolarization(0,0,0),
	mTFSFField(),
	mDataRequestName()
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


// constructor for LINKS, using source grid pointer
HuygensSurfaceDescription::
HuygensSurfaceDescription(string typeString, const GridDescPtr sourceGrid, 
	Rect3i sourceHalfRect, Rect3i destHalfRect,
	const std::set<Vector3i> & omittedSides)
	throw(Exception) :
	mType(kLink),
	mDestHalfRect(destHalfRect),
	mOmittedSides(omittedSides),
	mBuffers(6),
	mLinkSourceHalfRect(sourceHalfRect),
	mLinkSourceGridName(sourceGrid->getName()),
	mLinkSourceGrid(sourceGrid),
	mTFSFSourceSymmetries(0,0,0),
	mTFSFType(),
	mTFSFSourceParams(),
	mTFSFSourceDirection(0,0,0),
	mTFSFFormula(),
	mTFSFFileName(),
	mTFSFPolarization(0,0,0),
	mTFSFField(),
	mDataRequestName()
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
HuygensSurfaceDescription(Rect3i inTFRect, Vector3i direction,
	string formula, string fileName, Vector3f polarization,
	string field, string inTFSFType, const Map<string,string> & inParameters,
	const set<Vector3i> & omittedSides)
	throw(Exception) :
	mType(kTFSFSource),
	mDestHalfRect(inTFRect),
	mOmittedSides(omittedSides),
	mBuffers(6),
	mLinkSourceHalfRect(),
	mLinkSourceGridName(),
	mLinkSourceGrid(),
	mTFSFSourceSymmetries(0,0,0),
	mTFSFType(inTFSFType),
	mTFSFSourceParams(inParameters),
	mTFSFSourceDirection(direction),
	mTFSFFormula(),
	mTFSFFileName(),
	mTFSFPolarization(),
	mTFSFField(),
	mDataRequestName()
{
	if (!vec_ge(mDestHalfRect.size(), 0))
		throw(Exception("TFSFSource TF rect has some negative dimensions"));
	
	if (mTFSFSourceDirection != dominantComponent(mTFSFSourceDirection) ||
		norm1(mTFSFSourceDirection) != 1)
		throw(Exception("Source direction must be axis-oriented unit vector"));
	
	for (unsigned int nn = 0; nn < 3; nn++)
	if (mTFSFSourceDirection[nn] == 0)
		mTFSFSourceSymmetries[nn] = 1;
	
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
		throw(Exception("TFSFSource type must be 'TF' or 'SF'"));
}

// constructor for CustomSources
HuygensSurfaceDescription::
HuygensSurfaceDescription(Rect3i inTFRect, const string & inName,
	Vector3i symmetries, string inTFSFType,
	const Map<string,string> & inParameters,
	const std::set<Vector3i> & omittedSides)
	throw(Exception) :
	mType(kDataRequest),
	mDestHalfRect(inTFRect),
	mOmittedSides(omittedSides),
	mBuffers(6),
	mLinkSourceHalfRect(),
	mLinkSourceGridName(),
	mLinkSourceGrid(),
	mTFSFSourceSymmetries(symmetries),
	mTFSFType(inTFSFType),
	mTFSFSourceParams(inParameters),
	mTFSFSourceDirection(0,0,0),
	mTFSFFormula(),
	mTFSFFileName(),
	mTFSFPolarization(0,0,0),
	mTFSFField(),
	mDataRequestName(inName)
{
	if (!vec_ge(mDestHalfRect.size(), 0))
		throw(Exception("CustomSource TF rect has some negative dimensions"));
	for (unsigned int nn = 0; nn < 3; nn++)
	if (mTFSFSourceSymmetries[nn] != 1 && mTFSFSourceSymmetries[nn] != 0)
		throw(Exception("CustomSource symmetry vector must have "
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
		throw(Exception("CustomSource type must be 'TF' or 'SF'"));
}

void HuygensSurfaceDescription::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	vmlib::SMat<3,float> permuteForwardf;
	permuteForwardf = 0.0f,0.0f,1.0f,1.0f,0.f,0.f,0.f,1.f,0.f;
	
	// Rotate the rects and vectors
	mDestHalfRect = permuteForward*mDestHalfRect;
	mLinkSourceHalfRect = permuteForward*mLinkSourceHalfRect;
	mTFSFSourceSymmetries = permuteForward*mTFSFSourceSymmetries;
	mTFSFPolarization = permuteForwardf*mTFSFPolarization;
	mTFSFSourceDirection = permuteForward*mTFSFSourceDirection;
	
	// Rotate the omitted sides
	set<Vector3i> newOmittedSides;
	for (set<Vector3i>::iterator itr = mOmittedSides.begin();
		itr != mOmittedSides.end(); itr++)
	{
		newOmittedSides.insert(permuteForward * (*itr));
	}
	mOmittedSides = newOmittedSides;
	
	// Permute and rotate the buffers (the permutation is "backwards")
	vector<NeighborBufferDescPtr> newBuffers(6);
	for (int ii = 0; ii < 6; ii++)
	{
		newBuffers[ii] = mBuffers[ (ii+4)%6 ];
		if (newBuffers[ii] != 0L)
			newBuffers[ii]->cycleCoordinates();
	}
	mBuffers = newBuffers;
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
newLink(string typeString, GridDescPtr sourceGrid, Rect3i sourceHalfRect,
	Rect3i destHalfRect, const set<Vector3i> & omittedSides)
	throw(Exception)
{
	return new HuygensSurfaceDescription(typeString, sourceGrid,
		sourceHalfRect, destHalfRect, omittedSides);
}
HuygensSurfaceDescription* HuygensSurfaceDescription::
newTFSFSource(Rect3i inHalfRect, Vector3i direction, string formula,
	string fileName, Vector3f polarization, string field,
	string inTFSFType, const Map<string, string> & inParameters,
	const set<Vector3i> & omittedSides) throw(Exception)
{
	return new HuygensSurfaceDescription(inHalfRect, direction, formula,
		fileName, polarization, field, inTFSFType, inParameters, omittedSides);
}

HuygensSurfaceDescription* HuygensSurfaceDescription::
newCustomSource(Rect3i inHalfRect, const std::string & inName,
	Vector3i symmetries, std::string inTFSFType,
	const Map<std::string, std::string> & inParameters,
	const std::set<Vector3i> & omittedSides) throw(Exception)
{
	return new HuygensSurfaceDescription(inHalfRect, inName, symmetries,
		inTFSFType, inParameters, omittedSides);
}


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
		NeighborBufferDescPtr nb(new NeighborBufferDescription(
			mDestHalfRect, nDir, srcFactor));
		mBuffers[nDir] = nb;
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

void HuygensSurfaceDescription::
omitSide(int sideNum)
{
	Vector3i side = cardinalDirection(sideNum);
	mOmittedSides.insert(side);
}

#pragma mark *** NeighborBuffer ***

NeighborBufferDescription::
NeighborBufferDescription(const Rect3i & destHalfRect, int nSide, 
	float incidentFieldFactor) :
	mDestHalfRect(destHalfRect),
	mDestFactors(6),
	mSrcFactors(6)
{
	Rect3i outerTotalField = edgeOfRect(mDestHalfRect, nSide);
	
	// the fat boundary contains the cells on BOTH sides of the TFSF boundary
	// in the destination grid
	Rect3i fatBoundary = outerTotalField;
	if (nSide % 2 == 0) // if this is a low-x, low-y or low-z side
		fatBoundary.p1 += cardinalDirection(nSide);
	else
		fatBoundary.p2 += cardinalDirection(nSide);
	
	// the buffer half rect is the rect in the buffer that maps to the
	// fat boundary
	Rect3i bufferHalfRect = fatBoundary - Vector3i(2*(fatBoundary.p1/2));
	bufferHalfRect.p1[nSide/2] = 0;
	bufferHalfRect.p2[nSide/2] = 1;
	
	mBufferHalfRect = bufferHalfRect;
	mBufferYeeBounds = rectHalfToYee(bufferHalfRect);
	
	for (unsigned int fieldNum = 0; fieldNum < 6; fieldNum++) // on E, H
	{
		Vector3i fieldOffset = halfCellFieldOffset(fieldNum);
		Vector3i p1Offset = outerTotalField.p1 % 2;
		
		// Figure out if the given yee octant lies in the TF region or SF region
		if (fieldOffset[nSide/2] == p1Offset[nSide/2])
		{
			// total-field buffer
			mDestFactors[fieldNum] = 1.0f;
			mSrcFactors[fieldNum] = 1.0f * incidentFieldFactor;
		}
		else
		{
			// scattered-field buffer
			mDestFactors[fieldNum] = 1.0f;
			mSrcFactors[fieldNum] = -1.0f * incidentFieldFactor;
		}
	}
}

void NeighborBufferDescription::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	// Rotate the rects
	mDestHalfRect = permuteForward * mDestHalfRect;
	mBufferHalfRect = permuteForward * mBufferHalfRect;
	mBufferYeeBounds = permuteForward * mBufferYeeBounds;
	
	// Permute the source and destination factors.
	// This is tricky since the order is (Ex, Ey, Hz, Ez, Hy, Hx).
	// Might as well do it by hand (should I revisit this design later?)
	int permuteOrder[] = {3,0,4,1,5,2}; // Ex was Ez, Ey was Ex, etc.
	vector<float> newDestFactors(6);
	vector<float> newSrcFactors(6);
	for (int ii = 0; ii < 6; ii++)
	{
		newDestFactors[ii] = mDestFactors[ permuteOrder[ii] ];
		newSrcFactors[ii] = mSrcFactors[ permuteOrder[ii] ];
	}
	mDestFactors = newDestFactors;
	mSrcFactors = newSrcFactors;
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

void MaterialDescription::
cycleCoordinates()
{
	mCoordinatePermutationNumber = (mCoordinatePermutationNumber+1)%3;
}

ostream &
operator<<(ostream & out, const MaterialDescription & mat)
{
	out << mat.getName();
	return out;
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
			case kExtrudeType:
				// don't need to do anything here since there are no pointers
				//((Extrude&)*ii).setPointers(gridMap);
				break;
		}
	}
}

void AssemblyDescription::
cycleCoordinates()
{
	for (unsigned int nn = 0; nn < mInstructions.size(); nn++)
		mInstructions[nn]->cycleCoordinates();
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

void Instruction::
cycleCoordinates()
{
	LOG << "Warning: base cycleCoordinates() shouldn't have been called!\n";
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

void Block::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mFillRect = permuteForward*mFillRect;
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

void KeyImage::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mYeeRect = permuteForward*mYeeRect;
	mRow = permuteForward*mRow;
	mCol = permuteForward*mCol;
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

void HeightMap::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mYeeRect = permuteForward*mYeeRect;
	mRow = permuteForward*mRow;
	mCol = permuteForward*mCol;
	mUp = permuteForward*mUp;
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


void Ellipsoid::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mFillRect = permuteForward*mFillRect;
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
	// Easy validation: no inside-out rects
	if (!vec_ge(mSourceRect.size(), 0))
		throw(Exception("Some source rect dimensions are negative"));
	if (!vec_ge(mDestRect.size(), 0))
		throw(Exception("Some dest rect dimensions are negative"));
	
	// All copyFrom dimensions must equal copyTo or be 0.
	
	for (int nn = 0; nn < 3; nn++)
	if (mSourceRect.size(nn) != 0)
	{
		if (mSourceRect.size(nn) != mDestRect.size(nn))
		throw(Exception("Error: copy from region must be same size as "
			"copy to region or have size 0"));
		if (mSourceRect.p1[nn]%2 != mDestRect.p1[nn]%2)
		throw(Exception("Error: copy from region must start on same half-cell"
			" octant as copy to region or have size 0 (both should start on"
			" even indices or on odd indices)"));
		// it's sufficient to check p1 and not p2 because we already know
		// that the dimensions are equal.
	}
}

CopyFrom::
CopyFrom(Rect3i halfCellSourceRegion, Rect3i halfCellDestRegion,
	const GridDescPtr grid) throw(Exception) :
	Instruction(kCopyFromType),
	mSourceRect(halfCellSourceRegion),
	mDestRect(halfCellDestRegion),
	mGridName(grid->getName()),
	mGrid(grid)
{
	// Easy validation: no inside-out rects
	if (!vec_ge(mSourceRect.size(), 0))
		throw(Exception("Some source rect dimensions are negative"));
	if (!vec_ge(mDestRect.size(), 0))
		throw(Exception("Some dest rect dimensions are negative"));
	
	// All copyFrom dimensions must equal copyTo or be 0.
	
	for (int nn = 0; nn < 3; nn++)
	if (mSourceRect.size(nn) != 0)
	{
		if (mSourceRect.size(nn) != mDestRect.size(nn))
		throw(Exception("Error: copy from region must be same size as "
			"copy to region or have size 0"));
		if (mSourceRect.p1[nn]%2 != mDestRect.p1[nn]%2)
		throw(Exception("Error: copy from region must start on same half-cell"
			" octant as copy to region or have size 0 (both should start on"
			" even indices or on odd indices)"));
		// it's sufficient to check p1 and not p2 because we already know
		// that the dimensions are equal.
	}
}

void CopyFrom::
setPointers(const Map<string, GridDescPtr> & gridMap)
{
	mGrid = gridMap[mGridName];
}

void CopyFrom::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mSourceRect = permuteForward*mSourceRect;
	mDestRect = permuteForward*mDestRect;
}

Extrude::
Extrude(Rect3i halfCellExtrudeFrom, Rect3i halfCellExtrudeTo) throw(Exception) :
	Instruction(kExtrudeType),
	mExtrudeFrom(halfCellExtrudeFrom),
	mExtrudeTo(halfCellExtrudeTo)
{
	// Easy validation: no inside-out rects
	if (!vec_ge(mExtrudeFrom.size(), 0))
		throw(Exception("ExtrudeFrom dimensions are negative"));
	if (!vec_ge(mExtrudeTo.size(), 0))
		throw(Exception("ExtrudeTo dimensions are negative"));
	if (!mExtrudeTo.encloses(mExtrudeFrom))
		throw(Exception("ExtrudeTo does not enclose ExtrudeFrom"));
}

void Extrude::
cycleCoordinates()
{
	Mat3i permuteForward, permuteBackward;
	permuteForward = 0,0,1,1,0,0,0,1,0;
	permuteBackward = 0,1,0,0,0,1,1,0,0;
	
	mExtrudeFrom = permuteForward*mExtrudeFrom;
	mExtrudeTo = permuteForward*mExtrudeTo;
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



