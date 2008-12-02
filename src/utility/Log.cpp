/*
 *  Log.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 12/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Log.h"

std::ofstream TrogLog::sLogfile("log.out.txt");
StreamTee TrogLog::sTee(std::cout, TrogLog::sLogfile);
