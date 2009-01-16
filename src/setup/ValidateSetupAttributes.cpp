/*
 *  ValidateSetupAttributes.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/21/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "ValidateSetupAttributes.h"

#include <sstream>

using namespace std;

bool
validateSetupGrid(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    if (attribs.count("name") == 0)
    {
        str << "Error: Grid element needs name attribute, e.g.\n";
        str << "   <Grid name='Main Grid'...>\n";
        valid = 0;
    }
    if ( (attribs.count("nnx") == 0 && attribs.count("nx") == 0) ||
        (attribs.count("nny") == 0 && attribs.count("ny") == 0) ||
        (attribs.count("nnz") == 0 && attribs.count("nz") == 0 ) )
    {
        str << "Error: Grid element needs dimension attributes, e.g.\n";
        str << "   <Grid nx='50' ny='100' nz='1'...>  (Yee cells)\n";
        str << " or\n";
        str << "   <Grid nnx='100' nny='100' nnz='2'...>  (Half cells, use "
            "with caution)\n";
        valid = 0;
    }
    if (attribs.count("roi") == 0 && attribs.count("regionOfInterest") == 0)
    {
        str << "Error: Grid element needs region of interest attribute, e.g.\n";
        str << "   <Grid regionOfInterest='10 10 10 89 89 89'...> "
            "(Yee cells)\n";
        str << " or\n";
        str << "   <Grid roi='20 20 20 179 179 179'...> (Half cells, use "
            "with caution)\n";
        valid = 0;
    }
    //  Removed 10/13/06 -- now default active region is whole grid.
    /*
    if (attribs.count("activeRegion") == 0)
    {
        str << "Error: Grid element needs activeRegion attribute, e.g.\n";
        str << "   <Grid activeRegion='1 1 1 98 98 98'...>\n";
        valid = 0;
    }
    */
    
    if (!valid)
        str << "(line " << lineNumber << ".)\n";
    
    outMessage = str.str();
    return valid;
}

bool
validateSetupOutput(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("filePrefix") == 0)
    {
        valid = 0;
        str << "Error: Output element needs filePrefix attribute, e.g.\n";
        str << "   <Output filePrefix='Ez_out'...>\n";
    }
    if (attribs.count("class") == 0)
    {
        valid = 0;
        str << "Error: Output element needs class attribute, e.g.\n";
        str << "   <Output class='OneFieldOutput'...>\n";
    }
    //   the period attribute is optional.
    
    if (!valid)
        str << "(line " << lineNumber << ".)\n";
    
    outMessage = str.str();
    return valid;
}

bool
validateSetupInput(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("filePrefix") == 0)
    {
        valid = 0;
        str << "Error: Input element needs filePrefix attribute, e.g.\n";
        str << "   <Input filePrefix='Ez_out'...>\n";
    }
    if (attribs.count("class") == 0)
    {
        valid = 0;
        str << "Error: Input element needs class attribute, e.g.\n";
        str << "   <Input class='OneFieldInput'...>\n";
    }
    //   the period attribute is optional.
    
    if (!valid)
        str << "(line " << lineNumber << ".)\n";
    
    outMessage = str.str();
    return valid;
}

bool
validateSetupSource(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("class") == 0 && attribs.count("formula") == 0 &&
		attribs.count("file") == 0)
    {
        valid = 0;
        str << "Error: Source element needs class, file or formula "
			"attribute, e.g.\n";
        str << "   <Source class='RaisedCosineSource'>\n";
		str << " or\n";
		str << "   <Source formula='exp(-n)'>\n";
		str << " or\n";
		str << "   <Source file='sourceTrace.dat'>\n";
    }
	
	if (attribs.count("class") == 0)
	{
		if (attribs.count("polarization") == 0)
		{
			valid = 0;
			str << "Error: Source element needs polarization attribute, e.g.\n";
			str << "   <Source polarization='1 0 0'>\n";
		}
	}
	
	if (attribs.count("field"))
	{
		if (attribs["field"] != "electric" && attribs["field"] != "magnetic")
		{
			valid = 0;
			str << "Error: Source attribute 'field' must be 'electric' or "
				"'magnetic'.\n";
		}
	}
	else
	{
		valid = 0;
		str << "Error: Source element needs field attribute, e.g.\n";
		str << "   <Source field='electric'...>/n";
	}
    
    if (!valid)
        str << "(line " << lineNumber << ".)\n";
    
    outMessage = str.str();
    return valid;
}

bool
validateSetupMaterial(const Map<std::string, std::string> & attribs,
                      std::string & outMessage,
                      int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("name") == 0)
    {
        valid = 0;
        str << "Error: Material element needs name attribute, e.g.\n";
        str << "   <Material name='Gold'...>\n";
    }
    if (attribs.count("class") == 0)
    {
        valid = 0;
        str << "Error: Material element needs class attribute, e.g.\n";
        str << "   <Material class='DrudeMetalModel'...>\n";
    }
    
    if (!valid)
        str << "(line " << lineNumber << ".)\n";
    
    outMessage = str.str();
    return valid;
}



bool
validateSetupAssemblyBlock(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("material") == 0)
    {
        valid = 0;
        str << "Error: Block element needs material attribute.\n";
    }
    if (attribs.count("fillRect") == 0 && attribs.count("fineFillRect") == 0)
    {
        valid = 0;
        str << "Error: Block element needs fillRect attribute.\n";
    }
    
    if (!valid)
    {
        str << "e.g.\n";
        str << "   <Block fillRect='0 0 0 99 99 99' material='Free Space' />\n";
        str << "(line " << lineNumber << ".)\n";
    }
    
    outMessage = str.str();
    return valid;
}


bool
validateSetupAssemblyEllipsoid(const Map<std::string, std::string> & attribs,
	std::string & outMessage, int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("material") == 0)
    {
        valid = 0;
        str << "Error: Ellipsoid element needs material attribute.\n";
    }
    if (attribs.count("fillRect") == 0 && attribs.count("fineFillRect") == 0)
    {
        valid = 0;
        str << "Error: Ellipsoid element needs fillRect attribute.\n";
    }
    
    if (!valid)
    {
        str << "e.g.\n";
        str << "   <Ellipsoid fillRect='0 0 0 99 99 99' material='Free Space' />\n";
        str << "(line " << lineNumber << ".)\n";
    }
    
    outMessage = str.str();
    return valid;
}


bool
validateSetupAssemblyAscii(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("row") == 0)
    {
        valid = 0;
        str << "Error: Ascii element needs row attribute.\n";
    }
    if (attribs.count("column") == 0)
    {
        valid = 0;
        str << "Error: Ascii element needs column attribute.\n";
    }
    if (attribs.count("fillRect") == 0)
    {
        valid = 0;
        str << "Error: Ascii element needs fillRect attribute.\n";
    }
    
    if (!valid)
    {
        str << "e.g.\n";
        str << "   <Ascii row='x' column='-y' fillRect='0 0 0 99 99 99'>\n";
        str << "(line " << lineNumber << ".)\n";
    }
    
    outMessage = str.str();
    return valid;
}

bool
validateSetupAssemblyKeyImage(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("file") == 0)
    {
        valid = 0;
        str << "Error: KeyImage element needs file attribute.\n";
    }
    if (attribs.count("fillRect") == 0)
    {
        valid = 0;
        str << "Error: KeyImage element needs fillRect attribute.\n";
    }
    if (attribs.count("row") == 0 || attribs.count("column") == 0)
    {
        valid = 0;
        str << "Error: KeyImage element needs row and column attributes.\n";
    }
    else if (attribs["row"] == attribs["column"])
    {
        valid = 0;
        str << "Error: KeyImage row and column cannot be equal.\n";
    }
    
    if (!valid)
    {
        str << "e.g.\n";
        str << "   <KeyImage file='materials.png' fillRect='1 1 0 98 98 1' ";
        str << "row='y' column='z'>\n";
        str << "(line " << lineNumber << ".)\n";
    }
    
    outMessage = str.str();
    return valid;
}

bool
validateSetupAssemblyHeightMap(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("file") == 0)
    {
        valid = 0;
        str << "Error: HeightMap element needs file attribute.\n";
    }
    
    if (attribs.count("fillRect") == 0)
    {
        valid = 0;
        str << "Error: HeightMap element needs fillRect attribute.\n";
    }
    
    if (attribs.count("row") == 0 || attribs.count("column") == 0 ||
        attribs.count("up") == 0)
    {
        valid = 0;
        str << "Error: HeightMap element needs row, column, ";
        str << "and up attributes.\n";
    }
    
    if (attribs.count("material") == 0)
    {
        valid = 0;
        str << "Error: HeightMap element needs material attribute.\n";
    }
    
    if (!valid)
    {
        str << "e.g.\n";
        str << "   <HeightMap file='heights.gif' fillRect='11 11 11 88 88 88'";
        str << " row='y' column='z' up='-x' material='Gold'>\n";
        str << "(line " << lineNumber << ".)\n";
    }
    
    outMessage = str.str();
    return valid;
}

bool
validateSetupLink(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("type") == 0)
    {
        valid = 0;
        str << "Error: Link element needs type attribute, e.g.\n";
        str << "   <Link type='TF'...>\n";
    }
    
    if (attribs.count("sourceGrid") && attribs.count("sourceFilePrefix"))
    {
        valid = 0;
        str << "Error: Link element must have either sourceGrid or "
            "sourceFilePrefix attributes, but not both.\n";
    }
    
    
    if (attribs.count("sourceGrid") == 0 &&
        attribs.count("sourceFilePrefix") == 0)
    {
        valid = 0;
        str << "Error: Link element needs sourceGrid or sourceuFilePrefix "
            "attribute, e.g.\n";
        str << "   <Link sourceGrid='Incident Grid'...>\n";
        str << "or <Link sourceFilePrefix='mySourceFile'...>\n";
    }
    
    if (attribs.count("sourceGrid") != 0 && attribs.count("sourceRect") == 0 &&
        attribs.count("fineSourceRect") == 0)
    {
        valid = 0;
        str << "Error: Link element needs sourceRect attribute, e.g.\n";
        str << "   <Link sourceRect='30 0 0 70 0 0'...>\n";
        str << " or\n";
        str << "   <Link fineSourceRect='60 0 0 141 1 1'...> (Half-cell "
            "control, use with caution\n";
    }
    
    if (attribs.count("destRect") == 0 && attribs.count("fineDestRect") == 0)
    {
        valid = 0;
        str << "Error: Link element needs destRect attribute, e.g.\n";
        str << "   <Link destRect='30 30 30 70 70 70'...>\n";
        str << " or\n";
        str << "   <Link fineDestRect='60 60 60 141 141 141'...> (Half-cell "
            "control, use with caution)\n";
    }
    
    if (attribs.count("sourceFilePrefix") && attribs.count("fineDestRect"))
    {
        valid = 0;
        str << "Error: Link elements that use sourceFilePrefix should use "
            "destRect instead of fineDestRect.\n";
    }
    
    if (!valid)
        str << "(line " << lineNumber << ".)\n";
    
    outMessage = str.str();
    return valid;
}

bool
validateSetupTFSFSource(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber)
{
    bool valid = 1;
    ostringstream str;
    
    if (attribs.count("TFRect") == 0 &&
        attribs.count("fineTFRect") == 0)
    {
        valid = 0;
        str << "Error: TFSFSource element needs TFRect attribute, e.g.\n";
        str << "   <TFSFSource TFRect='30 0 0 70 0 0'...>\n";
        str << " or\n";
        str << "   <TFSFSource fineTFRect='60 0 0 141 1 1'...> (Half-cell "
            "control, use with caution\n";
    }
	
	if (attribs.count("class") == 0)
	{
		valid = 0;
		str << "Error: TFSFSource element needs class attribute, e.g.\n";
		str << "   <TFSFSource class='PlaneWave'...>\n";
	}
	
	if (attribs.count("direction") == 0)
	{
		valid = 0;
		str << "Error: TFSFSource element needs direction attribute, e.g.\n";
		str << "   <TFSFSource direction='1 0 0'...>\n";
	}
	
	// These attributes have been moved to the Params child element.
	/*
	if (attribs.count("formula") == 0 &&
		attribs.count("filePrefix") == 0)
	{
		valid = 0;
		str << "Error: TFSFSource element needs formula or filePrefix "
			"attribute, e.g.\n";
		str << "   <TFSFSource formula='sin(n)'.../>\n";
		str << " or\n";
		str << "   <TFSFSource filePrefix='rampedSineWave'...>\n";
	}
	*/
        
    if (!valid)
        str << "(line " << lineNumber << ".)\n";
    
    outMessage = str.str();
    return valid;
}




