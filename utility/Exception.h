/*
 *  Exception.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/1/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _PCH_EXCEPTION_
#define _PCH_EXCEPTION_

#include <exception>
#include <string>

class Exception: public std::exception
{
public:
	Exception() throw();
	Exception(const char* inWhat) throw();
	Exception(const std::string & inWhat) throw();
	virtual ~Exception() throw();
	virtual const char* what() const throw();
private:
	std::string mWhat;
};


#endif