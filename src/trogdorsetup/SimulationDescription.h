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
class TFSFSourceDescription;
class LinkDescription;
class MaterialDescription;
class AssemblyDescription;

typedef Pointer<SimulationDescription> SimulationDescPtr;
typedef Pointer<GridDescription> GridDescPtr;
typedef Pointer<InputEHDescription> InputEHDescPtr;
typedef Pointer<OutputDescription> OutputDescPtr;
typedef Pointer<SourceDescription> SourceDescPtr;
typedef Pointer<TFSFSourceDescription> TFSFSourceDescPtr;
typedef Pointer<LinkDescription> LinkDescPtr;
typedef Pointer<MaterialDescription> MaterialDescPtr;
typedef Pointer<AssemblyDescription> AssemblyDescPtr;

class SimulationDescription
{
	friend class XMLParameterFile;
public:
	SimulationDescription(const XMLParameterFile & file)  throw(Exception);
	
	void setGrids(const std::vector<GridDescPtr> & grids) {
		mGrids = grids; }
	
	void setDiscretization(Vector3f dxyz, float dt);
	void setDuration(int numT);
	
	const std::vector<GridDescPtr> & getGrids() const { return mGrids; }
	//std::vector<GridDescPtr> & getGrids() { return mGrids; }
	
	float getDt() const { return m_dt; }
	Vector3f getDxyz() const { return m_dxyz; }
	int getDuration() const { return mNumTimesteps; }
private:
	
	std::vector<GridDescPtr> mGrids;
	float m_dt;
	Vector3f m_dxyz;
	int mNumTimesteps;
};


class GridDescription
{
public:
	GridDescription(std::string name, Vector3i numYeeCells,
		Vector3i numHalfCells, Rect3i calcRegionHalf, Rect3i nonPMLHalf)
		throw(Exception);
	
	void setOutputs(const std::vector<OutputDescPtr> & outputs) {
		mOutputs = outputs; }
	void setInputs(const std::vector<InputEHDescPtr> & inputs) {
		mInputs = inputs; }
	void setSources(const std::vector<SourceDescPtr> & sources) {
		mSources = sources; }
	void setTFSFSources(const std::vector<TFSFSourceDescPtr> & tfsfSources) {
		mTFSFSources = tfsfSources; }
	void setLinks(const std::vector<LinkDescPtr> & links) {
		mLinks = links; }
	void setMaterials(const std::vector<MaterialDescPtr> & materials) {
		mMaterials = materials; }
	void setAssembly(AssemblyDescPtr assembly) {
		mAssembly = assembly; }
	
	const std::string & getName() const { return mName; }
	Vector3i getNumYeeCells() const { return mNumYeeCells; }
	Vector3i getNumHalfCells() const { return mNumHalfCells; }
	const Rect3i & getCalcRegion() const { return mCalcRegionHalf; }
	const Rect3i & getNonPMLRegion() const { return mNonPMLHalf; }
	
	const std::vector<OutputDescPtr> & getOutputs() const { return mOutputs; }
	const std::vector<InputEHDescPtr> & getInputs() const { return mInputs; }
	const std::vector<SourceDescPtr> & getSources() const { return mSources; }
	const std::vector<TFSFSourceDescPtr> & getTFSFSources() const
		{ return mTFSFSources; }
	const std::vector<LinkDescPtr> & getLinks() const { return mLinks; }
	const std::vector<MaterialDescPtr> & getMaterials() const
		{ return mMaterials; }
	const AssemblyDescPtr getAssembly() const { return mAssembly; }
	
private:
	std::string mName;
	
	Vector3i mNumYeeCells;
	Vector3i mNumHalfCells;
	Rect3i mCalcRegionHalf;
	Rect3i mNonPMLHalf;
	
	std::vector<OutputDescPtr> mOutputs;
	std::vector<InputEHDescPtr> mInputs;
	std::vector<SourceDescPtr> mSources;
	std::vector<TFSFSourceDescPtr> mTFSFSources;
	std::vector<LinkDescPtr> mLinks;
	std::vector<MaterialDescPtr> mMaterials;
	AssemblyDescPtr mAssembly;
};

class InputEHDescription
{
public:
	InputEHDescription(std::string fileName, std::string inClass, 
		const Map<std::string, std::string> & inParameters) throw(Exception);
	
	std::string getFileName() const { return mFileName; }
	std::string getClass() const { return mClass; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
private:
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
	
	std::string getFileName() const { return mFileName; }
	std::string getClass() const { return mClass; }
	int getPeriod() const { return mPeriod; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
private:
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
	
	std::string getFormula() const { return mFormula; }
	std::string getFileName() const { return mInputFileName; }
	Vector3f getPolarization() const { return mPolarization; }
	const Rect3i & getYeeRegion() const { return mRegion; }
	std::string getField() const { return mField; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
private:
	std::string mFormula;
	std::string mInputFileName;
	Vector3f mPolarization;
	Rect3i mRegion;
	std::string mField;
	Map<std::string, std::string> mParams;
};

class TFSFSourceDescription
{
public:
	TFSFSourceDescription(std::string inClass, Rect3i inTFRect,
		Vector3f direction, std::string inTFSFType,
		const Map<std::string, std::string> & inParameters) throw(Exception);
	
	void omitSide(Vector3i side);
	
	std::string getClass() const { return mClass; }
	std::string getTypeStr() const { return mTypeStr; }
	Vector3f getDirection() const { return mDirection; }
	const Rect3i & getYeeRegion() const { return mRegion; }
	const std::set<Vector3i> & getOmittedSides() const { return mOmittedSides; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
	
private:
	std::string mClass;
	std::string mTypeStr;
	Vector3f mDirection;
	Rect3i mRegion;
	//std::bitset<6> mOmitSideFlags;  // bad: can't be rotated easily
	std::set<Vector3i> mOmittedSides;
	Map<std::string, std::string> mParams;
};

class LinkDescription
{
public:
	LinkDescription(std::string typeString, std::string sourceGridName, 
		Rect3i sourceRect, Rect3i destRect)
		throw(Exception);
	
	std::string getTypeString() const { return mTypeString; }
	TFSFType getType() const { return mLinkType; }
	std::string getSourceGridName() const { return mSourceGridName; }
	Rect3i getSourceHalfRect() const { return mSourceHalfRect; }
	Rect3i getDestHalfRect() const { return mDestHalfRect; }
	const std::set<Vector3i> & getOmittedSides() const { return mOmittedSides; }
	
	void omitSide(Vector3i side);

private:
	std::string mTypeString;
	TFSFType mLinkType;
	std::string mSourceGridName;
	Rect3i mSourceHalfRect;
	Rect3i mDestHalfRect;
	//std::bitset<6> mOmitSideFlags;  // bad: can't be rotated easily
	std::set<Vector3i> mOmittedSides;
};

class MaterialDescription
{
public:
	MaterialDescription(std::string name, std::string inClass,
		const Map<std::string, std::string> & inParams) throw(Exception);
	
	std::string getName() const { return mName; }
	std::string getClass() const { return mClass; }
	const Map<std::string, std::string> & getParams() const { return mParams; }
private:
	std::string mName;
	std::string mClass;
	Map<std::string, std::string> mParams;
};

class AssemblyDescription
{
public:
	enum InstructionType
	{
		kBlockType,
		kKeyImageType,
		kHeightMapType,
		kEllipsoidType,
		kCopyFromType
	};
	
	class ColorKey
	{
	public:
		ColorKey(std::string hexColor, std::string materialName,
			FillStyle style) throw(Exception);
		ColorKey(Vector3i rgbColor, std::string materialName, FillStyle style)
			throw(Exception);
		
		Vector3i getColor() const { return mColor; }
		const std::string & getMaterial() const { return mMaterial; }
		FillStyle getFillStyle() const { return mFillStyle; } 
	private:
		Vector3i mColor;
		std::string mMaterial;
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
		//const Rect3i & getFillRect() const { return mFillRect };
		const Rect3i & getYeeRect() const;
		const Rect3i & getHalfRect() const;
		FillStyle getFillStyle() const { return mStyle; }
		const std::string & getMaterial() const { return mMaterial; }
	private:
		Rect3i mFillRect;
		FillStyle mStyle;
		std::string mMaterial;
	};
	
	class KeyImage : public Instruction
	{
	public:
		KeyImage(Rect3i yeeCellRect, std::string imageFileName,
			Vector3i rowDirection, Vector3i colDirection,
			std::vector<ColorKey> keys) throw(Exception);
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
		const Rect3i & getYeeRect() const { return mYeeRect; }
		FillStyle getFillStyle() const { return mStyle; }
		const std::string & getMaterial() const { return mMaterial; }
		const std::string & getImageFileName() const { return mImageFileName; }
		Vector3i getRowDirection() const { return mRow; }
		Vector3i getColDirection() const { return mCol; }
		Vector3i getUpDirection() const { return mUp; }
	private:
		Rect3i mYeeRect;
		FillStyle mStyle;
		std::string mMaterial;
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
		//const Rect3i & getFillRect() const { return mFillRect; }
		const Rect3i & getYeeRect() const;
		const Rect3i & getHalfRect() const;
		FillStyle getFillStyle() const { return mStyle; }
		const std::string & getMaterial() const { return mMaterial; }
	private:
		Rect3i mFillRect;
		FillStyle mStyle;
		std::string mMaterial;
	};
	
	class CopyFrom : public Instruction
	{
	public:
		CopyFrom(Rect3i halfCellSourceRegion, Rect3i halfCellDestRegion,
			std::string gridName)
			throw(Exception);
		const Rect3i & getSourceHalfRect() const { return mSourceRect; }
		const Rect3i & getDestHalfRect() const { return mDestRect; }
		const std::string & getGridName() const { return mGridName; }
	private:
		Rect3i mSourceRect;
		Rect3i mDestRect;
		std::string mGridName;
	};
	
	
	AssemblyDescription(const std::vector<InstructionPtr> & recipe)
		throw(Exception);
	
	void setInstructions(const std::vector<InstructionPtr> & instructions);
	
	const std::vector<InstructionPtr> & getInstructions() const
		{ return mInstructions; }
private:
	std::vector<InstructionPtr> mInstructions;
};





#endif
