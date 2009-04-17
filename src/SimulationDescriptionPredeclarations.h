/*
 *  SimulationDescriptionPredeclarations.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SIMULATIONDESCRIPTIONPREDECLARATIONS_
#define _SIMULATIONDESCRIPTIONPREDECLARATIONS_

#include "Pointer.h"

class GridDescription;
typedef Pointer<GridDescription> GridDescPtr;
class VoxelizedPartition;
typedef Pointer<VoxelizedPartition> VoxelizedPartitionPtr;
class MaterialDescription;
typedef Pointer<MaterialDescription> MaterialDescPtr;
class HuygensSurfaceDescription;
typedef Pointer<HuygensSurfaceDescription> HuygensSurfaceDescPtr;
class NeighborBufferDescription;
typedef Pointer<NeighborBufferDescription> NeighborBufferDescPtr;

class AssemblyDescription;
class Block;
class KeyImage;
class HeightMap;
class Ellipsoid;
class CopyFrom;
class Extrude;

#endif
