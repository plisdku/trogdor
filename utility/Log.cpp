/*
 *  Log.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 12/10/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "Log.h"

std::ofstream TrogLog::sLogfile("log.out.txt");
StreamTee TrogLog::sTee(std::cout, TrogLog::sLogfile);
