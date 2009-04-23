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

//#include "XMLParameterFile.h"
#include "geometry.h"
#include "SetupConstants.h"
#include "Pointer.h"

#include "Map.h"
#include "Exception.h"

#include <vector>
#include <string>
//#include <bitset>
#include <set>

class XMLParameterFile;

class SimulationDescription;
class GridDescription;
class InputEHDescription;
class OutputDescription;
class SourceDescription;
class HuygensSurfaceDescription;
class NeighborBufferDescription;
class MaterialDescription;
class AssemblyDescription;

typedef Pointer<SimulationDescription> SimulationDescPtr;
typedef Pointer<GridDescription> GridDescPtr;
typedef Pointer<InputEHDescription> InputEHDescPtr;
typedef Pointer<OutputDescription> OutputDescPtr;
typedef Pointer<SourceDescription> SourceDescPtr;
typedef Pointer<HuygensSurfaceDescription> HuygensSurfaceDescPtr;
typedef Pointer<NeighborBufferDescription> NeighborBufferDescPtr;
typedef Pointer<MaterialDescription> MaterialDescPtr;
typedef Pointer<AssemblyDescription> AssemblyDescPtr;

class SimulationDescription
{
	friend class XMLParameterFile;
public:
	SimulationDescription(const XMLParameterFile & file)  throw(Exception);
	
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
	
	float getDt() const { return m_dt; }
	Vector3f getDxyz() const { return m_dxyz; }
	int getDuration() const { return mNumTimesteps; }
	
private:
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
		Vector3i numHalfCells, Rect3i calcRegionHalf, Rect3i nonPMLHalf,
		Vector3i originYee)
		throw(Exception);
	
	// Mutators
	void setOutputs(const std::vector<OutputDescPtr> & outputs) {
		mOutputs = outputs; }
	void setInputs(const std::vector<InputEHDescPtr> & inputs) {
		mInputs = inputs; }
	void setSources(const std::vector<SourceDescPtr> & sources) {
		mSources = sources; }
	void setHuygensSurfaces(
		const std::vector<HuygensSurfaceDescPtr> & surfaces);
	void setAssembly(AssemblyDescPtr assembly) {
		mAssembly = assembly; }
	
	void setPointers(const Map<std::string, MaterialDescPtr> & materialMap,
		const Map<std::string, GridDescPtr> & gridMap);
	
	void cycleCoordinates();  // rotate entire sim, x->y, y->z, z->x
	
	// Accessors
	const std::string & getName() const { return mName; }
	Vector3i getNumYeeCells() const { return mNumYeeCells; }
	Vector3i getNumHalfCells() const { return mNumHalfCells; }
	Rect3i getYeeBounds() const;
	Rect3i getHalfCellBounds() const;
	const Rect3i & getCalcRegion() const { return mCalcRegionHalf; }
	const Rect3i & getNonPMLRegion() const { return mNonPMLHalf; }
	Vector3i getOriginYee() const { return mOriginYee; }
	int getNumDimensions() const;
	
	const std::vector<OutputDescPtr> & getOutputs() const { return mOutputs; }
	const std::vector<InputEHDescPtr> & getInputs() const { return mInputs; }
	const std::vector<SourceDescPtr> & getSources() const { return mSources; }
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
	
	Vector3i mOriginYee;
	
	std::vector<OutputDescPtr> mOutputs;
	std::vector<InputEHDescPtr> mInputs;
	std::vector<SourceDescPtr> mSources;
	std::vector<HuygensSurfaceDescPtr> mHuygensSurfaces;
	AssemblyDescPtr mAssembly;
};

class InputEHDescription
{
public:
	InputEHDescription(std::string fileName, std::string inClass, 
		const Map<std::string, std::string> & inParameters) throw(Exception);
	
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	std::string getFileName() const { return mFileName; }
	std::string getClass() const { return mClass; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
private:
	int mCoordinatePermutationNumber; // 0,1 or 2
	std::string mFileName;
	std::string mClass;
	Map<std::string, std::string> mParams;
};

class OutputDescription
{
public:
	OutputDescription(std::string fileName, std::string inClass,
		int inPeriod, const Map<std::string, std::string> & inParameters)
		throw(Exception);
	
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	std::string getFileName() const { return mFileName; }
	std::string getClass() const { return mClass; }
	int getPeriod() const { return mPeriod; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
private:
	int mCoordinatePermutationNumber; // 0,1 or 2
	std::string mFileName;
	std::string mClass;
	int mPeriod;
	Map<std::string, std::string> mParams;
};

class SourceDescription
{
public:
	SourceDescription(std::string formula, std::string inFileName,
		Vector3f polarization, Rect3i region, std::string field,
		const Map<std::string, std::string> & inParameters) throw(Exception);
	
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	std::string getFormula() const { return mFormula; }
	std::string getFileName() const { return mInputFileName; }
	Vector3f getPolarization() const { return mPolarization; }
	const Rect3i & getYeeRegion() const { return mRegion; }
	std::string getField() const { return mField; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
private:
	int mCoordinatePermutationNumber; // 0,1 or 2
	std::string mFormula;
	std::string mInputFileName;
	Vector3f mPolarization;
	Rect3i mRegion;
	std::string mField;
	Map<std::string, std::string> mParams;
};

enum HuygensSurfaceSourceType
{
	kLink,
	kTFSFSource,
	kDataRequest
};

class HuygensSurfaceDescription
{
private:
	// link constructor
	HuygensSurfaceDescription(std::string typeString,
		std::string sourceGridName, Rect3i sourceHalfRect, Rect3i destHalfRect,
		const std::set<Vector3i> & omittedSides)
		throw(Exception);
	HuygensSurfaceDescription(std::string typeString,
		const GridDescPtr sourceGrid, Rect3i sourceHalfRect, Rect3i destHalfRect,
		const std::set<Vector3i> & omittedSides)
		throw(Exception);
	
	// tfsf source constructor
	HuygensSurfaceDescription(Rect3i inTFRect,
		Vector3i direction, std::string formula, std::string fileName,
		Vector3f polarization, std::string field, std::string inTFSFType,
		const Map<std::string,std::string> & inParameters,
		const std::set<Vector3i> & omittedSides) throw(Exception);
	
	// custom TFSF source constructor
	HuygensSurfaceDescription(Rect3i inHalfRect,
		const std::string & inName, Vector3i symmetries, std::string inTFSFType,
		const Map<std::string, std::string> & inParameters,
		const std::set<Vector3i> & omittedSides) throw(Exception);

	// data request constructor
	HuygensSurfaceDescription(std::string requestyArgs) throw(Exception);
	
	
public: // user-accessible constructors
	static HuygensSurfaceDescription* newLink(std::string typeString,
		std::string sourceGridName, Rect3i sourceHalfRect, Rect3i destHalfRect,
		const std::set<Vector3i> & omittedSides)
		throw(Exception);
	static HuygensSurfaceDescription* newLink(std::string typeString,
		const GridDescPtr sourceGrid, Rect3i sourceHalfRect, Rect3i destHalfRect,
		const std::set<Vector3i> & omittedSides)
		throw(Exception);
	
	static HuygensSurfaceDescription* newTFSFSource(Rect3i inHalfRect,
		Vector3i direction, std::string formula, std::string fileName,
		Vector3f polarization, std::string field, std::string inTFSFType,
		const Map<std::string, std::string> & inParameters,
		const std::set<Vector3i> & omittedSides)
		throw(Exception);
	static HuygensSurfaceDescription* newCustomSource(Rect3i inHalfRect,
		const std::string & inName, Vector3i symmetries, std::string inTFSFType,
		const Map<std::string, std::string> & inParameters,
		const std::set<Vector3i> & omittedSides) throw(Exception);
	
	// modifiers
	void setPointers(const Map<std::string, GridDescPtr> & gridMap);
	void omitSide(int nSide);
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	// accessors
	HuygensSurfaceSourceType getType() const { return mType; }
	const Rect3i & getDestHalfRect() const {
		return mDestHalfRect; }
	const std::set<Vector3i> & getOmittedSides() const {
		return mOmittedSides; }
	const std::vector<NeighborBufferDescPtr> & getBuffers()
		const { return mBuffers; }
	
	// links only
	const Rect3i & getLinkSourceHalfRect() const {
		assert(mType == kLink); return mLinkSourceHalfRect; }
	const std::string & getLinkSourceGridName() const {
		assert(mType == kLink); return mLinkSourceGridName; }
	
	// kTFSFSource or kDataRequest
	Vector3i getTFSFSourceSymmetries() const {
		assert(mType == kTFSFSource || mType == kDataRequest);
		return mTFSFSourceSymmetries; }
	const std::string & getTFSFType() const { 
		assert(mType == kTFSFSource || mType == kDataRequest);
		return mTFSFType; }
	
	// kTFSFSource
	const Map<std::string, std::string> & getTFSFSourceParameters() const {
		assert(mType == kTFSFSource); return mTFSFSourceParams; }
	Vector3i getTFSFSourceDirection() const {
		assert(mType == kTFSFSource); return mTFSFSourceDirection; }
	const std::string & getTFSFSourceFormula() const {
		assert(mType == kTFSFSource); return mTFSFFormula; }
	const std::string & getTFSFSourceFileName() const {
		assert(mType == kTFSFSource); return mTFSFFileName; }
	Vector3f getTFSFSourcePolarization() const {
		assert(mType == kTFSFSource); return mTFSFPolarization; }
	const std::string & getTFSFSourceField() const {
		assert(mType == kTFSFSource); return mTFSFField; }
	
	// kDataRequest only
	const std::string & getName() const {
		assert(mType == kDataRequest); return mDataRequestName; }
	
private:
	
	// buffer maker
	void initTFSFBuffers(float srcFactor); // 1 adds, -1 subtracts
	void initFloquetBuffers();
	
private:
	HuygensSurfaceSourceType mType;
	
	Rect3i mDestHalfRect;
	std::set<Vector3i> mOmittedSides;
	
	std::vector<NeighborBufferDescPtr> mBuffers; // one per side, 0-5	
	// These members are only applicable for mType = kLink
	Rect3i mLinkSourceHalfRect;
	std::string mLinkSourceGridName;
	GridDescPtr mLinkSourceGrid;
	
	// These members are only applicable for mType = kTFSFSource or kDataRequest
	Vector3i mTFSFSourceSymmetries;
	std::string mTFSFType;
	
	// These members are only applicable for mType = kTFSFSource
	Map<std::string, std::string> mTFSFSourceParams;
	Vector3i mTFSFSourceDirection;
	std::string mTFSFFormula;
	std::string mTFSFFileName;
	Vector3f mTFSFPolarization;
	std::string mTFSFField;
	
	// Only applicable for data request
	std::string mDataRequestName;
};

class NeighborBufferDescription
{
public:
	// add constructors for things like Floquet boundaries
	NeighborBufferDescription(const Rect3i & destHalfRect, int nSide,
		float incidentFieldFactor);
	
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	const Rect3i & getDestHalfRect() const { return mDestHalfRect; }
	const Rect3i & getBufferHalfRect() const { return mBufferHalfRect; }
	const Rect3i & getBufferYeeBounds() const { return mBufferYeeBounds; }
	
private:
	Rect3i mDestHalfRect;
	Rect3i mBufferHalfRect;
	Rect3i mBufferYeeBounds;
	std::vector<float> mDestFactors; // per field # 0-5
	std::vector<float> mSrcFactors;  // per field # 0-5
};

class MaterialDescription
{
public:
	MaterialDescription(std::string name, std::string inClass,
		const Map<std::string, std::string> & inParams) throw(Exception);
	
	std::string getName() const { return mName; }
	std::string getClass() const { return mClass; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
	
	void cycleCoordinates();  // rotate x->y, y->z, z->x
	
	friend std::ostream & operator<<(std::ostream & out,
		const MaterialDescription & mat);
private:
	int mCoordinatePermutationNumber; // 0,1 or 2
	std::string mName;
	std::string mClass;
	Map<std::string, std::string> mParams;
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
	/*
	ColorKey(std::string hexColor, MaterialDescPtr materialName,
		FillStyle style) throw(Exception);
	ColorKey(Vector3i rgbColor, MaterialDescPtr materialName, FillStyle style)
		throw(Exception);
	*/
	
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
	/*
	Block(Rect3i halfCellRect, MaterialDescPtr material) throw(Exception);
	Block(Rect3i yeeCellRect, FillStyle style, MaterialDescPtr material)
		throw(Exception);
	*/
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
	/*
	HeightMap(Rect3i yeeCellRect, FillStyle style, MaterialDescPtr material,
		std::string imageFileName, Vector3i rowDirection,
		Vector3i colDirection, Vector3i upDirection) throw(Exception);
	*/
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
	/*
	Ellipsoid(Rect3i halfCellRect, MaterialDescPtr material) throw(Exception);
	Ellipsoid(Rect3i yeeCellRect, FillStyle style, MaterialDescPtr material)
		throw(Exception);
	*/
	
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
