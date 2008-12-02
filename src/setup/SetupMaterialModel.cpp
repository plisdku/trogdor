/*
 *  SetupMaterialModel.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/15/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SetupMaterialModel.h"
#include "tinyxml.h"

#include "ValidateSetupAttributes.h"
#include "StreamFromString.h"
#include <sstream>

using namespace std;


SetupMaterialModel::
SetupMaterialModel() :
    mName("Default"),
    mClass("Default"),
	mParameters()
{
    LOGF << "Default constructor\n";
}

SetupMaterialModel::
SetupMaterialModel(const std::string & inName,
                   const std::string & inClass,
                   const Map<std::string, std::string> & inParams) :
    mName(inName),
    mClass(inClass),
    mParameters(inParams)
{
    LOGF << "Parameterized constructor\n";
}

SetupMaterialModel::
~SetupMaterialModel()
{
    LOGF << "Destructor\n";
}





