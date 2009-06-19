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
#include "SimulationDescription.h"
#include "Pointer.h"
#include "geometry.h"
#include "calc.hh"
#include <vector>

class FormulaSetupSource : public SetupSource
{
public:
    FormulaSetupSource(const SourceDescPtr & desc);
    
    virtual SourcePtr makeSource(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    SourceDescPtr mDesc;
};

class FormulaSource : public Source
{
public:
    FormulaSource(const SourceDescription & desc,
        const VoxelizedPartition & vp, const CalculationPartition & cp);
    
    virtual void sourceEPhase(CalculationPartition & cp, int timestep);
    virtual void sourceHPhase(CalculationPartition & cp, int timestep);
private:
    void uniformSourceE(CalculationPartition & cp, int timestep);
    void uniformSourceH(CalculationPartition & cp, int timestep);
    void polarizedSourceE(CalculationPartition & cp, int timestep);
    void polarizedSourceH(CalculationPartition & cp, int timestep);
    
    int mCurrentDuration;
    
    std::string mFormula;
	calc_defs::Calculator<float> mCalculator;
    SourceFields mFields;
    
    float mDt;
    bool mIsSpaceVarying;
    bool mIsSoft;
    
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
    
    Map<std::string, std::string> mParams;
};






#endif
