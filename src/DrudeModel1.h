/*
 *  DrudeModel1.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _DRUDEMODEL1_
#define _DRUDEMODEL1_

#include "MaterialBoss.h"
#include "Map.h"
#include "MemoryUtilities.h"
#include <string>

class DrudeModel1Delegate : public SimpleBulkMaterialDelegate
{
public:
	DrudeModel1Delegate();
	
	virtual void setNumCells(int octant, int number);
private:
	MemoryBufferPtr mCurrents[3];
};



#endif
