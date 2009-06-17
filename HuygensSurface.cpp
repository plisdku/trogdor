/*
 *  HuygensSurface.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/16/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "HuygensSurface.h"
#include "VoxelizedPartition.h"
#include "SimulationDescription.h"
#include "Exception.h"

#include "HuygensLink.h"

using namespace std;

HuygensSurfaceDelegatePtr HuygensSurfaceFactory::
getDelegate(const VoxelizedPartition & vp,
    const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
    const HuygensSurfaceDescPtr & desc)
{
    HuygensSurfaceDelegatePtr hs;
    
    if (desc->getType() == kLink)
    {
        hs = HuygensSurfaceDelegatePtr(new HuygensLinkDelegate(
            vp, grids, desc));
    }
    else if (desc->getType() == kTFSFSource)
    {
    }
    else if (desc->getType() == kCustomTFSFSource)
    {
    }
    else
        throw(Exception("Unknown HuygensSurface type!"));
    
    return hs;
}
