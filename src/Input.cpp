/*
 *  Input.cpp
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

#include "Input.h"

#include <cstdlib>
#include <cstring>
#include <cerrno>
using namespace std;


Input::
Input(Fields & inFields, const string & inFilePrefix) :
	mFile(),
    mFields(inFields),
    mFileName(inFilePrefix + ".dat"),
    mSpecsFileName(inFilePrefix + ".txt")
{
    mFile.open(mFileName.c_str(), ios::binary);
    if (mFile.good())
        LOGF << "Successfully opened binary file " << mFileName << ".\n";
    else
    {
        cerr << "Error: could not open binary file " << mFileName << ".\n";
        exit(1);
    }
}


Input::
~Input()
{
    mFile.close();
}

void Input::
setPrefix(const string & inPrefix)
{
    if (!mFile.is_open())
    {
        mFileName = inPrefix + ".dat";
        mSpecsFileName = inPrefix + ".txt";
        mFile.open(mFileName.c_str(), ios::binary);
        assert(mFile.good());
        LOGF << "Opened output file " << mFileName << "\n";
    }
    else
    {
        LOG << "Why change the file prefix?  File is open.\n";
        LOG << "  old name: " << mFileName << "\n";
        LOG << "  new prefix: " << inPrefix << "\n";
        LOG << "Upshot: didna change nuthin.\n";
    }
}

void Input::
readSpecsFile()
{
    ifstream fin;
    fin.open(mSpecsFileName.c_str());
    if (fin.good())
    {
        readSpecsData(fin);
        fin.close();
    }
    else
    {
        cerr << "Unable to open spec file " << mSpecsFileName << ".\n";
        exit(1);
    }
}

void Input::
readDataFileE()
{
    if (mFile.good())
        readDataE(mFile);
}

void Input::
readDataFileH()
{
    if (mFile.good())
        readDataH(mFile);
}

void Input::
readSpecsData(ifstream & fin)
{
}



void Input::
readDataE(std::ifstream & str)
{
}


void Input::
readDataH(std::ifstream & str)
{
}


