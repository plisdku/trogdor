/*
 *  SetupOutput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/19/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SETUPOUTPUT_
#define _SETUPOUTPUT_

#include "Pointer.h"

#include <string>

class SetupOutput
{
public:
    SetupOutput(std::string inFilePrefix, std::string inClass,
                int inPeriod, Map<std::string, std::string> inParams);
    
    const std::string & getClass() const { return mClass; }
    const std::string & getFilePrefix() const { return mFilePrefix; }
    const Map<std::string, std::string> & getParameters() const
        { return mParams; }
    
    int getPeriod() const { return mPeriod; }
    
    ~SetupOutput();
private:
    int mPeriod;
    std::string mFilePrefix;
    std::string mClass;
    Map<std::string, std::string> mParams;
    
};

typedef Pointer<SetupOutput> SetupOutputPtr;

#endif
