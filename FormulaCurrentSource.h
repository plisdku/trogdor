/*
 *  FormulaCurrentSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/14/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _FORMULACURRENTSOURCE_
#define _FORMULACURRENTSOURCE_

#include "CurrentSource.h"
#include "calc.hh"

class SetupFormulaCurrentSource : public SetupCurrentSource
{
public:
    SetupFormulaCurrentSource(const CurrentSourceDescPtr & description);
    
    virtual Pointer<CurrentSource> makeCurrentSource(
        const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
};


class FormulaCurrentSource : public CurrentSource
{
public:
    FormulaCurrentSource(const CurrentSourceDescPtr & description,
        const VoxelizedPartition & vp, const CalculationPartition & cp);
    
    virtual void prepareJ(long timestep);
    virtual void prepareK(long timestep);
    
    virtual float getJ(int direction) const;
    virtual float getK(int direction) const;
private:
    void updateJ(long timestep);
    void updateK(long timestep);
    
    std::string mFormula;
    calc_defs::Calculator<float> mCalculator;
    SourceCurrents mCurrents;
    
    float mDt;
    
    std::vector<Duration> mDurations;
    long mCurrentDuration;
    
    Vector3f mJ;
    Vector3f mK;
};


#endif