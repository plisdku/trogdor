/*
 *  PML.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "PML.h"



PML::
~PML()
{
}

void PML::
setRunlinesE(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    mRunlinesE[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
    {
        mRunlinesE[direction][nn] = PMLRunline(*rls[nn]);
        //LOG << mRunlinesE[direction][nn] << "\n";
    }
}

void PML::
setRunlinesH(int direction, const std::vector<SBPMRunlinePtr> & rls)
{
    mRunlinesH[direction].resize(rls.size());
    for (int nn = 0; nn < rls.size(); nn++)
    {
        mRunlinesH[direction][nn] = PMLRunline(*rls[nn]);
        //LOG << mRunlinesH[direction][nn] << "\n";
    }
}
