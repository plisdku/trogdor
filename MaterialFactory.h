/*
 *  MaterialFactory.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _MATERIALFACTORY_
#define _MATERIALFACTORY_

#include "RunlineEncoder.h"
#include "UpdateEquation.h"
#include "Map.h"
#include "geometry.h"
#include "Pointer.h"
#include "SimulationDescriptionPredeclarations.h"
#include <string>
#include <vector>

class VoxelGrid;
class PartitionCellCount;
typedef Pointer<PartitionCellCount> PartitionCellCountPtr;
class Paint;

class MaterialFactory
{
public:
	static RunlineEncoderPtr newRunlineEncoder(const VoxelGrid & vg,
		const PartitionCellCountPtr cg,
        const GridDescription & gridDesc,
		Paint* parentPaint,
        std::vector<int> numCellsE,
        std::vector<int> numCellsH,
        std::vector<Rect3i> pmlRects,
        int runlineDirection );
    
    static Map<Vector3i, Map<std::string, std::string> > defaultPMLParams();
};



#endif
