/*
 *  Exception.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/1/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "Exception.h"

Exception::
Exception() throw():
	std::exception(),
	mWhat("")
{
}

Exception::
Exception(const char* inWhat) throw():
	std::exception(),
	mWhat(inWhat)
{
}

Exception::
Exception(const std::string & inWhat) throw():
	std::exception(),
	mWhat(inWhat)
{
}

Exception::
~Exception() throw()
{
}

const char* Exception::
what() const throw()
{
	return mWhat.c_str();
}

