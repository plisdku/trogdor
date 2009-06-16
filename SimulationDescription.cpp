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
#include <sstream>

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
    m_dxyz = cyclicPermute(m_dxyz, 1);
	//m_dxyz = Vector3f(m_dxyz[2], m_dxyz[0], m_dxyz[1]);
	unsigned int nn;
	for (nn = 0; nn < mGrids.size(); nn++)
		mGrids[nn]->cycleCoordinates();
	for (nn = 0; nn < mMaterials.size(); nn++)
		mMaterials[nn]->cycleCoordinates();
}

#pragma mark *** Grid ***

GridDescription::
GridDescription(string name, Vector3i numYeeCells,
	Rect3i calcRegionHalf, Rect3i nonPMLHalf, Vector3i originYee,
    Vector3f dxyz, float dt)
	throw(Exception) :
	mName(name),
	mNumYeeCells(numYeeCells),
	mNumHalfCells(2*numYeeCells),
	mCalcRegionHalf(calcRegionHalf),
	mNonPMLHalf(nonPMLHalf),
	mOriginYee(originYee),
    mDxyz(dxyz),
    mDt(dt)
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
		Rect3i destHalfRect = mHuygensSurfaces[nn]->getHalfCells();
		
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
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
	mNumYeeCells = permuteForward*mNumYeeCells;
	mNumHalfCells = permuteForward*mNumHalfCells;
	mCalcRegionHalf = permuteForward*mCalcRegionHalf;
	mNonPMLHalf = permuteForward*mNonPMLHalf;
	mOriginYee = permuteForward*mOriginYee;
	
	unsigned int nn;
	
	for (nn = 0; nn < mOutputs.size(); nn++)
		mOutputs[nn]->cycleCoordinates();
	for (nn = 0; nn < mSources.size(); nn++)
		mSources[nn]->cycleCoordinates();
	for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
		mHuygensSurfaces[nn]->cycleCoordinates();
	mAssembly->cycleCoordinates();
    
    Map<Vector3i, Map<string, string> > newPMLParams;
    for (int sideNum = 0; sideNum < 6; sideNum++)
        newPMLParams[cyclicPermute(cardinalDirection(sideNum), 1)] =
            mPMLParams[cardinalDirection(sideNum)];
    mPMLParams = newPMLParams;
    
    mDxyz = cyclicPermute(mDxyz, 1);
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

#pragma mark *** Region ***

void Region::
cycleCoordinates()
{
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
    
    mYeeCells = permuteForward*mYeeCells;
    mStride = permuteForward*mStride;
}


#pragma mark *** Output ***

OutputDescription::
OutputDescription(std::string fields, std::string file) throw(Exception) :
    mCoordinatePermutationNumber(0),
    mFile(file),
    mIsInterpolated(0),
    mInterpolationPoint(0.0,0.0,0.0), // specified but not used
    mRegions(vector<Region>(1,Region())),
    mDurations(vector<Duration>(1,Duration()))
{
    determineWhichFields(fields);
}

OutputDescription::
OutputDescription(std::string fields, std::string file,
    Region region, Duration duration) throw(Exception) :
    mCoordinatePermutationNumber(0),
    mFile(file),
    mIsInterpolated(0),
    mInterpolationPoint(0.0,0.0,0.0), // specified but not used
    mRegions(vector<Region>(1,region)),
    mDurations(vector<Duration>(1,duration))
{
    determineWhichFields(fields);
}

OutputDescription::
OutputDescription(std::string fields, std::string file,
    const std::vector<Region> & regions,
    const std::vector<Duration> & durations) throw(Exception) :
    mCoordinatePermutationNumber(0),
    mFile(file),
    mIsInterpolated(0),
    mInterpolationPoint(0.0,0.0,0.0), // specified but not used
    mRegions(regions),
    mDurations(durations)
{
    determineWhichFields(fields);
}

OutputDescription::
OutputDescription(std::string fields, std::string file,
    Vector3f interpolationPoint, const std::vector<Region> & regions,
    const std::vector<Duration> & durations) throw(Exception) :
    mCoordinatePermutationNumber(0),
    mFile(file),
    mWhichE(0,0,0),
    mWhichH(0,0,0),
    mWhichJ(0,0,0),
    mWhichP(0,0,0),
    mWhichM(0,0,0),
    mIsInterpolated(1),
    mInterpolationPoint(interpolationPoint),
    mRegions(regions),
    mDurations(durations)
{
    determineWhichFields(fields);
}

void OutputDescription::
cycleCoordinates()
{
    unsigned int nn;
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
	mCoordinatePermutationNumber += 1;
	mCoordinatePermutationNumber %= 3;
    
    mWhichE = permuteForward*mWhichE;
    mWhichH = permuteForward*mWhichH;
    mWhichJ = permuteForward*mWhichJ;
    mWhichP = permuteForward*mWhichP;
    mWhichM = permuteForward*mWhichM;
    
    mInterpolationPoint = permuteForward*mInterpolationPoint;
    
    for (nn = 0; nn < mRegions.size(); nn++)
        mRegions[nn].cycleCoordinates();
}

void OutputDescription::
determineWhichFields(std::string fields) throw(Exception)
{
    istringstream istr(fields);
    string field;
    while(istr >> field && field != "")
    {
        if (field == "electric")
            mWhichE = Vector3i(1,1,1);
        else if (field == "magnetic")
            mWhichH = Vector3i(1,1,1);
        else if (field == "polarization")
            mWhichP = Vector3i(1,1,1);
        else if (field == "current")
            mWhichJ = Vector3i(1,1,1);
        else if (field == "magnetization")
            mWhichM = Vector3i(1,1,1);
        else if (field.length() == 2)
        {
            char c1 = field[0];
            char c2 = field[1];
            
            int direction = int(c2 - 'x');
            if (direction < 0 || direction > 2)
                throw(Exception(string("Invalid field ") + field));
            
            switch (c1)
            {
                case 'e':
                    mWhichE[direction] = 1;
                    break;
                case 'h':
                    mWhichH[direction] = 1;
                    break;
                case 'j':
                    mWhichJ[direction] = 1;
                    break;
                case 'p':
                    mWhichP[direction] = 1;
                    break;
                case 'm':
                    mWhichM[direction] = 1;
                    break;
                default:
                    throw(Exception(string("Invalid field ") + field));
                    break;
            };
        }
        else
            throw(Exception(string("Invalid field ") + field));
    }
    
    Vector3i threeFalses(false,false,false);
    if (mIsInterpolated)
    if (mWhichJ != threeFalses ||
        mWhichP != threeFalses ||
        mWhichM != threeFalses)
        throw(Exception("Only E and H fields may be interpolated."));
}

#pragma mark *** MaterialOutput ***

MaterialOutputDescription::
MaterialOutputDescription()
{
}

void MaterialOutputDescription::
cycleCoordinates()
{
}

#pragma mark *** SourceFields ***

SourceFields::
SourceFields() :
    mUsesPolarization(0),
    mPolarization(0.0, 0.0, 0.0),
    mWhichE(0,0,0),
    mWhichH(0,0,0)
{
}

SourceFields::
SourceFields(string fields) :
    mUsesPolarization(0),
    mPolarization(0.0, 0.0, 0.0),
    mWhichE(0,0,0),
    mWhichH(0,0,0)
{
    istringstream istr(fields);
    string field;
    while(istr >> field && field != "")
    {
        if (field == "electric")
            mWhichE = Vector3i(1,1,1);
        else if (field == "magnetic")
            mWhichH = Vector3i(1,1,1);
        else if (field.length() == 2)
        {
            char c1 = field[0];
            char c2 = field[1];
            
            int direction = int(c2 - 'x');
            if (direction < 0 || direction > 2)
                throw(Exception(string("Invalid field ") + field));
            
            if (c1 == 'e')
                mWhichE[direction] = 1;
            else if (c1 == 'h')
                mWhichH[direction] = 1;
            else
                throw(Exception(string("Invalid field ") + field));
        }
        else
            throw(Exception(string("Invalid field ") + field));
    }
}

SourceFields::
SourceFields(string fields, Vector3f polarization) :
    mUsesPolarization(1),
    mPolarization(polarization),
    mWhichE(0,0,0),
    mWhichH(0,0,0)
{
    if (fields == "electric" || fields == "e")
        mWhichE = Vector3i(1,1,1);
    else if (fields == "magnetic" || fields == "h")
        mWhichH = Vector3i(1,1,1);
    else
        throw(Exception(string("When polarization is provided, the field "
            "must be 'electric' or 'magnetic'; field is ") + fields));
}

void SourceFields::
cycleCoordinates()
{
    unsigned int nn;
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
	//mCoordinatePermutationNumber += 1;
	//mCoordinatePermutationNumber %= 3;
    
    mPolarization = permuteForward*mPolarization;
    mWhichE = permuteForward*mWhichE;
    mWhichH = permuteForward*mWhichH;
}


#pragma mark *** Source ***

SourceDescription* SourceDescription::
newTimeSource(string timeFile, SourceFields fields, bool isSoft,
    const vector<Region> & regions, const vector<Duration> & durations)
{
    return new SourceDescription(fields, "", timeFile, "",
        isSoft, regions, durations);
}

SourceDescription* SourceDescription::
newSpaceTimeSource(string spaceTimeFile, SourceFields fields, bool isSoft,
    const vector<Region> & regions, const vector<Duration> & durations)
{
    return new SourceDescription(fields, "", "", spaceTimeFile,
        isSoft, regions, durations);
}

SourceDescription* SourceDescription::
newFormulaSource(string formula, SourceFields fields, bool isSoft,
    const vector<Region> & regions, const vector<Duration> & durations)
{
    return new SourceDescription(fields, formula, "", "",
        isSoft, regions, durations);
}

void SourceDescription::
cycleCoordinates()
{
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
	mCoordinatePermutationNumber += 1;
	mCoordinatePermutationNumber %= 3;
    
    for (unsigned int nn = 0; nn < mRegions.size(); nn++)
        mRegions[nn].cycleCoordinates();
    
    mFields.cycleCoordinates();
    
    LOG << "Not permuting coordinates of source formula!!\n";
}

SourceDescription::
SourceDescription(SourceFields fields, string formula, string timeFile,
    string spaceTimeFile, bool isSoft, const vector<Region> & regions,
    const vector<Duration> & durations) throw(Exception) :
    mCoordinatePermutationNumber(0),
    mFormula(formula),
    mTimeFile(timeFile),
    mSpaceFileDoThisLaterOkay("whatever you say, man"),
    mSpaceTimeFile(spaceTimeFile),
    mFields(fields),
    mRegions(regions),
    mDurations(durations),
    mIsSoft(isSoft)
{
}

#pragma mark *** SourceCurrents ***

SourceCurrents::
SourceCurrents() :
    mUsesPolarization(0),
    mPolarization(0.0, 0.0, 0.0),
    mWhichJ(0,0,0),
    mWhichK(0,0,0)
{
}

SourceCurrents::
SourceCurrents(string fields) :
    mUsesPolarization(0),
    mPolarization(0.0, 0.0, 0.0),
    mWhichJ(0,0,0),
    mWhichK(0,0,0)
{
    istringstream istr(fields);
    string field;
    while(istr >> field && field != "")
    {
        if (field == "electric" || field == "j")
            mWhichJ = Vector3i(1,1,1);
        else if (field == "magnetic" || field == "k")
            mWhichK = Vector3i(1,1,1);
        else if (field.length() == 2)
        {
            char c1 = field[0];
            char c2 = field[1];
            
            int direction = int(c2 - 'x');
            if (direction < 0 || direction > 2)
                throw(Exception(string("Invalid field ") + field));
            
            if (c1 == 'j')
                mWhichJ[direction] = 1;
            else if (c1 == 'k')
                mWhichK[direction] = 1;
            else
                throw(Exception(string("Invalid field ") + field));
        }
        else
            throw(Exception(string("Invalid field ") + field));
    }
}

SourceCurrents::
SourceCurrents(string fields, Vector3f polarization) :
    mUsesPolarization(1),
    mPolarization(polarization),
    mWhichJ(0,0,0),
    mWhichK(0,0,0)
{
    if (fields == "electric")
        mWhichJ = Vector3i(1,1,1);
    else if (fields == "magnetic")
        mWhichK = Vector3i(1,1,1);
    else
        throw(Exception(string("When polarization is provided, the field "
            "must be 'electric' or 'magnetic'; field is ") + fields));
}

void SourceCurrents::
cycleCoordinates()
{
    unsigned int nn;
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
	//mCoordinatePermutationNumber += 1;
	//mCoordinatePermutationNumber %= 3;
    
    mPolarization = permuteForward*mPolarization;
    mWhichJ = permuteForward*mWhichJ;
    mWhichK = permuteForward*mWhichK;
}

#pragma mark *** CurrentSource ***


CurrentSourceDescription* CurrentSourceDescription::
newTimeSource(string timeFile, SourceCurrents currents,
    const vector<Region> & regions, const vector<Duration> & durations)
{
    return new CurrentSourceDescription(currents, "", timeFile, "",
        regions, durations);
}

CurrentSourceDescription* CurrentSourceDescription::
newSpaceTimeSource(string spaceTimeFile, SourceCurrents currents,
    const vector<Region> & regions, const vector<Duration> & durations)
{
    return new CurrentSourceDescription(currents, "", "", spaceTimeFile,
        regions, durations);
}

CurrentSourceDescription* CurrentSourceDescription::
newFormulaSource(string formula, SourceCurrents currents,
    const vector<Region> & regions, const vector<Duration> & durations)
{
    return new CurrentSourceDescription(currents, formula, "", "",
        regions, durations);
}

void CurrentSourceDescription::
cycleCoordinates()
{
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
	mCoordinatePermutationNumber += 1;
	mCoordinatePermutationNumber %= 3;
    
    for (unsigned int nn = 0; nn < mRegions.size(); nn++)
        mRegions[nn].cycleCoordinates();
    
    mCurrents.cycleCoordinates();
    
    LOG << "Not permuting coordinates of source formula!!\n";
}


CurrentSourceDescription::
CurrentSourceDescription(SourceCurrents currents, string formula,
    string timeFile, string spaceTimeFile, const vector<Region> & regions,
    const vector<Duration> & durations) throw(Exception) :
    mCoordinatePermutationNumber(0),
    mFormula(formula),
    mTimeFile(timeFile),
    mSpaceFileDoThisLaterOkay("whatever you say, man"),
    mSpaceTimeFile(spaceTimeFile),
    mCurrents(currents),
    mRegions(regions),
    mDurations(durations)
{
}


#pragma mark *** HuygensSurface ***

HuygensSurfaceDescription::
HuygensSurfaceDescription() :
    mCoordinatePermutationNumber(0),
    mBuffers(6),
    mDirection(0,0,0),
    mSymmetries(0,0,0)
{
}

HuygensSurfaceDescription::
HuygensSurfaceDescription(HuygensSurfaceSourceType type) :
    mCoordinatePermutationNumber(0),
    mType(type),
    mBuffers(6),
    mDirection(0,0,0),
    mSymmetries(0,0,0)
{
}

HuygensSurfaceDescription::
HuygensSurfaceDescription(const HuygensSurfaceDescription & parent,
    Rect3i newHalfCells)
{
    *this = parent;
    mHalfCells = newHalfCells;
}

void HuygensSurfaceDescription::
setPointers(const Map<string, GridDescPtr> & gridMap)
{
	assert(mType == kLink);
	mSourceGrid = gridMap[mSourceGridName];
    
    if (!mSourceGrid->getHalfCellBounds().encloses(mFromHalfCells))
        throw(Exception("Link fromYeeCells or fromHalfCells must be entirely"
            " contained within sourceGrid's bounds."));
}

void HuygensSurfaceDescription::
omitSide(int sideNum)
{
	Vector3i side = cardinalDirection(sideNum);
	mOmittedSides.insert(side);
}

void HuygensSurfaceDescription::
omitSide(Vector3i direction)
{
    mOmittedSides.insert(direction);
}

void HuygensSurfaceDescription::
cycleCoordinates()
{
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
	// Rotate the rects and vectors
	mHalfCells = permuteForward*mHalfCells;
    mDirection = permuteForward*mDirection;
	mSymmetries = permuteForward*mSymmetries;
	mFromHalfCells = permuteForward*mFromHalfCells;
	
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
    
    mCoordinatePermutationNumber += 1;
    mCoordinatePermutationNumber %= 3;
}

HuygensSurfaceDescription* HuygensSurfaceDescription::
newTFSFTimeSource(SourceFields fields, string timeFile, Vector3i direction,
    Rect3i halfCells, set<Vector3i> omittedSides, bool isTF)
{
	if (!vec_ge(halfCells.size(), 0))
		throw(Exception("TFSFSource rect has some negative dimensions"));
    if (direction != dominantComponent(direction))
        throw(Exception("direction needs to be an axis-oriented unit vector."));
    
    HuygensSurfaceDescription* hs2 = new HuygensSurfaceDescription(kTFSFSource);
    hs2->mFields = fields;
    hs2->mTimeFile = timeFile;
    hs2->mDirection = direction;
    for (int nn = 0; nn < 3; nn++)
    if (direction[nn] == 0)
        hs2->mSymmetries[nn] = 1;
    hs2->mHalfCells = halfCells;
    hs2->mOmittedSides = omittedSides;
    hs2->mIsTotalField = isTF;
	
	if (isTF)
		hs2->initTFSFBuffers(1.0);
	else
		hs2->initTFSFBuffers(-1.0);
    
    return hs2;
}

HuygensSurfaceDescription* HuygensSurfaceDescription::
newTFSFFormulaSource(SourceFields fields, string formula, Vector3i direction,
    Rect3i halfCells, set<Vector3i> omittedSides, bool isTF)
{
	if (!vec_ge(halfCells.size(), 0))
		throw(Exception("TFSFSource rect has some negative dimensions"));
    if (direction != dominantComponent(direction))
        throw(Exception("direction needs to be an axis-oriented unit vector."));
    LOG << "Warning: not validating formula.\n";
    
    HuygensSurfaceDescription* hs2 = new HuygensSurfaceDescription(kTFSFSource);
    hs2->mFields = fields;
    hs2->mFormula = formula;
    hs2->mDirection = direction;
    for (int nn = 0; nn < 3; nn++)
    if (direction[nn] == 0)
        hs2->mSymmetries[nn] = 1;
    hs2->mDuration = Duration();
    hs2->mHalfCells = halfCells;
    hs2->mOmittedSides = omittedSides;
    hs2->mIsTotalField = isTF;
	
	if (isTF)
		hs2->initTFSFBuffers(1.0);
	else
		hs2->initTFSFBuffers(-1.0);
    
    return hs2;
}

HuygensSurfaceDescription* HuygensSurfaceDescription::
newCustomTFSFSource(string file, Vector3i symmetries, Rect3i halfCells,
    Duration duration, set<Vector3i> omittedSides, bool isTF)
{
	if (!vec_ge(halfCells.size(), 0))
		throw(Exception("TFSFSource rect has some negative dimensions"));
    
    HuygensSurfaceDescription* hs2 =
        new HuygensSurfaceDescription(kCustomTFSFSource);
    hs2->mFile = file;
    hs2->mDuration = duration;
    hs2->mSymmetries = symmetries;
    hs2->mHalfCells = halfCells;
    hs2->mOmittedSides = omittedSides;
    hs2->mIsTotalField = isTF;
    
	if (isTF)
		hs2->initTFSFBuffers(1.0);
	else
		hs2->initTFSFBuffers(-1.0);
    
    return hs2;
}

HuygensSurfaceDescription* HuygensSurfaceDescription::
newLink(string sourceGrid, Rect3i fromHalfCells, Rect3i toHalfCells,
    set<Vector3i> omittedSides, bool isTF)
{
	if (!vec_ge(fromHalfCells.size(), 0))
		throw(Exception("Link from rect has some negative dimensions"));
	if (!vec_ge(toHalfCells.size(), 0))
		throw(Exception("Link to rect has some negative dimensions"));
    for (int nn = 0; nn < 3; nn++)
    if (fromHalfCells.size(nn) != toHalfCells.size(nn) &&
            fromHalfCells.size(nn) != 1)
        throw(Exception("All dimensions of fromYeeCells or fromHalfCells must"
            " either be the same as toYeeCells/toHalfCells or span the entire"
            " dimension of the source grid, which must be one Yee cell across"
            " in that direction."));
    
    HuygensSurfaceDescription* hs2 = new HuygensSurfaceDescription(kLink);
    hs2->mSourceGridName = sourceGrid;
    hs2->mFromHalfCells = fromHalfCells;
    hs2->mHalfCells = toHalfCells;
    hs2->mOmittedSides = omittedSides;
    hs2->mIsTotalField = isTF;
    
	if (isTF)
		hs2->initTFSFBuffers(1.0);
	else
		hs2->initTFSFBuffers(-1.0);
    
    return hs2;
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
			mHalfCells, nDir, srcFactor));
		mBuffers[nDir] = nb;
	}
}

void HuygensSurfaceDescription::
initFloquetBuffers()
{
	LOG << "Not doing anything for Floquet buffers.\n";
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
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
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
MaterialDescription(string name, string inModelName,
	const Map<string,string> & inParams,
    const Map<Vector3i, Map<string, string> > & inPMLParams) throw(Exception) :
    mCoordinatePermutationNumber(0),
	mName(name),
	mModelName(inModelName),
	mParams(inParams),
    mPMLParams(inPMLParams)
{
	cerr << "Warning: MaterialDescription does not validate model name.\n";
}

void MaterialDescription::
cycleCoordinates()
{
	mCoordinatePermutationNumber = (mCoordinatePermutationNumber+1)%3;
    
    Map<Vector3i, Map<string, string> > newPMLParams;
    
    for (int sideNum = 0; sideNum < 6; sideNum++)
        newPMLParams[cyclicPermute(cardinalDirection(sideNum), 1)] =
            mPMLParams[cardinalDirection(sideNum)];
    
    mPMLParams = newPMLParams;
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
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
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
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
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
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
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
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
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
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
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
	Mat3i permuteForward(Mat3i::cyclicPermutation());
    Mat3i permuteBackward(inverse(permuteForward));
	
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



