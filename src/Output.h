/*
 *  Output.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/20/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#ifndef _OUTPUT_
#define _OUTPUT_

#include "Fields.h"
#include "Pointer.h"

#include <string>
#include <fstream>

class Output
{
public:
    Output(Fields & inFields, const std::string & inFilePrefix);
    virtual ~Output();
    
    void setPrefix(const std::string & inFilePrefix);
    void writeSpecsFile();
    void writeDataFile(int timestep, float dt);
    virtual void onEndSimulation();
    
    //  This should only be available for AuxOutput.
    //std::vector<int> getRunlinesFor(const std::string & tag); // e.g. "Jx"

protected:
    virtual void writeSpecsData(std::ofstream & str);
    virtual void writeSpecsFooter(std::ofstream & str);
    virtual void writeData(std::ofstream & str, int timestep,
        float dt);
    virtual int get_nx() const;
    virtual int get_ny() const;
    virtual int get_nz() const;
    
    Fields & getFields() { return mFields; }
    
    std::ofstream mFile;
private:
    Fields & mFields;
    std::string mFileName;
    std::string mSpecsFileName;
};

typedef Pointer<Output> OutputPtr;

#endif

