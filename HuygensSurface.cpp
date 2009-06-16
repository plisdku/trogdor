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

using namespace std;

HuygensSurfaceDelegatePtr HuygensSurfaceFactory::
getDelegate(const Map<GridDescPtr, VoxelizedPartitionPtr> & grids,
    const HuygensSurfaceDescPtr & desc)
{
    HuygensSurfaceDelegatePtr hs;
    
    if (desc->getType() == kLink)
    {
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

/*
HuygensSurfaceDelegate::
HuygensSurfaceDelegate()
{
    
}



HuygensSurface::
HuygensSurface()
{
}




void HuygensSurface::
updateE()
{
    for (int nk = 0; nk < mNumYeeCells[2]; nk++)
    for (int nj = 0; nj < mNumYeeCells[1]; nj++)
    for (int ni = 0; ni < mNumYeeCells[0]; ni++)
    {
        
    }
}




void HuygensSurface::
updateH()
{
}
*/