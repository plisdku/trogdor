/*
 *  ThreeFieldOutput.cpp
 *  v4_trogdor
 *
 *  Created by Paul Hansen on 1/30/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ThreeFieldOutput.h"
#include "StreamFromString.h"

using namespace std;


ThreeFieldOutput::
ThreeFieldOutput(Fields & inFields,
    const Map<std::string, std::string> & inParams,
    const std::string & inFilePrefix, int period) :
    Output(inFields, inFilePrefix),
	mYeeRegion(),
    mPeriod(period)
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
        cerr << "Error: ThreeFieldOutput field must be \"electric\""
            " or \"magnetic\".\n";
        assert(!"Quitting.");
    }
    
    //LOGMORE << inFields.get_nx() << " " << inFields.get_ny() << " "
    //    << inFields.get_nz() << endl;
    
	assert(vec_ge(mYeeRegion.p1, 0));
	assert(vec_lt(mYeeRegion.p2, Vector3i(inFields.get_nx(), inFields.get_ny(),
		inFields.get_nz())));
    //LOG << "Output " << inFilePrefix << endl;
    //LOGMORE << "(Yee region: " << mYeeRegion << endl;
}

void ThreeFieldOutput::
writeSpecsData(ofstream & str)
{
    //  The specs line so far is just nx, ny, nz.  We add the fourth dimension.
    str << "3\n";
	str << "period " << mPeriod << "\n";
	str << "bounds " <<
		mYeeRegion.p1[0] << mYeeRegion.p1[1] << mYeeRegion.p1[2] <<
		mYeeRegion.p2[0] << mYeeRegion.p2[1] << mYeeRegion.p2[2];
}

void ThreeFieldOutput::
writeSpecsFooter(ofstream & str)
{
}

void ThreeFieldOutput::
writeData(ofstream & str, int timestep, float dt)
{
    int nx = getFields().get_nx();
    int ny = getFields().get_ny();
    int numWritten = 0;
    int i, j, k, ff;
    
    if ( (timestep+1) % mPeriod != 0)
        return;
        
    for (ff = 0; ff < 3; ff++) // must iterate last over the field components!
    for (k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
    for (j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
    for (i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
    {
        numWritten++;
        str.write((char*)(mField[ff]+i + nx*j + nx*ny*k),
                  (std::streamsize)sizeof(float));
    }
    
    str << flush;
    
    //LOGF << "Wrote " << numWritten << " elements in timestep " << timestep << ".\n";
}


int ThreeFieldOutput::
get_nx() const
{
    return mYeeRegion.p2[0] - mYeeRegion.p1[0] + 1;
}

int ThreeFieldOutput::
get_ny() const
{
    return mYeeRegion.p2[1] - mYeeRegion.p1[1] + 1;
}

int ThreeFieldOutput::
get_nz() const
{
    return mYeeRegion.p2[2] - mYeeRegion.p1[2] + 1;
}


