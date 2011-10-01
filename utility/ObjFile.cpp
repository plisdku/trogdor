/*
 *  ObjFile.cpp
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

#include "ObjFile.h"

using namespace std;

ObjFile::ObjFile() :
	mVertices(),
	mNormals(),
	mGroups(),
	mVertexIndices(),
	mNormalIndices()
{
}

void
ObjFile::appendGroup(const string & name, const list<OrientedRect3i> & faces)
{
	int mm, nn;
	Group & g = mGroups[name];
	vector<Vector3i> rawVerts[4];
	vector<Vector3i> rawNormals;
	list<OrientedRect3i>::const_iterator itr;
	// First pass: extract normals and vertices.
	
//	for (nn = 0; nn < faces.size(); nn++)
	for (itr = faces.begin(); itr != faces.end(); itr++)
	{
		const OrientedRect3i & f = *itr;
		rawVerts[0].push_back(f.rect.p1);
		rawVerts[2].push_back(f.rect.p2);
		
		if (f.normal[0] == 1)
		{
			rawVerts[1].push_back(Vector3i(f.rect.p1[0],
				f.rect.p2[1], f.rect.p1[2]));
			rawVerts[3].push_back(Vector3i(f.rect.p1[0],
				f.rect.p1[1], f.rect.p2[2]));
		}
		else if (f.normal[0] == -1)
		{
			rawVerts[1].push_back(Vector3i(f.rect.p1[0],
				f.rect.p1[1], f.rect.p2[2]));
			rawVerts[3].push_back(Vector3i(f.rect.p1[0],
				f.rect.p2[1], f.rect.p1[2]));
		}
		else if (f.normal[1] == 1)
		{
			rawVerts[1].push_back(Vector3i(f.rect.p1[0],
				f.rect.p1[1], f.rect.p2[2]));
			rawVerts[3].push_back(Vector3i(f.rect.p2[0],
				f.rect.p1[1], f.rect.p1[2]));
		}
		else if (f.normal[1] == -1) // y-normal
		{
			rawVerts[1].push_back(Vector3i(f.rect.p2[0],
				f.rect.p1[1], f.rect.p1[2]));
			rawVerts[3].push_back(Vector3i(f.rect.p1[0],
				f.rect.p1[1], f.rect.p2[2]));
		}
		else if (f.normal[2] == 1)
		{
			rawVerts[1].push_back(Vector3i(f.rect.p2[0],
				f.rect.p1[1], f.rect.p1[2]));
			rawVerts[3].push_back(Vector3i(f.rect.p1[0],
				f.rect.p2[1], f.rect.p1[2]));
		}
		else if (f.normal[2] == -1)
		{
			rawVerts[1].push_back(Vector3i(f.rect.p1[0],
				f.rect.p2[1], f.rect.p1[2]));
			rawVerts[3].push_back(Vector3i(f.rect.p2[0],
				f.rect.p1[1], f.rect.p1[2]));
		}
		rawNormals.push_back(f.normal);
	}
	
	//	2.  Assign to each vertex and normal a linear index starting at 1
	//	(obj files have arrays that start at 1, not 0!)
	for (nn = 0; nn < faces.size(); nn++)
	{
		QuadFace qface;
		for (mm = 0; mm < 4; mm++)
		{
			// this inserts the new vertex into the map if not already
			// present.
			int & vIndex = mVertexIndices[rawVerts[mm][nn]];
			if (vIndex == 0)
			{
				vIndex = mVertexIndices.size();  // new vert gets 1-based index
				mVertices.push_back(rawVerts[mm][nn]);
			}
			qface.vertices[mm] = vIndex;
		}
		
		int & vnIndex = mNormalIndices[rawNormals[nn]];
		if (vnIndex == 0)
		{
			vnIndex = mNormalIndices.size();
			mNormals.push_back(rawNormals[nn]);
		}
		qface.normal = vnIndex;
		
		g.faces.push_back(qface);
	}
}


//void
//ObjFile::write(ostream & stream, float scaleFactor) const
//{
//	int mm,nn;
//	
//	for (nn = 0; nn < mVertices.size(); nn++)
//	{
//		stream << "v " << scaleFactor*mVertices[nn][0] << " "
//			<< scaleFactor*mVertices[nn][1] << " "
//			<< scaleFactor*mVertices[nn][2] << "\n";
//	}
//	
//	for (nn = 0; nn < mNormals.size(); nn++)
//	{
//		stream << "vn " << mNormals[nn][0] << " "
//			<< mNormals[nn][1] << " "
//			<< mNormals[nn][2] << "\n";
//	}
//	
//	//for (nn = 0; nn < mGroups.size(); nn++)
//	for (map<string, Group>::const_iterator itr = mGroups.begin();
//		itr != mGroups.end(); itr++)
//	{
//		const Group & g = itr->second;
//		string groupName = itr->first;
//		string matName = groupName;
//		
//		for (int jj = 0; jj < matName.length(); jj++)
//			if (matName[jj] == ' ')
//				matName[jj] = '_';
//		
//		stream << "g " << itr->first << "\n";
//		stream << "usemtl " << matName << "\n";
//		
//		for (mm = 0; mm < g.faces.size(); mm++)
//		{
//			const QuadFace & face = g.faces[mm];
//			stream << "f " << face.vertices[0] << "//" << face.normal << " "
//				<< face.vertices[1] << "//" << face.normal << " "
//				<< face.vertices[2] << "//" << face.normal << " "
//				<< face.vertices[3] << "//" << face.normal << "\n";
//		}
//	}
//}

void
ObjFile::write(ostream & stream, Vector3f addOrigin, Vector3f scaleFactor) const
{
	int mm,nn;
	
	for (nn = 0; nn < mVertices.size(); nn++)
	{
		stream << "v " << addOrigin[0] + scaleFactor[0]*mVertices[nn][0] << " "
			<< addOrigin[1] + scaleFactor[0]*mVertices[nn][1] << " "
			<< addOrigin[2] + scaleFactor[0]*mVertices[nn][2] << "\n";
	}
	
	for (nn = 0; nn < mNormals.size(); nn++)
	{
		stream << "vn " << mNormals[nn][0] << " "
			<< mNormals[nn][1] << " "
			<< mNormals[nn][2] << "\n";
	}
	
	//for (nn = 0; nn < mGroups.size(); nn++)
	for (map<string, Group>::const_iterator itr = mGroups.begin();
		itr != mGroups.end(); itr++)
	{
		const Group & g = itr->second;
		string groupName = itr->first;
		string matName = groupName;
		
		for (int jj = 0; jj < matName.length(); jj++)
			if (matName[jj] == ' ')
				matName[jj] = '_';
		
		stream << "g " << itr->first << "\n";
		stream << "usemtl " << matName << "\n";
		
		for (mm = 0; mm < g.faces.size(); mm++)
		{
			const QuadFace & face = g.faces[mm];
			stream << "f " << face.vertices[0] << "//" << face.normal << " "
				<< face.vertices[1] << "//" << face.normal << " "
				<< face.vertices[2] << "//" << face.normal << " "
				<< face.vertices[3] << "//" << face.normal << "\n";
		}
	}
}


