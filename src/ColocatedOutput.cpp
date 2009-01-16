/*
 *  ColocatedOutput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "ColocatedOutput.h"

#include "StreamFromString.h"

using namespace std;


ColocatedOutput::
ColocatedOutput(Fields & inFields,
    const Map<std::string, std::string> & inParams,
    const std::string & inFilePrefix, int period) :
    Output(inFields, inFilePrefix),
	mFieldType(kElectric),
	mYeeRegion(),
    mPeriod(period)
{
    string fieldNom = inParams["field"];
    inParams["region"] >> mYeeRegion;
    
    if (fieldNom == "electric")
    {
        mFieldType = kElectric;
        mField[0] = inFields.getEx();
        mField[1] = inFields.getEy();
        mField[2] = inFields.getEz();
    }
    else if (fieldNom == "magnetic")
    {
        mFieldType = kMagnetic;
        mField[0] = inFields.getHx();
        mField[1] = inFields.getHy();
        mField[2] = inFields.getHz();
    }
    else
    {
        cerr << "Error: ColocatedOutput field must be \"electric\""
            " or \"magnetic\".\n";
        assert(!"Quitting.");
    }
    
	assert(vec_ge(mYeeRegion.p1, 0));
	assert(vec_lt(mYeeRegion.p2, Vector3i(inFields.get_nx(), inFields.get_ny(),
		inFields.get_nz())));
	}



void ColocatedOutput::
writeSpecsData(ofstream & str)
{
    str << "3\n";
	str << "period " << mPeriod << "\n";
	str << "bounds " <<
		mYeeRegion.p1[0] << " " << mYeeRegion.p1[1] << " " <<
		mYeeRegion.p1[2] << " " << mYeeRegion.p2[0] << " " << 
		mYeeRegion.p2[1] << " " << mYeeRegion.p2[2];
}

void ColocatedOutput::
writeSpecsFooter(ofstream & str)
{
}

void ColocatedOutput::
writeData(ofstream & str, int timestep, float dt)
{
    int nx = getFields().get_nx(); 
    int ny = getFields().get_ny();
    int nz = getFields().get_nz();
    int numWritten = 0;
    int i, j, k;
    
    float field1, field2, field3, field4;
    float field;
    
    if ( (timestep+1) % mPeriod != 0)
        return;
    
    if (mFieldType == kElectric)
    {
        // Ex
        for (k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numWritten++;
            
            field1 = *(mField[0] + i + nx*j + nx*ny*k);
            field2 = *(mField[0] + i + nx*( (j+1)%ny ) + nx*ny*k);
            field3 = *(mField[0] + i + nx*j + nx*ny*( (k+1)%nz ));
            field4 = *(mField[0] + i + nx*( (j+1)%ny ) + nx*ny*( (k+1)%nz ));
            
            field = 0.25*(field1+field2+field3+field4);
            
            str.write((char*)(&field),
                (std::streamsize)sizeof(float));
        }
        
        // Ey
        for (k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numWritten++;
            
            field1 = *(mField[1] + i + nx*j + nx*ny*k);
            field2 = *(mField[1] + (i+1)%nx + nx*j + nx*ny*k);
            field3 = *(mField[1] + i + nx*j + nx*ny*( (k+1)%nz ));
            field4 = *(mField[1] + (i+1)%nx + nx*j + nx*ny*( (k+1)%nz ));
            
            field = 0.25*(field1+field2+field3+field4);
            
            str.write((char*)(&field),
                (std::streamsize)sizeof(float));
        }
        
        // Ez
        for (k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numWritten++;
            
            field1 = *(mField[2] + i + nx*j + nx*ny*k);
            field2 = *(mField[2] + (i+1)%nx + nx*j + nx*ny*k);
            field3 = *(mField[2] + i + nx*( (j+1)%ny ) + nx*ny*k );
            field4 = *(mField[2] + (i+1)%nx + nx*( (j+1)%ny ) + nx*ny*k );
            
            field = 0.25*(field1+field2+field3+field4);
            
            str.write((char*)(&field),
                (std::streamsize)sizeof(float));
        }
    }
    else if (mFieldType == kMagnetic)
    {
        // Hx
        for (k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numWritten++;
            
            field1 = *(mField[0] + i + nx*j + nx*ny*k);
            field2 = *(mField[0] + (i+1)%nx + nx*j + nx*ny*k);
            field = 0.5*(field1+field2);
            
            str.write((char*)(&field), (std::streamsize)sizeof(float));
        }
        
        // Hy
        for (k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numWritten++;
            
            field1 = *(mField[1] + i + nx*j + nx*ny*k);
            field2 = *(mField[1] + i + nx*( (j+1)%ny ) + nx*ny*k);
            field = 0.5*(field1+field2);
            
            str.write((char*)(&field), (std::streamsize)sizeof(float));
        }
        
        // Hz
        for (k = mYeeRegion.p1[2]; k <= mYeeRegion.p2[2]; k++)
        for (j = mYeeRegion.p1[1]; j <= mYeeRegion.p2[1]; j++)
        for (i = mYeeRegion.p1[0]; i <= mYeeRegion.p2[0]; i++)
        {
            numWritten++;
            
            field1 = *(mField[2] + i + nx*j + nx*ny*k);
            field2 = *(mField[2] + i + nx*j + nx*ny*( (k+1)%nz ) );
            field = 0.5*(field1+field2);
            
            str.write((char*)(&field), (std::streamsize)sizeof(float));
        }
    }
    else
        assert(!"mFieldType unknown.");
    
    str << flush;
    
    //LOG << "Wrote " << numWritten << " elements in timestep " << timestep << ".\n";

}

int ColocatedOutput::
get_nx() const
{
    return mYeeRegion.p2[0] - mYeeRegion.p1[0] + 1;
}

int ColocatedOutput::
get_ny() const
{
    return mYeeRegion.p2[1] - mYeeRegion.p1[1] + 1;
}

int ColocatedOutput::
get_nz() const
{
    return mYeeRegion.p2[2] - mYeeRegion.p1[2] + 1;
}














