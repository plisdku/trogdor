/*
 *  DrudeModel1.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "DrudeModel1.h"

#include "YeeUtilities.h"
using namespace YeeUtilities;
using namespace std;

DrudeModel1Delegate::
DrudeModel1Delegate() :
	SimpleBulkMaterialDelegate()
{
}

void DrudeModel1Delegate::
setNumCells(int octant, int number)
{
	const int STRIDE = 1;
	int eIndex = octantENumber(number);
	if (eIndex == -1)  // if this isn't an E-field octant
		return;
	
	mCurrents[eIndex] = MemoryBufferPtr(new MemoryBuffer(number, STRIDE));
}


