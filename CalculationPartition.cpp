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

#include "Log.h"
#include "VoxelizedPartition.h"
#include "Map.h"
#include "YeeUtilities.h"

#include <cmath>

using namespace std;
using namespace YeeUtilities;

CalculationPartition::
CalculationPartition(const VoxelizedPartition & vp, Vector3f dxyz, float dt,
    long numT) :
    m_dxyz(dxyz),
    m_dt(dt),
    m_numT(numT),
    mAllocOriginYee(vp.getAllocYeeCells().p1),
    mAllocYeeCells(vp.getAllocYeeCells().size() + 1),
    mBuffersE(vp.getBuffersE()),
    mBuffersH(vp.getBuffersH()),
    mNBBuffersE(vp.getNBBuffersE()),
    mNBBuffersH(vp.getNBBuffersH())
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
        allocate(mFieldsE, mBuffersE);
        allocate(mFieldsH, mBuffersH);
        map<NeighborBufferDescPtr, vector<MemoryBufferPtr> >::iterator itr;
        for (itr = mNBBuffersE.begin(); itr != mNBBuffersE.end(); itr++)
            allocate(mNBFieldsE[itr->first], itr->second);
        for (itr = mNBBuffersH.begin(); itr != mNBBuffersH.end(); itr++)
            allocate(mNBFieldsH[itr->first], itr->second);
    }
    
    const Map<Paint*, SetupMaterialPtr> & delegs = vp.getDelegates();
    map<Paint*, SetupMaterialPtr>::const_iterator itr;
    for (itr = delegs.begin(); itr != delegs.end(); itr++)
    {
        //LOG << "Dealing with paint " << *itr->first << endl;
        mMaterials.push_back(itr->second->makeCalcMaterial(vp, *this));
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
    
    
    // Prepare helper variables for the field accessors.
    // We cache the head pointers just so they're in a quick order for later.
    for (int dir = 0; dir < 3; dir++)
    {
        mHeadE[dir] = mBuffersE[dir]->getHeadPointer();
        mHeadH[dir] = mBuffersH[dir]->getHeadPointer();
        mEOffset[dir] = eFieldPosition(dir);
        mHOffset[dir] = hFieldPosition(dir);
        /*
        LOG << "mHeadE[" << dir << "] is "
            << MemoryBuffer::identify(mHeadE[dir]) << "\n";
        LOG << "mHeadH[" << dir << "] is "
            << MemoryBuffer::identify(mHeadH[dir]) << "\n";
        */
        
    }
    mMemStride[0] = mBuffersE[0]->getStride(); // should be same for all
    mMemStride[1] = mAllocYeeCells[0]*mMemStride[0];
    mMemStride[2] = mAllocYeeCells[1]*mMemStride[1];
}

CalculationPartition::
~CalculationPartition()
{
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
    // Update Huygens surfaces (add H stuff)
    
    //LOG << "Update E " << timestep << "\n";
    
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
    
    printFields(cout, octantE(2), 1.0);
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



float CalculationPartition::
getE(int direction, int xi, int xj, int xk) const
{
    // wrap coordinates within alloc region (this handles periodic boundaries
    // when there's only one partition, e.g. non-MPI).
    xi = (xi - mAllocOriginYee[0] + mAllocYeeCells[0])%mAllocYeeCells[0];
    xj = (xj - mAllocOriginYee[1] + mAllocYeeCells[1])%mAllocYeeCells[1];
    xk = (xk - mAllocOriginYee[2] + mAllocYeeCells[2])%mAllocYeeCells[2];
    
    return mHeadE[direction][xi*mMemStride[0] + xj*mMemStride[1] +
        xk*mMemStride[2] ];
}
    
float CalculationPartition::
getH(int direction, int xi, int xj, int xk) const
{
    // wrap coordinates within alloc region (this handles periodic boundaries
    // when there's only one partition, e.g. non-MPI).
    xi = (xi - mAllocOriginYee[0] + mAllocYeeCells[0])%mAllocYeeCells[0];
    xj = (xj - mAllocOriginYee[1] + mAllocYeeCells[1])%mAllocYeeCells[1];
    xk = (xk - mAllocOriginYee[2] + mAllocYeeCells[2])%mAllocYeeCells[2];
    
    return mHeadH[direction][xi*mMemStride[0] + xj*mMemStride[1] +
        xk*mMemStride[2] ];
}

float CalculationPartition::
getE(int direction, Vector3i xx) const
{
    // wrap coordinates within alloc region (this handles periodic boundaries
    // when there's only one partition, e.g. non-MPI).
    xx = (xx - mAllocOriginYee + mAllocYeeCells)%mAllocYeeCells;
    
    return mHeadE[direction][dot(xx, mMemStride)];
}
    
float CalculationPartition::
getH(int direction, Vector3i xx) const
{
    // wrap coordinates within alloc region (this handles periodic boundaries
    // when there's only one partition, e.g. non-MPI).
    xx = (xx - mAllocOriginYee + mAllocYeeCells)%mAllocYeeCells;
    
    return mHeadH[direction][dot(xx, mMemStride)];
}

float CalculationPartition::
getE(int direction, float xi, float xj, float xk) const
{
    xi = xi - mEOffset[direction][0];
    xj = xj - mEOffset[direction][1];
    xk = xk - mEOffset[direction][2];
    const int ilow(floor(xi)), jlow(floor(xj)), klow(floor(xk));
    const int ihigh(ilow+1), jhigh(jlow+1), khigh(klow+1);
    const float dx(xi-floor(xi)), dy(xj-floor(xj)), dz(xk-floor(xk));
    
    return dx*dy*dz*getE(direction, ihigh, jhigh, khigh) +
        (1.0f-dx)*dy*dz*getE(direction, ilow, jhigh, khigh) +
        dx*(1.0f-dy)*dz*getE(direction, ihigh, jlow, khigh) +
        (1.0f-dx)*(1.0f-dy)*dz*getE(direction, ilow, jlow, khigh) +
        dx*dy*(1.0f-dz)*getE(direction, ihigh, jhigh, klow) +
        (1.0f-dx)*dy*(1.0f-dz)*getE(direction, ilow, jhigh, klow) +
        dx*(1.0f-dy)*(1.0f-dz)*getE(direction, ihigh, jlow, klow) +
        (1.0f-dx)*(1.0f-dy)*(1.0f-dz)*getE(direction, ilow, jlow, klow);
}

float CalculationPartition::
getH(int direction, float xi, float xj, float xk) const
{
    xi = xi - mHOffset[direction][0];
    xj = xj - mHOffset[direction][1];
    xk = xk - mHOffset[direction][2];
    const int ilow(floor(xi)), jlow(floor(xj)), klow(floor(xk));
    const int ihigh(ilow+1), jhigh(jlow+1), khigh(klow+1);
    const float dx(xi-floor(xi)), dy(xj-floor(xj)), dz(xk-floor(xk));
    
    return dx*dy*dz*getH(direction, ihigh, jhigh, khigh) +
        (1.0f-dx)*dy*dz*getH(direction, ilow, jhigh, khigh) +
        dx*(1.0f-dy)*dz*getH(direction, ihigh, jlow, khigh) +
        (1.0f-dx)*(1.0f-dy)*dz*getH(direction, ilow, jlow, khigh) +
        dx*dy*(1.0f-dz)*getH(direction, ihigh, jhigh, klow) +
        (1.0f-dx)*dy*(1.0f-dz)*getH(direction, ilow, jhigh, klow) +
        dx*(1.0f-dy)*(1.0f-dz)*getH(direction, ihigh, jlow, klow) +
        (1.0f-dx)*(1.0f-dy)*(1.0f-dz)*getH(direction, ilow, jlow, klow);
}


float CalculationPartition::
getE(int direction, Vector3f xx) const
{
    return getE(direction, xx[0], xx[1], xx[2]); // lazy
}

float CalculationPartition::
getH(int direction, Vector3f xx) const
{
    return getH(direction, xx[0], xx[1], xx[2]); // also lazy, help me gcc!!!
}

void CalculationPartition::
setE(int direction, int xi, int xj, int xk, float val)
{
    xi = (xi - mAllocOriginYee[0] + mAllocYeeCells[0])%mAllocYeeCells[0];
    xj = (xj - mAllocOriginYee[1] + mAllocYeeCells[1])%mAllocYeeCells[1];
    xk = (xk - mAllocOriginYee[2] + mAllocYeeCells[2])%mAllocYeeCells[2];
    
    mHeadE[direction][xi*mMemStride[0] + xj*mMemStride[1] +
        xk*mMemStride[2] ] = val;
    //LOG << MemoryBuffer::identify(&mHeadE[direction][xi*mMemStride[0] + xj*mMemStride[1] +
    //    xk*mMemStride[2]]);
}

void CalculationPartition::
setH(int direction, int xi, int xj, int xk, float val)
{
    xi = (xi - mAllocOriginYee[0] + mAllocYeeCells[0])%mAllocYeeCells[0];
    xj = (xj - mAllocOriginYee[1] + mAllocYeeCells[1])%mAllocYeeCells[1];
    xk = (xk - mAllocOriginYee[2] + mAllocYeeCells[2])%mAllocYeeCells[2];
    
    mHeadH[direction][xi*mMemStride[0] + xj*mMemStride[1] +
        xk*mMemStride[2] ] = val;
    //LOG << MemoryBuffer::identify(&mHeadH[direction][xi*mMemStride[0] + xj*mMemStride[1] +
    //    xk*mMemStride[2]]);
}


void CalculationPartition::
printFields(std::ostream & str, int field, float scale)
{
    float spaceMax = 0.2*scale;
    float periodMax = 0.5*scale;
    int direction;
    bool isEHere = isE(field);
    Rect3i r(mAllocOriginYee, mAllocOriginYee+mAllocYeeCells-1);
    
    int ni, nj, nk;
    
    if (isEHere)
    {
        direction = xyz(field);
        assert(direction != -1);
        for (nk = r.p1[2]; nk <= r.p2[2]; nk++)
        {
            str << "+";
            for (ni = r.p1[0]; ni <= r.p2[0]; ni++)
                str << "-";
            str << "+\n";
            for (nj = r.p2[1]; nj >= r.p1[1]; nj--)
            {
                str << "|";
                for (ni = r.p1[0]; ni <= r.p2[0]; ni++)
                {
                    float field = fabs(getE(direction, ni, nj, nk));
                    if (field < spaceMax)
                        str << " ";
                    else if (field < periodMax)
                        str << ".";
                    else
                        str << "•";
                }
                str << "|\n";
            }
            str << "+";
            for (ni = r.p1[0]; ni <= r.p2[0]; ni++)
                str << "-";
            str << "+\n";
        }
    }
    else
    {
        direction = xyz(field);
        assert(direction != -1);
        for (int nk = r.p1[2]; nk <= r.p2[2]; nk++)
        {
            for (int nj = r.p2[1]; nj >= r.p1[1]; nj--)
            {
                for (int ni = r.p1[0]; ni <= r.p2[0]; ni++)
                {
                    float field = fabs(getH(direction, ni, nj, nk));
                    if (field < spaceMax)
                        str << " ";
                    else if (field < periodMax)
                        str << ".";
                    else
                        str << "•";
                }
                str << "\n";
            }
        }
    }
}

void CalculationPartition::
createHuygensSurfaces(const VoxelizedPartition & vp)
{
    const vector<SetupHuygensSurfacePtr> & huyg =
        vp.getSetupHuygensSurfaces();
    for (unsigned int nn = 0; nn < huyg.size(); nn++)
    {
        mHuygensSurfaces.push_back(huyg[nn]->makeHuygensSurface());
    }
}


void CalculationPartition::
allocate(std::vector<float> & data, vector<MemoryBufferPtr> & buffers)
{
    int nn;
    int bufsize = 0;
    long offset = 0;
    
    for (nn = 0; nn < 3; nn++)
        bufsize += buffers[nn]->getLength();
    //LOG << "Bufsize is " << bufsize << endl;
    data.resize(bufsize);
    
    for (nn = 0; nn < 3; nn++)
    {
        buffers[nn]->setHeadPointer(&(data[offset]));
        offset += buffers[nn]->getLength();
    }
}




