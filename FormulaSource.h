/*
 *  FormulaSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/7/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _FORMULASOURCE_
#define _FORMULASOURCE_

#include "SourceBoss.h"
#include "Pointer.h"
#include "geometry.h"
#include <vector>

class FormulaSourceDelegate : public SourceDelegate
{
public:
    FormulaSourceDelegate(const SourceDescPtr & desc);
    
    virtual SourcePtr makeSource(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    SourceDescPtr mDesc;
};

class FormulaSource : public Source
{
public:
    FormulaSource(const SourceDescription & desc);
    
private:
    std::string mFormula;
    Vector3f mPolarization;
    
    Vector3b mWhichE;
    Vector3b mWhichH;
    
    bool mIsSpaceVarying;
    bool mIsSoft;
    
    std::vector<Rect3i> mYeeRects;
    std::vector<long> mFirstTimestep;
    std::vector<long> mLastTimestep;
    
    Map<std::string, std::string> mParams;
};






#endif
