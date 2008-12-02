/*
 *  TimeWrapper.cpp
 *  version02
 *
 *  Created by Paul Hansen on 7/12/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "TimeWrapper.h"

//#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef __MINGW32__

#include <sys/time.h>

double getTimeInMicroseconds()
{
    timeval tv;
    struct timezone tz;
    
    gettimeofday(&tv, &tz);
    
    return ((double)tv.tv_usec + 1e6*tv.tv_sec);
}

#else

double getTimeInMicroseconds()
{
    return 1.0;
}


#endif
