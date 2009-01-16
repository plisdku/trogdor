/*
 *  SimulationGrid.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/14/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "SimulationGrid.h"
#include "TimeWrapper.h"
//#include <ctime>

#include "MaterialModel.h"
#include "Output.h"
#include "Input.h"
#include "Source.h"
#include "TFSFBufferSet.h"
#include "Fields.h"

#include <cassert>

//#include <boost/thread/thread.hpp>

using namespace std;

SimulationGrid::
SimulationGrid(FieldsPtr inFields, std::vector<SourcePtr>& sources,
    std::vector<MaterialPtr>& materials,
    std::vector<OutputPtr>& outputs,
    std::vector<InputPtr>& inputs,
	std::vector<TFSFBufferSetPtr>& buffers,
	int numThreads ) :
    mFields(inFields),
    mSources(sources),
    mOutputs(outputs),
    mInputs(inputs),
    mMaterials(materials),
	mTFSFBuffers(buffers),
	mMaterialMicroseconds(materials.size(), 0),
	mOutputMicroseconds(outputs.size(), 0),
	mInputMicroseconds(inputs.size(), 0),
	mSourceMicroseconds(sources.size(), 0),
	mBufferMicroseconds(buffers.size(), 0),
	mNumThreads(numThreads)
{
    LOGF << "constructor, num threads = " << mNumThreads << endl;
	
	if (mNumThreads != 1)
	{
		LOG << "Warning: multithreading disabled.\n";
	}
}


SimulationGrid::
~SimulationGrid()
{
    LOGF << "destructor." << endl;
}


void SimulationGrid::
calculateE(float dxinv, float dyinv, float dzinv, float dt)
{
	//boost::mutex mutex;
    double t0, t1;
	double ta, tb;
	double t_accum = 0;
    //clock_t t0, t1;
	
	//if (mNumThreads == 1)
	{
		for (unsigned int n = 0; n < mMaterials.size(); n++)
		{
			MaterialPtr & mat = mMaterials[n];
			t0 = getTimeInMicroseconds();
			//t0 = clock();
			//sanityCheck();
			mat->calcEx(dxinv, dyinv, dzinv, dt);
			//sanityCheck();
			mat->calcEy(dxinv, dyinv, dzinv, dt);
			//sanityCheck();
			mat->calcEz(dxinv, dyinv, dzinv, dt);
			//sanityCheck();
			t1 = getTimeInMicroseconds();
			//t1 = clock();
			mMaterialMicroseconds[n] += (t1 - t0);
			t_accum += t1 - t0;
			//mMaterialSeconds[n] += ((double) t1 - t0)/CLOCKS_PER_SECOND;
		}
	}
	
	/*
	else if (mNumThreads == 3)
	{
		//LOG << "Calculating E fields (" << mMaterials.size() << " materials)\n";
		for (int n = 0; n < mMaterials.size(); n++)
		{
			boost::thread_group threads;
			MaterialPtr & mat = mMaterials[n];
			
			t0 = getTimeInMicroseconds();
			
			threads.create_thread(HelpCalcEx(mat, 0, 1, dxinv, dyinv, dzinv, dt));
			threads.create_thread(HelpCalcEy(mat, 0, 1, dxinv, dyinv, dzinv, dt));
			threads.create_thread(HelpCalcEz(mat, 0, 1, dxinv, dyinv, dzinv, dt));
						
			threads.join_all();
			
			t1 = getTimeInMicroseconds();
			mMaterialMicroseconds[n] += (t1 - t0);
		}
	}
	
	else
	{
		//LOG << "Calculating E fields (" << mMaterials.size() << " materials)\n";
		for (int n = 0; n < mMaterials.size(); n++)
		{
			boost::thread_group threads;
			MaterialPtr & mat = mMaterials[n];
			
			t0 = getTimeInMicroseconds();
			
			for (int nthread = 0; nthread < mNumThreads-1; nthread++)
				threads.create_thread(HelpCalcEAll(mat, nthread, mNumThreads,
					dxinv, dyinv, dzinv, dt));
			
			HelpCalcEAll hc(mat, mNumThreads-1, mNumThreads,
				dxinv, dyinv, dzinv, dt);
			hc();
			
			threads.join_all();
			
			t1 = getTimeInMicroseconds();
			mMaterialMicroseconds[n] += (t1 - t0);
		}
	}
	*/
}

void SimulationGrid::
sourceE(float dx, float dy, float dz, float dt, int timestep)
{
    double t0, t1;
	//clock_t t0, t1;
	
    for (int n = 0; n < mSources.size(); n++)
    {
        t0 = getTimeInMicroseconds();
		//t0 = clock();
        mSources[n]->doSourceE(timestep, dx, dy, dz, dt);
        t1 = getTimeInMicroseconds();
		//t1 = clock();
        mSourceMicroseconds[n] += (t1 - t0);
		//mSourceSeconds[n] += ((double)(t1 - t0))/CLOCKS_PER_SECOND;
    }
}

void SimulationGrid::
calculateH(float dxinv, float dyinv, float dzinv, float dt)
{
	double t0, t1;
	//clock_t t0, t1;
	
	//if (mNumThreads == 1)
	{
		for (unsigned int n = 0; n < mMaterials.size(); n++)
		{
			MaterialPtr & mat = mMaterials[n];
			t0 = getTimeInMicroseconds();
			//t0 = clock();
			//sanityCheck();
			mat->calcHx(dxinv, dyinv, dzinv, dt);
			//sanityCheck();
			mat->calcHy(dxinv, dyinv, dzinv, dt);
			//sanityCheck();
			mat->calcHz(dxinv, dyinv, dzinv, dt);
			//sanityCheck();
			//t1 = clock();
			t1 = getTimeInMicroseconds();
			mMaterialMicroseconds[n] += (t1 - t0);
			//mMaterialSeconds[n] += ((double)(t1 - t0))/CLOCKS_PER_SECOND;
		}
	}
	/*
	else if (mNumThreads == 3)
	{
		//LOG << "Calculating H fields (" << mMaterials.size() << " materials)\n";
		for (int n = 0; n < mMaterials.size(); n++)
		{
			boost::thread_group threads;
			MaterialPtr & mat = mMaterials[n];
			
			t0 = getTimeInMicroseconds();
			
			threads.create_thread(HelpCalcHx(mat, 0, 1, dxinv, dyinv, dzinv, dt));
			threads.create_thread(HelpCalcHy(mat, 0, 1, dxinv, dyinv, dzinv, dt));
			threads.create_thread(HelpCalcHz(mat, 0, 1, dxinv, dyinv, dzinv, dt));
						
			threads.join_all();
			
			t1 = getTimeInMicroseconds();
			mMaterialMicroseconds[n] += (t1 - t0);
		}
	}
	else 
	{
		//LOG << "Calculating H fields (" << mMaterials.size() << " materials)\n";
		for (int n = 0; n < mMaterials.size(); n++)
		{
			boost::thread_group threads;
			MaterialPtr & mat = mMaterials[n];
			
			t0 = getTimeInMicroseconds();
			
			for (int nthread = 0; nthread < mNumThreads-1; nthread++)
				threads.create_thread(HelpCalcHAll(mat, nthread, mNumThreads,
					dxinv, dyinv, dzinv, dt));
			
			HelpCalcHAll hc(mat, mNumThreads-1, mNumThreads,
				dxinv, dyinv, dzinv, dt);
			hc();
			
			threads.join_all();
			
			t1 = getTimeInMicroseconds();
			mMaterialMicroseconds[n] += (t1 - t0);
		}
	}
	*/
}

void SimulationGrid::
sourceH(float dx, float dy, float dz, float dt, int timestep)
{
    double t0, t1;
	//clock_t t0, t1;
	
    for (unsigned int n = 0; n < mSources.size(); n++)
    {
        t0 = getTimeInMicroseconds();
		//t0 = clock();
        mSources[n]->doSourceH(timestep, dx, dy, dz, dt);
        t1 = getTimeInMicroseconds();
		//t1 = clock();
		mSourceMicroseconds[n] += (t1 - t0);
		//mSourceSeconds[n] += ((double)(t1 - t0))/CLOCKS_PER_SECOND;
    }
}

void SimulationGrid::updateBuffersE()
{
	//clock_t t0, t1;
	double t0, t1;
	for (unsigned int n = 0; n < mTFSFBuffers.size(); n++)
	{
		//t0 = clock();
		t0 = getTimeInMicroseconds();
		mTFSFBuffers[n]->updateE();
		//t1 = clock();
		t1 = getTimeInMicroseconds();
		//mBufferSeconds[n] += ((double)(t1 - t0))/CLOCKS_PER_SECOND;
		mBufferMicroseconds[n] += (t1 - t0);
	}
}

void SimulationGrid::updateBuffersH()
{
	//clock_t t0, t1;
	double t0, t1;
	
	for (unsigned int n = 0; n < mTFSFBuffers.size(); n++)
	{
		//t0 = clock();
		t0 = getTimeInMicroseconds();
		mTFSFBuffers[n]->updateH();
		//t1 = clock(); 
		t1 = getTimeInMicroseconds();
		//mBufferSeconds[n] += ((double)(t1 - t0))/CLOCKS_PER_SECOND;
		mBufferMicroseconds[n] += (t1 - t0);
	}
}

void SimulationGrid::
inputE()
{
    double t0, t1;
    //clock_t t0, t1;
    for (unsigned int n = 0; n < mInputs.size(); n++)
    {
        t0 = getTimeInMicroseconds();
		//t0 = clock();
        mInputs[n]->readDataFileE();
		//t1 = clock();
        t1 = getTimeInMicroseconds();
        mInputMicroseconds[n] += (t1 - t0);
		//mInputSeconds[n] += ((double)(t1 - t0))/CLOCKS_PER_SECOND;
    }
}
void SimulationGrid::
inputH()
{
    double t0, t1;
    //clock_t t0, t1;
	
    for (unsigned int n = 0; n < mInputs.size(); n++)
    {
        t0 = getTimeInMicroseconds();
		//t0 = clock();
        mInputs[n]->readDataFileH();
        //t1 = clock();
		t1 = getTimeInMicroseconds();
        mInputMicroseconds[n] += (t1 - t0);
		//mInputSeconds[n] += ((double)(t1 - t0))/CLOCKS_PER_SECOND;
    }
}

void SimulationGrid::
output(float dx, float dy, float dz, float dt, int timestep)
{
    double t0, t1;
    //clock_t t0, t1;
	
    for (unsigned int n = 0; n < mOutputs.size(); n++)
    {
        t0 = getTimeInMicroseconds();
		//t0 = clock();
        mOutputs[n]->writeDataFile(timestep, dt);
        //t1 = clock();
		t1 = getTimeInMicroseconds();
        mOutputMicroseconds[n] += (t1 - t0);
		//mOutputSeconds[n] += ((double)(t1 - t0))/CLOCKS_PER_SECOND;
    }
}

void SimulationGrid::
sanityCheck()
{
    mFields->sanityCheck();
}

void SimulationGrid::
writeCoordinatesToFields()
{
	mFields->writeCoordinates();
}
void SimulationGrid::
assertCoordinatesInFields()
{
	mFields->assertCoordinates();
}

/*
#pragma mark *** HelpCalculate ***


SimulationGrid::HelpCalcBase::
HelpCalcBase(MaterialPtr & mat, int start, int stride,
		float dxinv, float dyinv, float dzinv, float dt) :
	mMat(mat),
	mStart(start),
	mStride(stride),
	m_dxinv(dxinv),
	m_dyinv(dyinv),
	m_dzinv(dzinv),
	m_dt(dt)
{
}


void SimulationGrid::HelpCalcEAll::
operator()()
{
	mMat->calcEx(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
	mMat->calcEy(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
	mMat->calcEz(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
}

void
SimulationGrid::HelpCalcHAll::
operator()()
{
	mMat->calcHx(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
	mMat->calcHy(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
	mMat->calcHz(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
}

void
SimulationGrid::HelpCalcEx::
operator()()
{
	mMat->calcEx(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
}

void
SimulationGrid::HelpCalcEy::
operator()()
{
	mMat->calcEy(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
}

void
SimulationGrid::HelpCalcEz::
operator()()
{
	mMat->calcEz(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
}

void
SimulationGrid::HelpCalcHx::
operator()()
{
	mMat->calcHx(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
}

void
SimulationGrid::HelpCalcHy::
operator()()
{
	mMat->calcHy(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
}

void
SimulationGrid::HelpCalcHz::
operator()()
{
	mMat->calcHz(m_dxinv, m_dyinv, m_dzinv, m_dt, mStart, mStride);
}
*/

