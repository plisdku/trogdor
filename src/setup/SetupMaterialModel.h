/*
 *  SetupMaterialModel.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/15/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _SETUPMATERIALMODEL_
#define _SETUPMATERIALMODEL_

#include "Pointer.h"
#include "Map.h"
#include <string>

class TiXmlElement;

class SetupMaterialModel
{
public:
    SetupMaterialModel();
    SetupMaterialModel(const TiXmlElement* inXml);
    SetupMaterialModel(const std::string & inName,
                       const std::string & inClass,
                       const Map<std::string, std::string> & inParams);
    
    const std::string & getName() const { return mName; }
    const std::string & getClass() const { return mClass; }
    const Map<std::string, std::string> & getParameters() const
        { return mParameters; }
    
    ~SetupMaterialModel();
private:
    std::string mName; // e.g. "Gold"
    std::string mClass; //  e.g. "DrudeMetalModel"
    
    
    
    //  This data gets passed into the runtime MaterialModel
    Map<std::string, std::string> mParameters;
    
};

typedef Pointer<SetupMaterialModel> SetupMaterialPtr;

#endif
