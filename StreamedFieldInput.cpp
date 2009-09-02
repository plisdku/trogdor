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

/*
    Streamed input!
    
    accessors:
        field*mask at timestep (used by source & huygens surface)
        field at timestep (used by current source)
        mask (used by current source; provide pointer)
    
    time            load once per (half) timestep
    time, mask      load once, step through mask
    timespace       step through file
    
    formula         load once per (half) timestep
    formula, mask   load once
    
    SO:
    on half timestep:   load time source/evaluate formula
    per cell:           step through mask, step 
    
    
    
    My seeming problem is that I have not one but two access models here.  So
    maybe I should handle E/H and TFSF sources like the buffered current: give
    a mask pointer and a mask stride, and force the user to always do the
    plutification.  Sure! 
*/

StreamedFieldInput::
StreamedFieldInput(SourceDescPtr sourceDescription, float dt) :
    mIsSpaceVarying(0),
    mPolarizationFactor(1,1,1),
    m_dt(dt)
{
    if (sourceDescription->formula() != "")
    {
        mType = FORMULATYPE;
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
        string fname = sourceDescription->timeFile();
        mFile.open(fname.c_str(), ios::binary);
        if (mFile.good())
            LOGF << "Opened binary file " << fname << ".\n";
        else
            throw(Exception(string("Could not open binary file") + fname));
    }
    else if (sourceDescription->spaceTimeFile() != "")
    {
        mType = FILETYPE;
        mIsSpaceVarying = 1;
        string fname = sourceDescription->spaceTimeFile();
        mFile.open(fname.c_str(), ios::binary);
        if (mFile.good())
            LOGF << "Opened binary file " << fname << ".\n";
        else
            throw(Exception(string("Could not open binary file") + fname));
    }
    
    if (sourceDescription->sourceFields().usesPolarization())
    {
        assert(!sourceDescription->isSpaceVarying());
        mPolarizationFactor = sourceDescription->sourceFields().polarization();
    }
}


StreamedFieldInput::
~StreamedFieldInput()
{
}

void StreamedFieldInput::
startHalfTimestep(int timestep)
{ 
    if (mType == FORMULATYPE)
    {
        mCalculator.set("n", timestep);
        mCalculator.set("t", m_dt*timestep);
        mCalculator.parse(mFormula);
        
        mCurrentValue = mCalculator.get_value();
    }
    else if (mType == FILETYPE)
    {
        //  Space-varying sources read from the file on calls to getField().
        //  Otherwise we can read the value here and cache it.
        if (!mIsSpaceVarying)
        {
            if (mFile.good())
                mFile.read((char*)&mCurrentValue,
                    (std::streamsize)sizeof(float));
            else
                throw(Exception("Cannot read further from file."));
        }
    }
    else //if (mType == BUFFERFILETYPE)
    {
        // If space-varying
        //  read
    }
}

void StreamedFieldInput::
stepToNextField()
{
    // Save file position of start of field
}

void StreamedFieldInput::
stepToNextFieldDirection(int direction)
{
    // Either scoot back to start of field or do nothing
}

float StreamedFieldInput::
getField(int direction)
{
    return mPolarizationFactor[direction]*mCurrentValue;
}

void StreamedFieldInput::
stepToNextValue()
{
    //  Generally, does nothing... hmm?
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
