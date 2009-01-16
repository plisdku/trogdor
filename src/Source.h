/*
 *  Source.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/20/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _SOURCE_
#define _SOURCE_

#include "Fields.h"
#include "Pointer.h"

class Source
{
public:
    Source();
    virtual ~Source();
    
    virtual void
    doSourceE(int timestep, float dx, float dy, float dz, float dt);
    
    virtual void
    doSourceH(int timestep, float dx, float dy, float dz, float dt);
    
private:
};

typedef Pointer<Source> SourcePtr;

#endif
