/*
 *  XMLParameterFile.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 1/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _XMLPARAMETERFILE_
#define _XMLPARAMETERFILE_

#include "SimulationDescription.h"
#include "tinyxml.h"
#include "Exception.h"

#include <string>
#include <vector>

class XMLParameterFile
{
	friend class SimulationDescription;
public:
	XMLParameterFile(const std::string & filename) throw(Exception); 
private:
	void load(SimulationDescription & sim) const throw(Exception);
	
	void loadTrogdor4(SimulationDescription & sim) const throw(Exception);
	
	std::vector<GridDescPtr> loadGrids(const TiXmlElement* parent) const;
	std::set<std::string> collectGridNames(const TiXmlElement* parent) const;
	std::set<std::string> collectMaterialNames(const TiXmlElement* parent)
		const;
	
	std::vector<MaterialDescPtr> loadMaterials(const TiXmlElement* parent)
		const;
	std::vector<InputEHDescPtr> loadInputEHs(const TiXmlElement* parent) const;
	std::vector<OutputDescPtr> loadOutputs(const TiXmlElement* parent) const;
	std::vector<SourceDescPtr> loadSources(const TiXmlElement* parent) const;
	std::vector<HuygensSurfaceDescPtr> loadTFSFSources(
		const TiXmlElement* parent, const std::set<std::string> & allGridNames)
		const;
	std::vector<HuygensSurfaceDescPtr> loadLinks(
		const TiXmlElement* parent, const std::set<std::string> & allGridNames)
		const;
	//std::vector<MaterialDescPtr> loadMaterials(const TiXmlElement* parent) const;
	AssemblyDescPtr loadAssembly(const TiXmlElement* parent,
		const std::set<std::string> & allGridNames,
		const std::set<std::string> & allMaterialNames) const;
	
	InstructionPtr loadABlock(const TiXmlElement* elem,
		const std::set<std::string> & allMaterialNames) const;
	InstructionPtr loadAKeyImage(const TiXmlElement* elem,
		const std::set<std::string> & allMaterialNames) const;
	std::vector<ColorKey> loadAColorKeys(const TiXmlElement* elem,
		const std::set<std::string> & allMaterialNames) const;
	InstructionPtr loadAHeightMap(const TiXmlElement* elem,
		const std::set<std::string> & allMaterialNames) const;
	InstructionPtr loadAEllipsoid(const TiXmlElement* elem,
		const std::set<std::string> & allMaterialNames) const;
	InstructionPtr loadACopyFrom(const TiXmlElement* elem,
		const std::set<std::string> & allGridNames) const;
	
	
	TiXmlDocument mDocument;
};



#endif
