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
 *  Unified source of E, H, J and K field data for hard sources, soft sources,
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
     *  is read from a binary file, it will be streamed and not buffered.
     */
    StreamedFieldInput(SourceDescPtr sourceDescription, float dt);
    
    virtual ~StreamedFieldInput();
    
    // Streaming functions
    void startHalfTimestep(int timestep);
    void stepToNextField();
    void stepToNextFieldDirection(int direction);
    
    float getField(int direction);
    float getMask(int direction);
    void stepToNextValue();
    

private:
    std::ifstream mFile;
    std::string mFormula;
	calc_defs::Calculator<float> mCalculator;
    
    std::vector<float> mDataMaskJ[3];
    std::vector<float> mDataMaskK[3];
    
    bool mIsSpaceVarying;
    Vector3f mPolarizationFactor;
    
    static const int FILETYPE = 0;
    static const int FORMULATYPE = 1;
    int mType;
    
    float m_dt;
    float mCurrentValue;
};




#endif
