/*
 *  SimulationDescription.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 1/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SIMULATIONDESCRIPTION_
#define _SIMULATIONDESCRIPTION_

#include "SimulationDescriptionPredeclarations.h"
#include "geometry.h"
#include "SetupConstants.h"
#include "Pointer.h"

#include "Map.h"
#include "Exception.h"

#include <climits>
#include <vector>
#include <string>
#include <set>

class XMLParameterFile;

class SimulationDescription
{
	friend class XMLParameterFile;
public:
	SimulationDescription(const XMLParameterFile & file) throw(Exception);
	
	void setGrids(const std::vector<GridDescPtr> & grids) {
		mGrids = grids; }
	void setMaterials(const std::vector<MaterialDescPtr> & materials) {
		mMaterials = materials; }
	void setAllPointers();
	
	void setDiscretization(Vector3f dxyz, float dt);
	void setNumTimesteps(int numT);
	
	const std::vector<GridDescPtr> & grids() const { return mGrids; }
	const std::vector<MaterialDescPtr> & materials() const {
		return mMaterials; }
	
    const std::string & version() const { return mVersionString; }
	float dt() const { return m_dt; }
	Vector3f dxyz() const { return m_dxyz; }
	int numTimesteps() const { return mNumTimesteps; }
	
private:
    std::string mVersionString;
	std::vector<GridDescPtr> mGrids;
	std::vector<MaterialDescPtr> mMaterials;
	
	float m_dt;
	Vector3f m_dxyz;
	long mNumTimesteps;
};

class GridDescription
{
public:
	GridDescription(std::string name, Vector3i numYeeCells,
        Rect3i calcRegionHalf, Rect3i nonPMLHalf,
		Vector3i originYee, Vector3f mDxyz, float mDt)
		throw(Exception);
	
	// Mutators
	void setOutputs(const std::vector<OutputDescPtr> & outputs)
        { mOutputs = outputs; }
	void setGridReports(const std::vector<GridReportDescPtr> & gridReports);
	void setSources(const std::vector<SourceDescPtr> & sources)
        { mSources = sources; }
    void setCurrentSources(const std::vector<CurrentSourceDescPtr> & currents)
        { mCurrentSources = currents; }
	void setHuygensSurfaces(
		const std::vector<HuygensSurfaceDescPtr> & surfaces);
	void setAssembly(AssemblyDescPtr assembly) {
		mAssembly = assembly; }
    void setPMLParams(const Map<Vector3i, Map<std::string, std::string> > & p)
        throw(Exception);
    
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap,
		const Map<std::string, GridDescPtr> & gridMap);
	
	// Accessors
	const std::string & name() const { return mName; }
	Vector3i numYeeCells() const { return mNumYeeCells; }
	Vector3i numHalfCells() const { return mNumHalfCells; }
	Rect3i yeeBounds() const;
	Rect3i halfCellBounds() const;
	const Rect3i & calcHalfCells() const { return mCalcRegionHalf; }
	const Rect3i & nonPMLHalfCells() const { return mNonPMLHalf; }
    const Map<Vector3i, Map<std::string, std::string> > & pmlParams() const
        { return mPMLParams; }
	Vector3i originYee() const { return mOriginYee; }
	int numDimensions() const;
    Vector3f dxyz() const { return mDxyz; }
    float dt() const { return mDt; }
	
	const std::vector<OutputDescPtr> & outputs() const { return mOutputs; }
    const std::vector<GridReportDescPtr> & gridReports() const
        { return mGridReports; }
	const std::vector<SourceDescPtr> & sources() const { return mSources; }
    const std::vector<CurrentSourceDescPtr> & currentSources() const
        { return mCurrentSources; }
	const std::vector<HuygensSurfaceDescPtr> & huygensSurfaces() const
		{ return mHuygensSurfaces; }
	std::vector<HuygensSurfaceDescPtr> & huygensSurfaces()
		{ return mHuygensSurfaces; }
	const AssemblyDescPtr assembly() const { return mAssembly; }
	
private:
	std::string mName;
	
	Vector3i mNumYeeCells;
	Vector3i mNumHalfCells;
	Rect3i mCalcRegionHalf;
	Rect3i mNonPMLHalf;
    
	Vector3i mOriginYee;
    Vector3f mDxyz;
    float mDt;
    
    Map<Vector3i, Map<std::string, std::string> > mPMLParams;
	
	std::vector<OutputDescPtr> mOutputs;
    std::vector<GridReportDescPtr> mGridReports;
	std::vector<SourceDescPtr> mSources;
    std::vector<CurrentSourceDescPtr> mCurrentSources;
    std::vector<HuygensSurfaceDescPtr> mHuygensSurfaces;
	AssemblyDescPtr mAssembly;
};

class Duration
{
public:
    Duration() :
        mFirst(0),
        mLast(INT_MAX),
        mPeriod(1) {}
    Duration(int firstTimestep, int lastTimestep, int period = 1) :
        mFirst(firstTimestep),
        mLast(lastTimestep),
        mPeriod(period) {}
    static Duration from(int firstTimestep, int period = 1)
    {
        return Duration(firstTimestep, INT_MAX, period);
    }
    static Duration periodic(int period)
    {
        return Duration(0, INT_MAX, period);
    }
    
    long first() const { return mFirst; }
    long last() const { return mLast; }
    long period() const { return mPeriod; }
    
    void setFirst(int first) { mFirst = first; }
    void setLast(int last) { mLast = last; }
    void setPeriod(int period) { mPeriod = period; }
    
private:
    long mFirst;
    long mLast;
    long mPeriod;
};
//std::ostream & operator<<(std::ostream & str, const Duration & dur);

class Region
{
public:
    Region() :
        mYeeCells(-INT_MAX, -INT_MAX, -INT_MAX, INT_MAX, INT_MAX, INT_MAX),
        mStride(1,1,1) {}
    Region(const Rect3i & yeeCells) :
        mYeeCells(yeeCells),
        mStride(1,1,1) {}
    Region(const Rect3i & yeeCells, Vector3i stride) :
        mYeeCells(yeeCells),
        mStride(stride) {}
    
    void setYeeCells(const Rect3i & rect) { mYeeCells = rect; }
    void setStride(const Vector3i & stride) { mStride = stride; }
    const Rect3i & yeeCells() const { return mYeeCells; }
    const Vector3i & stride() const { return mStride; }
private:
    Rect3i mYeeCells;
    Vector3i mStride;
};
//std::ostream & operator<<(std::ostream & str, const Region & reg);

class OutputDescription
{
public:
    OutputDescription(std::string fields, std::string file)
        throw(Exception);
    OutputDescription(std::string fields, std::string file,
        Region region, Duration duration = Duration()) throw(Exception);
    OutputDescription(std::string fields, std::string file,
        const std::vector<Region> & regions,
        const std::vector<Duration> & durations) throw(Exception);
    OutputDescription(std::string fields, std::string file,
        Vector3f interpolationPoint, const std::vector<Region> & regions,
        const std::vector<Duration> & durations) throw(Exception);
    ~OutputDescription();
    
    const std::string & file() const { return mFile; }
    Vector3i whichE() const { return mWhichE; }
    Vector3i whichH() const { return mWhichH; }
    Vector3i whichJ() const { return mWhichJ; }
    Vector3i whichK() const { return mWhichK; }
    Vector3i whichP() const { return mWhichP; }
    Vector3i whichM() const { return mWhichM; }
    bool isInterpolated() const { return mIsInterpolated; }
    Vector3f interpolationPoint() const { return mInterpolationPoint; }
    const std::vector<Region> & regions() const { return mRegions; }
    const std::vector<Duration> & durations() const { return mDurations; }
    
private:
    void determineWhichFields(std::string fields) throw(Exception);
    std::string mFile;
    Vector3i mWhichE;
    Vector3i mWhichH;
    Vector3i mWhichJ;
    Vector3i mWhichK;
    Vector3i mWhichP;
    Vector3i mWhichM;
    bool mIsInterpolated;
    Vector3f mInterpolationPoint;
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
};

class MaterialOutputDescription
{
public:
    MaterialOutputDescription();
private:
};

class GridReportDescription
{
public:
    GridReportDescription();
    GridReportDescription(std::string fields, std::string file,
        std::vector<Region> regions);
    GridReportDescription(std::string file, std::vector<Region> regions);
    
    std::string fileName() const { return mFileName; }
    std::vector<Region> regions() const { return mRegions; }
    std::vector<Region> & regions() { return mRegions; }
    bool usesOctant(int octant) const;
private:
    std::string mFileName;
    std::vector<Region> mRegions;
    bool mOctants[8];
};

class SourceFields
{
public:
    SourceFields();
    SourceFields(std::string fields);
    SourceFields(std::string fields, Vector3f polarization);
    
    bool usesPolarization() const { return mUsesPolarization; }
    Vector3f polarization() const
        { assert(mUsesPolarization); return mPolarization; }
    Vector3i whichE() const { return mWhichE; }
    Vector3i whichH() const { return mWhichH; }
private:
    bool mUsesPolarization;
    Vector3f mPolarization;
    Vector3i mWhichE;
    Vector3i mWhichH;
};

class SourceDescription
{
public:
    static SourceDescription* newTimeSource(std::string timeFile,
        SourceFields fields, bool isSoft, const std::vector<Region> & regions,
            const std::vector<Duration> & durations);
    static SourceDescription* newSpaceTimeSource(std::string spaceTimeFile,
        SourceFields fields, bool isSoft, const std::vector<Region> & regions,
        const std::vector<Duration> & durations);
    static SourceDescription* newFormulaSource(std::string formula,
        SourceFields fields, bool isSoft, const std::vector<Region> & regions,
        const std::vector<Duration> & durations);
    
    SourceDescription(SourceFields fields, std::string formula,
        std::string timeFile, std::string spaceFile, std::string spaceTimeFile,
        bool isSoft, const std::vector<Region> & regions,
        const std::vector<Duration> & durations) throw(Exception);
    
    const std::string & formula() const { return mFormula; }
    const std::string & timeFile() const { return mTimeFile; }
    const std::string & spaceFile() const { return mSpaceFile; }
    const std::string & spaceTimeFile() const { return mSpaceTimeFile; }
    const SourceFields & sourceFields() const { return mFields; }
    
    bool isHardSource() const { return !mIsSoft; }
    bool isSoftSource() const { return mIsSoft; }
    bool hasMask() const { return (mSpaceFile != ""); }
    bool isSpaceVarying() const { return (mSpaceTimeFile != ""); }
    
    const std::vector<Region> & regions() const { return mRegions; }
    const std::vector<Duration> & durations() const { return mDurations; }
    
private:
    std::string mFormula;
    std::string mTimeFile;
    std::string mSpaceFile;
    std::string mSpaceTimeFile;
    SourceFields mFields;
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
    
    bool mIsSoft;
};

// This incredibly goofy class is exactly the same as SourceFields but uses
// "j" and "k" instead of "e" and "h".  That's all.  I'm so silly.
class SourceCurrents
{
public:
    SourceCurrents();
    SourceCurrents(std::string fields);
    SourceCurrents(std::string fields, Vector3f polarization);
    
    bool usesPolarization() const { return mUsesPolarization; }
    Vector3f polarization() const
        { assert(mUsesPolarization); return mPolarization; }
    Vector3i whichJ() const { return mWhichJ; }
    Vector3i whichK() const { return mWhichK; }
private:
    bool mUsesPolarization;
    Vector3f mPolarization;
    Vector3i mWhichJ;
    Vector3i mWhichK;
};

class CurrentSourceDescription
{
public:
    static CurrentSourceDescription* newTimeSource(std::string timeFile,
        SourceCurrents currents, const std::vector<Region> & regions,
            const std::vector<Duration> & durations);
    static CurrentSourceDescription* newMaskedTimeSource(std::string timeFile,
        std::string spaceFile, SourceCurrents currents,
        const std::vector<Region> & regions,
        const std::vector<Duration> & durations);
    static CurrentSourceDescription* newSpaceTimeSource(
        std::string spaceTimeFile, SourceCurrents currents, 
        const std::vector<Region> & regions,
        const std::vector<Duration> & durations);
    static CurrentSourceDescription* newFormulaSource(std::string formula,
        SourceCurrents fields, const std::vector<Region> & regions,
        const std::vector<Duration> & durations);
    
    bool hasMask() const
        { return (mSpaceFile != ""); }
    bool isSpaceVarying() const
        { return (mSpaceTimeFile != ""); }
    const std::string & formula() const { return mFormula; }
    const std::string & timeFile() const { return mTimeFile; }
    const std::string & spaceFile() const { return mSpaceFile; }
    const std::string & spaceTimeFile() const { return mSpaceTimeFile; }
    const SourceCurrents & sourceCurrents() const { return mCurrents; }
    
    const std::vector<Region> & regions() const { return mRegions; }
    const std::vector<Duration> & durations() const { return mDurations; }
private:
    CurrentSourceDescription(SourceCurrents currents, std::string formula,
        std::string timeFile, std::string spaceFile, std::string spaceTimeFile,
        const std::vector<Region> & regions,
        const std::vector<Duration> & durations) throw(Exception);
    
    std::string mFormula;
    std::string mTimeFile;
    std::string mSpaceFile;
    std::string mSpaceTimeFile;
    SourceCurrents mCurrents;
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
};

enum HuygensSurfaceSourceType
{
	kLink,
	kTFSFSource,
	kCustomTFSFSource
};

class HuygensSurfaceDescription
{
public:
    HuygensSurfaceDescription();
    HuygensSurfaceDescription(const HuygensSurfaceDescription & parent,
        Rect3i newHalfCells);
    HuygensSurfaceDescription(HuygensSurfaceSourceType type);
    
    static HuygensSurfaceDescription*
    newTFSFTimeSource(SourceFields fields,
        std::string timeFile, Vector3i direction, Rect3i halfCells,
        std::set<Vector3i> omittedSides, bool isTF = 1);
        
    static HuygensSurfaceDescription*
    newTFSFFormulaSource(SourceFields fields,
        std::string formula, Vector3i direction, Rect3i halfCells,
        std::set<Vector3i> omittedSides, bool isTF = 1);
    
    static HuygensSurfaceDescription*
    newCustomTFSFSource(std::string file,
        Vector3i symmetries, Rect3i halfCells, Duration duration,
        std::set<Vector3i> omittedSides, bool isTF = 1);
    
    static HuygensSurfaceDescription*
    newLink(std::string sourceGrid,
        Rect3i fromHalfCells, Rect3i toHalfCells,
        std::set<Vector3i> omittedSides, bool isTF = 1);
    
	// modifiers
	void setPointers(const Map<std::string, GridDescPtr> & gridMap);
	void omitSide(int nSide);
    void omitSide(Vector3i dir);
    void becomeLink(GridDescPtr sourceGrid,
        const Rect3i & sourceHalfCells);
    
    // Common accessors
    HuygensSurfaceSourceType type() const { return mType; }
    const Rect3i & halfCells() const { return mHalfCells; }
    const std::set<Vector3i> & omittedSides() const { return mOmittedSides; }
    bool isTotalField() const { return mIsTotalField; }
    bool isScatteredField() const { return !mIsTotalField; }
    
    // TFSFSource accessors
    SourceFields sourceFields() const
        { assert(mType == kTFSFSource); return mFields; }
    std::string timeFile() const
        { assert(mType == kTFSFSource); return mTimeFile; }
    std::string formula() const
        { assert(mType == kTFSFSource); return mFormula; }
    Vector3i direction() const
        { assert(mType == kTFSFSource); return mDirection; }
    
    // Custom or ordinary TFSFSource accessor
    Vector3i symmetries() const
        { assert(mType == kTFSFSource || mType == kCustomTFSFSource);
          return mSymmetries; }
    Duration duration() const
        { assert(mType == kCustomTFSFSource || mType == kTFSFSource);
          return mDuration; }
    
    // Custom source accessors
    std::string file() const
        { assert(mType == kCustomTFSFSource); return mFile; }
    
    // Link accessors
    std::string sourceGridName() const
        { assert(mType == kLink); return mSourceGridName; }
    GridDescPtr sourceGrid() const
        { assert(mType == kLink); return mSourceGrid; }
    Rect3i fromHalfCells() const { return mFromHalfCells; }
    
private:
    // Common data
    HuygensSurfaceSourceType mType;
    Rect3i mHalfCells;
    std::set<Vector3i> mOmittedSides;
    bool mIsTotalField;
    
    // Source data
    SourceFields mFields;
    std::string mTimeFile;
    std::string mFormula;
    Vector3i mDirection;
    
     // TFSF or custom TFSF source
    Vector3i mSymmetries;
    Duration mDuration;
    
    // Custom source data
    std::string mFile;
    
    // Link data
    std::string mSourceGridName;
    GridDescPtr mSourceGrid;
    Rect3i mFromHalfCells;
};

class MaterialDescription
{
public:
	MaterialDescription(int ID, std::string name, std::string inModelName,
		const Map<std::string, std::string> & inParams,
        const Map<Vector3i, Map<std::string, std::string> > & inPMLParams)
        throw(Exception);
	
    int id() const { return mID; }
	std::string name() const { return mName; }
	std::string modelName() const { return mModelName; }
	const Map<std::string, std::string> & params() const { return mParams; }
    const Map<Vector3i, Map<std::string, std::string> > & pmlParams() const
        { return mPMLParams; }
	
	friend std::ostream & operator<<(std::ostream & out,
		const MaterialDescription & mat);
private:
    int mID;
	std::string mName;
	std::string mModelName;
	Map<std::string, std::string> mParams;
    Map<Vector3i, Map<std::string, std::string> > mPMLParams;
};
std::ostream & operator<<(std::ostream & out, const MaterialDescription & mat);

#pragma mark *** Assembly things ***

class Instruction;
typedef Pointer<Instruction> InstructionPtr;

class AssemblyDescription
{
public:
	AssemblyDescription(const std::vector<InstructionPtr> & recipe)
		throw(Exception);
	
	void setInstructions(const std::vector<InstructionPtr> & instructions);
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap,
		const Map<std::string, GridDescPtr> & gridMap);
	
	const std::vector<InstructionPtr> & instructions() const
		{ return mInstructions; }
	
private:
	std::vector<InstructionPtr> mInstructions;
};

enum InstructionType
{
	kBlockType,
	kKeyImageType,
	kHeightMapType,
	kEllipsoidType,
	kCopyFromType,
	kExtrudeType
};
	
class ColorKey
{
public:
	ColorKey(std::string hexColor, std::string materialName,
		FillStyle style) throw(Exception);
	ColorKey(Vector3i rgbColor, std::string materialName, FillStyle style)
		throw(Exception);
    
	Vector3i color() const { return mColor; }
	const std::string & materialName() const { return mMaterialName; }
	const MaterialDescPtr & material() const { return mMaterial; }
	FillStyle fillStyle() const { return mFillStyle; }
	
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap);
private:
	Vector3i mColor;
	std::string mMaterialName;
	MaterialDescPtr mMaterial;
	FillStyle mFillStyle;
};

// The purpose of the Instruction base class is to provide some by-hand
// runtime type information without having to turn on RTTI, which I think
// is kind of evil.  (Well, C++ is evil.)
//
// A little RTTI will permit outside methods to determine which Instruction
// we are dealing with and handle each differently, while still receiving
// the Instructions in one vector or list.
class Instruction
{
public:
	Instruction(InstructionType inType);
	InstructionType type() const { return mType; }
    virtual ~Instruction() {}
    
protected:
	InstructionType mType;
};
typedef Pointer<Instruction> InstructionPtr;

class Block : public Instruction
{
public:
	Block(Rect3i halfCellRect, std::string material) throw(Exception);
	Block(Rect3i yeeCellRect, FillStyle style, std::string material)
		throw(Exception);
    
	const Rect3i & yeeRect() const;
	const Rect3i & halfRect() const;
	FillStyle fillStyle() const { return mStyle; }
	const std::string & materialName() const { return mMaterialName; }
	const MaterialDescPtr & material() const { return mMaterial; }
	
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap);
private:
	Rect3i mFillRect;
	FillStyle mStyle;
	std::string mMaterialName;
	MaterialDescPtr mMaterial;
};

class KeyImage : public Instruction
{
public:
	KeyImage(Rect3i yeeCellRect, std::string imageFileName,
		Vector3i rowDirection, Vector3i colDirection,
		std::vector<ColorKey> keys) throw(Exception);
		
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap);
	
	const Rect3i & yeeRect() const { return mYeeRect; }
	Vector3i rowDirection() const { return mRow; }
	Vector3i columnDirection() const { return mCol; }
	const std::string & imageFileName() const { return mImageFileName; }
	const std::vector<ColorKey> & keys() const { return mKeys; }
	
private:
	Rect3i mYeeRect;
	Vector3i mRow;
	Vector3i mCol;
	std::string mImageFileName;
	std::vector<ColorKey> mKeys;
};

class HeightMap : public Instruction
{
public:
	HeightMap(Rect3i yeeCellRect, FillStyle style, std::string material,
		std::string imageFileName, Vector3i rowDirection,
		Vector3i colDirection, Vector3i upDirection) throw(Exception);
    
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap);
    
	const Rect3i & yeeRect() const { return mYeeRect; }
	FillStyle fillStyle() const { return mStyle; }
	const std::string & materialName() const { return mMaterialName; }
	const MaterialDescPtr & material() const { return mMaterial; }
	const std::string & imageFileName() const { return mImageFileName; }
	Vector3i rowDirection() const { return mRow; }
	Vector3i columnDirection() const { return mCol; }
	Vector3i upDirection() const { return mUp; }
	
private:
	Rect3i mYeeRect;
	FillStyle mStyle;
	std::string mMaterialName;
	MaterialDescPtr mMaterial;
	std::string mImageFileName;
	Vector3i mRow;
	Vector3i mCol;
	Vector3i mUp;
};

class Ellipsoid : public Instruction
{
public:
	Ellipsoid(Rect3i halfCellRect, std::string material) throw(Exception);
	Ellipsoid(Rect3i yeeCellRect, FillStyle style, std::string material)
		throw(Exception);
	
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap);
	
	const Rect3i & yeeRect() const;
	const Rect3i & halfRect() const;
	FillStyle fillStyle() const { return mStyle; }
	const std::string & materialName() const { return mMaterialName; }
	const MaterialDescPtr & material() const { return mMaterial; }
	
private:
	Rect3i mFillRect;
	FillStyle mStyle;
	std::string mMaterialName;
	MaterialDescPtr mMaterial;
};

class CopyFrom : public Instruction
{
public:
	CopyFrom(Rect3i halfCellSourceRegion, Rect3i halfCellDestRegion,
		std::string gridName)
		throw(Exception);
	CopyFrom(Rect3i halfCellSourceRegion, Rect3i halfCellDestRegion,
		const GridDescPtr grid)
		throw(Exception);
	
	void setPointers(const Map<std::string, GridDescPtr> & gridMap);
	
	const Rect3i & sourceHalfRect() const { return mSourceRect; }
	const Rect3i & destHalfRect() const { return mDestRect; }
	const std::string & gridName() const { return mGridName; }
	const GridDescPtr & grid() const { return mGrid; }
	
private:
	Rect3i mSourceRect;
	Rect3i mDestRect;
	std::string mGridName;
	GridDescPtr mGrid;
};

class Extrude : public Instruction
{
public:
	Extrude(Rect3i halfCellExtrudeFrom, Rect3i halfCellExtrudeTo)
		throw(Exception);
	
	const Rect3i & extrudeFrom() const { return mExtrudeFrom; }
	const Rect3i & extrudeTo() const { return mExtrudeTo; }
private:
	Rect3i mExtrudeFrom;
	Rect3i mExtrudeTo;
};




#endif
