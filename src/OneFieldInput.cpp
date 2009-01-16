/*
 *  OneFieldInput.cpp
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

#include "OneFieldInput.h"
#include "StreamFromString.h"

using namespace std;


OneFieldInput::
OneFieldInput(Fields & inFields,
    const Map<std::string, std::string> & inParams,
    const std::string & inFilePrefix) :
    Input(inFields, inFilePrefix),
	mField(0L),
	mYeeRegion(),
	mFieldNom()
{
    string fieldNom = inParams["field"];
    inParams["region"] >> mYeeRegion;
    
    if (fieldNom == "ex")
        mField = inFields.getEx();
    else if (fieldNom == "ey")
        mField = inFields.getEy();
    else if (fieldNom == "ez")
        mField = inFields.getEz();
    else if (fieldNom == "hx")
        mField = inFields.getHx();
    else if (fieldNom == "hy")
        mField = inFields.getHy();
    else if (fieldNom == "hz")
        mField = inFields.getHz();
    
    mFieldNom = fieldNom;
    
    //LOGF << inFields.get_nx() << " " << inFields.get_ny() << " "
    //    << inFields.get_nz() << endl;
    
	assert(vec_ge(mYeeRegion.p1, 0));
	assert(vec_lt(mYeeRegion.p2, Vector3i(inFields.get_nx(), inFields.get_ny(),
		inFields.get_nz())));
    
    //LOGF << "(Yee region: " << mYeeRegion << ")" << endl;
}

void OneFieldInput::
readSpecsData(ifstream & str)
{
}

void OneFieldInput::
readDataE(ifstream & str)
{
    if (mFieldNom[0] == 'e')
    {
        int nx = getFields().get_nx();
        int ny = getFields().get_ny();
        int numRead = 0;
        
        // this could be accelerated by reading a whole stripe at a time.  (meh.)
        for (int k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (int j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (int i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numRead++;
            str.read((char*)(mField+i + nx*j + nx*ny*k),
                (std::streamsize)sizeof(float));
        }
        
        LOGF << "Read " << numRead << " elements.\n";
    }    
}


void OneFieldInput::
readDataH(ifstream & str)
{
    if (mFieldNom[0] == 'h')
    {
        int nx = getFields().get_nx();
        int ny = getFields().get_ny();
        int numRead = 0;
        
        // this could be accelerated by reading a whole stripe at a time.  (meh.)
        for (int k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (int j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (int i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numRead++;
            str.read((char*)(mField+i + nx*j + nx*ny*k),
                (std::streamsize)sizeof(float));
        }
        
        LOGF << "Read " << numRead << " elements.\n";
    }
}







