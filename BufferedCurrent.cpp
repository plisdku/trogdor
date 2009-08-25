/*
 *  BufferedCurrent.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "BufferedCurrent.h"

using namespace std;

BufferedCurrent::
BufferedCurrent(std::vector<int> numCellsE, std::vector<int> numCellsH) :
    mSourceOfData(0L)
{
    
}

void BufferedCurrent::
setCurrentSource(CurrentSource* sourceOfData)
{
    mSourceOfData = sourceOfData;
}

void BufferedCurrent::
allocateAuxBuffers()
{
}







