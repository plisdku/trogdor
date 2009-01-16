/*
 *  ThreeFieldInput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/13/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "ThreeFieldInput.h"
#include "StreamFromString.h"

using namespace std;


ThreeFieldInput::
ThreeFieldInput(Fields & inFields,
    const Map<std::string, std::string> & inParams,
    const std::string & inFilePrefix) :
    Input(inFields, inFilePrefix),
	mYeeRegion()
{
    string fieldNom = inParams["field"];
    inParams["region"] >> mYeeRegion;
    
    if (fieldNom == "electric")
    {
        mField[0] = inFields.getEx();
        mField[1] = inFields.getEy();
        mField[2] = inFields.getEz();
    }
    else if (fieldNom == "magnetic")
    {
        mField[0] = inFields.getHx();
        mField[1] = inFields.getHy();
        mField[2] = inFields.getHz();
    }
    else
    {
        cerr << "Error: ThreeFieldInput field must be \"electric\""
            " or \"magnetic\".\n";
        assert(!"Quitting.");
    }
    
    //LOGMORE << inFields.get_nx() << " " << inFields.get_ny() << " "
    //    << inFields.get_nz() << endl;
    
    assert(mYeeRegion.p1[0] >= 0);
    assert(mYeeRegion.p1[1] >= 0);
    assert(mYeeRegion.p1[2] >= 0);
    assert(mYeeRegion.p2[0] < inFields.get_nx());
    assert(mYeeRegion.p2[1] < inFields.get_ny());
    assert(mYeeRegion.p2[2] < inFields.get_nz());
    
    //LOGMORE << "(Yee region: " << mYeeRegion << endl;
}

void ThreeFieldInput::
readSpecsData(ifstream & str)
{
}

void ThreeFieldInput::
readDataE(ifstream & str)
{
    if (mFieldNom[0] == 'e')
    {
        int nx = getFields().get_nx();
        int ny = getFields().get_ny();
        int numRead = 0;
        
        // this could be accelerated by reading a whole stripe at a time.
        for (int ff = 0; ff < 3; ff++) // must iterate last over field components!
        for (int k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (int j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (int i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numRead++;
            str.read((char*)(mField[ff]+i + nx*j + nx*ny*k),
                (std::streamsize)sizeof(float));
        }
        
        //LOGF << "Read " << numRead << " elements.\n";
    }    
}


void ThreeFieldInput::
readDataH(ifstream & str)
{
    if (mFieldNom[0] == 'h')
    {
        int nx = getFields().get_nx();
        int ny = getFields().get_ny();
		int numRead = 0;
        
        // this could be accelerated by reading a whole stripe at a time.
        for (int ff = 0; ff < 3; ff++)
        for (int k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (int j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (int i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numRead++;
            str.read((char*)(mField[ff]+i + nx*j + nx*ny*k),
                (std::streamsize)sizeof(float));
        }
        
        //LOGF << "Read " << numRead << " elements.\n";
    }
}




