/*
 *  SimulationGrid.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/14/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#ifndef _SIMULATIONGRID_
#define _SIMULATIONGRID_

#include "Pointer.h"
#include "Runline.h"

#include "SetupOutput.h"
#include "SetupInput.h"
#include "SetupMaterialModel.h"
#include "SetupSource.h"
#include "RunlineType.h"

#include <string>
#include <vector>

#include <boost/thread/mutex.hpp>

class MaterialModel;
typedef Pointer<MaterialModel> MaterialPtr;
class Output;
typedef Pointer<Output> OutputPtr;
class Input;
typedef Pointer<Input> InputPtr;
class Source;
typedef Pointer<Source> SourcePtr;
class TFSFBufferSet;
typedef Pointer<TFSFBufferSet> TFSFBufferSetPtr;
class Fields;
typedef Pointer<Fields> FieldsPtr;

class SimulationGrid
{
public:
    SimulationGrid(FieldsPtr inFields, std::vector<SourcePtr>& sources,
        std::vector<MaterialPtr>& materials,
        std::vector<OutputPtr>& outputs,
        std::vector<InputPtr>& inputs,
		std::vector<TFSFBufferSetPtr>& buffers,
		int numThreads);
    virtual ~SimulationGrid();
    
    void calculateE(float dx, float dy, float dz, float dt);
    void sourceE(float dx, float dy, float dz, float dt, int timestep);
    
    void calculateH(float dx, float dy, float dz, float dt);
    void sourceH(float dx, float dy, float dz, float dt, int timestep);
    
	void updateBuffersE();
	void updateBuffersH();
	
    void inputE();
    void inputH();
    void output(float dx, float dy, float dz, float dt, int timestep);
    
    void sanityCheck();
	void writeCoordinatesToFields();
	void assertCoordinatesInFields();
	
	/*
	const std::vector<double> & getMaterialSeconds() const
		{ return mMaterialSeconds; }
	const std::vector<double> & getOutputSeconds() const
		{ return mOutputSeconds; }
	const std::vector<double> & getInputSeconds() const
		{ return mInputSeconds; }
	const std::vector<double> & getSourceSeconds() const
		{ return mSourceSeconds; }
	const std::vector<double> & getBufferSeconds() const
		{ return mBufferSeconds; }
	*/
	
	
    const std::vector<double> & getMaterialMicroseconds() const
		{ return mMaterialMicroseconds; }
    const std::vector<double> & getOutputMicroseconds() const
		{ return mOutputMicroseconds; }
    const std::vector<double> & getInputMicroseconds() const
		{ return mInputMicroseconds; }
    const std::vector<double> & getSourceMicroseconds() const
		{ return mSourceMicroseconds; }
    const std::vector<double> & getBufferMicroseconds() const
		{ return mBufferMicroseconds; }
    
	const std::vector<MaterialPtr> & getMaterials() const
		{ return mMaterials; }
	const std::vector<OutputPtr> & getOutputs() const
		{ return mOutputs; }
	const std::vector<InputPtr> & getInputs() const
		{ return mInputs; }
	const std::vector<SourcePtr> & getSources() const
		{ return mSources; }
	
private:
	/*
	class HelpCalcBase
	{
	public:
		HelpCalcBase(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt);
		void operator() () {}
	
	protected:
		MaterialPtr & mMat;
		int mStart;
		int mStride;
		float m_dxinv;
		float m_dyinv;
		float m_dzinv;
		float m_dt;
	};
	
	class HelpCalcEAll : public HelpCalcBase
	{
	public:
		HelpCalcEAll(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt) :
			HelpCalcBase(mat, start, stride, dxinv, dyinv, dzinv, dt)
		{}
		void operator() ();
	};
	
	class HelpCalcHAll : public HelpCalcBase
	{
	public:
		HelpCalcHAll(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt) :
			HelpCalcBase(mat, start, stride, dxinv, dyinv, dzinv, dt)
		{}
		void operator() ();
	};
	
	class HelpCalcEx : public HelpCalcBase
	{
	public:
		HelpCalcEx(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt) :
			HelpCalcBase(mat, start, stride, dxinv, dyinv, dzinv, dt)
		{}
		void operator() ();
	};
	
	class HelpCalcEy : public HelpCalcBase
	{
	public:
		HelpCalcEy(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt) :
			HelpCalcBase(mat, start, stride, dxinv, dyinv, dzinv, dt)
		{}
		void operator() ();
	};
	
	class HelpCalcEz : public HelpCalcBase
	{
	public:
		HelpCalcEz(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt) :
			HelpCalcBase(mat, start, stride, dxinv, dyinv, dzinv, dt)
		{}
		void operator() ();
	};
	
	class HelpCalcHx : public HelpCalcBase
	{
	public:
		HelpCalcHx(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt) :
			HelpCalcBase(mat, start, stride, dxinv, dyinv, dzinv, dt)
		{}
		void operator() ();
	};
	
	class HelpCalcHy : public HelpCalcBase
	{
	public:
		HelpCalcHy(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt) :
			HelpCalcBase(mat, start, stride, dxinv, dyinv, dzinv, dt)
		{}
		void operator() ();
	};
	
	class HelpCalcHz : public HelpCalcBase
	{
	public:
		HelpCalcHz(MaterialPtr & mat, int start, int stride,
			float dxinv, float dyinv, float dzinv, float dt) :
			HelpCalcBase(mat, start, stride, dxinv, dyinv, dzinv, dt)
		{}
		void operator() ();
	};
	*/
    
    FieldsPtr mFields;
    std::vector<SourcePtr> mSources;
    std::vector<OutputPtr> mOutputs; // including output runlines
    std::vector<InputPtr> mInputs; // including output runlines
    std::vector<MaterialPtr> mMaterials; // including material runlines
	std::vector<TFSFBufferSetPtr> mTFSFBuffers;
    
	//std::vector<double> mMaterialSeconds;
	//std::vector<double> mOutputSeconds;
	//std::vector<double> mInputSeconds;
	//std::vector<double> mSourceSeconds;
	
    std::vector<double> mMaterialMicroseconds;
    std::vector<double> mOutputMicroseconds;
    std::vector<double> mInputMicroseconds;
    std::vector<double> mSourceMicroseconds;
	std::vector<double> mBufferMicroseconds;
	
	int mNumThreads;
};

typedef Pointer<SimulationGrid> SimulationGridPtr;




#endif
