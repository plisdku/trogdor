/*
 *  Runline.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _RUNLINE_
#define _RUNLINE_

#include "MemoryUtilities.h"
#include "geometry.h"
#include <iostream>

#pragma mark *** Setup Runlines ***

struct SBMRunline
{
    long length;
    long auxIndex;
    BufferPointer f_i;
    BufferPointer f_j[2];
    BufferPointer f_k[2];
};
typedef Pointer<SBMRunline> SBMRunlinePtr;

struct SBPMRunline : public SBMRunline
{
    Vector3i pmlDepthIndex;
};
typedef Pointer<SBPMRunline> SBPMRunlinePtr;

#pragma mark *** Runlines ***

struct SimpleRunline
{
    SimpleRunline() {}
    SimpleRunline(const SBMRunline & setupRunline);
    
    float* fi;
    float* gj[2];
    float* gk[2];
    unsigned long length;
};

struct SimpleAuxRunline : public SimpleRunline
{
    SimpleAuxRunline() : SimpleRunline() {}
    SimpleAuxRunline(const SBMRunline & setupRunline);
    unsigned long auxIndex;
};

// The PMLRunline is a mix-in, so it probably won't be instantiated by itself.
struct PMLRunline
{
    PMLRunline() {}
    PMLRunline(const SBPMRunline & setupRunline);
    
    unsigned long pmlIndex[3];
};

struct SimpleAuxPMLRunline : public SimpleAuxRunline, public PMLRunline
{
    SimpleAuxPMLRunline() : SimpleAuxRunline(), PMLRunline() {}
    SimpleAuxPMLRunline(const SBPMRunline & setupRunline);
    SimpleAuxPMLRunline(const SBMRunline & setupRunline)
        { assert(!"This should never be run."); }
};

std::ostream & operator<<(std::ostream & str, const SimpleRunline & rl);
std::ostream & operator<<(std::ostream & str, const SimpleAuxRunline & rl);
std::ostream & operator<<(std::ostream & str, const SimpleAuxPMLRunline & rl);


#endif