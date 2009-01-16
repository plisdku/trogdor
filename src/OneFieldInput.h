/*
 *  OneFieldInput.h
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

#ifndef _ONEFIELDINPUT_
#define _ONEFIELDINPUT_

#include "Input.h"
#include "geometry.h"

class OneFieldInput: public Input
{
public:
    OneFieldInput(Fields & inFields,
        const Map<std::string, std::string> & inParams,
        const std::string & inFilePrefix);

protected:
    virtual void readSpecsData(std::ifstream & str);
    virtual void readDataE(std::ifstream & str);
    virtual void readDataH(std::ifstream & str);
    
    
private:
    const float* mField;
    Rect3i mYeeRegion;
    
    std::string mFieldNom;
};



#endif
