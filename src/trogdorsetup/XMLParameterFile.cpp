/*
 *  XMLParameterFile.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 1/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLParameterFile.h"

#include "ValidateSetupAttributes.h"

#include "StreamFromString.h"
#include <sstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <set>

using namespace std;

#pragma mark *** Static Prototypes ***

static Map<string, string> sGetAttributes(const TiXmlElement* elem);
/*
template <class T>
static bool sGet(const Map<string, string> & inMap,
	const string & key, T & val);

template <class T, class T2>
static bool sGetOptional(const Map<string, string> & inMap,
	const string & key, T & val, const T2 & defaultVal);
*/
template <class T>
static void sGetMandatoryAttribute(const TiXmlElement* elem,
	const string & attribute, T & val) throw(Exception);

template <class T>
static bool sTryGetAttribute(const TiXmlElement* elem,
	const string & attribute, T & val);

template <class T, class T2>
static void sGetOptionalAttribute(const TiXmlElement* elem,
	const string & attribute, T & val, const T2 & defaultVal);

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
	
	try {
		sim.setDiscretization(Vector3f(dx,dy,dz), dt);
		sim.setDuration(numTimesteps);
		sim.setGrids(loadGrids(elem));
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem)));
	}
}

vector<GridDescPtr> XMLParameterFile::
loadGrids(const TiXmlElement* parent) const
{
	assert(parent);
	vector<GridDescPtr> gridVector;
	const TiXmlElement* elem = parent->FirstChildElement("Grid");
	
    while (elem) // FOR EACH GRID: Load data
	{
		GridDescPtr gridDesc;
		int nnx, nny, nnz;
		Rect3i activeRegion, regionOfInterest;
		string name;
		
		sGetMandatoryAttribute(elem, "name", name);
		
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
				regionOfInterest));
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		// However, these function calls all take care of their own XML file
		// row/column information so we don't need to re-throw their
		// exceptions.
		gridDesc->setOutputs(loadOutputs(elem));
		gridDesc->setInputs(loadInputEHs(elem));
		gridDesc->setSources(loadSources(elem));
		gridDesc->setMaterials(loadMaterials(elem));
		gridDesc->setAssembly(loadAssembly(elem));
		gridDesc->setLinks(loadLinks(elem));
		
		gridVector.push_back(gridDesc);
        elem = elem->NextSiblingElement("Grid");
	}
	
	return gridVector;
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
		if (paramXML == 0L)
			throw(Exception(sErr("Source needs Params element", elem)));
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

vector<TFSFSourceDescPtr> XMLParameterFile::
loadTFSFSources(const TiXmlElement* parent) const
{
	assert(parent);
	vector<TFSFSourceDescPtr> tfsfSources;
	
	const TiXmlElement* elem = parent->FirstChildElement("TFSFSource");
    while (elem)
	{
		string inClass;
		string tfsfType;
		Vector3f direction;
		Rect3i region;
		Map<string,string> params;
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("TFSFSource needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		sGetMandatoryAttribute(elem, "class", inClass);
		if (sTryGetAttribute(elem, "TFRect", region))
		{
			region *= 2;
			region.p2 += Vector3i(1,1,1);
		}
		else if (!sTryGetAttribute(elem, "fineTFRect", region))
			throw(Exception(sErr("TFSFSource needs TFRect or fineTFRect "
				"attribute", elem)));
		sGetMandatoryAttribute(elem, "direction", direction);
		sGetOptionalAttribute(elem, "type", tfsfType, "TF");
			
		try {
			TFSFSourceDescPtr source(new TFSFSourceDescription(inClass, region,
				direction, tfsfType, params));
			
			const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
			while (omitElem)
			{
				Vector3i omitSide;
				omitElem->GetText() >> omitSide;
				cerr << "Warning: no error checking on omit side input.\n";
				source->omitSide(omitSide);
				omitElem = omitElem->NextSiblingElement("OmitSide");
			}

			tfsfSources.push_back(source);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("Source");
	}
	return tfsfSources;
}

vector<LinkDescPtr> XMLParameterFile::
loadLinks(const TiXmlElement* parent) const
{
	assert(parent);
	vector<LinkDescPtr> links;
	cerr << "Warning: links are not validated.\n";
	
	const TiXmlElement* elem = parent->FirstChildElement("Link");
    while (elem)
	{
		string linkTypeStr;
		string sourceGridName;
		Rect3i sourceHalfRect;
		Rect3i destHalfRect;
		Map<string,string> params;
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Link needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
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
			LinkDescPtr link(new LinkDescription(linkTypeStr, sourceGridName,
				sourceHalfRect, destHalfRect));
			
			const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
			while (omitElem)
			{
				Vector3i omitSide;
				omitElem->GetText() >> omitSide;
				link->omitSide(omitSide);
				omitElem = omitElem->NextSiblingElement("OmitSide");
			}
			
			links.push_back(link);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("Link");
	}
	
	return links;
}

vector<MaterialDescPtr> XMLParameterFile::
loadMaterials(const TiXmlElement* parent) const
{
	assert(parent);
	vector<MaterialDescPtr> materials;
	
	const TiXmlElement* elem = parent->FirstChildElement("Material");
    while (elem)
	{
		string name;
		string inClass;
		Map<string,string> params;
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Material needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		sGetMandatoryAttribute(elem, "name", name);
		sGetMandatoryAttribute(elem, "class", inClass);
		
		MaterialDescPtr material(new MaterialDescription(name, inClass,
			params));
		materials.push_back(material);
		
		elem = elem->NextSiblingElement("Material");
	}
	
	return materials;
}

AssemblyDescPtr XMLParameterFile::
loadAssembly(const TiXmlElement* parent) const
{
	assert(parent);
	AssemblyDescPtr assembly;
	vector<AssemblyDescription::InstructionPtr> recipe;
	
	const TiXmlElement* elem = parent->FirstChildElement("Assembly");
	
	if (elem->NextSiblingElement("Assembly") != 0L)
		throw(Exception(sErr("Only one Assembly element allowed per Grid",
			elem->NextSiblingElement("Assembly"))));
	
	const TiXmlElement* instruction = elem->FirstChildElement();
	
	while (instruction)
	{
		string instructionType = instruction->Value();
		
		if (instructionType == "Block")
			recipe.push_back(loadABlock(instruction));
		else if (instructionType == "KeyImage")
			recipe.push_back(loadAKeyImage(instruction));
		else if (instructionType == "HeightMap")
			recipe.push_back(loadAHeightMap(instruction));
		else if (instructionType == "Ellipsoid")
			recipe.push_back(loadAEllipsoid(instruction));
		else if (instructionType == "CopyFrom")
			recipe.push_back(loadACopyFrom(instruction));
		
		instruction = instruction->NextSiblingElement();
	}
	
	assembly = AssemblyDescPtr(new AssemblyDescription(recipe));
	return assembly;
}


AssemblyDescription::InstructionPtr XMLParameterFile::
loadABlock(const TiXmlElement* elem) const
{
	assert(elem);
	assert(elem->Value() == "Block");
	
	AssemblyDescription::InstructionPtr blockPtr;
	Rect3i rect;
	string material;
	string fillStyleStr;
	FillStyle style;
	
	sGetMandatoryAttribute(elem, "material", material);
	
	
	// Does the Block have a Yee region, called "fillRect"?
	if (sTryGetAttribute(elem, "fillRect", rect))
	{
		sGetOptionalAttribute(elem, "fillStyle", fillStyleStr, "PECStyle");
		try {
			style = sFillStyleFromString(fillStyleStr); // may fail
		} catch (const Exception & e) {
			throw(Exception(sErr(e.what(), elem))); // rethrow with XML row/col
		}
		try {
			blockPtr = AssemblyDescription::InstructionPtr(
				(AssemblyDescription::Block*)new AssemblyDescription::Block(
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
			blockPtr = AssemblyDescription::InstructionPtr(
				new AssemblyDescription::Block(rect, material));
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

AssemblyDescription::InstructionPtr XMLParameterFile::
loadAKeyImage(const TiXmlElement* elem) const
{
	assert(elem);
	assert(elem->Value() == "KeyImage");
	
	AssemblyDescription::InstructionPtr keyImage;
	Rect3i yeeRect;
	string imageFileName;
	string rowDirStr;
	string colDirStr;
	Vector3i rowDir;
	Vector3i colDir;
	vector<AssemblyDescription::ColorKey> keys;
	
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
	keys = loadAColorKeys(elem);
	
	// Package it up and ship.
	
	try {
		keyImage = AssemblyDescription::InstructionPtr(
			new AssemblyDescription::KeyImage(yeeRect, imageFileName, rowDir,
				colDir, keys));
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem)));
	}
	
	return keyImage;
}

vector<AssemblyDescription::ColorKey> XMLParameterFile::
loadAColorKeys(const TiXmlElement* parent) const
{
	assert(parent);
	vector<AssemblyDescription::ColorKey> keys;

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
		
		sGetMandatoryAttribute(elem, "color", colorStr);
		sGetOptionalAttribute(elem, "fillStyle", fillStyleStr, "PECStyle");
		
		try {
			style = sFillStyleFromString(fillStyleStr);
			AssemblyDescription::ColorKey key(colorStr, material, style);
			keys.push_back(key);
		} catch (const Exception & e) {
			throw(Exception(sErr(e.what(), elem))); // rethrow w/ row/col
		}
		
		elem = elem->NextSiblingElement("Tag");
	}
	
	return keys;
}

AssemblyDescription::InstructionPtr XMLParameterFile::
loadAHeightMap(const TiXmlElement* elem) const
{
	assert(elem);
	assert(elem->Value() == "HeightMap");
	
	AssemblyDescription::InstructionPtr heightMap;
	
	Rect3i yeeRect;
	string styleStr;
	FillStyle style;
	string material;
	string imageFileName;
	string rowDirStr, colDirStr, upDirStr;
	Vector3i rowDir, colDir, upDir;
	
	sGetMandatoryAttribute(elem, "fillRect", yeeRect);
	sGetMandatoryAttribute(elem, "material", material);
	sGetMandatoryAttribute(elem, "file", imageFileName);
	sGetMandatoryAttribute(elem, "row", rowDirStr);
	sGetMandatoryAttribute(elem, "column", colDirStr);
	sGetMandatoryAttribute(elem, "up", upDirStr);
	sGetOptionalAttribute(elem, "fillStyle", styleStr, "PECStyle");
	
	try {
		style = sFillStyleFromString(styleStr);
		rowDir = sUnitVectorFromString(rowDirStr);
		colDir = sUnitVectorFromString(colDirStr);
		upDir = sUnitVectorFromString(upDirStr);
		
		heightMap = AssemblyDescription::InstructionPtr(
			new AssemblyDescription::HeightMap(yeeRect, style, material,
				imageFileName, rowDir, colDir, upDir));
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem))); // append XML row/col info
	}
	
	return heightMap;
}
AssemblyDescription::InstructionPtr XMLParameterFile::
loadAEllipsoid(const TiXmlElement* elem) const
{
	assert(elem);
	assert(elem->Value() == "Ellipsoid");
	
	AssemblyDescription::InstructionPtr ellipsoid;
	Rect3i rect;
	string material;
	string styleStr;
	FillStyle style;
	
	sGetMandatoryAttribute(elem, "material", material);
	
	if (sTryGetAttribute(elem, "fillRect", rect))
	{
		sGetOptionalAttribute(elem, "fillStyle", styleStr, "PECStyle");
		
		try {
			style = sFillStyleFromString(styleStr);
			ellipsoid = AssemblyDescription::InstructionPtr(
				new AssemblyDescription::Ellipsoid(rect, style, material));
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
	}
	else
	{
		if (!sTryGetAttribute(elem, "fineFillRect", rect))
			throw(Exception(sErr("Ellipsoid needs fillRect or fineFillRect "
				"attribute", elem)));
		
		ellipsoid = AssemblyDescription::InstructionPtr(
			new AssemblyDescription::Ellipsoid(rect, material));
	}
	return ellipsoid;
}
AssemblyDescription::InstructionPtr XMLParameterFile::
loadACopyFrom(const TiXmlElement* elem) const
{
	assert(elem);
	assert(elem->Value() == "CopyFrom");
	
	AssemblyDescription::InstructionPtr copyFrom;
	Rect3i sourceRect, destRect;
	string sourceGridName;
	
	sGetMandatoryAttribute(elem, "sourceRect", sourceRect);
	sGetMandatoryAttribute(elem, "destRect", destRect);
	sGetMandatoryAttribute(elem, "sourceGrid", sourceGridName);
	
	try {
		copyFrom = AssemblyDescription::InstructionPtr(
			new AssemblyDescription::CopyFrom(
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

/*
template <class T>
static bool sGet(const Map<string, std::string> & inMap,
	const string & key, T & val)
{
	if (inMap.count(key) == 0)
		return 0;
	
	istringstream istr(inMap[key]);
	
	if (!(istr >> val))
		return 0;
	
	return 1;
}

template <class T, class T2>
static bool sGetOptional(const Map<string, string> & inMap,
	const string & key, T & val, const T2 & defaultVal)
{
	if (inMap.count(key) == 0)
	{
		val = defaultVal;
		return 1;
	}
	
	istringstream istr(inMap[key]);
	
	if (!(istr >> val))
		return 0;
	
	return 1;
}
*/

template <class T>
static void sGetMandatoryAttribute(const TiXmlElement* elem,
	const string & attribute, T & val) throw(Exception)
{
	assert(elem);
	ostringstream err;
	
	if (elem->Attribute(attribute.c_str()) == 0L)
	{
		err << elem->Value() << " needs attribute " << attribute;
		throw(Exception(sErr(err.str(), elem)));
	}
	
	istringstream istr(elem->Attribute(attribute.c_str()));
	
	if (!(istr >> val))
	{
		err << elem->Value() << " has invalid " << attribute << " attribute";
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
			err << elem->Value() << " has invalid " << attribute
				<< " attribute";
			throw(Exception(sErr(err.str(), elem)));
		}
	}
	return 1;
}

template <class T, class T2>
static void sGetOptionalAttribute(const TiXmlElement* elem,
	const string & attribute, T & val, const T2 & defaultVal)
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
			err << elem->Value() << " has invalid " << attribute
				<< " attribute";
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

