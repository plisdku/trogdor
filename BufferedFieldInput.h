/*
 *  BufferedFieldInput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 9/2/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _BUFFEREDFIELDINPUT_
#define _BUFFEREDFIELDINPUT_

#include "SimulationDescription.h"

#include <string>
#include <fstream>
#include <vector>
#include <calc.hh>
#include "MemoryUtilities.h"

class BufferedFieldInput
{
public:
    /**
     *  Initialize a field input for an electric or magnetic current source.
     *  If the data is read from a binary file, it will be buffered and not
     *  streamed; this will permit the current source update equations to run
     *  more efficiently and will also simplify multithreading, although it will
     *  increase the memory requirements of the sim.
     *
     *  The main difference between this constructor and a constructor for
     *  a per-material source is that this one calculates the buffer size
     *  directly from the currentSourceDescription, and each J and K component
     *  has the same buffer size if any.
     */
    BufferedFieldInput(CurrentSourceDescPtr currentSourceDescription);
    
    /**
     *  Allocate field buffers.  This function is provided to help postpone
     *  allocation of big chunks of memory until the setup objects are all
     *  cleared out.
     */
    void allocate();
    
    BufferPointer pointerE(int fieldDirection, long offset) const;
    BufferPointer pointerH(int fieldDirection, long offset) const;
    BufferPointer pointerMaskE(int fieldDirection, long offset) const;
    BufferPointer pointerMaskH(int fieldDirection, long offset) const;
    
    /**
     *  Provide an opportunity for generation of fields for this
     *  timestep.  Sources using a formula will evaluate it now; sources using
     *  time-varying input will read a floating point value now.  Sources that
     *  have to load new data for each cell on each timestep will do nothing.
     *
     *  @param timestep     current simulation timestep (0 to timesteps-1)
     *  @param time         the actual time in seconds (should be n*dt for E,
     *                      and (n+1/2)*dt for H)
     */
    void startHalfTimestepE(long timestep, float time);
    void startHalfTimestepH(long timestep, float time);
    
    /**
     *  Set the contents of the field buffers to zero.  This is useful when an
     *  input file only provides fields for a certain number of timesteps.
     */
    void zeroBuffersE();
    void zeroBuffersH();

private:
    /**
     *  If the CurrentSourceDescription indicates that the source has a mask
     *  (a space-varying prefactor), then allocate the mask buffers here and
     *  load them.
     *
     *  @param source       source description (to obtain spatial extent)
     */
    void loadMask(CurrentSourceDescPtr source);
    
    std::ifstream mFile;
    std::string mFormula;
	calc_defs::Calculator<float> mCalculator;
    
    MemoryBuffer mBufferE[3];
    MemoryBuffer mBufferH[3];
    MemoryBuffer mMaskBufferE[3];
    MemoryBuffer mMaskBufferH[3];
    
    std::vector<float> mDataE[3];
    std::vector<float> mDataH[3];
    std::vector<float> mDataMaskE[3];
    std::vector<float> mDataMaskH[3];
    
    enum FieldValueType
    {
        kSpaceTimeVaryingField,
        kTimeVaryingField
    };
    FieldValueType mFieldValueType;
    bool mHasMask;
    
    //Vector3f mPolarizationFactor;
    
    static const int FILETYPE = 0;
    static const int FORMULATYPE = 1;
    int mType;
    
    float mCurrentValue;
};



#endif
