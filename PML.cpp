/*
 *  PML.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "PML.h"

PMLRunline::
PMLRunline(const SBPMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
    auxIndex = setupRunline.auxIndex;
    
    for (int nn = 0; nn < 3; nn++)
        pmlIndex[nn] = setupRunline.pmlDepthIndex[nn];
}



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

