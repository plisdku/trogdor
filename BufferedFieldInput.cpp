/*
 *  BufferedFieldInput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 9/2/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "BufferedFieldInput.h"


BufferedFieldInput::
BufferedFieldInput(CurrentSourceDescPtr sourceDescription, float dt) :
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
        string fname = sourceDescription->spaceTimeFile();
        mFile.open(fname.c_str(), ios::binary);
        if (mFile.good())
            LOGF << "Opened binary file " << fname << ".\n";
        else
            throw(Exception(string("Could not open binary file") + fname));
    }
    
    if (sourceDescription->sourceCurrents().usesPolarization())
    {
        assert(!sourceDescription->isSpaceVarying());
        mPolarizationFactor = sourceDescription->sourceCurrents().polarization();
    }
}

BufferedFieldInput::
~BufferedFieldInput()
{
}
