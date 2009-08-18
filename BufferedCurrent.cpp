/*
 *  BufferedCurrent.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "BufferedCurrent.h"

BufferedCurrent::
BufferedCurrent() :
    mSourceOfData(0L)
{
//    LOG << "Actually creating the buffered current.\n";
}

void BufferedCurrent::
setCurrentSource(CurrentSource* sourceOfData)
{
    mSourceOfData = sourceOfData;
    
    //LOG << "Value is " << mSourceOfData->value() << "!\n";
}

void BufferedCurrent::
allocateAuxBuffers()
{
    //LOG << "Allocating the single timestep buffer.\n";
    //mSingleTimestepData.resize(mBuffer.length(), 0.0f);
}


