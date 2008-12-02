/*
 *  ThreeFieldOutput.h
 *  v4_trogdor
 *
 *  Created by Paul Hansen on 1/30/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _THREEFIELDOUTPUT_
#define _THREEFIELDOUTPUT_


#include "Output.h"
#include "geometry.h"


class ThreeFieldOutput : public Output
{
public:
    ThreeFieldOutput(Fields & inFields,
        const Map<std::string, std::string> & inParams,
        const std::string & inFilePrefix, int period);
    
protected:
    virtual void writeSpecsData(std::ofstream & str);
    virtual void writeSpecsFooter(std::ofstream & str);
    virtual void writeData(std::ofstream & str, int timestep, float data);
    
    virtual int get_nx() const;
    virtual int get_ny() const;
    virtual int get_nz() const;
    
private:
    const float* mField[3];
    
    Rect3i mYeeRegion;
    int mPeriod;
    
private:
};


#endif
