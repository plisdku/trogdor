/*
 *  StreamedFieldInput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 8/21/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifndef _STREAMEDFIELDINPUT_
#define _STREAMEDFIELDINPUT_

#include "SimulationDescription.h"

#include <string>
#include <fstream>
#include <vector>
#include <calc.hh>
#include "MemoryUtilities.h"

/**
 *  Unified source of E and H field data for hard sources, soft sources,
 *  and custom TFSF sources (as for example
 *  for loading waveguide modes, doing analytical field propagation, whatnot).
 *  Fields may be time or time-space varying, and fields with separable time
 *  and space dependencies can be specified in two parts to use less memory.
 */
class StreamedFieldInput
{
public:
    /**
     *  Initialize a field input for a hard or soft E/H source.  If the data
     *  is read from a binary file, it will be streamed and not buffered; if
     *  a mask is requested it will be stored in memory.
     */
    StreamedFieldInput(SourceDescPtr sourceDescription);
    
    virtual ~StreamedFieldInput();
    
    /**
     *  Provide an opportunity for generation of E or H fields for this
     *  timestep.  Sources using a formula will evaluate it now; sources using
     *  time-varying input will read a floating point value now.  Sources that
     *  have to load new data for each cell on each timestep will do nothing.
     *
     *  @param timestep     current simulation timestep (0 to timesteps-1)
     *  @param time         the actual time in seconds (should be n*dt for E,
     *                      and (n+1/2)*dt for H)
     */
    void startHalfTimestep(int timestep, float time);
    
    /**
     *  For sources that store a mask in a buffer, reset the pointer to the
     *  current mask position to zero.
     *
     *  @param direction    field direction (0, 1 or 2) (unused)
     */
    void restartMaskPointer(int direction);
    
    /**
     *  Stream in the next E field value for this source.  Sources that load
     *  a mask or space-time-varying fields will stream them in the order they
     *  were written to file.  Successive calls will read values for successive
     *  cells in the grid.
     *
     *  @param direction    field direction (0, 1 or 2); will be used to select
     *                      the right mask buffer or polarization factor if
     *                      needed.
     *  @returns            an electric field value
     */
    float getFieldE(int direction);
    
    /**
     *  Stream in the next H field value for this source.  Sources that load
     *  a mask or space-time-varying fields will stream them in the order they
     *  were written to file.  Successive calls will read values for successive
     *  cells in the grid.
     *
     *  @param direction    field direction (0, 1 or 2); will be used to select
     *                      the right mask buffer or polarization factor if
     *                      needed.
     *  @returns            a magnetic field value
     */
    float getFieldH(int direction);
    
private:
    /**
     *  If the SourceDescription indicates that the source has a mask (a space-
     *  varying prefactor), then allocate the mask buffers here and load them.
     *
     *  @param source       source description (to obtain spatial extent)
     */
    void loadMask(SourceDescPtr source);
    
    std::ifstream mFile;
    std::string mFormula;
	calc_defs::Calculator<float> mCalculator;
    
    std::vector<float> mDataMaskE[3];
    std::vector<float> mDataMaskH[3];
    
    enum FieldValueType
    {
        kSpaceTimeVaryingField,
        kTimeVaryingField
    };
    FieldValueType mFieldValueType;
    bool mHasMask;
    
    Vector3f mPolarizationFactor;
    
    static const int FILETYPE = 0;
    static const int FORMULATYPE = 1;
    int mType;
    
    float mCurrentValue;
    long mMaskIndex;
};




#endif
