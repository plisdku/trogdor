/*
 *  CurrentPolarizationOutput.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/12/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "CurrentPolarizationOutput.h"
#include "SimulationDescription.h"
#include "CalculationPartition.h"
#include "VoxelizedPartition.h"
#include "InterleavedLattice.h"

#include "UpdateEquation.h"

#include "YeeUtilities.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Version.h"

using namespace std;
using namespace boost::posix_time;
using namespace YeeUtilities;

#pragma mark *** Local class for run length encoding ***

class CPRunlineEncoder : public RunlineEncoder
{
public:
    CPRunlineEncoder()
    {
        setMaterialContinuity(kMaterialContinuityRequired);
    }
    
    virtual void endRunline(const VoxelizedPartition & vp);
    
    vector<SetupCPOutputRunline> mRunlinesE[3];
    vector<SetupCPOutputRunline> mRunlinesH[3];
};

void CPRunlineEncoder::
endRunline(const VoxelizedPartition & vp)
{
    SetupCPOutputRunline rl;
    rl.length = length();
    rl.startingIndex = vp.indices()(firstHalfCell());
    rl.startingField = vp.lattice().pointer(firstHalfCell());
    rl.materialID = vp.setupMaterials()[vp.voxels()(firstHalfCell())]->id();
    
    int oct = octant(firstHalfCell());
    if (isE(oct))
        mRunlinesE[xyz(oct)].push_back(rl);
    else
        mRunlinesH[xyz(oct)].push_back(rl);
}

#pragma mark *** Setup class ***

CurrentPolarizationSetupOutput::
CurrentPolarizationSetupOutput(const OutputDescPtr & desc,
    const VoxelizedPartition & vp) :
    SetupOutput(),
    mDescription(desc)
{
    // Make the runlines!
        
    assert(desc->regions().size() > 0);
    for (int rr = 0; rr < desc->regions().size(); rr++)
    {
        Rect3i yeeCells(clip(desc->regions()[rr].yeeCells(),
            vp.gridYeeCells()));
        
        for (int direction = 0; direction < 3; direction++)
        {
            CPRunlineEncoder encoder;
            if (desc->whichJ()[direction] != 0 ||
                desc->whichP()[direction] != 0)
            {
                vp.runLengthEncode(encoder, yeeCells, octantE(direction));
                mRunlinesE[direction] = encoder.mRunlinesE[direction];
            }
            if (desc->whichM()[direction] != 0)
            {
                vp.runLengthEncode(encoder, yeeCells, octantH(direction));
                mRunlinesH[direction] = encoder.mRunlinesH[direction];
            }
//            LOG << "RunlinesE " << direction << ": " <<
//                mRunlinesE[direction].size() << "\n";
//            LOG << "RunlinesH " << direction << ": " <<
//                mRunlinesH[direction].size() << "\n";
        }
    }
}

CurrentPolarizationSetupOutput::
~CurrentPolarizationSetupOutput()
{
//    LOG << "Destroy destroy destroy!\n";
}

OutputPtr CurrentPolarizationSetupOutput::
makeOutput(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    CurrentPolarizationOutput* o = new CurrentPolarizationOutput(
        *mDescription, vp, cp);
    
    for (int nn = 0; nn < 3; nn++)
    {
        o->setRunlinesE(nn, cp, mRunlinesE[nn]);
        o->setRunlinesH(nn, cp, mRunlinesH[nn]);
    }
    return OutputPtr(o);
}

#pragma mark *** Output ***

CurrentPolarizationOutput::
CurrentPolarizationOutput(const OutputDescription & desc,
    const VoxelizedPartition & vp,
    const CalculationPartition & cp) :
    mDatafile(),
    mCurrentSampleInterval(0),
    mWhichJ(desc.whichJ()),
    mWhichP(desc.whichP()),
    mWhichK(desc.whichK()),
    mWhichM(desc.whichM()),
    mDurations(desc.durations())
{
    // Clip the regions to the current partition bounds (calc bounds, not
    //     allocation bounds!)
    LOGF << "Clipping output regions to partition bounds.  This is not in the"
        " right place; it should be performed earlier somehow.\n";
    assert(desc.regions().size() > 0);
    for (int rr = 0; rr < desc.regions().size(); rr++)
    {
        Rect3i outRect(clip(desc.regions()[rr].yeeCells(),
            vp.gridYeeCells()));        
        mRegions.push_back(Region(outRect, desc.regions()[rr].stride()));
    }
    LOGF << "Truncating durations to simulation duration.  This is in the "
        "wrong place; can't it be done earlier?\n";
    int numTimesteps = cp.duration();
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].last() > (numTimesteps-1))
        mDurations[dd].setLast(numTimesteps-1);
    
    string specfile(desc.file() + string(".txt"));
    string datafile(desc.file());
    string materialfile(desc.file() + string(".mat"));
    
    writeDescriptionFile(vp, cp, specfile, datafile, materialfile);
    
    mDatafile.open(datafile.c_str());
}

CurrentPolarizationOutput::
~CurrentPolarizationOutput()
{
    mDatafile.close();
}


void CurrentPolarizationOutput::
setRunlinesE(int direction, const CalculationPartition & cp,
    const std::vector<SetupCPOutputRunline> & setupRunlines)
{
    assert(direction >= 0 && direction < 3);
    mRunlinesE[direction].resize(setupRunlines.size());
    for (int nn = 0; nn < setupRunlines.size(); nn++)
    {
        CPOutputRunline rl;
        rl.material = cp.materials().at(setupRunlines[nn].materialID);
        rl.startingIndex = setupRunlines[nn].startingIndex;
        rl.startingField = setupRunlines[nn].startingField.pointer();
        rl.length = setupRunlines[nn].length;
//        LOG << "nn " << nn << " material " << rl.material->substanceName()
//            << "\n";
        mRunlinesE[direction][nn] = rl;
    }
}
    
void CurrentPolarizationOutput::
setRunlinesH(int direction, const CalculationPartition & cp,
    const std::vector<SetupCPOutputRunline> & setupRunlines)
{
    assert(direction >= 0 && direction < 3);
    mRunlinesH[direction].resize(setupRunlines.size());
    for (int nn = 0; nn < setupRunlines.size(); nn++)
    {
        CPOutputRunline rl;
        rl.material = cp.materials().at(setupRunlines[nn].materialID);
        rl.startingIndex = setupRunlines[nn].startingIndex;
        rl.startingField = setupRunlines[nn].startingField.pointer();
        rl.length = setupRunlines[nn].length;
        
//        LOG << "nn " << nn << " material " << rl.material->substanceName()
//            << "\n";
        
        mRunlinesH[direction][nn] = rl;
    }
}

void CurrentPolarizationOutput::
outputEPhase(const CalculationPartition & cp, int timestep)
{
    if (norm2(mWhichJ) == 0)
        return;
    if (mCurrentSampleInterval >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentSampleInterval].last())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDurations.size())
            return;
    }
    
    int firstT = mDurations[mCurrentSampleInterval].first();
    int period = mDurations[mCurrentSampleInterval].period();
    
    if (timestep >= firstT && (timestep - firstT)%period == 0)
        writeJ(cp);
}

void CurrentPolarizationOutput::
outputHPhase(const CalculationPartition & cp, int timestep)
{
    if (norm2(mWhichK) == 0)
        return;
    if (mCurrentSampleInterval >= mDurations.size())
        return;
    while (timestep > mDurations[mCurrentSampleInterval].last())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDurations.size())
            return;
    }
    
    int firstT = mDurations[mCurrentSampleInterval].first();
    int period = mDurations[mCurrentSampleInterval].period();
    
    if (timestep >= firstT && (timestep - firstT)%period == 0)
        writeK(cp);
}

void CurrentPolarizationOutput::
writeJ(const CalculationPartition & cp)
{
    for (int outDir = 0; outDir < 3; outDir++)
    {
        if (mWhichJ[outDir] != 0)
        {
            for (int rr = 0; rr < mRunlinesE[outDir].size(); rr++)
            {
                mRunlinesE[outDir][rr].material->writeJ(outDir, mDatafile,
                    mRunlinesE[outDir][rr].startingIndex,
                    mRunlinesE[outDir][rr].startingField,
                    mRunlinesE[outDir][rr].length);
            }
        }
    }
    assert(mDatafile.good());
    mDatafile << flush;
}

void CurrentPolarizationOutput::
writeK(const CalculationPartition & cp)
{
    for (int outDir = 0; outDir < 3; outDir++)
    {
        if (mWhichJ[outDir] != 0)
        {
            for (int rr = 0; rr < mRunlinesE[outDir].size(); rr++)
            {
                mRunlinesH[outDir][rr].material->writeK(outDir, mDatafile,
                    mRunlinesH[outDir][rr].startingIndex,
                    mRunlinesH[outDir][rr].startingField,
                    mRunlinesH[outDir][rr].length);
            }
        }
    }
    assert(mDatafile.good());
    mDatafile << flush;
}

void CurrentPolarizationOutput::
writeDescriptionFile(const VoxelizedPartition & vp,
    const CalculationPartition & cp, string specfile,
    string datafile, string materialfile) const
{
    int nn;
    
    Vector3i unitVector0 = cardinal(2*vp.lattice().runlineDirection()+1);
    Vector3i unitVector1 = cyclicPermute(unitVector0, 1);
    Vector3i unitVector2 = cyclicPermute(unitVector1, 1);
    
    ofstream descFile(specfile.c_str());
    
    //date today = day_clock::local_day();
    //date todayUTC = day_clock::universal_day();
    ptime now(second_clock::local_time());
    
    descFile << "trogdor5output\n";
    descFile << "trogdorMajorVersion " << TROGDOR_MAJOR_VERSION << "\n";
    descFile << "trogdorSVNVersion NOTUSED\n";
    descFile << "trogdorBuildDate " << __DATE__ << "\n";
    descFile << "specfile " << specfile << "\n";
    descFile << "datafile " << datafile << "\n";
    //descFile << "date " << to_iso_extended_string(today) << " "
    descFile << "date " << to_iso_extended_string(now) << "\n";
    descFile << "dxyz " << cp.dxyz() << "\n";
    descFile << "dt " << cp.dt() << "\n";
    
    // E fields
    for (nn = 0; nn < 3; nn++)
    if (mWhichJ[nn])
    {
        descFile << "field j" << char('x' + nn) << " "
            << eFieldPosition(nn) << " 0.0 \n";
    }
    
    // H fields
    for (nn = 0; nn < 3; nn++)
    if (mWhichK[nn])
    {
        descFile << "field k" << char('x' + nn) << " "
            << hFieldPosition(nn) << " 0.5\n";
    }
    
    descFile << "unitVector0 " << unitVector0 << "\n";
    descFile << "unitVector1 " << unitVector1 << "\n";
    descFile << "unitVector2 " << unitVector2 << "\n";
    
    for (nn = 0; nn < mRegions.size(); nn++)
    {
        descFile << "region "
            << mRegions[nn].yeeCells()-vp.originYee()
            << " stride "
            << mRegions[nn].stride()
            << "\n";
    }
    
    for (nn = 0; nn < mDurations.size(); nn++)
    {
        descFile << "duration from "
            << mDurations[nn].first() << " to "
            << mDurations[nn].last() << " period "
            << mDurations[nn].period() << "\n";
    }
    
    descFile.close();
}

