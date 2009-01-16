/*
 *  Input.h
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

#ifndef _INPUT_
#define _INPUT_

#include "Fields.h"
#include "Pointer.h"

#include <string>
#include <fstream>

class Input
{
public:
    Input(Fields & inFields, const std::string & inFilePrefix);
    virtual ~Input();
    
    void setPrefix(const std::string & inFilePrefix);
    void readSpecsFile();
    
    void readDataFileE();
    void readDataFileH();

protected:
    virtual void readSpecsData(std::ifstream & str);
    virtual void readDataE(std::ifstream & str);
    virtual void readDataH(std::ifstream & str);
    
    Fields & getFields() { return mFields; }
    
    std::ifstream mFile;

private:
    Fields & mFields;
    std::string mFileName;
    std::string mSpecsFileName;
};

typedef Pointer<Input> InputPtr;

#endif
