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
setCurrentSource(CurrentSource* sourceOfData, int materialID)
{
    LOG << "Right here!\n";
    mSourceOfData = sourceOfData;
    
    for (int direction = 0; direction < 3; direction++)
    {
        mPointerJ[direction] = sourceOfData->pointerJ(direction, materialID);
        mPointerK[direction] = sourceOfData->pointerK(direction, materialID);
        if (sourceOfData->description()->hasMask())
        {
            mHasMask = 1;
            mPointerMaskJ[direction] =
                sourceOfData->pointerMaskJ(direction, materialID);
            mPointerMaskK[direction] =
                sourceOfData->pointerMaskK(direction, materialID);
        }
        else
            mHasMask = 0;
        
        LOG << "Pointers are:\n";
        LOGMORE << mPointerJ[direction] << "\n"
            << mPointerK[direction] << "\n"
            << mPointerMaskJ[direction] << "\n"
            << mPointerMaskK[direction] << "\n";
    }
    
    if (sourceOfData->description()->hasMask())
        mStrideMask = 1;
    else
        mStrideMask = 0;
    
    if (sourceOfData->description()->isSpaceVarying())
        mStride = 1;
    else
        mStride = 0;
    
    if (sourceOfData->description()->sourceCurrents().usesPolarization())
    {
        mPolarizationVector =
            sourceOfData->description()->sourceCurrents().polarization();
    }
    else
        mPolarizationVector = Vector3f(1.0, 1.0, 1.0);
}

void BufferedCurrent::
allocateAuxBuffers()
{
}







