/*
 *  SourceFactory.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/14/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#include "SourceFactory.h"


#include "FileSource.h"
#include "FormulaSource.h"

using namespace std;


Source* SourceFactory::
createSource(const SetupGrid & inGrid, Fields & inFields,
    const SetupSource & inSource)
{
    //Source* source = NULL;
	
	Source* source;
    //const string & nom = inSource.getClass();
    //const Map<string, string> & params = inSource.getParameters();
    if (inSource.getFormula() != "")
	{
		source = new FormulaSource(inFields, inSource.getFormula(),
			inSource.getPolarization(), inSource.getField(),
			inSource.getRegion(), inSource.getParameters());
	}
	else if (inSource.getFile() != "")
	{
		source = new FileSource(inFields, inSource.getFile(),
			inSource.getPolarization(), inSource.getField(),
			inSource.getRegion());
	}
	
    return source;
}


SourceFactory::
SourceFactory()
{
    LOG << "How did you do this?\n";
}
