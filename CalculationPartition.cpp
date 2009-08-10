/*
 *  CalculationPartition.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 5/4/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#include "CalculationPartition.h"
#include "SimulationDescription.h"
#include "HuygensSurface.h"
#include "InterleavedLattice.h"

#include "Log.h"
#include "VoxelizedPartition.h"
#include "Map.h"
#include "YeeUtilities.h"

#include "TimeWrapper.h"

#include <cmath>

using namespace std;
using namespace YeeUtilities;

CalculationPartition::
CalculationPartition(const VoxelizedPartition & vp, Vector3f dxyz, float dt,
    long numT) :
    //mGridName(vp.getGridName()),
    m_dxyz(dxyz),
    m_dt(dt),
    m_numT(numT),
    mHuygensSurfaces(vp.getHuygensSurfaces()),
    mLattice(vp.getLattice())
{
//    LOG << "New calc partition.\n";
    unsigned int nn;
    
    // Allocate memory, both main grid and Huygens surfaces
    mLattice->allocate();
    for (int nn = 0; nn < mHuygensSurfaces.size(); nn++)
        mHuygensSurfaces.at(nn)->allocate();
    
    // Fill out other denizens.
    const Map<Paint*, SetupMaterialPtr> & delegs = vp.getDelegates();
    map<Paint*, SetupMaterialPtr>::const_iterator itr;
    for (itr = delegs.begin(); itr != delegs.end(); itr++)
    {
        //LOG << "Dealing with paint " << *itr->first << endl;
        MaterialPtr newMaterial = itr->second->makeCalcMaterial(vp, *this);
        newMaterial->setSubstanceName(itr->first->getFullName());
        mMaterials.push_back(newMaterial);
    }
    
    const vector<SetupOutputPtr> & outs = vp.getSetupOutputs();
    for (nn = 0; nn < outs.size(); nn++)
        mOutputs.push_back(outs[nn]->makeOutput(vp,
            *this));
    
    const vector<SetupSourcePtr> & softSrcs = vp.getSoftSetupSources();
    for (nn = 0; nn < softSrcs.size(); nn++)
    {
        mSoftSources.push_back(softSrcs[nn]->makeSource(vp, *this));
    }
    
    const vector<SetupSourcePtr> & hardSrcs = vp.getHardSetupSources();
    for (nn = 0; nn < hardSrcs.size(); nn++)
    {
        mHardSources.push_back(hardSrcs[nn]->makeSource(vp, *this));
    }
    
    mStatistics.setNumMaterials(mMaterials.size());
    mStatistics.setNumOutputs(mOutputs.size());
    mStatistics.setNumHardSources(mHardSources.size());
    mStatistics.setNumSoftSources(mSoftSources.size());
    mStatistics.setNumHuygensSurfaces(mHuygensSurfaces.size());
}

CalculationPartition::
~CalculationPartition()
{
}

const InterleavedLattice & CalculationPartition::
getLattice() const
{
    return *mLattice; 
}

InterleavedLattice & CalculationPartition::
getLattice()
{
    return *mLattice; 
}

void CalculationPartition::
allocateAuxBuffers()
{
    unsigned int nn;
    for (nn = 0; nn < mMaterials.size(); nn++)
    {
        //LOG << "Aux alloc for material " << nn << "\n";
        mMaterials[nn]->allocateAuxBuffers();
    }
    
    for (nn = 0; nn < mOutputs.size(); nn++)
    {
        mOutputs[nn]->allocateAuxBuffers();
    }
}

void CalculationPartition::
updateE(int timestep)
{
    unsigned int nn;
    
    for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
        mHuygensSurfaces[nn]->updateH(); // need to update H here before E.
    
    for (int eNum = 0; eNum < 3; eNum++)
    for (nn = 0; nn < mMaterials.size(); nn++)
        mMaterials[nn]->calcEPhase(eNum);
}

void CalculationPartition::
sourceE(int timestep)
{
    //LOG << "Source E " << timestep << "\n";
    int nn;
    for (nn = 0; nn < mSoftSources.size(); nn++)
        mSoftSources[nn]->sourceEPhase(*this, timestep);
    for (nn = 0; nn < mHardSources.size(); nn++)
        mHardSources[nn]->sourceEPhase(*this, timestep);
}

void CalculationPartition::
outputE(int timestep)
{
    //LOG << "Output E " << timestep << "\n";
    int nn;
    for (nn = 0; nn < mOutputs.size(); nn++)
        mOutputs[nn]->outputEPhase(*this, timestep);
    
    //printFields(cout, octantE(2), 1.0);
}

void CalculationPartition::
updateH(int timestep)
{
    //LOG << "Update H " << timestep << "\n";
    unsigned int nn;
    
    // Update E fields in Huygens surfaces
    for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
        mHuygensSurfaces[nn]->updateE(); // need to update E here before H.
        
    for (int hNum = 0; hNum < 3; hNum++)
    for (nn = 0; nn < mMaterials.size(); nn++)
        mMaterials[nn]->calcHPhase(hNum);
}

void CalculationPartition::
sourceH(int timestep)
{
    //LOG << "Source H " << timestep << "\n";
    int nn;
    for (nn = 0; nn < mSoftSources.size(); nn++)
        mSoftSources[nn]->sourceHPhase(*this, timestep);
    for (nn = 0; nn < mHardSources.size(); nn++)
        mHardSources[nn]->sourceHPhase(*this, timestep);
}

void CalculationPartition::
outputH(int timestep)
{
    //LOG << "Output H " << timestep << "\n";
    int nn;
    for (nn = 0; nn < mOutputs.size(); nn++)
        mOutputs[nn]->outputHPhase(*this, timestep);
}

void CalculationPartition::
timedUpdateE(int timestep)
{
    unsigned int nn;
    double t1, t2;
    
    for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mHuygensSurfaces[nn]->updateH(); // need to update H here before E.
        t2 = getTimeInMicroseconds();
        mStatistics.addHuygensSurfaceMicroseconds(nn, t2-t1);
    }
    
    for (int eNum = 0; eNum < 3; eNum++)
    for (nn = 0; nn < mMaterials.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mMaterials[nn]->calcEPhase(eNum);
        t2 = getTimeInMicroseconds();
        mStatistics.addMaterialMicrosecondsE(nn, t2-t1);
    }
    
    //LOG << "Finished with " << mMaterials.size() << " materials.\n";
}

void CalculationPartition::
timedSourceE(int timestep)
{
    //LOG << "Source E " << timestep << "\n";
    int nn;
    double t1, t2;
    for (nn = 0; nn < mSoftSources.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mSoftSources[nn]->sourceEPhase(*this, timestep);
        t2 = getTimeInMicroseconds();
        mStatistics.addSoftSourceMicroseconds(nn, t2-t1);
    }
    for (nn = 0; nn < mHardSources.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mHardSources[nn]->sourceEPhase(*this, timestep);
        t2 = getTimeInMicroseconds();
        mStatistics.addHardSourceMicroseconds(nn, t2-t1);
    }
}

void CalculationPartition::
timedOutputE(int timestep)
{
    //LOG << "Timed output E " << timestep << "\n";
    int nn;
    double t1, t2;
    for (nn = 0; nn < mOutputs.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mOutputs[nn]->outputEPhase(*this, timestep);
        t2 = getTimeInMicroseconds();
        mStatistics.addOutputMicroseconds(nn, t2-t1);
    }
    //printFields(cout, octantE(2), 1.0);
    /*
    LOG << "Output E (3)\n";
    printFields(cout, octantE(0), 1.0);
    printFields(cout, octantE(1), 1.0);
    printFields(cout, octantE(2), 1.0);
    */
}

void CalculationPartition::
timedUpdateH(int timestep)
{
    //LOG << "Update H " << timestep << "\n";
    unsigned int nn;
    double t1, t2;
    
    // Update E fields in Huygens surfaces
    for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mHuygensSurfaces[nn]->updateE(); // need to update E here before H.
        t2 = getTimeInMicroseconds();
        mStatistics.addHuygensSurfaceMicroseconds(nn, t2-t1);
    }
        
    for (int hNum = 0; hNum < 3; hNum++)
    for (nn = 0; nn < mMaterials.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mMaterials[nn]->calcHPhase(hNum);
        t2 = getTimeInMicroseconds();
        mStatistics.addMaterialMicrosecondsH(nn, t2-t1);
    }
    //LOG << "Finished with " << mMaterials.size() << " materials.\n";
}

void CalculationPartition::
timedSourceH(int timestep)
{
    //LOG << "Source H " << timestep << "\n";
    int nn;
    double t1, t2;
    for (nn = 0; nn < mSoftSources.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mSoftSources[nn]->sourceHPhase(*this, timestep);
        t2 = getTimeInMicroseconds();
        mStatistics.addSoftSourceMicroseconds(nn, t2-t1);
    }
    for (nn = 0; nn < mHardSources.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mHardSources[nn]->sourceHPhase(*this, timestep);
        t2 = getTimeInMicroseconds();
        mStatistics.addHardSourceMicroseconds(nn, t2-t1);
    }
}

void CalculationPartition::
timedOutputH(int timestep)
{
    //LOG << "Output H " << timestep << "\n";
    int nn;
    double t1, t2;
    for (nn = 0; nn < mOutputs.size(); nn++)
    {
        t1 = getTimeInMicroseconds();
        mOutputs[nn]->outputHPhase(*this, timestep);
        t2 = getTimeInMicroseconds();
        mStatistics.addOutputMicroseconds(nn, t2-t1);
    }
    /*
    LOG << "Output H (3)\n";
    printFields(cout, octantH(0), 1.0/377);
    printFields(cout, octantH(1), 1.0/377);
    printFields(cout, octantH(2), 1.0/377);
    */
}



void CalculationPartition::
printFields(std::ostream & str, int octant, float scale)
{
    if (isH(octant))
        mLattice->printH(str, xyz(octant), scale);
    else if (isE(octant))
        mLattice->printE(str, xyz(octant), scale);
}

void CalculationPartition::
printPerformanceForMatlab(std::ostream & str, string prefix)
{
    mStatistics.printForMatlab(str, prefix, mMaterials, m_numT);
}



