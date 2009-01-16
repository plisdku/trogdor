/*
 *  ColocatedOutput.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/18/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _COLOCATEDOUTPUT_
#define _COLOCATEDOUTPUT_

#include "Output.h"
#include "geometry.h"

class ColocatedOutput : public Output
{
public:
    ColocatedOutput(Fields & inFields,
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
    enum fieldType { kElectric, kMagnetic };
    fieldType mFieldType;
    const float* mField[3];
    
    Rect3i mYeeRegion;
    int mPeriod;
};



#endif
