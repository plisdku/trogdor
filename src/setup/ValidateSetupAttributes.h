/*
 *  ValidateSetupAttributes.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/21/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _VALIDATESETUPATTRIBUTES_
#define _VALIDATESETUPATTRIBUTES_

#include "tinyxml.h"
#include <string>
#include "Map.h"

bool
validateSetupGrid(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);

bool
validateSetupOutput(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);
bool
validateSetupInput(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);

bool
validateSetupSource(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);

bool
validateSetupMaterial(const Map<std::string, std::string> & attribs,
                      std::string & outMessage,
                      int lineNumber);

bool
validateSetupAssemblyBlock(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);
				  
bool
validateSetupAssemblyEllipsoid(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);

bool
validateSetupAssemblyAscii(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);

bool
validateSetupAssemblyKeyImage(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);

bool
validateSetupAssemblyHeightMap(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);

bool
validateSetupLink(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);

bool
validateSetupTFSFSource(const Map<std::string, std::string> & attribs,
                  std::string & outMessage,
                  int lineNumber);



#endif
