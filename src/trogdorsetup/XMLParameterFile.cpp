/*
 *  XMLParameterFile.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 1/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLParameterFile.h"

//#include "ValidateSetupAttributes.h"

#include "StreamFromString.h"
#include <sstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <set>

using namespace std;

#pragma mark *** Static Prototypes ***

static Map<string, string> sGetAttributes(const TiXmlElement* elem);

template <class T>
static void sGetMandatoryAttribute(const TiXmlElement* elem,
	const string & attribute, T & val) throw(Exception);

template <class T>
static bool sTryGetAttribute(const TiXmlElement* elem,
	const string & attribute, T & val);

template <class T>
static void sGetOptionalAttribute(const TiXmlElement* elem,
	const string & attribute, T & val, const T & defaultVal);

template <>
static void sGetMandatoryAttribute<string>(const TiXmlElement* elem,
	const string & attribute, string & val) throw(Exception);

template <>
static bool sTryGetAttribute<string>(const TiXmlElement* elem,
	const string & attribute, string & val);

template <>
static void sGetOptionalAttribute<string>(const TiXmlElement* elem,
	const string & attribute, string & val, const string & defaultVal);

static string sErr(const string & str, const TiXmlElement* elem);

static FillStyle sFillStyleFromString(const string & str) throw(Exception);

static Vector3i sUnitVectorFromString(const string & axisString)
	throw(Exception);

#pragma mark *** XMLParameterFile Implementation ***

XMLParameterFile::
XMLParameterFile(const string & filename) throw(Exception) :
	mDocument(filename.c_str())
{
	string errorMessage;
	
    TiXmlBase::SetCondenseWhiteSpace(0); // for ascii material layers with \n
	
	if (!(mDocument.LoadFile()))
    {
		ostringstream err;
        err << "Could not load parameter file " << filename << endl;
        err << "Reason: " << mDocument.ErrorDesc() << endl;
        err << "(possible location: row " << mDocument.ErrorRow() << " column "
             << mDocument.ErrorCol() << " .)" << endl;
        throw(Exception(err.str()));
    }
}

// It is the job of XMLParameterFile to validate the XML and to verify that 
// all the necessary elements and attributes are present and properly-formatted.
// It is the job of the SimulationDescription etc. classes to verify that the
// information in the XML file makes sense; this is handled in the constructors.

void XMLParameterFile::
load(SimulationDescription & sim) const throw(Exception)
{
	const TiXmlElement* elem = mDocument.RootElement();
	if (!elem)
		throw Exception("XML file has no root element.");
	
	string versionString;
	
	Map<string, string> attributes = sGetAttributes(elem);
	if (!sTryGetAttribute(elem, "version", versionString))
	{
		loadTrogdor4(sim);
	}
	else
	{
		ostringstream err;
		err << "Cannot read version " << versionString << " file.";
		throw Exception(sErr(err.str(), elem));
	}
}

void XMLParameterFile::
loadTrogdor4(SimulationDescription & sim) const throw(Exception)
{
    const TiXmlElement* elem = mDocument.RootElement();
	assert(elem);  // this condition was checked in load().
	
	float dx, dy, dz, dt;
	int numTimesteps;
	
	Map<string, string> simulationParams;
    simulationParams = sGetAttributes(elem);
	
	sGetMandatoryAttribute(elem, "dx", dx);
	sGetMandatoryAttribute(elem, "dy", dy);
	sGetMandatoryAttribute(elem, "dz", dz);
	sGetMandatoryAttribute(elem, "dt", dt);
	sGetMandatoryAttribute(elem, "numT", numTimesteps);
	
	// Only the calls that may throw exceptions are in the try-catch block.
	try {
		sim.setDiscretization(Vector3f(dx,dy,dz), dt);
		sim.setDuration(numTimesteps);
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem)));
	}
	// (i.e. these three calls don't throw exceptions.)
	sim.setMaterials(loadMaterials(elem));
	sim.setGrids(loadGrids(elem));
	sim.setAllPointers();
}

vector<GridDescPtr> XMLParameterFile::
loadGrids(const TiXmlElement* parent) const
{
	assert(parent);
	vector<GridDescPtr> gridVector;
	const TiXmlElement* elem = parent->FirstChildElement("Grid");
	
	set<string> allGridNames = collectGridNames(parent);
	set<string> allMaterialNames = collectMaterialNames(parent);
	
    while (elem) // FOR EACH GRID: Load data
	{
		GridDescPtr gridDesc;
		int nnx, nny, nnz;
		Rect3i activeRegion, regionOfInterest;
		string name;
		Vector3i origin;
		
		sGetMandatoryAttribute(elem, "name", name);
		sGetOptionalAttribute(elem, "origin", origin, Vector3i(0,0,0));
		
		if (sTryGetAttribute(elem, "nx", nnx))
			nnx *= 2;
		else if (!sTryGetAttribute(elem, "nnx", nnx))
			throw(Exception(sErr("Can't load nnx or nx", elem)));
		
		if (sTryGetAttribute(elem, "ny", nny))
			nny *= 2;
		else if (!sTryGetAttribute(elem, "nny", nny))
			throw(Exception(sErr("Can't load nny or ny", elem)));
		
		if (sTryGetAttribute(elem, "nz", nnz))
			nnz *= 2;
		else if (!sTryGetAttribute(elem, "nnz", nnz))
			throw(Exception(sErr("Can't load nnz or nz", elem)));
		
		if (sTryGetAttribute(elem, "regionOfInterest", regionOfInterest))
		{
			regionOfInterest *= 2;
			regionOfInterest.p2 += Vector3i(1,1,1);
		}
		else if (!sTryGetAttribute(elem, "roi", regionOfInterest))
			throw(Exception(sErr("Can't load region of interest", elem)));
		
		if (!sTryGetAttribute(elem, "activeRegion", activeRegion))
		{
			activeRegion = Rect3i(1, 1, 1, nnx-2, nny-2, nnz-2);
			if (regionOfInterest.p1[0] == 0)
				activeRegion.p1[0] = 0;
			if (regionOfInterest.p1[1] == 0)
				activeRegion.p1[1] = 0;
			if (regionOfInterest.p1[2] == 0)
				activeRegion.p1[2] = 0;
			if (regionOfInterest.p2[0] == (nnx-1))
				activeRegion.p2[0] = nnx-1;
			if (regionOfInterest.p2[1] == (nny-1))
				activeRegion.p2[1] = nny-1;
			if (regionOfInterest.p2[2] == (nnz-1))
				activeRegion.p2[2] = nnz-1;
		}
		
		// We need to wrap the constructor errors from GridDescription() to
		// put XML row/column information in.
		try {
			gridDesc = GridDescPtr(new GridDescription(name,
				Vector3i(nnx,nny,nnz)/2, Vector3i(nnx,nny,nnz), activeRegion,
				regionOfInterest, origin));
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		// However, these function calls all take care of their own XML file
		// row/column information so we don't need to re-throw their
		// exceptions.
		gridDesc->setOutputs(loadOutputs(elem));
		gridDesc->setInputs(loadInputEHs(elem));
		gridDesc->setSources(loadSources(elem));
		//gridDesc->setMaterials(loadMaterials(elem));
		gridDesc->setAssembly(loadAssembly(elem, allGridNames,
			allMaterialNames));
		
		// half the huygens surfaces come from links, and half from TFSF
		// sources.
		vector<HuygensSurfaceDescPtr> huygensSurfaces =
			loadTFSFSources(elem, allGridNames);
		vector<HuygensSurfaceDescPtr> huygensLinks =
			loadLinks(elem, allGridNames);
		vector<HuygensSurfaceDescPtr> customSources =
			loadCustomSources(elem);
		huygensSurfaces.insert(huygensSurfaces.begin(), huygensLinks.begin(),
			huygensLinks.end());
		huygensSurfaces.insert(huygensSurfaces.begin(), customSources.begin(),
			customSources.end());
		
		gridDesc->setHuygensSurfaces(huygensSurfaces);
		//gridDesc->implementLinksAsBuffers(loadLinks(elem));
		
		gridVector.push_back(gridDesc);
        elem = elem->NextSiblingElement("Grid");
	}
	
	return gridVector;
}

set<string> XMLParameterFile::
collectGridNames(const TiXmlElement* parent) const
{
	assert(parent);
	set<string> names;
	
	const TiXmlElement* gridElem = parent->FirstChildElement("Grid");
	
	while (gridElem)
	{
		string gridName;
		sGetMandatoryAttribute(gridElem, "name", gridName);
		
		if (names.count(gridName) > 0)
		{
			throw(Exception(sErr("Multiple grids with the same name!",
				gridElem)));
		}
		
		names.insert(gridName);
		gridElem = gridElem->NextSiblingElement("Grid");
	}
	return names;
}

set<string> XMLParameterFile::
collectMaterialNames(const TiXmlElement* parent) const
{
	assert(parent);
	set<string> names;
	
	const TiXmlElement* gridElem = parent->FirstChildElement("Grid");
	
	while (gridElem)
	{
		const TiXmlElement* matElem = gridElem->FirstChildElement("Material");
		
		while (matElem)
		{
			string matName;
			sGetMandatoryAttribute(matElem, "name", matName);
			names.insert(matName);
			matElem = matElem->NextSiblingElement("Material");
		}
		
		gridElem = gridElem->NextSiblingElement("Grid");
	}
	return names;
}

vector<MaterialDescPtr> XMLParameterFile::
loadMaterials(const TiXmlElement* parent) const
{
	assert(parent);
	vector<MaterialDescPtr> materials;
	
	const TiXmlElement* gridElem = parent->FirstChildElement("Grid");
	while (gridElem)
	{
		const TiXmlElement* elem = gridElem->FirstChildElement("Material");
		while (elem)
		{
			string name;
			string inClass;
			Map<string,string> params;
			const TiXmlElement* paramXML = elem->FirstChildElement("Params");
			// not every material needs params, e.g. PEC/PMC
			if (paramXML != 0L)
				params = sGetAttributes(paramXML);
			
			sGetMandatoryAttribute(elem, "name", name);
			sGetMandatoryAttribute(elem, "class", inClass);
			
			MaterialDescPtr material(new MaterialDescription(name, inClass,
				params));
			materials.push_back(material);
			
			elem = elem->NextSiblingElement("Material");
		}
		gridElem = gridElem->NextSiblingElement("Grid");
	}
	
	return materials;	
}
		
vector<InputEHDescPtr> XMLParameterFile::
loadInputEHs(const TiXmlElement* parent) const
{
	assert(parent);
	vector<InputEHDescPtr> inputs;
	
	const TiXmlElement* elem = parent->FirstChildElement("Input");
    while (elem)
	{
		string filePrefix;
		string inputClass;
		Map<string,string> params;
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Input needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		sGetMandatoryAttribute(elem, "filePrefix", filePrefix);
		sGetMandatoryAttribute(elem, "class", inputClass);
		
		try {
			InputEHDescPtr input(new InputEHDescription(filePrefix, inputClass,
				params));
			inputs.push_back(input);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("Input");
	}

	return inputs;
}

vector<OutputDescPtr> XMLParameterFile::
loadOutputs(const TiXmlElement* parent) const
{
	assert(parent);
	vector<OutputDescPtr> outputs;
	
	const TiXmlElement* elem = parent->FirstChildElement("Output");
    while (elem)
	{
		string filePrefix;
		string outputClass;
		int period;
		Map<string,string> params;
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Output needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		sGetMandatoryAttribute(elem, "filePrefix", filePrefix);
		sGetMandatoryAttribute(elem, "class", outputClass);
		sGetOptionalAttribute(elem, "period", period, 1);
		
		try {
			OutputDescPtr output(new OutputDescription(filePrefix, outputClass,
				period, params));
			outputs.push_back(output);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("Output");
	}

	return outputs;
}

vector<SourceDescPtr> XMLParameterFile::
loadSources(const TiXmlElement* parent) const
{
	assert(parent);
	vector<SourceDescPtr> sources;
	
	const TiXmlElement* elem = parent->FirstChildElement("Source");
    while (elem)
	{
		string formula;
		string filename;
		string field;
		Vector3f polarization;
		Rect3i region;
		Map<string,string> params;
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		//if (paramXML == 0L)
		//	throw(Exception(sErr("Source needs Params element", elem)));
		if (paramXML != 0L)
			params = sGetAttributes(paramXML);
		
		if (!sTryGetAttribute(elem, "formula", formula) &&
			!sTryGetAttribute(elem, "file", filename))
			throw(Exception(sErr("Source needs formula or file attribute",
				elem)));
		
		sGetMandatoryAttribute(elem, "field", field);
		sGetMandatoryAttribute(elem, "polarization", polarization);
		sGetMandatoryAttribute(elem, "region", region);
		
		try {
			SourceDescPtr source(new SourceDescription(formula, filename,
				polarization, region, field, params));
			sources.push_back(source);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("Source");
	}
	return sources;
}

vector<HuygensSurfaceDescPtr> XMLParameterFile::
loadTFSFSources(const TiXmlElement* parent, const set<string> & allGridNames)
	const
{
	assert(parent);
	vector<HuygensSurfaceDescPtr> tfsfSources;
	
	const TiXmlElement* elem = parent->FirstChildElement("TFSFSource");
    while (elem)
	{
		string tfsfType;
		Vector3i direction;
		Vector3f polarization;
		string formula;
		string filename;
		string field;
		Rect3i region;
		Map<string,string> params;
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("TFSFSource needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		if (sTryGetAttribute(elem, "TFRect", region))
		{
			region *= 2;
			region.p2 += Vector3i(1,1,1);
		}
		else if (!sTryGetAttribute(elem, "fineTFRect", region))
			throw(Exception(sErr("TFSFSource needs TFRect or fineTFRect "
				"attribute", elem)));
		sGetMandatoryAttribute(elem, "direction", direction);
		sGetMandatoryAttribute(elem, "polarization", polarization);
		sGetOptionalAttribute(elem, "type", tfsfType, string("TF"));
		sGetOptionalAttribute(elem, "formula", formula, string(""));
		sGetOptionalAttribute(elem, "filename", filename, string(""));
		sGetOptionalAttribute(elem, "field", field, string("electric"));
			
		try {
			set<Vector3i> omittedSides;
			const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
			while (omitElem)
			{
				Vector3i omitSide;
				omitElem->GetText() >> omitSide;
				cerr << "Warning: no error checking on omit side input.\n";
				omittedSides.insert(omitSide);
				omitElem = omitElem->NextSiblingElement("OmitSide");
			}
			HuygensSurfaceDescPtr source(
				HuygensSurfaceDescription::newTFSFSource(region,
				direction, formula, filename, polarization, field,
				tfsfType, params, omittedSides));
			
			tfsfSources.push_back(source);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("TFSFSource");
	}
	return tfsfSources;
}

vector<HuygensSurfaceDescPtr> XMLParameterFile::
loadCustomSources(const TiXmlElement* parent) const
{
	assert(parent);
	vector<HuygensSurfaceDescPtr> customSources;
	
	const TiXmlElement* elem = parent->FirstChildElement("CustomSource");
    while (elem)
	{
		string tfsfType;
		Vector3i symmetries;
		Rect3i region;
		string name;
		Map<string,string> params;
		/*
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("TFSFSource needs Params element", elem)));
		params = sGetAttributes(paramXML);
		*/
		
		sGetMandatoryAttribute(elem, "name", name);
		
		if (sTryGetAttribute(elem, "TFRect", region))
		{
			region *= 2;
			region.p2 += Vector3i(1,1,1);
		}
		else if (!sTryGetAttribute(elem, "fineTFRect", region))
			throw(Exception(sErr("CustomSource needs TFRect or fineTFRect "
				"attribute", elem)));
		sGetOptionalAttribute(elem, "symmetries", symmetries, Vector3i(0,0,0));
		sGetOptionalAttribute(elem, "type", tfsfType, string("TF"));
			
		try {
			set<Vector3i> omittedSides;
			const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
			while (omitElem)
			{
				Vector3i omitSide;
				omitElem->GetText() >> omitSide;
				cerr << "Warning: no error checking on omit side input.\n";
				omittedSides.insert(omitSide);
				omitElem = omitElem->NextSiblingElement("OmitSide");
			}
			HuygensSurfaceDescPtr source(
				HuygensSurfaceDescription::newCustomSource(region, name,
				symmetries, tfsfType, params, omittedSides));
			
			customSources.push_back(source);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("TFSFSource");
	}
	return customSources;
}

vector<HuygensSurfaceDescPtr> XMLParameterFile::
loadLinks(const TiXmlElement* parent, const set<string> & allGridNames) const
{
	assert(parent);
	vector<HuygensSurfaceDescPtr> links;
	cerr << "Warning: links are not validated.\n";
	
	const TiXmlElement* elem = parent->FirstChildElement("Link");
    while (elem)
	{
		string linkTypeStr;
		string sourceGridName;
		Rect3i sourceHalfRect;
		Rect3i destHalfRect;
		Map<string,string> params;
		// I don't think Links have parameters.  this is erroneous...?
		/*
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Link needs Params element", elem)));
		params = sGetAttributes(paramXML);
		*/
		sGetMandatoryAttribute(elem, "type", linkTypeStr);
		sGetMandatoryAttribute(elem, "sourceGrid", sourceGridName);
		
		if (sTryGetAttribute(elem, "sourceRect", sourceHalfRect))
		{
			sourceHalfRect.p1 *= 2;
			sourceHalfRect.p2 = 2*sourceHalfRect.p2 + Vector3i(1,1,1);
		}
		else if (!sTryGetAttribute(elem, "fineSourceRect", sourceHalfRect))
			throw(Exception(sErr("Link needs sourceRect or fineSourceRect "
				"attribute", elem)));
		if (sTryGetAttribute(elem, "destRect", destHalfRect))
		{
			destHalfRect.p1 *= 2;
			destHalfRect.p2 = 2*destHalfRect.p2 + Vector3i(1,1,1);
		}
		else if (!sTryGetAttribute(elem, "fineDestRect", destHalfRect))
			throw(Exception(sErr("Link needs destRect or fineDestRect "
				"attribute", elem)));
		
		try {
			set<Vector3i> omittedSides;
			const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
			while (omitElem)
			{
				Vector3i omitSide;
				omitElem->GetText() >> omitSide;
				//link->omitSide(omitSide);
				omittedSides.insert(omitSide);
				omitElem = omitElem->NextSiblingElement("OmitSide");
			}
			
			HuygensSurfaceDescPtr link(
				HuygensSurfaceDescription::newLink(linkTypeStr, sourceGridName,
				sourceHalfRect, destHalfRect, omittedSides));
			links.push_back(link);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("Link");
	}
	
	return links;
}

AssemblyDescPtr XMLParameterFile::
loadAssembly(const TiXmlElement* parent, const set<string> & allGridNames,
	const set<string> & allMaterialNames) const
{
	assert(parent);
	AssemblyDescPtr assembly;
	vector<InstructionPtr> recipe;
	
	const TiXmlElement* elem = parent->FirstChildElement("Assembly");
	
	if (elem->NextSiblingElement("Assembly") != 0L)
		throw(Exception(sErr("Only one Assembly element allowed per Grid",
			elem->NextSiblingElement("Assembly"))));
	
	const TiXmlElement* instruction = elem->FirstChildElement();
	
	while (instruction)
	{
		string instructionType = instruction->Value();
		
		if (instructionType == "Block")
			recipe.push_back(loadABlock(instruction, allMaterialNames));
		else if (instructionType == "KeyImage")
			recipe.push_back(loadAKeyImage(instruction, allMaterialNames));
		else if (instructionType == "HeightMap")
			recipe.push_back(loadAHeightMap(instruction, allMaterialNames));
		else if (instructionType == "Ellipsoid")
			recipe.push_back(loadAEllipsoid(instruction, allMaterialNames));
		else if (instructionType == "CopyFrom")
			recipe.push_back(loadACopyFrom(instruction, allGridNames));
		
		instruction = instruction->NextSiblingElement();
	}
	
	assembly = AssemblyDescPtr(new AssemblyDescription(recipe));
	return assembly;
}


InstructionPtr XMLParameterFile::
loadABlock(const TiXmlElement* elem, const set<string> & allMaterialNames) const
{
	assert(elem);
	assert(elem->Value() == string("Block"));
	
	InstructionPtr blockPtr;
	Rect3i rect;
	string material;
	string fillStyleStr;
	FillStyle style;
	
	sGetMandatoryAttribute(elem, "material", material);
	if (allMaterialNames.count(material) == 0)
		throw(Exception(sErr("Unknown material for Block", elem)));
	
	// Does the Block have a Yee region, called "fillRect"?
	if (sTryGetAttribute(elem, "fillRect", rect))
	{
		sGetOptionalAttribute(elem, "fillStyle", fillStyleStr,
			string("PECStyle"));
		try {
			style = sFillStyleFromString(fillStyleStr); // may fail
		} catch (const Exception & e) {
			throw(Exception(sErr(e.what(), elem))); // rethrow with XML row/col
		}
		try {
			blockPtr = InstructionPtr(
				(Block*)new Block(
					rect, style, material)
				);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		// Yee rect
	}
	else if (sTryGetAttribute(elem, "fineFillRect", rect))
	{
		if (elem->Attribute("fillStyle"))
			throw(Exception(sErr("Block with fineFillRect does not need "
				"fillStyle attribute", elem)));
		
		try {
			blockPtr = InstructionPtr(
				new Block(rect, material));
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		// half-cell rect
	}
	else
	{
		throw(Exception(sErr("Block needs fillRect or fineFillRect "
			"attribute", elem)));
	}
	
	return blockPtr;
}

InstructionPtr XMLParameterFile::
loadAKeyImage(const TiXmlElement* elem, const set<string> & allMaterialNames)
	const
{
	assert(elem);
	assert(elem->Value() == string("KeyImage"));
	
	InstructionPtr keyImage;
	Rect3i yeeRect;
	string imageFileName;
	string rowDirStr;
	string colDirStr;
	Vector3i rowDir;
	Vector3i colDir;
	vector<ColorKey> keys;
	
	// Step 1 of 2: load all the attributes (and not the tag elements)
	sGetMandatoryAttribute(elem, "fillRect", yeeRect);
	sGetMandatoryAttribute(elem, "file", imageFileName);
	sGetMandatoryAttribute(elem, "row", rowDirStr);
	sGetMandatoryAttribute(elem, "column", colDirStr);
	
	try {
		rowDir = sUnitVectorFromString(rowDirStr);
		colDir = sUnitVectorFromString(colDirStr);
	} catch (const Exception & e) {
		throw(Exception(sErr(e.what(), elem))); // rethrow with XML row/col
	}
	
	// Step 2 of 2: load all the tag elements (and not the attributes)
	keys = loadAColorKeys(elem, allMaterialNames);
	
	// Package it up and ship.
	
	try {
		keyImage = InstructionPtr(
			new KeyImage(yeeRect, imageFileName, rowDir,
				colDir, keys));
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem)));
	}
	
	return keyImage;
}

vector<ColorKey> XMLParameterFile::
loadAColorKeys(const TiXmlElement* parent, const set<string> & allMaterialNames)
	const
{
	assert(parent);
	vector<ColorKey> keys;

	const TiXmlElement* elem = parent->FirstChildElement("Tag");
	if (elem == 0L)
		throw(Exception(sErr("KeyImage needs at least one Tag element",
			parent)));
	
	while (elem)
	{
		string colorStr;
		string material;
		string fillStyleStr;
		FillStyle style;
		
		sGetMandatoryAttribute(elem, "material", material);
		if (allMaterialNames.count(material) == 0)
			throw(Exception(sErr("Unknown material for Tag", elem)));
		sGetMandatoryAttribute(elem, "color", colorStr);
		sGetOptionalAttribute(elem, "fillStyle", fillStyleStr,
			string("PECStyle"));
		
		try {
			style = sFillStyleFromString(fillStyleStr);
			ColorKey key(colorStr, material, style);
			keys.push_back(key);
		} catch (const Exception & e) {
			throw(Exception(sErr(e.what(), elem))); // rethrow w/ row/col
		}
		
		elem = elem->NextSiblingElement("Tag");
	}
	
	return keys;
}

InstructionPtr XMLParameterFile::
loadAHeightMap(const TiXmlElement* elem, const set<string> & allMaterialNames)
	const
{
	assert(elem);
	assert(elem->Value() == string("HeightMap"));
	
	InstructionPtr heightMap;
	
	Rect3i yeeRect;
	string styleStr;
	FillStyle style;
	string material;
	string imageFileName;
	string rowDirStr, colDirStr, upDirStr;
	Vector3i rowDir, colDir, upDir;
	
	sGetMandatoryAttribute(elem, "fillRect", yeeRect);
	sGetMandatoryAttribute(elem, "material", material);
	if (allMaterialNames.count(material) == 0)
		throw(Exception(sErr("Unknown material for HeightMap", elem)));
	sGetMandatoryAttribute(elem, "file", imageFileName);
	sGetMandatoryAttribute(elem, "row", rowDirStr);
	sGetMandatoryAttribute(elem, "column", colDirStr);
	sGetMandatoryAttribute(elem, "up", upDirStr);
	sGetOptionalAttribute(elem, "fillStyle", styleStr, string("PECStyle"));
	
	try {
		style = sFillStyleFromString(styleStr);
		rowDir = sUnitVectorFromString(rowDirStr);
		colDir = sUnitVectorFromString(colDirStr);
		upDir = sUnitVectorFromString(upDirStr);
		
		heightMap = InstructionPtr(
			new HeightMap(yeeRect, style, material,
				imageFileName, rowDir, colDir, upDir));
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem))); // append XML row/col info
	}
	
	return heightMap;
}

InstructionPtr XMLParameterFile::
loadAEllipsoid(const TiXmlElement* elem, const set<string> & allMaterialNames)
	const
{
	assert(elem);
	assert(elem->Value() == string("Ellipsoid"));
	
	InstructionPtr ellipsoid;
	Rect3i rect;
	string material;
	string styleStr;
	FillStyle style;
	
	sGetMandatoryAttribute(elem, "material", material);
	if (allMaterialNames.count(material) == 0)
		throw(Exception(sErr("Unknown material for Ellipsoid", elem)));
	
	if (sTryGetAttribute(elem, "fillRect", rect))
	{
		sGetOptionalAttribute(elem, "fillStyle", styleStr, string("PECStyle"));
		
		try {
			style = sFillStyleFromString(styleStr);
			ellipsoid = InstructionPtr(
				new Ellipsoid(rect, style, material));
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
	}
	else
	{
		if (!sTryGetAttribute(elem, "fineFillRect", rect))
			throw(Exception(sErr("Ellipsoid needs fillRect or fineFillRect "
				"attribute", elem)));
		
		ellipsoid = InstructionPtr(
			new Ellipsoid(rect, material));
	}
	return ellipsoid;
}

InstructionPtr XMLParameterFile::
loadACopyFrom(const TiXmlElement* elem, const set<string> & allGridNames) const
{
	assert(elem);
	assert(elem->Value() == string("CopyFrom"));
	
	InstructionPtr copyFrom;
	Rect3i sourceRect, destRect;
	string sourceGridName;
	
	sGetMandatoryAttribute(elem, "sourceRect", sourceRect);
	sGetMandatoryAttribute(elem, "destRect", destRect);
	sGetMandatoryAttribute(elem, "sourceGrid", sourceGridName);
	if (allGridNames.count(sourceGridName) == 0)
		throw(Exception(sErr("Unknown source grid name for CopyFrom", elem)));
	
	try {
		copyFrom = InstructionPtr(
			new CopyFrom(
				sourceRect, destRect, sourceGridName));
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem)));
	}
	
	return copyFrom;
}



#pragma mark *** Static Method Implementations ***


Map<string, string> sGetAttributes(const TiXmlElement* elem)
{
    Map<string, string> attribs;
    if (elem)
    {
        const TiXmlAttribute* attrib = elem->FirstAttribute();
        while (attrib)
        {
            attribs[attrib->Name()] = attrib->Value();
            attrib = attrib->Next();
        }
    }
    return attribs;
}



template <>
static void sGetMandatoryAttribute<string>(const TiXmlElement* elem,
	const string & attribute, string & val) throw(Exception)
{
	assert(elem);
	ostringstream err;
	
	if (elem->Attribute(attribute.c_str()) == 0L)
	{
		err << elem->Value() << " needs attribute \"" << attribute << "\"";
		throw(Exception(sErr(err.str(), elem)));
	}
	/*
	istringstream istr(elem->Attribute(attribute.c_str()));
	
	if (!(istr >> val))
	{
		err << elem->Value() << " has invalid \"" << attribute
			<<"\" attribute";
		throw(Exception(sErr(err.str(), elem)));
	}*/
	val = elem->Attribute(attribute.c_str());
}

template <>
static bool sTryGetAttribute<string>(const TiXmlElement* elem,
	const string & attribute, string & val)
{
	assert(elem);
	ostringstream err;
	
	if (elem->Attribute(attribute.c_str()) == 0L)
		return 0;
	else
	{
		/*
		istringstream istr(elem->Attribute(attribute.c_str()));
		
		if (!(istr >> val))
		{
			err << elem->Value() << " has invalid \"" << attribute
				<< "\" attribute";
			throw(Exception(sErr(err.str(), elem)));
		}*/
		val = elem->Attribute(attribute.c_str());
	}
	return 1;
}

template <>
static void sGetOptionalAttribute<string>(const TiXmlElement* elem,
	const string & attribute, string & val, const string & defaultVal)
{
	assert(elem);
	ostringstream err;
	
	if (elem->Attribute(attribute.c_str()) == 0L)
	{
		val = defaultVal;
	}
	else
	{
		/*
		istringstream istr(elem->Attribute(attribute.c_str()));
		
		if (!(istr >> val))
		{
			err << elem->Value() << " has invalid \"" << attribute
				<< "\" attribute";
			throw(Exception(sErr(err.str(), elem)));
		}*/
		val = elem->Attribute(attribute.c_str());
	}
}

template <class T>
static void sGetMandatoryAttribute(const TiXmlElement* elem,
	const string & attribute, T & val) throw(Exception)
{
	assert(elem);
	ostringstream err;
	
	if (elem->Attribute(attribute.c_str()) == 0L)
	{
		err << elem->Value() << " needs attribute \"" << attribute << "\"";
		throw(Exception(sErr(err.str(), elem)));
	}
	
	istringstream istr(elem->Attribute(attribute.c_str()));
	
	if (!(istr >> val))
	{
		err << elem->Value() << " has invalid \"" << attribute
			<<"\" attribute";
		throw(Exception(sErr(err.str(), elem)));
	}
}

template <class T>
static bool sTryGetAttribute(const TiXmlElement* elem,
	const string & attribute, T & val)
{
	assert(elem);
	ostringstream err;
	
	if (elem->Attribute(attribute.c_str()) == 0L)
		return 0;
	else
	{
		istringstream istr(elem->Attribute(attribute.c_str()));
		
		if (!(istr >> val))
		{
			err << elem->Value() << " has invalid \"" << attribute
				<< "\" attribute";
			throw(Exception(sErr(err.str(), elem)));
		}
	}
	return 1;
}

template <class T>
static void sGetOptionalAttribute(const TiXmlElement* elem,
	const string & attribute, T & val, const T & defaultVal)
{
	assert(elem);
	ostringstream err;
	
	if (elem->Attribute(attribute.c_str()) == 0L)
	{
		val = defaultVal;
	}
	else
	{
		istringstream istr(elem->Attribute(attribute.c_str()));
		
		if (!(istr >> val))
		{
			err << elem->Value() << " has invalid \"" << attribute
				<< "\" attribute";
			throw(Exception(sErr(err.str(), elem)));
		}
	}
}


static string sErr(const string & str, const TiXmlElement* elem)
{
	ostringstream outStr;
	outStr << str << " (row " << elem->Row() << ", col " << elem->Column()
		<< ")";
	return outStr.str();
}

static FillStyle sFillStyleFromString(const string & str) throw(Exception)
{
	if (str == "PMCStyle")
		return kPMCStyle;
	else if (str == "PECStyle")
		return kPECStyle;
	throw(Exception("Invalid fill style"));
}

static Vector3i sUnitVectorFromString(const string & axisString)
	throw(Exception)
{
    Vector3i out(0,0,0);
    
    if (axisString == "x")
        out = Vector3i(1,0,0);
    else if (axisString == "-x")
        out = Vector3i(-1,0,0);
    else if (axisString == "y")
        out = Vector3i(0,1,0);
    else if (axisString == "-y")
        out = Vector3i(0, -1 ,0);
    else if (axisString == "z")
        out = Vector3i(0,0,1);
    else if (axisString == "-z")
        out = Vector3i(0,0,-1);
    else
        throw(Exception("Unit vector string must be x, -x, y, -y, z, or -z"));
	
    return out;
}

