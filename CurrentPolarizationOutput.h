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

#include "RunlineEncoder.h"
#include "OutputBoss.h"
#include "MemoryUtilities.h"
#include "geometry.h"
#include <vector>
#include <fstream>

class UpdateEquation;
class VoxelizedPartition;
class CalculationPartition;

struct SetupCPOutputRunline
{
    int materialID;
    int startingIndex;
    BufferPointer startingField;
    int length;
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
    RunlineEncoder mNew;
    
    void addRunlinesInOctant(int octant,
        Rect3i yeeCells,
        std::vector<SetupCPOutputRunline> & rls,
        const VoxelizedPartition & vp);
    OutputDescPtr mDescription;
    std::vector<SetupCPOutputRunline> mRunlinesE[3];
    std::vector<SetupCPOutputRunline> mRunlinesH[3];
};

struct CPOutputRunline
{
    UpdateEquation* material;
    int startingIndex;
    float* startingField;
    int length;
};

class CurrentPolarizationOutput : public Output
{
public:
    CurrentPolarizationOutput(const OutputDescription & desc,
        const VoxelizedPartition & vp,
        const CalculationPartition & cp);
    virtual ~CurrentPolarizationOutput();
    
    void setRunlinesE(int direction, const CalculationPartition & cp,
        const std::vector<SetupCPOutputRunline> & setupRunlines);
    void setRunlinesH(int direction, const CalculationPartition & cp,
        const std::vector<SetupCPOutputRunline> & setupRunlines);
    
    virtual void outputEPhase(const CalculationPartition & cp, int timestep);
    virtual void outputHPhase(const CalculationPartition & cp, int timestep);
    
private:
    void writeJ(const CalculationPartition & cp);
    void writeK(const CalculationPartition & cp);
    
    void writeDescriptionFile(const VoxelizedPartition & vp,
        const CalculationPartition & cp,
        std::string specfile, std::string datafile, std::string materialfile)
        const;
    
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