/*
 *  ThreeFieldInput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/13/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _THREEFIELDINPUT_
#define _THREEFIELDINPUT_

#include "Input.h"
#include "geometry.h"

class ThreeFieldInput: public Input
{
public:
    ThreeFieldInput(Fields & inFields,
        const Map<std::string, std::string> & inParams,
        const std::string & inFilePrefix);

protected:
    virtual void readSpecsData(std::ifstream & str);
    virtual void readDataE(std::ifstream & str);
    virtual void readDataH(std::ifstream & str);
    
    
private:
    const float* mField[3];
    Rect3i mYeeRegion;
    
    std::string mFieldNom;
    
};




#endif
