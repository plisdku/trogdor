/*
 *  ConvertOldXML.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/12/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "ConvertOldXML.h"
#include "XMLExtras.h"
#include "geometry.h"

using namespace std;

ConvertOldXML::
ConvertOldXML()
{
}

Pointer<TiXmlDocument> ConvertOldXML::
convert4to5(const Pointer<TiXmlDocument> & doc) throw(Exception)
{
    Pointer<TiXmlDocument> newDoc(new TiXmlDocument);
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
    newDoc->LinkEndChild(decl);
    newDoc->LinkEndChild(simulation(doc->FirstChildElement()));
    
    return newDoc;
}

TiXmlElement* ConvertOldXML::
simulation(const TiXmlElement* old)
{
    TiXmlElement* sim = new TiXmlElement("Simulation");
    
    Map<string,string> oldAttribs(sGetAttributes(old));
    Map<string,string> newAttribs;
    newAttribs["dx"] = oldAttribs["dx"];
    newAttribs["dy"] = oldAttribs["dy"];
    newAttribs["dz"] = oldAttribs["dz"];
    newAttribs["dt"] = oldAttribs["dt"];
    newAttribs["numT"] = oldAttribs["numT"];
    newAttribs["version"] = "5.0";
    sSetAttributes(sim, newAttribs);
    
    Map<string, TiXmlElement*> materialMap;
    
    const TiXmlElement* oldGrid = old->FirstChildElement("Grid");
    while(oldGrid)
    {
        TiXmlElement* newGrid = grid(oldGrid);
        
        TiXmlElement* mat = newGrid->FirstChildElement("Material");
        TiXmlElement* matNext;
        while(mat)
        {
            string matName;
            sGetMandatoryAttribute(mat, "name", matName);
            if (materialMap.count(matName) == 0)
                materialMap[matName] = (TiXmlElement*)mat->Clone();
            else
                LOG << "Ignoring repeated material.\n";
            
            matNext = mat->NextSiblingElement("Material");
            newGrid->RemoveChild(mat);
            mat = matNext;
        }
        
        sim->LinkEndChild(newGrid);
        
        oldGrid = oldGrid->NextSiblingElement("Grid");
    }
    
    map<string, TiXmlElement*>::iterator itr;
    for (itr = materialMap.begin(); itr != materialMap.end(); itr++)
        sim->LinkEndChild(itr->second);
    
    return sim;
}

TiXmlElement* ConvertOldXML::
grid(const TiXmlElement* old)
{
    TiXmlElement* grid = new TiXmlElement("Grid");
    Map<string,string> oldAttribs(sGetAttributes(old));
    Map<string,string> newAttribs;
    
    if (oldAttribs.count("nx"))
        newAttribs["nx"] = oldAttribs["nx"];
    else
        throw(Exception(sErr("Trogdor 5 only accepts nx, not nnx", old)));
    if (oldAttribs.count("ny"))
        newAttribs["ny"] = oldAttribs["ny"];
    else
        throw(Exception(sErr("Trogdor 5 only accepts ny, not nny", old)));
    if (oldAttribs.count("nz"))
        newAttribs["nz"] = oldAttribs["nz"];
    else
        throw(Exception(sErr("Trogdor 5 only accepts nz, not nnz", old)));
    
    newAttribs["name"] = oldAttribs["name"];
    newAttribs["nonPML"] = oldAttribs["regionOfInterest"];
    
    sSetAttributes(grid, newAttribs);
    
    const TiXmlElement* child = old->FirstChildElement();
    while (child)
    {
        string childType = child->Value();
        
        if (childType == "Assembly")
            grid->LinkEndChild(assembly(child));
        else if (childType == "Output")
            grid->LinkEndChild(output(child));
        else if (childType == "Source")
            grid->LinkEndChild(source(child));
        else if (childType == "TFSFSource")
            grid->LinkEndChild(tfsfSource(child));
        else if (childType == "Link")
            grid->LinkEndChild(link(child));
        else if (childType == "Material")
            grid->LinkEndChild(material(child));
        
        child = child->NextSiblingElement();
    }
    
    return grid;
}

TiXmlElement* ConvertOldXML::
material(const TiXmlElement* old)
{
    TiXmlElement* mat = new TiXmlElement("Material");
    Map<string,string> oldAttribs(sGetAttributes(old));
    Map<string,string> newAttribs;
    
    //newAttribs["model"] = oldAttribs["class"];
    string oldClass = oldAttribs["class"];
    if (oldClass == "DrudeMetalModel")
        newAttribs["model"] = "DrudeMetal1";
    else if (oldClass == "StaticDielectricModel")
        newAttribs["model"] = "StaticDielectric";
    else if (oldClass == "StaticLossyDielectricModel")
        newAttribs["model"] = "StaticLossyDielectric";
    else if (oldClass == "PECModel")
        newAttribs["model"] = "PerfectConductor";
    
    newAttribs["name"] = oldAttribs["name"];
    
    sSetAttributes(mat, newAttribs);
    
    const TiXmlElement* oldParams = old->FirstChildElement("Params");
    if (oldParams)
    {
        TiXmlElement* newParams = new TiXmlElement("Params");
        Map<string,string> oldParamsAttribs(sGetAttributes(oldParams));
        
        sSetAttributes(newParams, oldParamsAttribs);
        
        mat->LinkEndChild(newParams);
    }
    
    return mat;
}

TiXmlElement* ConvertOldXML::
block(const TiXmlElement* old)
{
    TiXmlElement* newBlock = new TiXmlElement("Block");
    Map<string,string> oldAttribs(sGetAttributes(old));
    Map<string,string> newAttribs;
    if (oldAttribs.count("fineFillRect"))
        newAttribs["halfCells"] = oldAttribs["fineFillRect"];
    else
        newAttribs["yeeCells"] = oldAttribs["fillRect"];
    
    newAttribs["material"] = oldAttribs["material"];
    
    if (oldAttribs.count("fillStyle"))
        newAttribs["fillStyle"] = oldAttribs["fillStyle"];
    
    sSetAttributes(newBlock, newAttribs);
    return newBlock;
}

TiXmlElement* ConvertOldXML::
ellipsoid(const TiXmlElement* old)
{
    TiXmlElement* newEllipsoid = new TiXmlElement("Ellipsoid");
    Map<string,string> oldAttribs(sGetAttributes(old));
    Map<string,string> newAttribs;
    if (oldAttribs.count("fineFillRect"))
        newAttribs["halfCells"] = oldAttribs["fineFillRect"];
    else
        newAttribs["yeeCells"] = oldAttribs["fillRect"];
    
    newAttribs["material"] = oldAttribs["material"];
    
    if (oldAttribs.count("fillStyle"))
        newAttribs["fillStyle"] = oldAttribs["fillStyle"];
    
    sSetAttributes(newEllipsoid, newAttribs);
    return newEllipsoid;
}

TiXmlElement* ConvertOldXML::
keyImage(const TiXmlElement* old)
{
    TiXmlElement* newKeyImage = new TiXmlElement("KeyImage");
    Map<string,string> oldAttribs(sGetAttributes(old));
    Map<string,string> newAttribs;
    
    newAttribs["yeeCells"] = oldAttribs["fillRect"];
    newAttribs["file"] = oldAttribs["file"];
    newAttribs["row"] = oldAttribs["row"];
    newAttribs["column"] = oldAttribs["column"];
    sSetAttributes(newKeyImage, newAttribs);
    
    const TiXmlElement* tag = old->FirstChildElement("Tag");
    while (tag)
    {
        Map<string,string> tagAttribs(sGetAttributes(tag));
        TiXmlElement* newTag = new TiXmlElement("Tag");
        sSetAttributes(newTag, tagAttribs);
        newKeyImage->LinkEndChild(newTag);
        tag = tag->NextSiblingElement("Tag");
    }
    
    return newKeyImage;
}

TiXmlElement* ConvertOldXML::
heightMap(const TiXmlElement* old)
{
    TiXmlElement* newHeightMap = new TiXmlElement("HeightMap");
    Map<string,string> oldAttribs(sGetAttributes(old));
    Map<string,string> newAttribs;
    
    newAttribs["yeeCells"] = oldAttribs["fillRect"];
    newAttribs["material"] = oldAttribs["material"];
    newAttribs["row"] = oldAttribs["row"];
    newAttribs["column"] = oldAttribs["column"];
    newAttribs["up"] = oldAttribs["up"];
    newAttribs["file"] = oldAttribs["file"];
    if (oldAttribs.count("fillStyle"))
        newAttribs["fillStyle"] = oldAttribs["fillStyle"];
    
    sSetAttributes(newHeightMap, newAttribs);
    
    return newHeightMap;
}


TiXmlElement* ConvertOldXML::
assembly(const TiXmlElement* old)
{
    TiXmlElement* newAssembly = new TiXmlElement("Assembly");
    
    const TiXmlElement* elem = old->FirstChildElement();
    
    while(elem)
    {
        string instructionType = elem->Value();
        
        if (instructionType == "Block")
            newAssembly->LinkEndChild(block(elem));
        else if (instructionType == "KeyImage")
            newAssembly->LinkEndChild(keyImage(elem));
        else if (instructionType == "HeightMap")
            newAssembly->LinkEndChild(heightMap(elem));
        else if (instructionType == "Ellipsoid")
            newAssembly->LinkEndChild(ellipsoid(elem));
        else
            LOG << "Not converting unused element " << instructionType << endl;
        
        elem = elem->NextSiblingElement();
    }
    return newAssembly;
}

TiXmlElement* ConvertOldXML::
output(const TiXmlElement* old)
{
    TiXmlElement* newOutput = new TiXmlElement("FieldOutput");
    Map<string,string> oldAttribs = sGetAttributes(old);
    Map<string, string> newAttribs;
    Map<string, string> durationAttribs;
    Map<string, string> regionAttribs;
    string outClass = oldAttribs["class"];
    
    newAttribs["file"] = oldAttribs["filePrefix"];
    
    const TiXmlElement* oldParams = old->FirstChildElement("Params");
    if (oldParams == 0L)
        throw(Exception(sErr("Output doesn't have Params.", old)));
    Map<string,string> oldParamsAttribs = sGetAttributes(oldParams);
    
    durationAttribs["firstTimestep"] = "1";
    if (oldParamsAttribs.count("period") != 0)
        durationAttribs["period"] = oldParamsAttribs["period"];
    regionAttribs["yeeCells"] = oldParamsAttribs["region"];
    
    if (oldParamsAttribs.count("stride"))
        regionAttribs["stride"] = oldParamsAttribs["stride"];
    
    TiXmlElement* duration = new TiXmlElement("Duration");
    sSetAttributes(duration, durationAttribs);
    
    TiXmlElement* region = new TiXmlElement("Region");
    sSetAttributes(region, regionAttribs);
    
    if (outClass == "OneFieldOutput")
        newAttribs["fields"] = oldParamsAttribs["field"]; // ex, ey, ez etc.
    else if (outClass == "ThreeFieldOutput" ||
        outClass == "ColocatedOutput")
    {
        if (oldParamsAttribs["field"] == "electric" ||
            oldParamsAttribs["field"] == "e")
            newAttribs["fields"] = "ex ey ez";
        else if (oldParamsAttribs["field"] == "magnetic" ||
            oldParamsAttribs["field"] == "h")
            newAttribs["fields"] = "hx hy hz";
        else
            throw(Exception(sErr("Output has weird field.", oldParams)));
    }
    
    if (outClass == "ColocatedOutput")
        newAttribs["interpolate"] = "0.5 0.5 0.5";
    
    sSetAttributes(newOutput, newAttribs);
    newOutput->LinkEndChild(duration);
    newOutput->LinkEndChild(region);
    
    return newOutput;
}

TiXmlElement* ConvertOldXML::
source(const TiXmlElement* old)
{
    TiXmlElement* newSource;
    TiXmlElement* region = new TiXmlElement("Region");
    Map<string,string> oldAttribs = sGetAttributes(old);
    const TiXmlElement* oldParams = old->FirstChildElement("Params");
    Map<string, string> newAttribs;
    Map<string, string> regionAttribs;
    bool isSoft = 0;
    
    regionAttribs["yeeCells"] = oldAttribs["region"];
    sSetAttributes(region, regionAttribs);
    
    if (oldAttribs.count("file"))
        newAttribs["timeFile"] = oldAttribs["file"];
    else if (oldAttribs.count("formula"))
        newAttribs["formula"] = oldAttribs["formula"];
    else
        throw(Exception(sErr("Source needs file or formula", old)));
    
    if (oldParams != 0L)
    {
        Map<string,string> oldParamsAttributes(sGetAttributes(oldParams));
        if (oldParamsAttributes.count("soft"))
            isSoft = 1;
    }
    
    if (isSoft)
    {
        newSource = new TiXmlElement("AdditiveSource");
        newAttribs["polarization"] = oldAttribs["polarization"];
        newAttribs["fields"] = oldAttribs["field"];
    }
    else
    {
        newSource = new TiXmlElement("HardSource");
        Vector3i polVec;
        string fieldName;
        if (oldAttribs["field"] == "electric" || oldAttribs["field"] == "e")
            fieldName = "e";
        else if (oldAttribs["field"] == "magnetic" ||
            oldAttribs["field"] == "h")
            fieldName = "h";
        else
            throw(Exception(sErr("Weird field.", old)));
        
        sGetMandatoryAttribute(old, "polarization", polVec);
        if (polVec == Vector3i(1,0,0))
            fieldName = fieldName + string("x");
        else if (polVec == Vector3i(0,1,0))
            fieldName = fieldName + 'y';
        else if (polVec == Vector3i(0,0,1))
            fieldName = fieldName + "z";
        else
            throw(Exception(sErr("Can't convert hard source with off-axis "
                "polarization", old)));
        
        newAttribs["fields"] = fieldName;
    }
    sSetAttributes(newSource, newAttribs);
    newSource->LinkEndChild(region);
    
    return newSource;
}

TiXmlElement* ConvertOldXML::
tfsfSource(const TiXmlElement* old)
{
    TiXmlElement* newTFSFSource = new TiXmlElement("TFSFSource");
    Map<string, string> newAttribs;
    Map<string, string> regionAttribs;
    
    Map<string,string> oldAttribs = sGetAttributes(old);
    
    const TiXmlElement* oldParams = old->FirstChildElement("Params");
    if (oldParams == 0L)
        throw(Exception(sErr("TFSFSource needs Params", old)));
    Map<string,string> oldParamsAttribs(sGetAttributes(oldParams));
    
    if (oldAttribs.count("TFRect"))
        newAttribs["yeeCells"] = oldAttribs["TFRect"];
    else
        newAttribs["halfCells"] = oldAttribs["fineTFRect"];
    newAttribs["direction"] = oldAttribs["direction"];
    newAttribs["polarization"] = oldParamsAttribs["polarization"];
    newAttribs["field"] = oldParamsAttribs["field"];
    
    if (oldParamsAttribs.count("formula"))
        newAttribs["formula"] = oldParamsAttribs["formula"];
    else if (oldParamsAttribs.count("filename"))
        newAttribs["file"] = oldParamsAttribs["filename"];
    else
        throw(Exception(sErr("TFSFSource Params needs formula or filename",
            oldParams)));
    
    const TiXmlElement* omit = old->FirstChildElement("OmitSide");
    while (omit)
    {
        TiXmlElement* newOmit = new TiXmlElement("OmitSide");
        newOmit->LinkEndChild(new TiXmlText(omit->GetText()));
        newTFSFSource->LinkEndChild(newOmit);
        omit = omit->NextSiblingElement("OmitSide");
    }
    
    sSetAttributes(newTFSFSource, newAttribs);
    
    return newTFSFSource;
}

TiXmlElement* ConvertOldXML::
link(const TiXmlElement* old)
{
    TiXmlElement* newLink = new TiXmlElement("Link");
    Map<string, string> newAttribs;
    Map<string,string> oldAttribs = sGetAttributes(old);
    
    if (oldAttribs.count("fineSourceRect"))
        newAttribs["fromHalfCells"] = oldAttribs["fineSourceRect"];
    else
        newAttribs["fromYeeCells"] = oldAttribs["sourceRect"];
    
    if (oldAttribs.count("fineDestRect"))
        newAttribs["toHalfCells"] = oldAttribs["fineDestRect"];
    else
        newAttribs["toYeeCells"] = oldAttribs["destRect"];
    
    if (oldAttribs.count("type"))
        newAttribs["type"] = oldAttribs["type"];
    else
        newAttribs["type"] = "TF";
    
    newAttribs["sourceGrid"] = oldAttribs["sourceGrid"];
    
    const TiXmlElement* omit = old->FirstChildElement("OmitSide");
    while (omit)
    {
        TiXmlElement* newOmit = new TiXmlElement("OmitSide");
        newOmit->LinkEndChild(new TiXmlText(omit->GetText()));
        newLink->LinkEndChild(newOmit);
        omit = omit->NextSiblingElement("OmitSide");
    }
    sSetAttributes(newLink, newAttribs);
    
    return newLink;
}


