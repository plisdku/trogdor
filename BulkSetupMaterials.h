/*
 *  BulkSetupMaterials.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SIMPLESETUPMATERIAL_
#define _SIMPLESETUPMATERIAL_

#include "SimulationDescription.h"
#include "SetupMaterial.h"
#include "MaterialRunlineEncoder.h"
#include "Pointer.h"
#include "geometry.h"
#include "MemoryUtilities.h"
#include "Runline.h"
#include <vector>

class Paint;

class VoxelizedPartition;
class GridDescription;


class BulkSetupMaterial : public SetupMaterial
{
public:
	BulkSetupMaterial(MaterialDescPtr description);
	virtual ~BulkSetupMaterial();
    
	virtual void printRunlines(std::ostream & out) const;
	
    const std::vector<SBMRunlinePtr> & runlinesE(int dir) const;
    const std::vector<SBMRunlinePtr> & runlinesH(int dir) const;
    
    virtual RunlineEncoder& encoder() { return mRunlineEncoder; }
protected:
    BulkMaterialRLE mRunlineEncoder;
};

class BulkPMLSetupMaterial : public SetupMaterial
{
public:
	BulkPMLSetupMaterial(MaterialDescPtr description);
	
	virtual void printRunlines(std::ostream & out) const;
	    
    const std::vector<SBPMRunlinePtr> & runlinesE(int dir) const;
    const std::vector<SBPMRunlinePtr> & runlinesH(int dir) const;
    
    virtual RunlineEncoder& encoder() { return mRunlineEncoder; }
protected:
    BulkPMLRLE mRunlineEncoder;
};














#endif
