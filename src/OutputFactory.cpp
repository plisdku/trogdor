/*
 *  OutputFactory.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/2/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "OutputFactory.h"
#include "OneFieldOutput.h"
#include "ThreeFieldOutput.h"
#include "ColocatedOutput.h"

using namespace std;


Output* OutputFactory::
createOutput(const SetupGrid & inGrid, Fields & inFields,
    const SetupOutput & inOutput)
{
    Output* output = NULL;
    const string & nom = inOutput.getClass();
    const string & file = inOutput.getFilePrefix();
    const Map<string, string> & params = inOutput.getParameters();
    
    if (nom == "OneFieldOutput")
        output = new OneFieldOutput(inFields, params, file,
            inOutput.getPeriod());
    else if (nom == "ThreeFieldOutput")
        output = new ThreeFieldOutput(inFields, params, file,
            inOutput.getPeriod());
    else if (nom == "ColocatedOutput")
        output = new ColocatedOutput(inFields, params, file,
            inOutput.getPeriod());
    else
    {
        LOG << "Unknown output type: " << nom << "\n";
        cerr << "The output type " << nom << " is not supported.\n"
            "Please check the parameter file.\n";
        exit(1);
    }
    
    return output;
}



OutputFactory::
OutputFactory()
{
    LOG << "How did you do this?\n";
}


