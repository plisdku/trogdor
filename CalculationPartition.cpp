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

#include "Log.h"
#include "VoxelizedPartition.h"
#include "Map.h"

using namespace std;

CalculationPartition::
CalculationPartition(const VoxelizedPartition & vp, Vector3f dxyz, float dt,
    long numT) :
    m_dxyz(dxyz),
    m_dt(dt),
    m_numT(numT),
    mEHBuffers(vp.getEHBuffers())
{
    LOG << "New calc partition.\n";
    unsigned int nn;
    
    // This allocates all the main fields and all the neighbor buffer fields.
    // This does not allocate any material aux variables.
    const int INTERLEAVE = 0;
    if (INTERLEAVE)
    {
        LOG << "Not going to do this yet.\n";
        exit(1);
    }
    else
    {
        allocate(mFields, *mEHBuffers);
        map<NeighborBufferDescPtr, EHBufferSet>::iterator itr;
        for (itr = mNBBuffers.begin(); itr != mNBBuffers.end(); itr++)
            allocate(mNBFields[itr->first], itr->second);
    }
    
    const Map<Paint*, MaterialDelegatePtr> & delegs = vp.getDelegates();
    map<Paint*, MaterialDelegatePtr>::const_iterator itr;
    for (itr = delegs.begin(); itr != delegs.end(); itr++)
    {
        LOG << "Dealing with paint " << *itr->first << endl;
        mMaterials.push_back(itr->second->makeCalcMaterial(vp, *this));
    }
    
    const std::vector<OutputDelegatePtr> & outs = vp.getOutputDelegates();
    for (nn = 0; nn < outs.size(); nn++)
        mOutputs.push_back(outs[nn]->makeOutput(vp, *this));
    
}

CalculationPartition::
~CalculationPartition()
{
}

void CalculationPartition::
allocateAuxBuffers()
{
    for (unsigned int nn = 0; nn < mMaterials.size(); nn++)
    {
        LOG << "Allocating aux buffers for material " << nn << ", in theory.\n";
        LOGMORE << "Not actually doing it.\n";
    }
}


void CalculationPartition::
calcE()
{
    for (int eNum = 0; eNum < 3; eNum++)
    for (unsigned int nn = 0; nn < mMaterials.size(); nn++)
        mMaterials[nn]->calcEPhase(eNum);
}

void CalculationPartition::
calcAfterE()
{
    unsigned int nn;
    int timestep = 5;
    
    for (nn = 0; nn < mOutputs.size(); nn++)
        mOutputs[nn]->outputEPhase(timestep);
}

void CalculationPartition::
calcH()
{
    for (int hNum = 0; hNum < 3; hNum++)
    for (unsigned int nn = 0; nn < mMaterials.size(); nn++)
        mMaterials[nn]->calcHPhase(hNum);
}

void CalculationPartition::
calcAfterH()
{
    unsigned int nn;
    int timestep = 5;
    
    for (nn = 0; nn < mOutputs.size(); nn++)
        mOutputs[nn]->outputHPhase(timestep);
}


void CalculationPartition::
allocate(std::vector<float> & data, EHBufferSet& buffers)
{
    int nn;
    int bufsize = 0;
    for (nn = 0; nn < 6; nn++)
        bufsize += buffers.buffers[nn].getLength();
    
    data.resize(bufsize);
    
    long offset = 0;
    for (nn = 0; nn < 6; nn++)
    {
        buffers.buffers[nn].setHeadPointer(&(data[offset]));
        offset += buffers.buffers[nn].getLength();
    }
}




