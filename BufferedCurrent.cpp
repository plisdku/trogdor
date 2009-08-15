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
BufferedCurrent()
{
}

void BufferedCurrent::
allocateAuxBuffers()
{
    mSingleTimestepData.resize(mBuffer.getLength(), 0.0f);
}


