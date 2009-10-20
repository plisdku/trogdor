/*
 *  TimeWrapper.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/12/05.
 *  Copyright 2005 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "TimeWrapper.h"

//#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef __MINGW32__

#include <sys/time.h>
//#include <iostream>

using namespace std;

double timeInMicroseconds()
{
    timeval tv;
    struct timezone tz;
    
    gettimeofday(&tv, &tz);
    
    return ((double)tv.tv_usec + 1e6*tv.tv_sec);
}

string timestamp()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strftime(buffer, 80, "%y.%m.%d %X", timeinfo);
    //cout << buffer << endl;
    
    return string(buffer);
}

#else

double timeInMicroseconds()
{
    return 1.0;
}

std::string timestamp()
{
    return "";
}


#endif
