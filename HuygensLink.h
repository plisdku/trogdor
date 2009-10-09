/*
 *  HuygensLink.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _HUYGENSLINK_
#define _HUYGENSLINK_

#include "HuygensSurface.h"
#include "geometry.h"
#include <vector>

/**
 * Update equation for a TFSF link.  On every timestep, sums (with appropriate
 * signs) incident fields from a source grid with total or scattered fields from
 * a main grid and stores the fields in NeighborBuffers.  All required data is
 * obtained from the HuygensSurface.
 */
class HuygensLink : public HuygensUpdate
{
public:
    HuygensLink(const HuygensSurface & hs);
    
    virtual void updateE(HuygensSurface & hs);
    virtual void updateH(HuygensSurface & hs);
};

#endif
