/*
 *  FileSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/11/08.
 *  Copyright 2008 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#ifndef _FILESOURCE_
#define _FILESOURCE_

#include "Source.h"
#include "geometry.h"
#include "Map.h"
#include "SetupConstants.h"

#include <fstream>

class FileSource : public Source
{
public:
	FileSource(Fields & inFields,
		const std::string & inFilename,
		const Vector3d & inPolarization,
		Field inWhichField,
		const Rect3i & inRegion);
	
	virtual ~FileSource();

private:
    virtual void doSourceE(int timestep, float dx, float dy, float dz,
        float dt);
        
    virtual void doSourceH(int timestep, float dx, float dy, float dz,
        float dt);
    
    void source(int timestep, float dx, float dy, float dz, float dt);
	
private:
    Rect3i mExtent;
	std::ifstream mFile;
	
	Vector3d mPolarization;
	
    Fields & mFields;
    float* mFieldPtr[3];
	Field mWhichField;
};



#endif
