/*
 *  HuygensCustomSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 10/20/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _HUYGENSCUSTOMSOURCE_
#define _HUYGENSCUSTOMSOURCE_

#include "HuygensSurface.h"
#include "StreamedFieldInput.h"
#include "geometry.h"
#include <vector>

/**
 * Update equation for custom TFSF sources.  On every timestep, sums (with
 * appropriate signs) incident fields from a source file with total or scattered
 * fields from a main grid and stores the fields in NeighborBuffers.  All
 * required data is obtained from the HuygensSurface.
 */
class HuygensCustomSource : public HuygensUpdate
{
public:
    HuygensCustomSource(const HuygensSurface & hs);
    
    virtual void updateE(HuygensSurface & hs, CalculationPartition & cp,
        long timestep);
    virtual void updateH(HuygensSurface & hs, CalculationPartition & cp,
        long timestep);
    
private:
    HuygensSurfaceDescPtr mDescription;
    StreamedFieldInput mFieldInput;
    Duration mDuration;
};


#endif
