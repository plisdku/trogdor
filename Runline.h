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
    SimpleRunline(const SBMRunline & setupRunline);
    unsigned long auxIndex;
};

// The PMLRunline is a mix-in, so it probably won't be instantiated by itself.
struct PMLRunline
{
    PMLRunline() {}
    SimpleRunline(const SBPMRunline & setupRunline);
    
    unsigned long pmlIndex[3];
};

struct SimpleAuxPMLRunline : public SimpleRunline, public PMLRunline
{
    SimpleAuxPMLRunline() : SimpleRunline(), PMLRunline() {}
    SimpleRunline(const SBPMRunline & setupRunline);
};


#endif