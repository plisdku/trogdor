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
#include <bitset>

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
		
private:
	void setGrids(const std::vector<GridDescPtr> & grids) {
		mGrids = grids; }
	
	std::vector<GridDescPtr> mGrids;
	float m_dt;
	Vector3f m_dxyz;
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
	void setLinks(const std::vector<LinkDescPtr> & links) {
		mLinks = links; }
	void setMaterials(const std::vector<MaterialDescPtr> & materials) {
		mMaterials = materials; }
	void setAssembly(AssemblyDescPtr assembly) {
		mAssembly = assembly; }
private:
	std::string mName;
	
	Vector3i mNumYeeCells;
	Vector3i mNumHalfCells;
	Rect3i mCalcRegionHalf;
	Rect3i mNonPMLHalf;
	
	std::vector<OutputDescPtr> mOutputs;
	std::vector<InputEHDescPtr> mInputs;
	std::vector<SourceDescPtr> mSources;
	std::vector<LinkDescPtr> mLinks;
	std::vector<MaterialDescPtr> mMaterials;
	AssemblyDescPtr mAssembly;
};

class InputEHDescription
{
public:
	InputEHDescription(std::string fileName, std::string inClass, 
		const Map<std::string, std::string> & inParameters) throw(Exception);
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
	
private:
	std::string mClass;
	std::string mTypeStr;
	Vector3f mDirection;
	Rect3i mRegion;
	std::bitset<6> mOmitSideFlags;
	Map<std::string, std::string> mParams;
};

class LinkDescription
{
public:
	LinkDescription(std::string typeString, std::string sourceGridName, 
		Rect3i sourceRect, Rect3i destRect)
		throw(Exception);
	
	void omitSide(Vector3i side);

private:
	std::string mTypeString;
	TFSFType mLinkType;
	std::string mSourceGridName;
	Rect3i mSourceHalfRect;
	Rect3i mDestHalfRect;
	std::bitset<6> mOmitSideFlags;
};

class MaterialDescription
{
public:
	MaterialDescription(std::string name, std::string inClass,
		const Map<std::string, std::string> & inParams) throw(Exception);
private:
	std::string mName;
	std::string mClass;
	Map<std::string, std::string> mParams;
};

class AssemblyDescription
{
public:
	AssemblyDescription() throw(Exception);

private:
};





#endif
