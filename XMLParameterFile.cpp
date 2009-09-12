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

#include "YeeUtilities.h"
#include "StreamFromString.h"
#include "ConvertOldXML.h"
#include "XMLExtras.h"
#include <sstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
#include <climits>

using namespace std;
using namespace YeeUtilities;

static FillStyle sFillStyleFromString(const string & str) throw(Exception);

static Vector3i sUnitVectorFromString(const string & axisString)
	throw(Exception);

#pragma mark *** XMLParameterFile Implementation ***

XMLParameterFile::
XMLParameterFile(const string & filename) throw(Exception)
{
	string errorMessage, versionString;
	
    TiXmlBase::SetCondenseWhiteSpace(0); // for ascii material layers with \n
	
    Pointer<TiXmlDocument> thisDoc(new TiXmlDocument(filename.c_str()));
	if (!(thisDoc->LoadFile()))
    {
		ostringstream err;
        err << "Could not load parameter file " << filename << endl;
        err << "Reason: " << thisDoc->ErrorDesc() << endl;
        err << "(possible location: row " << thisDoc->ErrorRow() << " column "
             << thisDoc->ErrorCol() << " .)" << endl;
        throw(Exception(err.str()));
    }
    if (!sTryGetAttribute(thisDoc->RootElement(), "version", versionString))
    {
        try {
            Pointer<TiXmlDocument> v5doc = ConvertOldXML::convert4to5(thisDoc);
            mDocument = v5doc;
            mDocument->SaveFile("converted.xml");
        } catch (Exception & e) {
            ostringstream myErr;
            myErr << "Could not convert version 4 to version 5 file.\n";
            myErr << e.what();
            throw(Exception(myErr.str()));
        }
    }
    else
        mDocument = thisDoc;
}

// It is the job of XMLParameterFile to validate the XML and to verify that 
// all the necessary elements and attributes are present and properly-formatted.
// It is the job of the SimulationDescription etc. classes to verify that the
// information in the XML file makes sense; this is handled in the constructors.

void XMLParameterFile::
load(SimulationDescription & sim) const throw(Exception)
{
	const TiXmlElement* elem = mDocument->RootElement();
	string versionString;
    ostringstream err;
    
	if (!elem)
		throw Exception("XML file has no root element.");
    
    if (!sTryGetAttribute(elem, "version", versionString))
        throw Exception("Simulation is unversioned!");
    if (versionString != "5.0")
    {
        err << "Cannot read version " << versionString << " file.";
        throw(Exception(sErr(err.str(), elem)));
    }
	
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
		sim.setNumTimesteps(numTimesteps);
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem)));
	}
	// (i.e. these three calls don't throw exceptions.)
	sim.setMaterials(loadMaterials(elem));
	sim.setGrids(loadGrids(elem, sim));
	sim.setAllPointers();
}

vector<GridDescPtr> XMLParameterFile::
loadGrids(const TiXmlElement* parent, const SimulationDescription & sim) const
{
	assert(parent);
	vector<GridDescPtr> gridVector;
	const TiXmlElement* elem = parent->FirstChildElement("Grid");
	
	set<string> allGridNames = collectGridNames(parent);
	set<string> allMaterialNames = collectMaterialNames(parent);
	
    while (elem) // FOR EACH GRID: Load data
	{
		GridDescPtr gridDesc;
        Vector3i numYeeCells;
        Vector3i numHalfCells;
		Rect3i nonPMLYeeCells;
        Rect3i nonPMLHalfCells, calcRegionHalfCells;
        Map<Vector3i, Map<string, string> > pmlParams;
		string name;
		Vector3i origin;
		
		sGetMandatoryAttribute(elem, "name", name);
		sGetOptionalAttribute(elem, "origin", origin, Vector3i(0,0,0));
		
        sGetMandatoryAttribute(elem, "nx", numYeeCells[0]);
        sGetMandatoryAttribute(elem, "ny", numYeeCells[1]);
        sGetMandatoryAttribute(elem, "nz", numYeeCells[2]);
        numHalfCells = 2*numYeeCells;
        
        sGetMandatoryAttribute(elem, "nonPML", nonPMLYeeCells);
        nonPMLHalfCells = yeeToHalf(nonPMLYeeCells);
		
        // The calc region should stop a half cell short of the boundary
        // when there is PML, to prevent leakage around the edge of the sim.
        // The default is a periodic boundary and the calc region needs to go
        // right to the edges.
		if (!sTryGetAttribute(elem, "calcRegionHalfCells", calcRegionHalfCells))
		{
            calcRegionHalfCells.p1 = Vector3i(1,1,1);
            calcRegionHalfCells.p2 = Vector3i(numHalfCells - Vector3i(2,2,2));
            for (int xyz = 0; xyz < 3; xyz++)
            {
                if (nonPMLHalfCells.p1[xyz] == 0)
                    calcRegionHalfCells.p1[xyz] = 0;
                if (nonPMLHalfCells.p2[xyz] == numHalfCells[xyz]-1)
                    calcRegionHalfCells.p2[xyz] = numHalfCells[xyz]-1;
            }
		}
        
        const TiXmlElement* pmlParamXML = elem->FirstChildElement("PML");
        while (pmlParamXML != 0L)
        {
            Vector3i direction;
            Map<string, string> theseParams = sGetAttributes(pmlParamXML);
            theseParams.erase("direction");
            
            if (sTryGetAttribute(elem, "direction", direction))
                pmlParams[direction] = theseParams;
            else
            for (int sideNum = 0; sideNum < 6; sideNum++)
                pmlParams[cardinal(sideNum)] = theseParams;
            
            pmlParamXML = pmlParamXML->NextSiblingElement("PML");
        }
        
		
		// We need to wrap the constructor errors from GridDescription() to
		// put XML row/column information in.
		try {
			gridDesc = GridDescPtr(new GridDescription(name,
				numYeeCells, calcRegionHalfCells,
				nonPMLHalfCells, origin, sim.dxyz(),
                sim.dt()));
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		// However, these function calls all take care of their own XML file
		// row/column information so we don't need to re-throw their
		// exceptions.
		gridDesc->setOutputs(loadOutputs(elem));
        gridDesc->setGridReports(loadGridReports(elem));
		gridDesc->setSources(loadSources(elem));
        gridDesc->setCurrentSources(loadCurrentSources(elem));
		gridDesc->setAssembly(loadAssembly(elem, allGridNames,
			allMaterialNames));
        try {
            gridDesc->setPMLParams(pmlParams);
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
		
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
	
    const TiXmlElement* matElem = parent->FirstChildElement("Material");
    
    while (matElem)
    {
        string matName;
        sGetMandatoryAttribute(matElem, "name", matName);
        names.insert(matName);
        matElem = matElem->NextSiblingElement("Material");
    }
    
	return names;
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
        string inModel;
        Map<string,string> params;
        Map<Vector3i, Map<string, string> > pmlParams;
        const TiXmlElement* paramXML = elem->FirstChildElement("Params");
        const TiXmlElement* pmlParamXML;
        // not every material needs params, e.g. PEC/PMC
        if (paramXML != 0L)
            params = sGetAttributes(paramXML);
        
        sGetMandatoryAttribute(elem, "name", name);
        sGetMandatoryAttribute(elem, "model", inModel);
        
        pmlParamXML = elem->FirstChildElement("PML");
        while (pmlParamXML != 0L)
        {
            Vector3i direction;
            Map<string, string> theseParams = sGetAttributes(pmlParamXML);
            theseParams.erase("direction");
            
            if (sTryGetAttribute(elem, "direction", direction))
                pmlParams[direction] = theseParams;
            else
            for (int sideNum = 0; sideNum < 6; sideNum++)
                pmlParams[cardinal(sideNum)] = theseParams;
            
            pmlParamXML = pmlParamXML->NextSiblingElement("PML");
        }
        
        // The material ID will be the same as its index.
        try {
            MaterialDescPtr material(new MaterialDescription(
                materials.size(), name, inModel,
                params, pmlParams));
            materials.push_back(material);
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
        
        elem = elem->NextSiblingElement("Material");
    }
    
    for (int nn = 0; nn < materials.size(); nn++)
        assert(materials[nn]->id() == nn);
	
	return materials;	
}


vector<OutputDescPtr> XMLParameterFile::
loadOutputs(const TiXmlElement* parent) const
{
	assert(parent);
	vector<OutputDescPtr> outputs;
	
	const TiXmlElement* elem = parent->FirstChildElement("FieldOutput");
    while (elem)
	{
        outputs.push_back(loadAFieldOutput(elem));
        elem = elem->NextSiblingElement("FieldOutput");
    }
	return outputs;
}

OutputDescPtr XMLParameterFile::
loadAFieldOutput(const TiXmlElement* elem) const
{
    string fields;
    string file;
    Vector3f interpolationPoint;
    bool isInterpolated;
    Map<string,string> attribs = sGetAttributes(elem);
    Map<string,string> childAttribs;
    sGetMandatoryAttribute(elem, "fields", fields);
    sGetMandatoryAttribute(elem, "file", file);
    isInterpolated = sTryGetAttribute(elem, "interpolate", interpolationPoint);
    
    vector<Region> regions;
    vector<Duration> durations;
    
    const TiXmlElement* child = elem->FirstChildElement("Duration");
    while (child != 0L)
    {
        durations.push_back(loadADuration(child));
        child = child->NextSiblingElement("Duration");
    }
    child = elem->FirstChildElement("Region");
    while (child != 0L)
    {
        regions.push_back(loadARegion(child));
        child = child->NextSiblingElement("Region");
    }
    
    if (regions.size() == 0)
        regions.push_back(Region());
    if (durations.size() == 0)
        durations.push_back(Duration());
    
    if (isInterpolated)
    {
        try {
            OutputDescPtr f(new OutputDescription(
                fields, file, interpolationPoint, regions, durations));
            return f;
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else
    {
        try {
            OutputDescPtr f(new OutputDescription(
                fields, file, regions, durations));
            return f;
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    
    LOG << "Shouldn't have gotten here.\n";
    return OutputDescPtr(0L);
}


vector<GridReportDescPtr> XMLParameterFile::
loadGridReports(const TiXmlElement* parent) const
{
	assert(parent);
	vector<GridReportDescPtr> reports;
	
	const TiXmlElement* elem = parent->FirstChildElement("GridReport");
    while (elem)
	{
        reports.push_back(loadAGridReport(elem));
        elem = elem->NextSiblingElement("GridReport");
    }
	return reports;
}

GridReportDescPtr XMLParameterFile::
loadAGridReport(const TiXmlElement* elem) const
{
    assert(elem);
    const TiXmlElement* child;
    string fields;
    string fileName;
    vector<Region> regions;
    
    sGetMandatoryAttribute(elem, "file", fileName);
    
    child = elem->FirstChildElement("Region");
    while (child != 0L)
    {
        regions.push_back(loadARegion(child));
        child = child->NextSiblingElement("Region");
    }
    
    GridReportDescPtr reportPtr;
    if (sTryGetAttribute(elem, "fields", fields))
    {
        cerr << "Warning: loading GridReport with specified fields, but for "
            "now the fields will be ignored and every octant will be present in"
            " the output file.\n";
        try {
            reportPtr = GridReportDescPtr(new GridReportDescription(
                fields, fileName, regions));
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else
    {
        try {
            reportPtr = GridReportDescPtr(new GridReportDescription(
                fileName, regions));
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    return reportPtr;
}

vector<SourceDescPtr> XMLParameterFile::
loadSources(const TiXmlElement* parent) const
{
    assert(parent);
    vector<SourceDescPtr> sources;
    
    const TiXmlElement* child = parent->FirstChildElement("HardSource");
    while (child != 0L)
    {
        sources.push_back(loadAFieldSource(child));
        child = child->NextSiblingElement("HardSource");
    }
    
    child = parent->FirstChildElement("AdditiveSource");
    while (child != 0L)
    {
        sources.push_back(loadAFieldSource(child));
        child = child->NextSiblingElement("AdditiveSource");
    }
    return sources;
}

SourceDescPtr XMLParameterFile::
loadAFieldSource(const TiXmlElement* elem) const
{
    assert(elem);
    string fields;
    string formula;
    string timeFile;
    string spaceFile;
    string spaceTimeFile;
    Vector3f polarization;
    SourceFields srcFields;
    vector<Duration> durations;
    vector<Region> regions;
    bool isSoft = (string("AdditiveSource") == elem->Value());
    
    sGetMandatoryAttribute(elem, "fields", fields);
    if (sTryGetAttribute(elem, "polarization", polarization))
    {
        try {
            srcFields = SourceFields(fields, polarization);
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else
    {
        try {
            srcFields = SourceFields(fields);
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    
    const TiXmlElement* child = elem->FirstChildElement("Duration");
    while (child != 0L)
    {
        durations.push_back(loadADuration(child));
        child = child->NextSiblingElement("Duration");
    }
    if (durations.size() == 0)
        durations.push_back(Duration());
    
    child = elem->FirstChildElement("Region");
    while (child != 0L)
    {
        regions.push_back(loadARegion(child));
        child = child->NextSiblingElement("Region");
    }
    
    SourceDescPtr src;
    if (sTryGetAttribute(elem, "timeFile", timeFile))
    {
        try {
            src = SourceDescPtr(SourceDescription::
                newTimeSource(timeFile, srcFields, isSoft, regions, durations));
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else if (sTryGetAttribute(elem, "spaceTimeFile", spaceTimeFile))
    {
        try {
            src = SourceDescPtr(SourceDescription::
                newSpaceTimeSource(spaceTimeFile, srcFields, isSoft, regions,
                    durations));
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else if (sTryGetAttribute(elem, "formula", formula))
    {
        try {
            src = SourceDescPtr(SourceDescription::
                newFormulaSource(formula, srcFields, isSoft, regions, durations));
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else
        throw(Exception(sErr("HardSource needs timeFile, spaceTimeFile or "
            "formula attribute.", elem)));
    
    return src;
}

vector<CurrentSourceDescPtr> XMLParameterFile::
loadCurrentSources(const TiXmlElement* parent) const
{    assert(parent);

    vector<CurrentSourceDescPtr> sources;
    
    const TiXmlElement* child = parent->FirstChildElement("CurrentSource");
    while (child != 0L)
    {
        sources.push_back(loadACurrentSource(child));
        child = child->NextSiblingElement("CurrentSource");
    }
    
    return sources;
}


CurrentSourceDescPtr XMLParameterFile::
loadACurrentSource(const TiXmlElement* elem) const
{
    assert(elem);
    string fields;
    string formula;
    string timeFile;
    string spaceFile;
    string spaceTimeFile;
    Vector3f polarization;
    SourceCurrents srcCurrents;
    vector<Duration> durations;
    vector<Region> regions;
    
    sGetMandatoryAttribute(elem, "fields", fields);
    if (sTryGetAttribute(elem, "polarization", polarization))
    {
        try {
            srcCurrents = SourceCurrents(fields, polarization);
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else
    {
        try {
            srcCurrents = SourceCurrents(fields);
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    
    const TiXmlElement* child = elem->FirstChildElement("Duration");
    while (child != 0L)
    {
        durations.push_back(loadADuration(child));
        child = child->NextSiblingElement("Duration");
    }
    if (durations.size() == 0)
        durations.push_back(Duration());
    
    child = elem->FirstChildElement("Region");
    while (child != 0L)
    {
        regions.push_back(loadARegion(child));
        child = child->NextSiblingElement("Region");
    }
    
    CurrentSourceDescPtr src;
    if (sTryGetAttribute(elem, "timeFile", timeFile))
    {
        try {
            src = CurrentSourceDescPtr(CurrentSourceDescription::
                newTimeSource(timeFile, srcCurrents, regions, durations));
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else if (sTryGetAttribute(elem, "spaceTimeFile", spaceTimeFile))
    {
        try {
            src = CurrentSourceDescPtr(CurrentSourceDescription::
                newSpaceTimeSource(spaceTimeFile, srcCurrents, regions,
                    durations));
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else if (sTryGetAttribute(elem, "formula", formula))
    {
        try {
            src = CurrentSourceDescPtr(CurrentSourceDescription::
                newFormulaSource(formula, srcCurrents, regions, durations));
        } catch (Exception & e) {
            throw(Exception(sErr(e.what(), elem)));
        }
    }
    else
        throw(Exception(sErr("CurrentSource needs timeFile, spaceTimeFile or "
            "formula attribute.", elem)));
    
    return src;
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
		string field;
		Vector3f polarization;
        Rect3i temp, halfCells;
		Vector3i direction;
		string tfsfType;
		string formula;
		string file;
        SourceFields srcFields;
        Duration duration;
        
        const TiXmlElement *durationXML;
        durationXML = elem->FirstChildElement("Duration");
        if (durationXML)
            duration = loadADuration(durationXML);
        
        if (sTryGetAttribute(elem, "yeeCells", temp))
            halfCells = yeeToHalf(temp);
        else if (!sTryGetAttribute(elem, "halfCells", halfCells))
            throw(Exception(sErr("TFSFSource needs yeeCells or halfCells "
                "attribute", elem)));
        
        sGetMandatoryAttribute(elem, "direction", direction);
		sGetOptionalAttribute(elem, "type", tfsfType, string("TF"));
        
        if (!sTryGetAttribute(elem, "formula", formula) &&
            !sTryGetAttribute(elem, "file", file))
            throw(Exception(sErr("TFSFSource needs either a formula or file"
                " attribute.", elem)));
        
		sGetMandatoryAttribute(elem, "fields", field);
        if (sTryGetAttribute(elem, "polarization", polarization))
            srcFields = SourceFields(field, polarization);
        else
            srcFields = SourceFields(field);
        
        set<Vector3i> omittedSides;
        const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
        while (omitElem)
        {
            omittedSides.insert(loadAnOmitSide(omitElem));
            omitElem = omitElem->NextSiblingElement("OmitSide");
        }
        
		try {
            if (file != "")
                tfsfSources.push_back(HuygensSurfaceDescPtr(
                    HuygensSurfaceDescription::newTFSFTimeSource(srcFields,
                        file, direction, halfCells, omittedSides,
                        tfsfType == "TF")));
            else
                tfsfSources.push_back(HuygensSurfaceDescPtr(
                    HuygensSurfaceDescription::newTFSFFormulaSource(srcFields,
                        formula, direction, halfCells, omittedSides,
                        tfsfType == "TF")));
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
	
	const TiXmlElement* elem = parent->FirstChildElement("CustomTFSFSource");
    while (elem)
	{
        string file;
        Vector3i symmetries(0,0,0);
        string tfsfType;
        Rect3i yeeCells;
        Rect3i halfCells;
        set<Vector3i> omittedSides;
        const TiXmlElement* durationXML;
        Duration duration;
		
		sGetMandatoryAttribute(elem, "file", file);
        sGetMandatoryAttribute(elem, "symmetries", symmetries);
        sGetOptionalAttribute(elem, "tfsfType", tfsfType, string("TF"));
        if (sTryGetAttribute(elem, "yeeCells", yeeCells))
            halfCells = yeeToHalf(yeeCells);
        else if (!sTryGetAttribute(elem, "halfCells", halfCells))
            throw(Exception(sErr("CustomTFSFSSource needs yeeCells or "
                "halfCells attribute.", elem)));
        
        durationXML = elem->FirstChildElement("Duration");
        if (durationXML != 0L)
            duration = loadADuration(durationXML);
        
        const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
        while (omitElem)
        {
            omittedSides.insert(loadAnOmitSide(omitElem));
            omitElem = omitElem->NextSiblingElement("OmitSide");
        }
        
        try {
            HuygensSurfaceDescPtr source(HuygensSurfaceDescription::
                newCustomTFSFSource(file, symmetries, halfCells, duration,
                omittedSides, tfsfType=="TF"));
            customSources.push_back(source);
		} catch (Exception & e) {
			throw(Exception(sErr(e.what(), elem)));
		}
		
		elem = elem->NextSiblingElement("CustomTFSFSource");
	}
	return customSources;
}

vector<HuygensSurfaceDescPtr> XMLParameterFile::
loadLinks(const TiXmlElement* parent, const set<string> & allGridNames) const
{
	assert(parent);
	vector<HuygensSurfaceDescPtr> links;
	
	const TiXmlElement* elem = parent->FirstChildElement("Link");
    while (elem)
	{
		string linkTypeStr;
		string sourceGridName;
		Rect3i temp;
		Rect3i fromHalfCells;
        Rect3i toHalfCells;
        set<Vector3i> omittedSides;
        
		sGetOptionalAttribute(elem, "type", linkTypeStr, string("TF"));
		sGetMandatoryAttribute(elem, "sourceGrid", sourceGridName);
        if (sTryGetAttribute(elem, "fromYeeCells", temp))
            fromHalfCells = yeeToHalf(temp);
        else if (!sTryGetAttribute(elem, "fromHalfCells", fromHalfCells))
            throw(Exception(sErr("Link needs fromYeeCells or fromHalfCells "
                "attribute.", elem)));
        if (sTryGetAttribute(elem, "toYeeCells", temp))
            toHalfCells = yeeToHalf(temp);
        else if (!sTryGetAttribute(elem, "toHalfCells", toHalfCells))
            throw(Exception(sErr("Link needs toYeeCells or toHalfCells "
                "attribute.", elem)));
		
        const TiXmlElement* omitElem = elem->FirstChildElement("OmitSide");
        while (omitElem)
        {
            omittedSides.insert(loadAnOmitSide(omitElem));
            omitElem = omitElem->NextSiblingElement("OmitSide");
        }
		try {	
			HuygensSurfaceDescPtr link(HuygensSurfaceDescription::
                newLink(sourceGridName, fromHalfCells, toHalfCells,
                    omittedSides, linkTypeStr == "TF"));
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
	if (sTryGetAttribute(elem, "yeeCells", rect))
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
	else if (sTryGetAttribute(elem, "halfCells", rect))
	{
		if (elem->Attribute("fillStyle"))
			throw(Exception(sErr("Block with halfCells does not need "
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
		throw(Exception(sErr("Block needs yeeCells or halfCells "
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
	sGetMandatoryAttribute(elem, "yeeCells", yeeRect);
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
	
	sGetMandatoryAttribute(elem, "yeeCells", yeeRect);
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
	
	if (sTryGetAttribute(elem, "yeeCells", rect))
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
		if (!sTryGetAttribute(elem, "halfCells", rect))
			throw(Exception(sErr("Ellipsoid needs yeeCells or halfCells "
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
	
	sGetMandatoryAttribute(elem, "fromYeeCells", sourceRect);
	sGetMandatoryAttribute(elem, "toYeeCells", destRect);
	sGetMandatoryAttribute(elem, "sourceGrid", sourceGridName);
	if (allGridNames.count(sourceGridName) == 0)
		throw(Exception(sErr("Unknown source grid name for CopyFrom", elem)));
	
	try {
		copyFrom = InstructionPtr(
			new CopyFrom(yeeToHalf(sourceRect), yeeToHalf(destRect),
                sourceGridName));
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem)));
	}
	
	return copyFrom;
}

InstructionPtr XMLParameterFile::
loadAnExtrude(const TiXmlElement* elem) const
{
	assert(elem);
	assert(elem->Value() == string("Extrude"));
	
	InstructionPtr extrude;
	Rect3i halfCellFrom, halfCellTo;
	
	sGetMandatoryAttribute(elem, "fromHalfCells", halfCellFrom);
	sGetMandatoryAttribute(elem, "toHalfCells", halfCellTo);
    
	try {
		extrude = InstructionPtr(
			new Extrude(
				halfCellFrom, halfCellTo));
	} catch (Exception & e) {
		throw(Exception(sErr(e.what(), elem)));
	}
	
	return extrude;
}


Duration XMLParameterFile::
loadADuration(const TiXmlElement* elem) const
{
    assert(elem);
    int firstTimestep;
    int lastTimestep;
    int period;
    
    if (sTryGetAttribute(elem, "timestep", firstTimestep))
    {
        lastTimestep = firstTimestep;
        period = 1;
    }
    else
    {
        sGetOptionalAttribute(elem, "firstTimestep", firstTimestep, 0);
        sGetOptionalAttribute(elem, "lastTimestep", lastTimestep, INT_MAX);
        sGetOptionalAttribute(elem, "period", period, 1);
    }
    
    return Duration(firstTimestep, lastTimestep, period);
}

Region XMLParameterFile::
loadARegion(const TiXmlElement* elem) const
{
    assert(elem);
    Rect3i yeeCells;
    Vector3i stride;
    sGetMandatoryAttribute(elem, "yeeCells", yeeCells);
    sGetOptionalAttribute(elem, "stride", stride, Vector3i(1,1,1));
    return Region(yeeCells, stride);
}

Vector3i XMLParameterFile::
loadAnOmitSide(const TiXmlElement* elem) const
{
    Vector3i omitSide;
    Vector3i omitSideDominantComponent;
    elem->GetText() >> omitSide;
    omitSideDominantComponent = dominantComponent(omitSide);
    
    if (omitSide != omitSideDominantComponent ||
        dot(omitSide, omitSide) != 1)
        throw(Exception(sErr("OmitSide must specify an axis-oriented unit "
            "vector", elem)));
    
    return omitSide;
}


#pragma mark *** Static Method Implementations ***

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

