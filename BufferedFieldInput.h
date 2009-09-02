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

class BufferedFieldInput
{
public:
    /**
     *  Initialize a field input for an electric or magnetic current source.
     *  If the data is read from a binary file, it will be buffered and not
     *  streamed; this will permit the current source update equations to run
     *  more efficiently and will also simplify multithreading, although it will
     *  increase the memory requirements of the sim.
     */
    BufferedFieldInput(CurrentSourceDescPtr currentSourceDescription, float dt);
    
    virtual ~BufferedFieldInput();

private:
    std::ifstream mFile;
    std::string mFormula;
	calc_defs::Calculator<float> mCalculator;
    
    std::vector<float> mDataJ[3];
    std::vector<float> mDataK[3];
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
