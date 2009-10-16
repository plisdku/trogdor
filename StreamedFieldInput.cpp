/*
 *  StreamedFieldInput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "StreamedFieldInput.h"
#include "Log.h"

using namespace std;

StreamedFieldInput::
StreamedFieldInput(SourceDescPtr sourceDescription) :
    mUsesPolarization(0),
    mPolarizationFactor(1,1,1),
    mWhichE(1,1,1),
    mWhichH(1,1,1)
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
    
    if (sourceDescription->sourceFields().usesPolarization())
    {
        assert(!sourceDescription->isSpaceVarying());
        mPolarizationFactor = sourceDescription->sourceFields().polarization();
        mUsesPolarization = 1;
    }
    else
    {
        mWhichE = sourceDescription->sourceFields().whichE();
        mWhichH = sourceDescription->sourceFields().whichH();
    }
}


StreamedFieldInput::
~StreamedFieldInput()
{
}

void StreamedFieldInput::
startHalfTimestepE(long timestep, float time)
{
    //LOG << "TODO: split this for E and H so it can read to the right buffer.\n";
    if (mType == FORMULATYPE)
    {
        mCalculator.set("n", timestep);
        mCalculator.set("t", time);
        mCalculator.parse(mFormula);
        
        float val = mCalculator.get_value();
        mCurrentValueVec = mPolarizationFactor * val;
    }
    else //if (mType == FILETYPE)
    {
        //  Space-varying sources read from the file on calls to getField().
        //  Otherwise we can read the value here and cache it.
        if (mFieldValueType == kTimeVaryingField)
        {
            //LOG << "TODO: Read 1 value if using polarization; read N values "
            //    "if not using polarization and using N fields.\n";
            
            if (mUsesPolarization)
            {
                float val;
                if (mFile.good())
                    mFile.read((char*)&val,
                        (std::streamsize)sizeof(float));
                else
                    throw(Exception("Cannot read further from file."));
                mCurrentValueVec = mPolarizationFactor * val;
            }
            else
            {
                for (int direction = 0; direction < 3; direction++)
                if (mWhichE[direction])
                {
                    if (mFile.good())
                        mFile.read((char*)&mCurrentValueVec[direction],
                            (std::streamsize)sizeof(float));
                    else
                        throw(Exception("Cannot read further from file."));
                }
                
            }
        }
    }
}

void StreamedFieldInput::
startHalfTimestepH(long timestep, float time)
{
    //LOG << "TODO: split this for E and H so it can read to the right buffer.\n";
    if (mType == FORMULATYPE)
    {
        mCalculator.set("n", timestep);
        mCalculator.set("t", time);
        mCalculator.parse(mFormula);
        
        float val = mCalculator.get_value();
        mCurrentValueVec = mPolarizationFactor * val;
    }
    else //if (mType == FILETYPE)
    {
        //  Space-varying sources read from the file on calls to getField().
        //  Otherwise we can read the value here and cache it.
        if (mFieldValueType == kTimeVaryingField)
        {
            //LOG << "TODO: Read 1 value if using polarization; read N values "
            //    "if not using polarization and using N fields.\n";
            
            if (mUsesPolarization)
            {
                float val;
                if (mFile.good())
                    mFile.read((char*)&val,
                        (std::streamsize)sizeof(float));
                else
                    throw(Exception("Cannot read further from file."));
                mCurrentValueVec = mPolarizationFactor * val;
            }
            else
            {
                for (int direction = 0; direction < 3; direction++)
                if (mWhichH[direction])
                {
                    if (mFile.good())
                        mFile.read((char*)&mCurrentValueVec[direction],
                            (std::streamsize)sizeof(float));
                    else
                        throw(Exception("Cannot read further from file."));
                }
            }
        }
    }
}

void StreamedFieldInput::
restartMaskPointer(int direction)
{
    mMaskIndex = 0;
}

float StreamedFieldInput::
getFieldE(int direction)
{
    float fieldValue;
    if (mFieldValueType == kSpaceTimeVaryingField)
    {
        assert(mFile.good());
        mFile.read((char*)&fieldValue, (std::streamsize)sizeof(float));
    }
    else if (mFieldValueType == kTimeVaryingField)
    {
        if (mHasMask)
        {
            assert(mMaskIndex >= 0 &&
                mMaskIndex < mDataMaskE[direction].size());
            fieldValue = mCurrentValueVec[direction] *
                mDataMaskE[direction][mMaskIndex];
            mMaskIndex++;
        }
        else
        {
            /*
            LOG << "TODO: If using polarization, use polarization.  If not,"
                " read from the right E field buffer component.\n";
            */
            //fieldValue = mCurrentValue * mPolarizationFactor[direction];
            fieldValue = mCurrentValueVec[direction];
        }
    }
    return fieldValue;
}

float StreamedFieldInput::
getFieldH(int direction)
{
    float fieldValue;
    if (mFieldValueType == kSpaceTimeVaryingField)
    {
        assert(mFile.good());
        mFile.read((char*)&fieldValue, (std::streamsize)sizeof(float));
    }
    else if (mFieldValueType == kTimeVaryingField)
    {
        if (mHasMask)
        {
            assert(mMaskIndex >= 0 &&
                mMaskIndex < mDataMaskH[direction].size());
            fieldValue = mCurrentValueVec[direction] *
                mDataMaskH[direction][mMaskIndex];
            mMaskIndex++;
        }
        else
        {
            /*
            LOG << "TODO: If using polarization, use polarization.  If not,"
                " read from the right H field buffer component.\n";
            */
            //fieldValue = mCurrentValue * mPolarizationFactor[direction];
            fieldValue = mCurrentValueVec[direction];
        }
    }
    return fieldValue;
}


void StreamedFieldInput::
loadMask(SourceDescPtr source)
{
    ifstream maskFile(source->spaceFile().c_str(), ios::binary);
    
    if (maskFile.good())
        LOGF << "Opened mask file.\n";
    else
        throw(Exception("Could not open mask file."));
    
    // Determine size of mask: sum of region volumes
    long volume = 0;
    for (int nn = 0; nn < source->regions().size(); nn++)
    {
        Rect3i regionYeeCells = source->regions()[nn].yeeCells();
        long vol = regionYeeCells.num(0)*regionYeeCells.num(1)*
            regionYeeCells.num(2);
        volume += vol;
    }
    
    // load E masks
    for (int xyz = 0; xyz < 3; xyz++)
    if (source->sourceFields().whichE()[xyz] != 0)
    {
        mDataMaskE[xyz].resize(volume);
        assert(maskFile.good());
        maskFile.read((char*)&(mDataMaskE[xyz][0]),
            (std::streamsize)volume*sizeof(float));
    }
    
    // load H masks
    for (int xyz = 0; xyz < 3; xyz++)
    if (source->sourceFields().whichH()[xyz] != 0)
    {
        mDataMaskE[xyz].resize(volume);
        assert(maskFile.good());
        maskFile.read((char*)&(mDataMaskH[xyz][0]),
            (std::streamsize)volume*sizeof(float));
    }
}


/*

What do I know about this StreamedFieldInput object?  Welllll...

It is supposed to somehow incorporate the mask, time-varying data, time-space
varying data and polarization vector, plus knowledge of which fields are
sourced, for E, H, J and K in various places (current source, hard or soft
source, custom TFSF source).  These different sources may be buffered or not
buffered, and this should be abstracted away or at least encapsulated nicely
in this class.

Hard source, soft source and AFP can all *use* the final result, E or H, of
a calculation here.  They really don't have much use for an internal buffer.
In fact, the NeighborBuffer properly ought to be the only buffer for custom
TFSF fields, so in that context they should either be read directly into the
neighbor buffer or be processed on the fly to include the mask or polarization
vector.  Regarding a mask, I do think that I should be able to load a mask from
a file and get it into the AFP buffer.  That should not be hard.  So in that
case, StreamedFieldInput ought to always be the place where the mask is buffered.

Anyway, the custom TFSF source, like the hard/soft source, will just need a way
to efficiently load data.

Question: does StreamedFieldInput need to know the data request information in any way?
Who verifies if a source file matches up to the XML description, or to what is
ultimately needed for the sim (in terms of scheduling)?  If so...

Let's see.

AFP sources: the HuygensSurface is in the best place to determine scheduling.

Hard/soft sources: scheduling is just one block after another, and it can be
flexible about runline direction (unlike perhaps the other source types).

Current sources: the setup current source run length encodes the destination
area to schedule the input.

Each case has unique scheduling requirements, so it seems that StreamedFieldInput should
not be responsible for scheduling.  Then I should have a unified way to present
StreamedFieldInput with a scheduling requirement, and from then on the StreamedFieldInput can
perform the data request, check to see if it's satisfied, and load the data.
This informs my to-do list.



TO-DO LIST:
1.  Determine unified format for data scheduling.  It's basically done already
in the CurrentSource stuff.  Connect this to StreamedFieldInput in a good way.
2.  Provide the StreamedFieldInput with a function to write the request to a stream,
which can be concatenated with grid description information for AFP requests.
3.  Give StreamedFieldInput a place for mask buffers and polarization vectors.  Teach
it to read them somehow.
4.  Split the rest of the StreamedFieldInput implementation so it has a buffered mode
and an unbuffered mode; provide outside access to the buffer somehow.

More to follow.







*/
