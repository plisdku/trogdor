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
	std::vector<InputEHDescPtr> loadInputEHs(const TiXmlElement* parent) const;
	std::vector<OutputDescPtr> loadOutputs(const TiXmlElement* parent) const;
	std::vector<SourceDescPtr> loadSources(const TiXmlElement* parent) const;
	std::vector<TFSFSourceDescPtr> loadTFSFSources(const TiXmlElement* parent) const;
	std::vector<LinkDescPtr> loadLinks(const TiXmlElement* parent) const;
	std::vector<MaterialDescPtr> loadMaterials(const TiXmlElement* parent) const;
	AssemblyDescPtr loadAssembly(const TiXmlElement* parent) const;
	
	AssemblyDescription::InstructionPtr loadABlock(
		const TiXmlElement* elem) const;
	AssemblyDescription::InstructionPtr loadAKeyImage(
		const TiXmlElement* elem) const;
	std::vector<AssemblyDescription::ColorKey> loadAColorKeys(
		const TiXmlElement* elem) const;
	AssemblyDescription::InstructionPtr loadAHeightMap(
		const TiXmlElement* elem) const;
	AssemblyDescription::InstructionPtr loadAEllipsoid(
		const TiXmlElement* elem) const;
	AssemblyDescription::InstructionPtr loadACopyFrom(
		const TiXmlElement* elem) const;
	
	
	TiXmlDocument mDocument;
};



#endif
