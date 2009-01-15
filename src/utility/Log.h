/*
 *  Log.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 12/5/07.
 *
 */

#ifndef _LOGGER_
#define _LOGGER_

#include "StreamTee.h"

#include <boost/current_function.hpp>

#include <fstream>
#include <iostream>

//std::ofstream sLogfile("log.out.txt");
//StreamTee sLogTee(std::cout, sLogfile);

// Due to the static variables in the TrogLog class, it's possible to get into
// static initialization purgatory if any static members in the project get
// initialized before TrogLog's members AND then make use of the logfile
// macros.

class TrogLog
{
public:
	static StreamTee & tee() { return sTee; }
	static std::ofstream & logf() { return sLogfile; }
private:
	static std::ofstream sLogfile;
	static StreamTee sTee;
	TrogLog() {}
	~TrogLog() {}
};


#define LOGF (TrogLog::logf() << "[" << BOOST_CURRENT_FUNCTION << ", " << __LINE__ << "]: ")
#define LOGFMORE TrogLog::logf()
#define LOG (TrogLog::tee() << "[" << BOOST_CURRENT_FUNCTION << ", " << __LINE__ << "]: ")
#define LOGMORE TrogLog::tee()

#endif

