/*
 *  OneFieldOutput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/2/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "OneFieldOutput.h"
#include "StreamFromString.h"

using namespace std;


OneFieldOutput::
OneFieldOutput(Fields & inFields,
    const Map<std::string, std::string> & inParams,
    const std::string & inFilePrefix, int period) :
    Output(inFields, inFilePrefix),
	mField(0L),
	mYeeRegion(),
    mPeriod(period)
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
    else
    {
        cerr << "Error: OneFieldOutput field must be \"ex\""
            " or \"hy\" etc. (receiived " << fieldNom << ".\n";
        assert(!"Quitting.");
    }
    
    //LOGF << inFields.get_nx() << " " << inFields.get_ny() << " "
    //    << inFields.get_nz() << endl;
    
	assert(vec_ge(mYeeRegion.p1, 0));
	assert(vec_lt(mYeeRegion.p2, Vector3i(inFields.get_nx(), inFields.get_ny(),
		inFields.get_nz())));
    
    //LOGFMORE << "(Yee region: " << mYeeRegion << endl;
}

void OneFieldOutput::
writeSpecsData(ofstream & str)
{
	str << "\n";
	str << "period " << mPeriod << "\n";
	str << "bounds " <<
		mYeeRegion.p1[0] << " " << mYeeRegion.p1[1] << " " <<
		mYeeRegion.p1[2] << " " << mYeeRegion.p2[0] << " " << 
		mYeeRegion.p2[1] << " " << mYeeRegion.p2[2];
}

void OneFieldOutput::
writeSpecsFooter(ofstream & str)
{
}

void OneFieldOutput::
writeData(ofstream & str, int timestep, float dt)
{
    int nx = getFields().get_nx();
    int ny = getFields().get_ny();
    int numWritten = 0;
    
    if ( (timestep+1) % mPeriod != 0)
        return;
    
    // this could be accelerated by writing a whole stripe at a time.  (meh.)
    for (int k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
    for (int j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
    for (int i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
    {
        numWritten++;
        str.write((char*)(mField+i + nx*j + nx*ny*k),
                  (std::streamsize)sizeof(float));
    }
    str << flush;
    
    //LOG << "Wrote " << numWritten << " elements in timestep " << timestep << ".\n";
}


int OneFieldOutput::
get_nx() const
{
    return mYeeRegion.p2[0] - mYeeRegion.p1[0] + 1;
}

int OneFieldOutput::
get_ny() const
{
    return mYeeRegion.p2[1] - mYeeRegion.p1[1] + 1;
}

int OneFieldOutput::
get_nz() const
{
    return mYeeRegion.p2[2] - mYeeRegion.p1[2] + 1;
}



