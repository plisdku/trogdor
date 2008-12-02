/*
 *  OneFieldOutput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/2/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _ONEFIELDOUTPUT_
#define _ONEFIELDOUTPUT_


#include "Output.h"
#include "geometry.h"


class OneFieldOutput : public Output
{
public:
    OneFieldOutput(Fields & inFields,
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
    const float* mField;
    Rect3i mYeeRegion;
    int mPeriod;
    
};







#endif
