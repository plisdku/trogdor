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

/*
    Interestingly this thing gets its BufferPointers on a two-step bounce from
    the BufferedFieldInput.  It's worth asking if maybe the CurrentSource should
    provide access directly to the BufferedFieldInput... hmm.
*/
void BufferedCurrent::
setCurrentSource(CurrentSource* sourceOfData, int myMaterialID)
{
    LOG << "Right here!\n";
    mSourceOfData = sourceOfData;
    
    mHasMask = sourceOfData->description()->hasMask();
    
    for (int xyz = 0; xyz < 3; xyz++)
    {
        if (sourceOfData->description()->sourceCurrents().whichJ()[xyz])
        {
            mPointerJ[xyz] = sourceOfData->pointerJ(xyz, myMaterialID);
            if (mHasMask)
                mPointerMaskJ[xyz] = sourceOfData->pointerMaskJ(
                    xyz, myMaterialID);
        }
        
        if (sourceOfData->description()->sourceCurrents().whichK()[xyz])
        {
            mPointerK[xyz] = sourceOfData->pointerK(xyz, myMaterialID);
            if (mHasMask)
                mPointerMaskK[xyz] = sourceOfData->pointerMaskK(
                    xyz, myMaterialID);
        }
        
        LOG << "Pointers are:\n";
        LOGMORE << mPointerJ[xyz] << "\n"
            << mPointerK[xyz] << "\n"
            << mPointerMaskJ[xyz] << "\n"
            << mPointerMaskK[xyz] << "\n";
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







