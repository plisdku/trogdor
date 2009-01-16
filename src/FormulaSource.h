/*
 *  FormulaSource.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 7/3/08.
 *  Copyright 2008 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _FORMULASOURCE_
#define _FORMULASOURCE_

#include "Source.h"
#include "geometry.h"
#include "Map.h"
#include "SetupConstants.h"
#include "calc.hh"

class FormulaSource : public Source 
{
public:
	FormulaSource(Fields & inFields,
		const std::string & inFormula,
		const Vector3d & inPolarization,  
		Field inWhichField,
		const Rect3i & inRegion,
		const Map<std::string, std::string> & inParams);
	
	virtual ~FormulaSource();
private:
    
    virtual void doSourceE(int timestep, float dx, float dy, float dz,
        float dt);
        
    virtual void doSourceH(int timestep, float dx, float dy, float dz,
        float dt);
    
    void source(int timestep, float dx, float dy, float dz, float dt);
    
private:
    Rect3i mExtent;
    std::string mFormula;
	Vector3d mPolarization;
	calc_defs::Calculator<float> mCalculator;
	
    Fields & mFields;
    float* mFieldPtr[3];
	Field mWhichField;
};























#endif
