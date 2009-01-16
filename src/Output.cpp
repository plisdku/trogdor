/*
 *  Output.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/20/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "Output.h"

#include <cstdlib>
#include <cstring>
#include <cerrno>
using namespace std;


Output::
Output(Fields & inFields, const string & inFilePrefix) :
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

Output::
~Output()
{
    mFile.close();
}

void Output::
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
    }
}

/*vector<int> Output::
getRunlinesForTag(const string & tag)
{
    return vector<int>(1);
}
*/

void Output::
writeSpecsFile()
{
    ofstream fout;
    fout.open(mSpecsFileName.c_str());
    if (fout.good())
    {
        fout << get_nx() << " " << get_ny() << " " << get_nz() << " ";
        writeSpecsData(fout);
        fout << "\n---- end data block ----\n";
        fout << "Header format:\n";
        fout << " nx ny nz      size of data array for each timestep\n";
        fout << "User format description:\n";
        writeSpecsFooter(fout);
        fout.close();
    }
    else
    {
        cerr << "Unable to open spec file " << mSpecsFileName << ".\n";
        exit(1);
    }
}

void Output::
writeDataFile(int timestep, float dt)
{
    writeData(mFile, timestep, dt);
}

void Output::
onEndSimulation()
{
    // do nothing.
}

void Output::
writeSpecsData(ofstream & str)
{
}

void Output::
writeSpecsFooter(ofstream & str)
{
    str << "No user footer to write.\n";
}

void Output::
writeData(std::ofstream & str, int timestep, float dt)
{
    float datum = 1.0;
    int nx = get_nx();
    int ny = get_ny();
    int nz = get_nz();
    for (int k = 0; k < nz; k++)
    for (int j = 0; j < ny; j++)
    for (int i = 0; i < nx; i++)
    {
        str.write((char*)&datum, (std::streamsize)sizeof(float));
    }
}

int Output::
get_nx() const
{
    LOG << "Error: Output child class must overload get_nx().\n";
    return -1;
}

int Output::
get_ny() const
{
    LOG << "Error: Output child class must overload get_ny().\n";
    return -1;
}

int Output::
get_nz() const
{
    LOG << "Error: Output child class must overload get_nz().\n";
    return -1;
}

