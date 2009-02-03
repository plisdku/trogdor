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

static Map<string, string> sGetAttributes(const TiXmlElement* elem);

template <class T>
static bool sGet(const Map<string, string> & map,
	const string & key, T & val);

static string sErr(const string & str, const TiXmlElement* elem);

XMLParameterFile::
XMLParameterFile(const string & filename) :
	mDocument(filename.c_str())
{
	string errorMessage;
	
    TiXmlBase::SetCondenseWhiteSpace(0); // for ascii material layers with \n
	
	if (!(mDocument.LoadFile()))
    {
        cerr << "Could not load parameter file " << filename << endl;
        cerr << "Reason: " << mDocument.ErrorDesc() << endl;
        cerr << "(possible location: row " << mDocument.ErrorRow() << " column "
             << mDocument.ErrorCol() << " .)" << endl;
        exit(1);
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
	
	Map<string, string> attributes = sGetAttributes(elem);
	if (attributes.count("version") == 0)
	{
		loadTrogdor4(sim);
	}
	else
	{
		ostringstream err;
		err << "Cannot read version " << attributes["version"] << " file.";
		throw Exception(err.str());
	}
}

void XMLParameterFile::
loadTrogdor4(SimulationDescription & sim) const throw(Exception)
{
    const TiXmlElement* elem = mDocument.RootElement();
	assert(elem);  // this condition was checked in load().
	
	Map<string, string> simulationParams;
	
    simulationParams = sGetAttributes(elem);
    
	sim.setGrids(loadGrids(elem));
}


vector<GridDescPtr> XMLParameterFile::
loadGrids(const TiXmlElement* parent) const
{
	assert(parent);
	vector<GridDescPtr> gridVector;
	const TiXmlElement* elem = parent->FirstChildElement("Grid");
	
    while (elem) // FOR EACH GRID: Load data
	{
		int nnx, nny, nnz;
		Rect3i activeRegion, regionOfInterest;
		string name;
		Map<string, string> gridParams = sGetAttributes(elem);
		
		if (!sGet(gridParams, "name", name))
			throw(Exception(sErr("Can't load name", elem)));
		
		if (sGet(gridParams, "nx", nnx))
			nnx *= 2;
		else if (!sGet(gridParams, "nnx", nnx))
			throw(Exception(sErr("Can't load nnx or nx", elem)));
		
		if (sGet(gridParams, "ny", nny))
			nny *= 2;
		else if (!sGet(gridParams, "nny", nny))
			throw(Exception(sErr("Can't load nny or ny", elem)));
		
		if (sGet(gridParams, "nz", nnz))
			nnz *= 2;
		else if (!sGet(gridParams, "nnz", nnz))
			throw(Exception(sErr("Can't load nnz or nz", elem)));
		
		if (sGet(gridParams, "regionOfInterest", regionOfInterest))
		{
			regionOfInterest *= 2;
			regionOfInterest.p2 += Vector3i(1,1,1);
		}
		else if (!sGet(gridParams, "roi", regionOfInterest))
			throw(Exception(sErr("Can't load region of interest", elem)));
		
		if (!sGet(gridParams, "activeRegion", activeRegion))
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
		
		GridDescPtr gridDesc(new GridDescription(name, Vector3i(nnx,nny,nnz)/2,
			Vector3i(nnx,nny,nnz), activeRegion, regionOfInterest));
		
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
		Map<string,string> attribs = sGetAttributes(elem);
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Input needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		if (!sGet(attribs, "filePrefix", filePrefix))
			throw(Exception(sErr("Input needs filePrefix attribute", elem)));
		if (!sGet(attribs, "class", inputClass))
			throw(Exception(sErr("Input needs class attribute", elem)));
		
		InputEHDescPtr input(new InputEHDescription(filePrefix, inputClass,
			params));
		inputs.push_back(input);
		
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
		Map<string,string> attribs = sGetAttributes(elem);
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Output needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		if (!sGet(attribs, "filePrefix", filePrefix))
			throw(Exception(sErr("Output needs filePrefix attribute", elem)));
		if (!sGet(attribs, "class", outputClass))
			throw(Exception(sErr("Output needs class attribute", elem)));
		if (!sGet(attribs, "period", period))
			period = 1;
		
		OutputDescPtr output(new OutputDescription(filePrefix, outputClass,
			period, params));
		outputs.push_back(output);
		
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
		Map<string,string> attribs = sGetAttributes(elem);
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Source needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		if (!sGet(attribs, "formula", formula) &&
			!sGet(attribs, "file", filename))
			throw(Exception(sErr("Source needs formula or file attribute",
				elem)));
		if (!sGet(attribs, "field", field))
			throw(Exception(sErr("Source needs field attribute", elem)));
		if (!sGet(attribs, "polarization", polarization))
			throw(Exception(sErr("Source needs polarization attribute", elem)));
		if (!sGet(attribs, "region", region))
			throw(Exception(sErr("Source needs region attribute", elem)));
		
		SourceDescPtr source(new SourceDescription(formula, filename,
			polarization, region, field, params));
		sources.push_back(source);
		
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
		Map<string,string> attribs = sGetAttributes(elem);
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("TFSFSource needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		if (!sGet(attribs, "class", inClass))
			throw(Exception(sErr("TFSFSource needs class attribute", elem)));
		if (sGet(attribs, "TFRect", region))
		{
			region *= 2;
			region.p2 += Vector3i(1,1,1);
		}
		else if (!sGet(attribs, "fineTFRect", region))
			throw(Exception(sErr("TFSFSource needs TFRect or fineTFRect "
				"attribute", elem)));
		if (!sGet(attribs, "direction", direction))
			throw(Exception(sErr("TFSFSource needs direction attribute",
				elem)));
		if (!sGet(attribs, "type", tfsfType))
			tfsfType = "TF";
		
		TFSFSourceDescPtr source(new TFSFSourceDescription(inClass, region,
			direction, tfsfType, params));
		tfsfSources.push_back(source);
		
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
		Map<string,string> attribs = sGetAttributes(elem);
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Link needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		if (!sGet(attribs, "type", linkTypeStr))
			throw(Exception(sErr("Link needs type attribute", elem)));
		if (!sGet(attribs, "sourceGrid", sourceGridName))
			throw(Exception(sErr("Link needs sourceGrid attribute", elem)));
		if (sGet(attribs, "sourceRect", sourceHalfRect))
		{
			sourceHalfRect.p1 *= 2;
			sourceHalfRect.p2 = 2*sourceHalfRect.p2 + Vector3i(1,1,1);
		}
		else if (!sGet(attribs, "fineSourceRect", sourceHalfRect))
			throw(Exception(sErr("Link needs sourceRect or fineSourceRect "
				"attribute", elem)));
		if (sGet(attribs, "destRect", destHalfRect))
		{
			destHalfRect.p1 *= 2;
			destHalfRect.p2 = 2*destHalfRect.p2 + Vector3i(1,1,1);
		}
		else if (!sGet(attribs, "fineDestRect", destHalfRect))
			throw(Exception(sErr("Link needs destRect or fineDestRect "
				"attribute", elem)));
		
		LinkDescPtr link(new LinkDescription(linkTypeStr, sourceGridName,
			sourceHalfRect, destHalfRect));
		
		set<Vector3i> omitSides;
        const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
        while (omitElem)
        {
            Vector3i omitSide;
            omitElem->GetText() >> omitSide;
            link->omitSide(omitSide);
			omitSides.insert(omitSide);
            omitElem = omitElem->NextSiblingElement("OmitSide");
        }
		
		links.push_back(link);
		
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
		Map<string,string> attribs = sGetAttributes(elem);
		const TiXmlElement* paramXML = elem->FirstChildElement("Params");
		if (paramXML == 0L)
			throw(Exception(sErr("Material needs Params element", elem)));
		params = sGetAttributes(paramXML);
		
		if (!sGet(attribs, "name", name))
			throw(Exception(sErr("Material needs name attribute", elem)));
		if (!sGet(attribs, "class", inClass))
			throw(Exception(sErr("Material needs class attribute", elem)));
		
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
	
	const TiXmlElement* elem = parent->FirstChildElement("Assembly");
	
	if (elem->NextSiblingElement("Assembly") != 0L)
		throw(Exception(sErr("Only one Assembly element allowed per Grid",
			elem->NextSiblingElement("Assembly"))));
	
	return assembly;
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


template <class T>
static bool sGet(const Map<std::string, std::string> & inMap,
	const std::string & key, T & val)
{
	if (inMap.count(key) == 0)
		return 0;
	
	istringstream istr(inMap[key]);
	
	if (!(istr >> val))
		return 0;
	
	return 1;
}

static string sErr(const string & str, const TiXmlElement* elem)
{
	ostringstream outStr;
	outStr << str << " (row " << elem->Row() << ", col " << elem->Column()
		<< ")";
	return outStr.str();
}

