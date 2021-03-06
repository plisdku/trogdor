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
#include "VoxelizedPartition.h"

#include "Log.h"
#include "Map.h"
#include "YeeUtilities.h"

#include "TimeWrapper.h"

#include <cmath>

using namespace std;
using namespace YeeUtilities;

CalculationPartition::
CalculationPartition(const VoxelizedPartition & vp, Vector3f dxyz, float dt,
    long numT) :
    mGridDescription(vp.gridDescription()),
    m_dxyz(dxyz),
    m_dt(dt),
    m_numT(numT),
    mHuygensSurfaces(vp.huygensSurfaces()),
    mLattice(vp.getLattice())
{
//    LOG << "New calc partition.\n";
    unsigned int nn;
    
    // Allocate memory, both main grid and Huygens surfaces
    mLattice->allocate();
    for (int nn = 0; nn < mHuygensSurfaces.size(); nn++)
        mHuygensSurfaces.at(nn)->allocate();
    
    // Fill out other denizens.
    
    Map<CurrentSourceDescPtr, CurrentSource*> sourceMap;
    
    const vector<SetupCurrentSourcePtr> & curSrcs = vp.setupCurrentSources();
    for (nn = 0; nn < curSrcs.size(); nn++)
    {
        CurrentSourcePtr src = curSrcs[nn]->makeCurrentSource(vp, *this);
        sourceMap[curSrcs[nn]->description()] = src;
        mCurrentSources.push_back(src);
    }
    
    const Map<Paint*, SetupUpdateEquationPtr> & setupMaterials
        = vp.setupMaterials();
    map<Paint*, SetupUpdateEquationPtr>::const_iterator itr;
    mMaterials.resize(setupMaterials.size());
    for (itr = setupMaterials.begin(); itr != setupMaterials.end(); itr++)
    {
        UpdateEquationPtr newMaterial =
            itr->second->makeUpdateEquation(vp, *this);
        newMaterial->setSubstanceName(itr->first->fullName());
        newMaterial->setID(itr->second->id());
        
        if (itr->first->hasCurrentSource())
        {
            assert(sourceMap.count(itr->first->currentSource()) != 0);
            newMaterial->setCurrentSource(
                sourceMap[itr->first->currentSource()]);
        }
        
        assert(newMaterial->id() >= 0);
        assert(newMaterial->id() < mMaterials.size());
        mMaterials[newMaterial->id()] = newMaterial;
    }
    
    const vector<SetupOutputPtr> & outs = vp.setupOutputs();
    for (nn = 0; nn < outs.size(); nn++)
    {
        OutputPtr out = outs[nn]->makeOutput(vp, *this);
        mOutputs.push_back(out);
    }
    
    const vector<SetupSourcePtr> & softSrcs = vp.softSetupSources();
    for (nn = 0; nn < softSrcs.size(); nn++)
    {
        mSoftSources.push_back(softSrcs[nn]->makeSource(vp, *this));
    }
    
    const vector<SetupSourcePtr> & hardSrcs = vp.hardSetupSources();
    for (nn = 0; nn < hardSrcs.size(); nn++)
    {
        mHardSources.push_back(hardSrcs[nn]->makeSource(vp, *this));
    }
    
    mStatistics.setNumMaterials(mMaterials.size());
    mStatistics.setNumOutputs(mOutputs.size());
    mStatistics.setNumHardSources(mHardSources.size());
    mStatistics.setNumSoftSources(mSoftSources.size());
    mStatistics.setNumCurrentSources(mCurrentSources.size());
    mStatistics.setNumHuygensSurfaces(mHuygensSurfaces.size());
}

CalculationPartition::
~CalculationPartition()
{
}

const InterleavedLattice & CalculationPartition::
lattice() const
{
    return *mLattice; 
}

InterleavedLattice & CalculationPartition::
lattice()
{
    return *mLattice; 
}

void CalculationPartition::
allocateAuxBuffers()
{
    unsigned int nn;
    for (nn = 0; nn < mMaterials.size(); nn++)
        mMaterials[nn]->allocateAuxBuffers();
    for (nn = 0; nn < mOutputs.size(); nn++)
        mOutputs[nn]->allocateAuxBuffers();
    for (nn = 0; nn < mCurrentSources.size(); nn++)
        mCurrentSources[nn]->allocateAuxBuffers();
}

void CalculationPartition::
updateE(long timestep)
{
    unsigned int nn;
    
    for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
        mHuygensSurfaces[nn]->updateH(*this, timestep); // sum H before E.
    
    for (nn = 0; nn < mCurrentSources.size(); nn++)
        mCurrentSources[nn]->prepareJ(timestep, timestep*m_dt);
    
    for (int eNum = 0; eNum < 3; eNum++)
    for (nn = 0; nn < mMaterials.size(); nn++)
        mMaterials[nn]->calcEPhase(eNum);
}

void CalculationPartition::
sourceE(long timestep)
{
    //LOG << "Source E " << timestep << "\n";
    int nn;
    for (nn = 0; nn < mSoftSources.size(); nn++)
        mSoftSources[nn]->sourceEPhase(*this, timestep);
    for (nn = 0; nn < mHardSources.size(); nn++)
        mHardSources[nn]->sourceEPhase(*this, timestep);
}

void CalculationPartition::
outputE(long timestep)
{
    //LOG << "Output E " << timestep << "\n";
    int nn;
    for (nn = 0; nn < mOutputs.size(); nn++)
        mOutputs[nn]->outputEPhase(*this, timestep);
    
    //printFields(cout, octantE(0), 1.0);
    //printFields(cout, octantE(1), 1.0);    
//    cout << gridDescription()->name() << ":\n";
//    printFields(cout, octantE(2), 1.0);
}

void CalculationPartition::
updateH(long timestep)
{
    //LOG << "Update H " << timestep << "\n";
    unsigned int nn;
    
    // Update E fields in Huygens surfaces
    for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
        mHuygensSurfaces[nn]->updateE(*this, timestep); // sum E before H
        
    for (nn = 0; nn < mCurrentSources.size(); nn++)
        mCurrentSources[nn]->prepareK(timestep, (timestep+0.5)*m_dt);
    
    for (int hNum = 0; hNum < 3; hNum++)
    for (nn = 0; nn < mMaterials.size(); nn++)
        mMaterials[nn]->calcHPhase(hNum);
}

void CalculationPartition::
sourceH(long timestep)
{
    //LOG << "Source H " << timestep << "\n";
    int nn;
    for (nn = 0; nn < mSoftSources.size(); nn++)
        mSoftSources[nn]->sourceHPhase(*this, timestep);
    for (nn = 0; nn < mHardSources.size(); nn++)
        mHardSources[nn]->sourceHPhase(*this, timestep);
}

void CalculationPartition::
outputH(long timestep)
{
    //LOG << "Output H " << timestep << "\n";
    int nn;
    for (nn = 0; nn < mOutputs.size(); nn++)
        mOutputs[nn]->outputHPhase(*this, timestep);
}

void CalculationPartition::
timedUpdateE(long timestep)
{
    unsigned int nn;
    double t1, t2;
    
    for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mHuygensSurfaces[nn]->updateH(*this, timestep); // H before E.
        t2 = timeInMicroseconds();
        mStatistics.addHuygensSurfaceMicroseconds(nn, t2-t1);
    }
    
    
    for (nn = 0; nn < mCurrentSources.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mCurrentSources[nn]->prepareJ(timestep, timestep*m_dt);
        t2 = timeInMicroseconds();
        mStatistics.addCurrentSourceMicroseconds(nn, t2-t1);
    }
    
    for (int eNum = 0; eNum < 3; eNum++)
    for (nn = 0; nn < mMaterials.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mMaterials[nn]->calcEPhase(eNum);
        t2 = timeInMicroseconds();
        mStatistics.addMaterialMicrosecondsE(nn, t2-t1);
    }
    
    //LOG << "Finished with " << mMaterials.size() << " materials.\n";
}

void CalculationPartition::
timedSourceE(long timestep)
{
    //LOG << "Source E " << timestep << "\n";
    int nn;
    double t1, t2;
    for (nn = 0; nn < mSoftSources.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mSoftSources[nn]->sourceEPhase(*this, timestep);
        t2 = timeInMicroseconds();
        mStatistics.addSoftSourceMicroseconds(nn, t2-t1);
    }
    for (nn = 0; nn < mHardSources.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mHardSources[nn]->sourceEPhase(*this, timestep);
        t2 = timeInMicroseconds();
        mStatistics.addHardSourceMicroseconds(nn, t2-t1);
    }
}

void CalculationPartition::
timedOutputE(long timestep)
{
    //LOG << "Timed output E " << timestep << "\n";
    int nn;
    double t1, t2;
    for (nn = 0; nn < mOutputs.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mOutputs[nn]->outputEPhase(*this, timestep);
        t2 = timeInMicroseconds();
        mStatistics.addOutputMicroseconds(nn, t2-t1);
    }
    
    //printFields(cout, octantE(2), 1.0);
    
    //LOG << "Output E (3)\n";
    //printFields(cout, octantE(0), 1.0);
    //printFields(cout, octantE(1), 1.0);
    //printFields(cout, octantE(2), 1.0);
}

void CalculationPartition::
timedUpdateH(long timestep)
{
    //LOG << "Update H " << timestep << "\n";
    unsigned int nn;
    double t1, t2;
    
    // Update E fields in Huygens surfaces
    for (nn = 0; nn < mHuygensSurfaces.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mHuygensSurfaces[nn]->updateE(*this, timestep); // E before H
        t2 = timeInMicroseconds();
        mStatistics.addHuygensSurfaceMicroseconds(nn, t2-t1);
    }
    
    for (nn = 0; nn < mCurrentSources.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mCurrentSources[nn]->prepareK(timestep, (timestep+0.5)*m_dt);
        t2 = timeInMicroseconds();
        mStatistics.addCurrentSourceMicroseconds(nn, t2-t1);
    }
        
    for (int hNum = 0; hNum < 3; hNum++)
    for (nn = 0; nn < mMaterials.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mMaterials[nn]->calcHPhase(hNum);
        t2 = timeInMicroseconds();
        mStatistics.addMaterialMicrosecondsH(nn, t2-t1);
    }
    //LOG << "Finished with " << mMaterials.size() << " materials.\n";
}

void CalculationPartition::
timedSourceH(long timestep)
{
    //LOG << "Source H " << timestep << "\n";
    int nn;
    double t1, t2;
    for (nn = 0; nn < mSoftSources.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mSoftSources[nn]->sourceHPhase(*this, timestep);
        t2 = timeInMicroseconds();
        mStatistics.addSoftSourceMicroseconds(nn, t2-t1);
    }
    for (nn = 0; nn < mHardSources.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mHardSources[nn]->sourceHPhase(*this, timestep);
        t2 = timeInMicroseconds();
        mStatistics.addHardSourceMicroseconds(nn, t2-t1);
    }
}

void CalculationPartition::
timedOutputH(long timestep)
{
    //LOG << "Output H " << timestep << "\n";
    int nn;
    double t1, t2;
    for (nn = 0; nn < mOutputs.size(); nn++)
    {
        t1 = timeInMicroseconds();
        mOutputs[nn]->outputHPhase(*this, timestep);
        t2 = timeInMicroseconds();
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



