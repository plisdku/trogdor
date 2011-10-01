/*
 *  ObjFile.h
 *  trogdor4.4.1
 *
 *  Created by Paul Hansen on 3/2/08.
 *  Copyright 2008 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _OBJFILE_
#define _OBJFILE_

#include "geometry.h"
#include <vector>
#include "Map.h"
#include <string>
#include <iostream>
#include <list>

class ObjFile
{
public:
	ObjFile();
	
	void appendGroup(const std::string & name, 
		const std::list<OrientedRect3i> & faces);
	
	//void write(std::ostream & stream, float scaleFactor = 1.0) const;
    void write(std::ostream & stream, Vector3f addOrigin, Vector3f scaleFactors)
        const;
	
private:
	struct QuadFace
	{
		int vertices[4];
		int normal;
	};
	
	struct Group
	{
		std::vector<QuadFace> faces;
	};
	
	std::vector<Vector3i> mVertices;
	std::vector<Vector3i> mNormals;
	Map<std::string, Group> mGroups;
	
	// this is just to help structure the object internally
	
	Map<Vector3i,int> mVertexIndices;
	Map<Vector3i,int> mNormalIndices;
};



#endif
