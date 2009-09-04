
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

class SimulationDescription;
class GridDescription;
class OutputDescription;
class GridReportDescription;
class SourceDescription;
class CurrentSourceDescription;
class HuygensSurfaceDescription;
class NeighborBufferDescription;
class MaterialDescription;
class AssemblyDescription;

class Block;
class KeyImage;
class HeightMap;
class Ellipsoid;
class CopyFrom;
class Extrude;

typedef Pointer<SimulationDescription> SimulationDescPtr;
typedef Pointer<GridDescription> GridDescPtr;
typedef Pointer<OutputDescription> OutputDescPtr;
typedef Pointer<GridReportDescription> GridReportDescPtr;
typedef Pointer<SourceDescription> SourceDescPtr;
typedef Pointer<CurrentSourceDescription> CurrentSourceDescPtr;
typedef Pointer<HuygensSurfaceDescription> HuygensSurfaceDescPtr;
typedef Pointer<NeighborBufferDescription> NeighborBufferDescPtr;
typedef Pointer<MaterialDescription> MaterialDescPtr;
typedef Pointer<AssemblyDescription> AssemblyDescPtr;


#endif
