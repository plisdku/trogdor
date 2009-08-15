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

#pragma mark *** Setup ***

CurrentPolarizationSetupOutput::
CurrentPolarizationSetupOutput(const OutputDescPtr & desc,
    const VoxelizedPartition & vp) :
    SetupOutput(),
    mDescription(desc)
{
    // Make the runlines!
    
    assert(desc->getRegions().size() > 0);
    for (int rr = 0; rr < desc->getRegions().size(); rr++)
    {
        Rect3i yeeCells(clip(desc->getRegions()[rr].getYeeCells(),
            vp.getGridYeeCells()));
        
        for (int direction = 0; direction < 3; direction++)
        {
            if (desc->getWhichJ()[direction] != 0 ||
                desc->getWhichP()[direction] != 0)
            {
                addRunlinesInOctant(octantE(direction), yeeCells,
                    mRunlinesE[direction], vp);
            }
            if (desc->getWhichM()[direction] != 0)
                addRunlinesInOctant(octantH(direction), yeeCells,
                    mRunlinesH[direction], vp);
            
            LOG << "RunlinesE " << direction << ": " <<
                mRunlinesE[direction].size() << "\n";
            LOG << "RunlinesH " << direction << ": " <<
                mRunlinesH[direction].size() << "\n";
        }
    }
}

CurrentPolarizationSetupOutput::
~CurrentPolarizationSetupOutput()
{
    LOG << "Destroy destroy destroy!\n";
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


void CurrentPolarizationSetupOutput::
addRunlinesInOctant(int octant, Rect3i yeeCells,
    vector<SetupCPOutputRunline> & outRunlines,
    const VoxelizedPartition & vp)
{
    Rect3i halfCellBounds(yeeToHalf(yeeCells, octant));
	// First task: generate a starting half-cell in the correct octant.
	Vector3i offset = halfCellOffset(octant);
	Vector3i p1 = halfCellBounds.p1;
	for (int nn = 0; nn < 3; nn++)
	if (p1[nn] % 2 != offset[nn])
		p1[nn]++;
    
    const VoxelGrid & voxels(vp.getVoxelGrid());
    const PartitionCellCount & cellCount(*vp.getIndices());
    const Map<Paint*, RunlineEncoderPtr> & setupMaterials(vp.getDelegates());
    const InterleavedLattice & lattice(*vp.getLattice());
    
    // d0 is the direction of memory allocation.  In the for-loops, this is
    // the innermost of the three Cartesian directions.
    const int d0 = vp.getLattice()->runlineDirection();
    const int d1 = (d0+1)%3;
    const int d2 = (d0+2)%3;
    
    SetupCPOutputRunline currentRunline;
    
	bool needNewRunline = 1;
	Vector3i x(p1), lastX(p1);
	Paint *xParentPaint = 0L, *lastXParentPaint = 0L;
	for (x[d2] = p1[d2]; x[d2] <= halfCellBounds.p2[d2]; x[d2] += 2)
	for (x[d1] = p1[d1]; x[d1] <= halfCellBounds.p2[d1]; x[d1] += 2)
	for (x[d0] = p1[d0]; x[d0] <= halfCellBounds.p2[d0]; x[d0] += 2)
	{
		xParentPaint = voxels(x)->withoutCurlBuffers();
		
		if (!needNewRunline && xParentPaint != lastXParentPaint)
        {
            assert(cellCount(lastX) - currentRunline.startingIndex ==
                currentRunline.length - 1);
            outRunlines.push_back(currentRunline);
            needNewRunline = 1;
        }
		if (needNewRunline)
		{
            currentRunline.length = 0;
            currentRunline.startingIndex = cellCount(x);
            currentRunline.startingField = lattice.pointer(x);
            currentRunline.materialID = setupMaterials[xParentPaint]->id();
			needNewRunline = 0;
            
//            LOG << "New runline at " << x << " material "
//                << currentRunline.materialID << "\n";
		}
        currentRunline.length++;
		lastX = x;
		lastXParentPaint = xParentPaint;
	}
    outRunlines.push_back(currentRunline);
}

#pragma mark *** Output ***

CurrentPolarizationOutput::
CurrentPolarizationOutput(const OutputDescription & desc,
    const VoxelizedPartition & vp,
    const CalculationPartition & cp) :
    mDatafile(),
    mCurrentSampleInterval(0),
    mWhichJ(desc.getWhichJ()),
    mWhichP(desc.getWhichP()),
    mWhichK(desc.getWhichK()),
    mWhichM(desc.getWhichM()),
    mDurations(desc.getDurations())
{
    // Clip the regions to the current partition bounds (calc bounds, not
    //     allocation bounds!)
    LOG << "Clipping output regions to partition bounds.  This is not in the"
        " right place; it should be performed earlier somehow.\n";
    assert(desc.getRegions().size() > 0);
    for (int rr = 0; rr < desc.getRegions().size(); rr++)
    {
        Rect3i outRect(clip(desc.getRegions()[rr].getYeeCells(),
            vp.getGridYeeCells()));        
        mRegions.push_back(Region(outRect, desc.getRegions()[rr].getStride()));
    }
    LOG << "Truncating durations to simulation duration.  This is in the "
        "wrong place; can't it be done earlier?\n";
    int numTimesteps = cp.getDuration();
    for (int dd = 0; dd < mDurations.size(); dd++)
    if (mDurations[dd].getLast() > (numTimesteps-1))
        mDurations[dd].setLast(numTimesteps-1);
    
    string specfile(desc.getFile() + string(".txt"));
    string datafile(desc.getFile());
    string materialfile(desc.getFile() + string(".mat"));
    
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
        rl.material = cp.getMaterials().at(setupRunlines[nn].materialID);
        rl.startingIndex = setupRunlines[nn].startingIndex;
        rl.startingField = setupRunlines[nn].startingField.getPointer();
        rl.length = setupRunlines[nn].length;
//        LOG << "nn " << nn << " material " << rl.material->getSubstanceName()
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
        rl.material = cp.getMaterials().at(setupRunlines[nn].materialID);
        rl.startingIndex = setupRunlines[nn].startingIndex;
        rl.startingField = setupRunlines[nn].startingField.getPointer();
        rl.length = setupRunlines[nn].length;
        
//        LOG << "nn " << nn << " material " << rl.material->getSubstanceName()
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
    while (timestep > mDurations[mCurrentSampleInterval].getLast())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDurations.size())
            return;
    }
    
    int firstT = mDurations[mCurrentSampleInterval].getFirst();
    int period = mDurations[mCurrentSampleInterval].getPeriod();
    
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
    while (timestep > mDurations[mCurrentSampleInterval].getLast())
    {
        mCurrentSampleInterval++;
        if (mCurrentSampleInterval >= mDurations.size())
            return;
    }
    
    int firstT = mDurations[mCurrentSampleInterval].getFirst();
    int period = mDurations[mCurrentSampleInterval].getPeriod();
    
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
    
    Vector3i unitVector0 = cardinal(2*vp.getLattice()->runlineDirection()+1);
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
    descFile << "dxyz " << cp.getDxyz() << "\n";
    descFile << "dt " << cp.getDt() << "\n";
    
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
            << mRegions[nn].getYeeCells()-vp.getOriginYee()
            << " stride "
            << mRegions[nn].getStride()
            << "\n";
    }
    
    for (nn = 0; nn < mDurations.size(); nn++)
    {
        descFile << "duration from "
            << mDurations[nn].getFirst() << " to "
            << mDurations[nn].getLast() << " period "
            << mDurations[nn].getPeriod() << "\n";
    }
    
    descFile.close();
}

