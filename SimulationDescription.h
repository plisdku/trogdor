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
	void setDuration(int numT);
	
	void cycleCoordinates();
	
	const std::vector<GridDescPtr> & getGrids() const { return mGrids; }
	const std::vector<MaterialDescPtr> & getMaterials() const {
		return mMaterials; }
	
    const std::string & getVersion() const { return mVersionString; }
	float getDt() const { return m_dt; }
	Vector3f getDxyz() const { return m_dxyz; }
	int getDuration() const { return mNumTimesteps; }
	
private:
    std::string mVersionString;
	std::vector<GridDescPtr> mGrids;
	std::vector<MaterialDescPtr> mMaterials;
	
	float m_dt;
	Vector3f m_dxyz;
	int mNumTimesteps;
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
	void setSources(const std::vector<SourceDescPtr> & sources)
        { mSources = sources; }
	void setHuygensSurfaces(
		const std::vector<HuygensSurfaceDescPtr> & surfaces);
	void setAssembly(AssemblyDescPtr assembly) {
		mAssembly = assembly; }
    void setPMLParams(const Map<Vector3i, Map<std::string, std::string> > & p)
        throw(Exception);
    
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap,
		const Map<std::string, GridDescPtr> & gridMap);
	
	void cycleCoordinates();  // rotate entire sim, x->y, y->z, z->x
	
	// Accessors
	const std::string & getName() const { return mName; }
	Vector3i getNumYeeCells() const { return mNumYeeCells; }
	Vector3i getNumHalfCells() const { return mNumHalfCells; }
	Rect3i getYeeBounds() const;
	Rect3i getHalfCellBounds() const;
	const Rect3i & getCalcHalfCells() const { return mCalcRegionHalf; }
	const Rect3i & getNonPMLHalfCells() const { return mNonPMLHalf; }
    const Map<Vector3i, Map<std::string, std::string> > & getPMLParams() const
        { return mPMLParams; }
	Vector3i getOriginYee() const { return mOriginYee; }
	int getNumDimensions() const;
    Vector3f getDxyz() const { return mDxyz; }
    float getDt() const { return mDt; }
	
	const std::vector<OutputDescPtr> & getOutputs() const { return mOutputs; }
	const std::vector<SourceDescPtr> & getSources() const { return mSources; }
    //const std::vector<SourceDesc2Ptr> & getSources2() const
    //    { return mSources2; }
	const std::vector<HuygensSurfaceDescPtr> & getHuygensSurfaces() const
		{ return mHuygensSurfaces; }
	std::vector<HuygensSurfaceDescPtr> & getHuygensSurfaces()
		{ return mHuygensSurfaces; }
	const AssemblyDescPtr getAssembly() const { return mAssembly; }
	
private:
	std::string mName;
	
	Vector3i mNumYeeCells;
	Vector3i mNumHalfCells;
	Rect3i mCalcRegionHalf;
	Rect3i mNonPMLHalf;
    
    Vector3f mDxyz;
    float mDt;
    
    Map<Vector3i, Map<std::string, std::string> > mPMLParams;
	
	Vector3i mOriginYee;
	
	std::vector<OutputDescPtr> mOutputs;
	std::vector<SourceDescPtr> mSources;
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
    
    int getFirst() const { return mFirst; }
    int getLast() const { return mLast; }
    int getPeriod() const { return mPeriod; }
    
    void setFirst(int first) { mFirst = first; }
    void setLast(int last) { mLast = last; }
    void setPeriod(int period) { mPeriod = period; }
    
private:
    int mFirst;
    int mLast;
    int mPeriod;
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
    
    void cycleCoordinates();
    
    void setYeeCells(const Rect3i & rect) { mYeeCells = rect; }
    void setStride(const Vector3i & stride) { mStride = stride; }
    const Rect3i & getYeeCells() const { return mYeeCells; }
    const Vector3i & getStride() const { return mStride; }
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
        
	void cycleCoordinates();  // rotate x->y, y->z, z->x
    int getPermutation() const { return mCoordinatePermutationNumber; }
    
    const std::string & getFile() const { return mFile; }
    Vector3i getWhichE() const { return mWhichE; }
    Vector3i getWhichH() const { return mWhichH; }
    Vector3i getWhichJ() const { return mWhichJ; }
    Vector3i getWhichP() const { return mWhichP; }
    Vector3i getWhichM() const { return mWhichM; }
    bool isInterpolated() const { return mIsInterpolated; }
    Vector3f getInterpolationPoint() const { return mInterpolationPoint; }
    const std::vector<Region> & getRegions() const { return mRegions; }
    const std::vector<Duration> & getDurations() const { return mDurations; }
    
private:
    void determineWhichFields(std::string fields) throw(Exception);
    int mCoordinatePermutationNumber;
    std::string mFile;
    Vector3i mWhichE;
    Vector3i mWhichH;
    Vector3i mWhichJ;
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
    
	void cycleCoordinates();  // rotate x->y, y->z, z->x
private:
};

class SourceFields
{
public:
    SourceFields();
    SourceFields(std::string fields);
    SourceFields(std::string fields, Vector3f polarization);
    
    void cycleCoordinates();
    
    bool usesPolarization() const { return mUsesPolarization; }
    Vector3f getPolarization() const
        { assert(mUsesPolarization); return mPolarization; }
    Vector3i getWhichE() const { return mWhichE; }
    Vector3i getWhichH() const { return mWhichH; }
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
        std::string timeFile, std::string spaceTimeFile,
        bool isSoft, const std::vector<Region> & regions,
        const std::vector<Duration> & durations) throw(Exception);
    
    void cycleCoordinates();
    
    const std::string & getFormula() const { return mFormula; }
    const std::string & getTimeFile() const { return mTimeFile; }
    const std::string & getSpaceTimeFile() const { return mSpaceTimeFile; }
    const SourceFields & getSourceFields() const { return mFields; }
    bool isHardSource() const { return !mIsSoft; }
    bool isSoftSource() const { return mIsSoft; }
    bool isSpaceVarying() const { return (mSpaceTimeFile != ""); }
    
    const std::vector<Region> & getRegions() const { return mRegions; }
    const std::vector<Duration> & getDurations() const { return mDurations; }
    
private:
    int mCoordinatePermutationNumber;
    
    std::string mFormula;
    std::string mTimeFile;
    std::string mSpaceFileDoThisLaterOkay;
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
    
    void cycleCoordinates();
    
    bool usesPolarization() const { return mUsesPolarization; }
    Vector3f getPolarization() const
        { assert(mUsesPolarization); return mPolarization; }
    Vector3i getWhichJ() const { return mWhichJ; }
    Vector3i getWhichK() const { return mWhichK; }
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
    static CurrentSourceDescription* newSpaceTimeSource(
        std::string spaceTimeFile, SourceCurrents currents, 
        const std::vector<Region> & regions,
        const std::vector<Duration> & durations);
    static CurrentSourceDescription* newFormulaSource(std::string formula,
        SourceCurrents fields, const std::vector<Region> & regions,
        const std::vector<Duration> & durations);
    
	void cycleCoordinates();  // rotate x->y, y->z, z->x
    
    const std::string & getFormula() const { return mFormula; }
    const std::string & getTimeFile() const { return mTimeFile; }
    const std::string & getSpaceTimeFile() const { return mSpaceTimeFile; }
    const SourceCurrents & getSourceCurrents() const { return mCurrents; }
private:
    CurrentSourceDescription(SourceCurrents currents, std::string formula,
        std::string timeFile, std::string spaceTimeFile,
        const std::vector<Region> & regions,
        const std::vector<Duration> & durations) throw(Exception);
    
    int mCoordinatePermutationNumber;
    std::string mFormula;
    std::string mTimeFile;
    std::string mSpaceFileDoThisLaterOkay;
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
    void cycleCoordinates();
    void becomeLink(GridDescPtr sourceGrid,
        const Rect3i & sourceHalfCells);
    
    // Common accessors
    HuygensSurfaceSourceType getType() const { return mType; }
    const Rect3i & getHalfCells() const { return mHalfCells; }
    const std::set<Vector3i> & getOmittedSides() const { return mOmittedSides; }
    bool isTotalField() const { return mIsTotalField; }
    bool isScatteredField() const { return !mIsTotalField; }
	const std::vector<NeighborBufferDescPtr> & getBuffers() const
        { return mBuffers; }
    
    // TFSFSource accessors
    SourceFields getSourceFields() const
        { assert(mType == kTFSFSource); return mFields; }
    std::string getTimeFile() const
        { assert(mType == kTFSFSource); return mTimeFile; }
    std::string getFormula() const
        { assert(mType == kTFSFSource); return mFormula; }
    Vector3i getDirection() const
        { assert(mType == kTFSFSource); return mDirection; }
    
    // Custom or ordinary TFSFSource accessor
    Vector3i getSymmetries() const
        { assert(mType == kTFSFSource || mType == kCustomTFSFSource);
          return mSymmetries; }
    Duration getDuration() const
        { assert(mType == kCustomTFSFSource || mType == kTFSFSource);
          return mDuration; }
    
    // Custom source accessors
    std::string getFile() const
        { assert(mType == kCustomTFSFSource); return mFile; }
    
    // Link accessors
    std::string getSourceGridName() const
        { assert(mType == kLink); return mSourceGridName; }
    GridDescPtr getSourceGrid() const
        { assert(mType == kLink); return mSourceGrid; }
    Rect3i getFromHalfCells() const { return mFromHalfCells; }
    
private:
    // buffer maker
	void initTFSFBuffers(float srcFactor); // 1 adds, -1 subtracts
	void initLinkTFSFBuffers(float srcFactor); // 1 adds, -1 subtracts
	void initFloquetBuffers();
    
    // Common data
    int mCoordinatePermutationNumber;
    HuygensSurfaceSourceType mType;
	std::vector<NeighborBufferDescPtr> mBuffers; // one per side, 0-5
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

class NeighborBufferDescription
{
public:
	// add constructors for things like Floquet boundaries
	NeighborBufferDescription(const Rect3i & huygensDestHalfCells, int nSide,
		float incidentFieldFactor);
    NeighborBufferDescription(const Rect3i & huygensSourceHalfCells,
        const Rect3i & huygensDestHalfCells, int nSide,
        float incidentFieldFactor);
	
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
    const Rect3i & getSourceHalfRect() const { return mSourceHalfRect; }
	const Rect3i & getDestHalfRect() const { return mDestHalfRect; }
	const Rect3i & getBufferHalfRect() const { return mBufferHalfRect; }
	//const Rect3i & getBufferYeeBounds() const { return mBufferYeeBounds; }
    
    const std::vector<float> & getDestFactorsE() const
        { return mDestFactorsE; }
    const std::vector<float> & getSourceFactorsE() const
        { return mSrcFactorsE; }
    const std::vector<float> & getDestFactorsH() const
        { return mDestFactorsH; }
    const std::vector<float> & getSourceFactorsH() const
        { return mSrcFactorsH; }
    
    void setSourceRects(const Rect3i & sourceHalfCells, int nSide);
	
private:
    Rect3i getEdgeHalfCells(const Rect3i & halfCells, int nSide);
    void initFactors(const Rect3i & huygensDestHalfCells, int nSide,
        float incFieldFactor);
    
    Rect3i mSourceHalfRect; // only used for links of course
	Rect3i mDestHalfRect;
	Rect3i mBufferHalfRect;
	//Rect3i mBufferYeeBounds;
	std::vector<float> mDestFactorsE;
	std::vector<float> mSrcFactorsE;
	std::vector<float> mDestFactorsH;
	std::vector<float> mSrcFactorsH;
};

class MaterialDescription
{
public:
	MaterialDescription(std::string name, std::string inModelName,
		const Map<std::string, std::string> & inParams,
        const Map<Vector3i, Map<std::string, std::string> > & inPMLParams)
        throw(Exception);
	
	std::string getName() const { return mName; }
	std::string getModelName() const { return mModelName; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
    const Map<Vector3i, Map<std::string, std::string> > & getPMLParams() const
        { return mPMLParams; }
	
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	friend std::ostream & operator<<(std::ostream & out,
		const MaterialDescription & mat);
private:
	int mCoordinatePermutationNumber; // 0,1 or 2
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
	
	void cycleCoordinates();
	
	const std::vector<InstructionPtr> & getInstructions() const
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
    
	Vector3i getColor() const { return mColor; }
	const std::string & getMaterialName() const { return mMaterialName; }
	const MaterialDescPtr & getMaterial() const { return mMaterial; }
	FillStyle getFillStyle() const { return mFillStyle; }
	
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
	InstructionType getType() const { return mType; }
    virtual ~Instruction() {}
	
	virtual void cycleCoordinates();  // rotate x->y, y->z, z->x
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
    
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	const Rect3i & getYeeRect() const;
	const Rect3i & getHalfRect() const;
	FillStyle getFillStyle() const { return mStyle; }
	const std::string & getMaterialName() const { return mMaterialName; }
	const MaterialDescPtr & getMaterial() const { return mMaterial; }
	
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
	
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	const Rect3i & getYeeRect() const { return mYeeRect; }
	Vector3i getRowDirection() const { return mRow; }
	Vector3i getColDirection() const { return mCol; }
	const std::string & getImageFileName() const { return mImageFileName; }
	const std::vector<ColorKey> & getKeys() const { return mKeys; }
	
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
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	const Rect3i & getYeeRect() const { return mYeeRect; }
	FillStyle getFillStyle() const { return mStyle; }
	const std::string & getMaterialName() const { return mMaterialName; }
	const MaterialDescPtr & getMaterial() const { return mMaterial; }
	const std::string & getImageFileName() const { return mImageFileName; }
	Vector3i getRowDirection() const { return mRow; }
	Vector3i getColDirection() const { return mCol; }
	Vector3i getUpDirection() const { return mUp; }
	
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
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	//const Rect3i & getFillRect() const { return mFillRect; }
	const Rect3i & getYeeRect() const;
	const Rect3i & getHalfRect() const;
	FillStyle getFillStyle() const { return mStyle; }
	const std::string & getMaterialName() const { return mMaterialName; }
	const MaterialDescPtr & getMaterial() const { return mMaterial; }
	
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
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	const Rect3i & getSourceHalfRect() const { return mSourceRect; }
	const Rect3i & getDestHalfRect() const { return mDestRect; }
	const std::string & getGridName() const { return mGridName; }
	const GridDescPtr & getGrid() const { return mGrid; }
	
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
	
	void cycleCoordinates();
	
	const Rect3i & getExtrudeFrom() const { return mExtrudeFrom; }
	const Rect3i & getExtrudeTo() const { return mExtrudeTo; }
private:
	Rect3i mExtrudeFrom;
	Rect3i mExtrudeTo;
};




#endif
