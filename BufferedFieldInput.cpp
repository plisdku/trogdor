/*
 *  BufferedFieldInput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 9/2/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "BufferedFieldInput.h"

#include "Log.h"

using namespace std;

BufferedFieldInput::
BufferedFieldInput(CurrentSourceDescPtr sourceDescription)
    //mPolarizationFactor(1,1,1)
{
    if (sourceDescription->formula() != "")
    {
        mType = FORMULATYPE;
        mFieldValueType = kTimeVaryingField;
        mFormula = sourceDescription->formula();
        LOGF << "Formula is " << mFormula << endl;
        
        mCalculator.set("n", 0);
        mCalculator.set("t", 0);
        
        bool err = mCalculator.parse(mFormula);
        if (err)
        {
            LOG << "Error found.";
            cerr << "Calculator cannot parse\n"
                << mFormula << "\n"
                << "Error message:\n";
            mCalculator.report_error(cerr);
            cerr << "Quitting.\n";
            assert(!"Assert death.");
            exit(1);
        }
    }
    else if (sourceDescription->timeFile() != "")
    {
        mType = FILETYPE;
        mFieldValueType = kTimeVaryingField;
        string fname = sourceDescription->timeFile();
        mFile.open(fname.c_str(), ios::binary);
        if (mFile.good())
            LOGF << "Opened binary file " << fname << ".\n";
        else
            throw(Exception(string("Could not open binary file ") + fname));
        
        if (sourceDescription->hasMask())
            loadMask(sourceDescription);
    }
    else if (sourceDescription->spaceTimeFile() != "")
    {
        mType = FILETYPE;
        mFieldValueType = kSpaceTimeVaryingField;
        string fname = sourceDescription->spaceTimeFile();
        mFile.open(fname.c_str(), ios::binary);
        if (mFile.good())
            LOGF << "Opened binary file " << fname << ".\n";
        else
            throw(Exception(string("Could not open binary file ") + fname));
    }
    
    if (sourceDescription->sourceCurrents().usesPolarization())
    {
        assert(!sourceDescription->isSpaceVarying());
        assert(!"Error: buffered input doesn't support polarization (?).");
//        mPolarizationFactor =
//            sourceDescription->sourceCurrents().polarization();
    }
    
    // Initialize memory buffers.  The total size can be found from the list of
    // regions, and is the same for all (used) J and K.  (This is a peculiarity
    // of the current source; for other BufferedFieldInputs, see other ctors.)
    
    // Find total number of Yee cells.
    long numYee = 0;
    for (int nn = 0; nn < sourceDescription->regions().size(); nn++)
        numYee += sourceDescription->regions()[nn].yeeCells().count();
    LOG << "Num yee cells = " << numYee << "\n";
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        if (sourceDescription->sourceCurrents().whichJ()[xyz])
            mBufferE[xyz] = MemoryBuffer(string("J")+char('x'+xyz), numYee);
        if (sourceDescription->sourceCurrents().whichK()[xyz])
            mBufferH[xyz] = MemoryBuffer(string("K")+char('x'+xyz), numYee);
        
        if (sourceDescription->hasMask())
        {
            if (sourceDescription->sourceCurrents().whichJ()[xyz])
                mMaskBufferE[xyz] = MemoryBuffer(string("Mask J")+char('x'+xyz),
                    numYee);
            if (sourceDescription->sourceCurrents().whichK()[xyz])
                mMaskBufferH[xyz] = MemoryBuffer(string("Mask K")+char('x'+xyz),
                    numYee);
        }
    }
}

void BufferedFieldInput::
allocate()
{
    for (int xyz = 0; xyz < 3; xyz++)
    {
        if (mBufferE[xyz].length() > 0)
        {
            mDataE[xyz].resize(mBufferE[xyz].length());
            mBufferE[xyz].setHeadPointer(&(mDataE[xyz][0]));
        }
        if (mBufferH[xyz].length() > 0)
        {
            mDataH[xyz].resize(mBufferH[xyz].length());
            mBufferH[xyz].setHeadPointer(&(mDataH[xyz][0]));
        }
        if (mMaskBufferE[xyz].length() > 0)
        {
            mDataMaskE[xyz].resize(mMaskBufferE[xyz].length());
            mMaskBufferE[xyz].setHeadPointer(&(mDataMaskE[xyz][0]));
        }
        if (mMaskBufferH[xyz].length() > 0)
        {
            mDataMaskH[xyz].resize(mMaskBufferH[xyz].length());
            mMaskBufferH[xyz].setHeadPointer(&(mDataMaskH[xyz][0]));
        }
    }
}

BufferPointer BufferedFieldInput::
pointerE(int fieldDirection, long offset) const
{
    assert(fieldDirection >= 0 && fieldDirection < 3);
    return BufferPointer(mBufferE[fieldDirection], offset);
}

BufferPointer BufferedFieldInput::
pointerH(int fieldDirection, long offset) const
{
    assert(fieldDirection >= 0 && fieldDirection < 3);
    return BufferPointer(mBufferH[fieldDirection], offset);
}

BufferPointer BufferedFieldInput::
pointerMaskE(int fieldDirection, long offset) const
{
    assert(fieldDirection >= 0 && fieldDirection < 3);
    return BufferPointer(mMaskBufferE[fieldDirection], offset);
}

BufferPointer BufferedFieldInput::
pointerMaskH(int fieldDirection, long offset) const
{
    assert(fieldDirection >= 0 && fieldDirection < 3);
    return BufferPointer(mMaskBufferH[fieldDirection], offset);
}



void BufferedFieldInput::
startHalfTimestep(int timestep, float time)
{
    LOG << "TODO: Split into E and H.\n";
    if (mType == FORMULATYPE)
    {
        mCalculator.set("n", timestep);
        mCalculator.set("t", time);
        mCalculator.parse(mFormula);
        
        mCurrentValue = mCalculator.get_value();
    }
    else //if (mType == FILETYPE)
    {
        //  Space-varying sources read from the file on calls to getField().
        //  Otherwise we can read the value here and cache it.
        if (mFieldValueType == kTimeVaryingField)
        {
            LOG << "TODO: read the right number of components depending on"
                " whether it's a polarization source (never will be?).\n";
            LOG << "Although each field is buffered separately, they may all"
                " read out of one stream, right?\n";
            if (mFile.good())
                mFile.read((char*)&mCurrentValue,
                    (std::streamsize)sizeof(float));
            else
                throw(Exception("Cannot read further from file."));
        }
    }
}

void BufferedFieldInput::
loadMask(CurrentSourceDescPtr source)
{
    ifstream maskFile(source->spaceFile().c_str(), ios::binary);
    
    if (maskFile.good())
        LOGF << "Opened mask file.\n";
    else
        throw(Exception("Could not open mask file."));
    
    // Determine size of mask: sum of region volumes
    long volume = 0;
    for (int nn = 0; nn < source->regions().size(); nn++)
        volume += source->regions()[nn].yeeCells().count();
    
    // load E masks
    for (int direction = 0; direction < 3; direction++)
    if (source->sourceCurrents().whichJ()[direction] != 0)
    {
        mDataMaskE[direction].resize(volume);
        assert(maskFile.good());
        maskFile.read((char*)&(mDataMaskE[direction][0]),
            (std::streamsize)volume*sizeof(float));
    }
    
    // load H masks
    for (int direction = 0; direction < 3; direction++)
    if (source->sourceCurrents().whichK()[direction] != 0)
    {
        mDataMaskE[direction].resize(volume);
        assert(maskFile.good());
        maskFile.read((char*)&(mDataMaskH[direction][0]),
            (std::streamsize)volume*sizeof(float));
    }
}
