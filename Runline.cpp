/*
 *  Runline.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "Runline.h"


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

/*
SimpleRunline::
SimpleRunline(const SBPMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
}
*/

SimpleAuxRunline::
SimpleAuxRunline(const SBMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
    auxIndex = setupRunline.auxIndex;
}

/*
SimpleAuxRunline::
SimpleAuxRunline(const SBPMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
    auxIndex = setupRunline.auxIndex;
}
*/

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

SimpleAuxPMLRunline::
SimpleAuxPMLRunline(const SBPMRunline & setupRunline) :
    SimpleRunline(setupRunline),
    PMLRunline(setupRunline)
{
}

