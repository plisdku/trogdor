/*
 *  InputFactory.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/26/07.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "InputFactory.h"

#include "SetupInput.h"

#include "OneFieldInput.h"
#include "ThreeFieldInput.h"

using namespace std;


Input* InputFactory::
createInput(const SetupGrid & inGrid, Fields & inFields,
    const SetupInput & inInput)
{
    Input* input = NULL;
    const string & nom = inInput.getClass();
    const string & file = inInput.getFilePrefix();
    const Map<string, string> & params = inInput.getParameters();
    
    if (nom == "OneFieldInput")
        input = new OneFieldInput(inFields, params, file);
    else if (nom == "ThreeFieldInput")
        input = new ThreeFieldInput(inFields, params, file);
    else
    {
        LOG << "Unknown input type: " << nom << "\n";
        cerr << "The input type " << nom << " is not supported.\n"
            "Please check the parameter file.\n";
        exit(1);
    }
    
    return input;
}


InputFactory::
InputFactory()
{
    LOG << "How did you do this?\n";
}
