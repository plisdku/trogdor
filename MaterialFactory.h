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

#include "SetupMaterial.h"
#include "Pointer.h"
#include "SimulationDescriptionPredeclarations.h"
#include "Map.h"
#include "geometry.h"
#include <string>
#include <vector>

class VoxelGrid;
class PartitionCellCount;
class Paint;

class MaterialFactory
{
public:
	static SetupMaterialPtr newSetupMaterial(const VoxelGrid & vg,
		const PartitionCellCount & cg,
        const GridDescription & gridDesc,
		Paint* parentPaint,
        std::vector<int> numCellsE,
        std::vector<int> numCellsH,
        std::vector<Rect3i> pmlRects,
        int runlineDirection );
    
    static Map<Vector3i, Map<std::string, std::string> > defaultPMLParams();
};



#endif
