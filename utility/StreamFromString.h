/*
 *  StreamFromString.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 12/11/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _STREAMFROMSTRING_
#define _STREAMFROMSTRING_

#include <iostream>
#include <sstream>

template<typename T>
std::string operator >> (std::string inString, T & value)
{
	std::string remainder;
	std::istringstream istr(inString);
	istr >> value;
	getline(istr, remainder, '\0');
	return remainder;
}

#endif
