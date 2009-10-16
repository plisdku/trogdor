/*
 *  CurrentPolarizationOutput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/12/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _CURRENTPOLARIZATIONOUTPUT_
#define _CURRENTPOLARIZATIONOUTPUT_

#include "SimulationDescription.h"

#include "Output.h"
#include "MemoryUtilities.h"
#include "geometry.h"
#include <vector>
#include <fstream>

class UpdateEquation;
class VoxelizedPartition;
class CalculationPartition;

struct SetupCPOutputRunline
{
    long materialID;
    long startingIndex;
    BufferPointer startingField;
    long length;
};

class CurrentPolarizationSetupOutput : public SetupOutput
{
public:
    CurrentPolarizationSetupOutput(const OutputDescPtr & desc,
        const VoxelizedPartition & vp);
    virtual ~CurrentPolarizationSetupOutput();
    
    virtual OutputPtr makeOutput(const VoxelizedPartition & vp,
        const CalculationPartition & cp) const;
private:
    std::vector<SetupCPOutputRunline> mRunlinesE[3];
    std::vector<SetupCPOutputRunline> mRunlinesH[3];
};

struct CPOutputRunline
{
    UpdateEquation* material;
    long startingIndex;
    float* startingField;
    long length;
};

class CurrentPolarizationOutput : public Output
{
public:
    CurrentPolarizationOutput(OutputDescPtr description,
        const VoxelizedPartition & vp,
        const CalculationPartition & cp);
    virtual ~CurrentPolarizationOutput();
    
    void setRunlinesE(int direction, const CalculationPartition & cp,
        const std::vector<SetupCPOutputRunline> & setupRunlines);
    void setRunlinesH(int direction, const CalculationPartition & cp,
        const std::vector<SetupCPOutputRunline> & setupRunlines);
    
    virtual void outputEPhase(const CalculationPartition & cp, long timestep);
    virtual void outputHPhase(const CalculationPartition & cp, long timestep);
    
private:
    void writeJ(const CalculationPartition & cp);
    void writeK(const CalculationPartition & cp);
    /*
    void writeDescriptionFile(const VoxelizedPartition & vp,
        const CalculationPartition & cp,
        std::string specfile, std::string datafile, std::string materialfile)
        const;
    */
    std::ofstream mDatafile;
    long mCurrentSampleInterval;
    
    Vector3i mWhichJ;
    Vector3i mWhichP;
    Vector3i mWhichK;
    Vector3i mWhichM;
    
    std::vector<Region> mRegions;
    std::vector<Duration> mDurations;
    std::vector<CPOutputRunline> mRunlinesE[3];
    std::vector<CPOutputRunline> mRunlinesH[3];
};
















#endif