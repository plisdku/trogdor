/*
 *  Runline.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "Runline.h"

using namespace std;

SimpleRunline::
SimpleRunline(const SBMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
}

SimpleAuxRunline::
SimpleAuxRunline(const SBMRunline & setupRunline) :
    SimpleRunline(setupRunline),
    auxIndex(setupRunline.auxIndex)
{
}

PMLRunline::
PMLRunline(const SBPMRunline & setupRunline)
{
    for (int nn = 0; nn < 3; nn++)
        pmlIndex[nn] = setupRunline.pmlDepthIndex[nn];
}

SimpleAuxPMLRunline::
SimpleAuxPMLRunline(const SBPMRunline & setupRunline) :
    SimpleAuxRunline(setupRunline),
    PMLRunline(setupRunline)
{
}

#pragma mark *** Output ***


ostream &
operator<<(std::ostream & str, const SimpleRunline & rl)
{
    /*
    str << hex << rl.fi << " " << rl.gj[0] << " " << rl.gj[1] << " "
        << rl.gk[0] << " " << rl.gk[1] << " " << dec << rl.length;
    */
    str << "length " << rl.length << "\n";
    str << hex << rl.fi << dec << ": " << MemoryBuffer::identify(rl.fi) << "\n";
    str << hex << rl.gj[0] << dec << ": " << MemoryBuffer::identify(rl.gj[0]) << "\n";
    str << hex << rl.gj[1] << dec << ": " << MemoryBuffer::identify(rl.gj[1]) << "\n";
    str << hex << rl.gk[0] << dec << ": " << MemoryBuffer::identify(rl.gk[0]) << "\n";
    str << hex << rl.gk[1] << dec << ": " << MemoryBuffer::identify(rl.gk[1]) << "\n";
    return str;
}


ostream &
operator<<(std::ostream & str, const SimpleAuxRunline & rl)
{
    str << "length " << rl.length << "\n";
    str << hex << rl.fi << dec << ": " << MemoryBuffer::identify(rl.fi) << "\n";
    str << hex << rl.gj[0] << dec << ": " << MemoryBuffer::identify(rl.gj[0]) << "\n";
    str << hex << rl.gj[1] << dec << ": " << MemoryBuffer::identify(rl.gj[1]) << "\n";
    str << hex << rl.gk[0] << dec << ": " << MemoryBuffer::identify(rl.gk[0]) << "\n";
    str << hex << rl.gk[1] << dec << ": " << MemoryBuffer::identify(rl.gk[1]) << "\n";
    str << "aux " << rl.auxIndex << "\n";
    return str;
}


ostream &
operator<<(std::ostream & str, const SimpleAuxPMLRunline & rl)
{
    str << "l = " << rl.length << "\n";
    str << hex << rl.fi << dec << ": " << MemoryBuffer::identify(rl.fi) << "\n";
    str << hex << rl.gj[0] << dec << ": " << MemoryBuffer::identify(rl.gj[0]) << "\n";
    str << hex << rl.gj[1] << dec << ": " << MemoryBuffer::identify(rl.gj[1]) << "\n";
    str << hex << rl.gk[0] << dec << ": " << MemoryBuffer::identify(rl.gk[0]) << "\n";
    str << hex << rl.gk[1] << dec << ": " << MemoryBuffer::identify(rl.gk[1]) << "\n";
    str << "aux " << rl.auxIndex << ", PML " << rl.pmlIndex[0] << " " << rl.pmlIndex[1]
        << " " << rl.pmlIndex[2] << "\n";
    return str;
}



