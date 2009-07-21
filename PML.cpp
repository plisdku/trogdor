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


ostream &
operator<<(std::ostream & str, const PMLRunline & rl)
{
    str << "l = " << rl.length << "\n";
    str << hex << rl.fi << dec << ": " << MemoryBuffer::identify(rl.fi) << "\n";
    str << hex << rl.gj[0] << dec << ": " << MemoryBuffer::identify(rl.gj[0]) << "\n";
    str << hex << rl.gj[1] << dec << ": " << MemoryBuffer::identify(rl.gj[1]) << "\n";
    str << hex << rl.gk[0] << dec << ": " << MemoryBuffer::identify(rl.gk[0]) << "\n";
    str << hex << rl.gk[1] << dec << ": " << MemoryBuffer::identify(rl.gk[1]) << "\n";
    str << rl.auxIndex << ", PML " << rl.pmlIndex[0] << " " << rl.pmlIndex[1]
        << " " << rl.pmlIndex[2] << "\n";
    return str;
}

