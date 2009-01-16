/*
 *  SetupInput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/26/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _SETUPINPUT_
#define _SETUPINPUT_

#include "Pointer.h"

#include "Map.h"
#include <string>

class SetupInput
{
public:
    SetupInput(std::string inFilePrefix, std::string inClass,
        Map<std::string, std::string> inParams);
    
    const std::string & getClass() const { return mClass; }
    const std::string & getFilePrefix() const { return mFilePrefix; }
    const Map<std::string, std::string> & getParameters() const
        { return mParams; }
    
    ~SetupInput();

private:
    std::string mFilePrefix;
    std::string mClass;
    Map<std::string, std::string> mParams;
    
};

typedef Pointer<SetupInput> SetupInputPtr;




#endif
