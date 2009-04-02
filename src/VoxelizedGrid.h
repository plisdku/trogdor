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
typedef Pointer<GridDescription> GridDescPtr;
class VoxelizedGrid;
typedef Pointer<VoxelizedGrid> VoxelizedGridPtr;
class MaterialDescription;
typedef Pointer<MaterialDescription> MaterialDescPtr;
class HuygensSurfaceDescription;
typedef Pointer<HuygensSurfaceDescription> HuygensSurfaceDescPtr;
class NeighborBufferDescription;
typedef Pointer<NeighborBufferDescription> NeighborBufferDescPtr;

class AssemblyDescription;
class Block;
class KeyImage;
class HeightMap;
class Ellipsoid;
class CopyFrom;
class Extrude;

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

class Paint;
typedef Pointer<Paint> PaintPtr;

class Paint
{
public:
	Paint(PaintType inType);
	~Paint();
	
	friend std::ostream & operator<<(std::ostream & out, const Paint & p);
private:
	Paint(const MaterialDescPtr & material); // bulk constructor
	Paint(const Paint & parent, int sideNum, // curl buffer constructor
		NeighborBufferDescPtr & curlBuffer);
	Paint(const Paint & parent, Vector3i pmlDir); // pml constructor
	Paint(const Paint & parent, int donothing); // parent paint constructor (no modification)
public:
	static Paint* getPaint(const MaterialDescPtr & material);
	static Paint* getCurlBufferedPaint(Paint* basePaint, int sideNum,
		NeighborBufferDescPtr & curlBuffer);
	static Paint* getPMLPaint(Paint* basePaint, Vector3i pmlDir);
	static Paint* getParentPaint(Paint* basePaint);
	
	static const Map<Paint, PaintPtr> & getPalette() {
		return mPalette; }
	
	static void clearPalette() { mPalette.clear(); }
	
	PaintType getType() const { return mType; }
private:
	
	PaintType mType;
	Vector3i mPMLDirections;
	std::vector<NeighborBufferDescPtr> mCurlBuffers;
	int mCurrentBufferIndex;
	
	// for BulkPaint
	MaterialDescPtr mBulkMaterial;
	
	// for BoundaryPaint
	// . . . nothing here yet
	
	static Map<Paint, PaintPtr> mPalette;
	
	friend bool operator<(const Paint & lhs, const Paint & rhs);
};
bool operator<(const Paint & lhs, const Paint & rhs);
std::ostream & operator<<(std::ostream & out, const Paint & p);



class VoxelizedGrid
{
public:
	VoxelizedGrid(const GridDescription & gridDesc, 
		const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids);
	
	const std::vector<Vector3i> & getHuygensRegionSymmetries() const {
		return mHuygensRegionSymmetries; }
	
	friend std::ostream & operator<< (std::ostream & out,
		const VoxelizedGrid & grid);
private:
	void paintFromAssembly(const GridDescription & gridDesc,
		const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids);
	void paintFromHuygensSurfaces(const GridDescription & gridDesc);
	void paintFromCurrentSources(const GridDescription & gridDesc);
	void paintPML();
	
	void calculateMaterialIndices();
	void calcMatIndHelp(Rect3i inRegion, std::vector<long> & indexVector);
	void printMatInd(Rect3i inRegion, const std::vector<long> & indexVector);
	
	void calculateHuygensSymmetries(const GridDescription & gridDesc);
	Vector3i huygensSymmetry(const HuygensSurfaceDescription & surf);
	
	
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
		const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids);
	void paintExtrude(const GridDescription & gridDesc,
		const Extrude & instruction);
	
	void paintHuygensSurface(const HuygensSurfaceDescription & surf);
	
	Paint* & operator() (int ii, int jj, int kk);
	Paint* operator() (int ii, int jj, int kk) const;
	Paint* & operator() (const Vector3i & pp);
	Paint* operator() (const Vector3i & pp) const;
	
	void paintPEC(Paint* paint, int iYee, int jYee, int kYee);
	void paintPMC(Paint* paint, int iYee, int jYee, int kYee);
	
	std::vector<Paint*> mMaterialHalfCells;
	std::vector<long> mMaterialIndexHalfCells;
	
	// There are also PML material indices for each of the six faces of the
	// grid.  These are the size of the faces of mMaterialIndexHalfCells.
	std::vector< std::vector<long> > mPMLFaceIndices;
	std::vector<Rect3i> mPMLFaces;
	
	Rect3i mNonPMLRegion;
	Rect3i mCalcRegion;
	Vector3i mOriginYee;
	int m_nnx;
	int m_nny;
	int m_nnz;
	int m_nx;
	int m_ny;
	int m_nz;
	
	std::vector<Vector3i> mHuygensRegionSymmetries;
};

std::ostream & operator<< (std::ostream & out, const VoxelizedGrid & grid);



#endif
