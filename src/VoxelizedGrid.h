/*
 *  VoxelizedGrid.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 *  $Rev:: 12                            $:  Revision of last commit
 *  $Author:: pch                        $:  Author of last commit
 *
 *  $Date: 2009-01-15 18:44:23 -0800 (Thu, 15 Jan 2009) $:
 *  $Id: MaterialType.h 12 2009-01-16 02:44:23Z pch $:
 *
 */

#ifndef _VOXELIZEDGRID_
#define _VOXELIZEDGRID_

#include "Pointer.h"
#include "geometry.h"
#include <vector>
#include <string>
#include "Map.h"

class GridDescription;
class VoxelizedGrid;
typedef Pointer<VoxelizedGrid> VoxelizedGridPtr;
class MaterialDescription;

class AssemblyDescription;
class Block;
class KeyImage;
class HeightMap;
class Ellipsoid;
class CopyFrom;

// Basic paint types: bulk materials, boundaries, PMLs.  These correspond to
// actually different update equations.
//
// Paint modifications: curl (neighbor) buffers, current buffers
// The curl buffer doesn't change the update equation
// The current buffer does change the update equation, I think, unless it's
// implemented by a four-direction curl buffer.  I suppose it could be, for
// transparent use by all material models.  I'd need a way to compose buffer
// operations then, or combine them in more complicated buffers.
//
// I'm leaning towards combining the effects of TFSF and currents into one
// TFSF buffer method... it would be transparent from a material implementation
// standpoint, which is nice, although it would also suffer from some pretty
// extensive extra buffer use.
//
// So, scratch that.  Now I'm leaning towards implementing the current source as
// a locally different update equation.  It should be easy enough to turn on
// by using templates, if needed; anyway the modification is simple enough.
//
// Perhaps marking it will be enough, for now.


enum PaintType
{
	kBulkPaintType,
	kBoundaryPaintType
};

class Paint
{
private:
	Paint(PaintType inType);
	
public:
	PaintType getType() { return mType; }
	
	static Paint bulkPaint( const MaterialDescription & material );
	static Paint boundaryPaint( int probablyLotsOfThings );
	
	static Paint pml( const Paint & baseMaterial, Vector3i direction );
	static Paint currentSource( const Paint & baseMaterial, 
		int currentSourceShouldHaveDescriptionTypeTooEventually );
	static Paint huygensSurface( const Paint & baseMaterial,
		int bufferSide, int bufferNumberOrSomeOtherID );
	
	friend bool operator<(const Paint & lhs, const Paint & rhs);
private:
	
	std::vector<int> mCurlBufferIndices; // any material can have this for free
	int mCurrentBufferIndex; // any material can have this with modified eqns
	Vector3i mPMLDirection; // if 0, this is not PML
	PaintType mType;
};
bool operator<(const Paint & lhs, const Paint & rhs);





class VoxelizedGrid
{
public:
	VoxelizedGrid(const GridDescription & gridDesc, 
		const Map<std::string, VoxelizedGridPtr> & voxelizedGrids, 
		Mat3i orientation);
private:
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<std::string, VoxelizedGridPtr> & voxelizedGrids,
		Mat3i orientation);
	void paintFromHuygensSurfaces(const GridDescription & gridDesc,
		Mat3i orientation);
	void paintFromCurrentSources(const GridDescription & gridDesc,
		Mat3i orientation);
	void paintPML();
	
	
	void paintBlock(const GridDescription & gridDesc,
		const Block & instruction);
	void paintKeyImage(const GridDescription & gridDesc,
		const KeyImage & instruction);
	void paintHeightMap(const GridDescription & gridDesc,
		const HeightMap & instruction);
	void paintEllipsoid(const GridDescription & gridDesc,
		const Ellipsoid & instruction);
	void paintCopyFrom(const GridDescription & gridDesc,
		const CopyFrom & instruction,
		const Map<std::string, VoxelizedGridPtr> & voxelizedGrids);
	
	
	std::vector<Paint> mIndexToPaint;
	Map<Paint, unsigned short> mPaintToIndex;
	
	std::vector<unsigned short> mMaterialHalfCells;
	std::vector<unsigned short> mMaterialIndexHalfCells;
	Rect3i mNonPMLRegion;
	Rect3i mCalcRegion;
	int m_nnx;
	int m_nny;
	int m_nnz;
	int m_nx;
	int m_ny;
	int m_nz;
	
	std::vector<Mat3i> mHuygensRegionSymmetries;
};




#endif
