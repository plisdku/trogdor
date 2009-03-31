/*
 *  VoxelizedGrid.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "VoxelizedGrid.h"

#include "SimulationDescription.h"
#include "Log.h"
#include "YeeUtilities.h"

#include <Magick++.h>
static Vector3i sConvertColor(const Magick::Color & inColor);

static Mat3i sMatWithCols(Vector3i v1, Vector3i v2, Vector3i v3);

using namespace std;
namespace YU = YeeUtilities;

Map<Paint, PaintPtr> Paint::mPalette;

#pragma mark *** Paint ***

Paint::
Paint(PaintType inType) :
	mType(inType),
	mPMLDirections(0,0,0),
	mCurlBuffers(6),
	mCurrentBufferIndex(-1),
	mBulkMaterial(0L)
{
	assert(!"This is evil.");
}

Paint::
~Paint()
{
	//LOG << "Killing me: \n"; // << hex << this << dec << endl;
}

Paint::
Paint(const MaterialDescPtr & material) :
	mType(kBulkPaintType),
	mPMLDirections(0,0,0),
	mCurlBuffers(6),
	mCurrentBufferIndex(-1),
	mBulkMaterial(material)
{
	assert(material != 0L);
}

Paint::
Paint(const Paint & parent, int sideNum, NeighborBufferDescPtr & curlBuffer) :
	mType(parent.mType),
	mPMLDirections(parent.mPMLDirections),
	mCurlBuffers(parent.mCurlBuffers),
	mCurrentBufferIndex(parent.mCurrentBufferIndex),
	mBulkMaterial(parent.mBulkMaterial)
{
	//LOG << mCurlBuffers[sideNum] << "\n";
	assert(mCurlBuffers[sideNum] == 0L);
	assert(mBulkMaterial != 0L);
	mCurlBuffers[sideNum] = curlBuffer;
	//LOG << "side num " << sideNum << " buffer " << hex
	//	<< (void*)mCurlBuffers[sideNum] << dec << "\n";
}

Paint::
Paint(const Paint & parent, Vector3i pmlDirection) :
	mType(parent.mType),
	mPMLDirections(pmlDirection),
	mCurlBuffers(6),
	mCurrentBufferIndex(-1),
	mBulkMaterial(parent.mBulkMaterial)
{
	assert(parent.mPMLDirections == Vector3i(0,0,0));
	assert(mBulkMaterial != 0L);
}


Paint::
Paint(const Paint & parent, int donothing) :
	mType(parent.mType),
	mPMLDirections(0,0,0),
	mCurlBuffers(6),
	mCurrentBufferIndex(-1),
	mBulkMaterial(parent.mBulkMaterial)
{
	assert(mBulkMaterial != 0L);
}



Paint* Paint::
getPaint(const MaterialDescPtr & material)
{
	PaintPtr bulkPaint(new Paint(material));
	if (mPalette.count(*bulkPaint) != 0)
		return mPalette[*bulkPaint];
	mPalette[*bulkPaint] = bulkPaint;
	return bulkPaint;
}

Paint* Paint::
getCurlBufferedPaint(Paint* basePaint, int sideNum,
	NeighborBufferDescPtr & curlBuffer)
{
	assert(basePaint != 0L);
	PaintPtr p(new Paint(*basePaint, sideNum, curlBuffer));
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
}

Paint* Paint::
getPMLPaint(Paint* basePaint, Vector3i pmlDir)
{
	assert(basePaint != 0L);
	PaintPtr p(new Paint(*basePaint, pmlDir));
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
}

Paint* Paint::
getParentPaint(Paint* basePaint)
{
	assert(basePaint != 0L);
	PaintPtr p(new Paint(*basePaint, 0));
	if (mPalette.count(*p) != 0)
		return mPalette[*p];
	mPalette[*p] = p;
	return p;
}

bool
operator<(const Paint & lhs, const Paint & rhs)
{
	if (&lhs == &rhs)
		return 0;
	if (lhs.mType < rhs.mType)
		return 1;
	else if (lhs.mType > rhs.mType)
		return 0;
	if (lhs.mPMLDirections < rhs.mPMLDirections)
		return 1;
	else if (lhs.mPMLDirections > rhs.mPMLDirections)
		return 0;
	else if (lhs.mCurlBuffers < rhs.mCurlBuffers)
		return 1;
	else if (lhs.mCurlBuffers > rhs.mCurlBuffers)
		return 0;
	else if (lhs.mCurrentBufferIndex < rhs.mCurrentBufferIndex)
		return 1;
	else if (lhs.mCurrentBufferIndex > rhs.mCurrentBufferIndex)
		return 0;
	else if (lhs.mBulkMaterial < rhs.mBulkMaterial) // if unused, these are 0L
		return 1;
	else if (lhs.mBulkMaterial > rhs.mBulkMaterial)
		return 0;
	
	return 0;
}

ostream &
operator<<(ostream & out, const Paint & p)
{
	if (p.mBulkMaterial != 0L)
		out << *p.mBulkMaterial;
	else
		out << "Null";
	if (p.mType == kBulkPaintType)
		out << " bulk";
	else
		out << " boundary";
	
	if (norm2(p.mPMLDirections) != 0)
		out << " PML " << p.mPMLDirections;
	
	for (int nSide = 0; nSide < 6; nSide++)
	if (p.mCurlBuffers[nSide] != 0L)
	{
		out << " (side " << nSide << " buffer " << hex <<
			(void*)p.mCurlBuffers[nSide] << dec << ")";
	}
	
	return out;
}


#pragma mark *** VoxelizedGrid ***

VoxelizedGrid::
VoxelizedGrid(const GridDescription & gridDesc, 
	const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids)
{
	LOG << "VoxelizedGrid()\n";
	
	Vector3i nxyz = gridDesc.getNumYeeCells();
	Vector3i nnxyz = gridDesc.getNumHalfCells();
	m_nx = nxyz[0]; m_ny = nxyz[1]; m_nz = nxyz[2];
	m_nnx = nnxyz[0]; m_nny = nnxyz[1]; m_nnz = nnxyz[2];
	mNonPMLRegion = gridDesc.getNonPMLRegion();
	mCalcRegion = gridDesc.getCalcRegion();
	mOriginYee = gridDesc.getOriginYee();
	
	mMaterialHalfCells.resize(m_nnx*m_nny*m_nnz);
	mMaterialIndexHalfCells.resize(m_nnx*m_nny*m_nnz);
	
	paintFromAssembly(gridDesc, voxelizedGrids);
	
	//cout << *this << endl;
	
	calculateHuygensSymmetries(gridDesc);
	paintFromHuygensSurfaces(gridDesc);
	paintFromCurrentSources(gridDesc);
	paintPML();
	
	// 1.  Paint in basic materials
	//	This includes marking boundaries as boundaries, subcell parts as subcell
	//	parts, etc.  Everything must be marked as the right *type* in this
	//	stage.  Nothing should be put into the PML here.  After painting,
	//  determine the material indices in a second pass, and write variable
	//  params like space-varying permittivity, etc.  (At least leave room.)
	// 2.  Calculate symmetries of Huygens surface interiors and paint edges
	// 3.  Paint PML
	
	calculateMaterialIndices();
	
	//loadSpaceVaryingData();
	
	//generateRunlines();
}


void VoxelizedGrid::
paintFromAssembly(const GridDescription & gridDesc,
	const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids)
{
	LOG << "Painting from assembly.\n";
	
	const vector<InstructionPtr> & instructions = gridDesc.getAssembly()->
		getInstructions();
	
	for (unsigned int nn = 0; nn < instructions.size(); nn++)
	{
		switch(instructions[nn]->getType())
		{
			case kBlockType:
				paintBlock(gridDesc, 
					(const Block&)*instructions[nn]);
				break;
			case kKeyImageType:
				paintKeyImage(gridDesc,
					(const KeyImage&)*instructions[nn]);
				break;
			case kHeightMapType:
				paintHeightMap(gridDesc,
					(const HeightMap&)*instructions[nn]);
				break;
			case kEllipsoidType:
				paintEllipsoid(gridDesc,
					(const Ellipsoid&)*instructions[nn]);
				break;
			case kCopyFromType:
				paintCopyFrom(gridDesc,
					(const CopyFrom&)*instructions[nn], voxelizedGrids);
				break;
			case kExtrudeType:
				paintExtrude(gridDesc, (const Extrude&)*instructions[nn]);
				break;
			default:
				throw(Exception("Unknown instruction type."));
				break;
		}
	}
}

void VoxelizedGrid::
paintFromHuygensSurfaces(const GridDescription & gridDesc)
{
	LOG << "Painting from Huygens surfaces.\n";
	
	const vector<HuygensSurfaceDescPtr> & surfaces =
		gridDesc.getHuygensSurfaces();
	
	for (unsigned int nn = 0; nn < surfaces.size(); nn++)
	{
		paintHuygensSurface(*surfaces[nn]);
	}
}

void VoxelizedGrid::
paintFromCurrentSources(const GridDescription & gridDesc)
{
	LOG << "Painting from current sources.\n";
}

void VoxelizedGrid::
paintPML()
{
	LOG << "Painting PML.\n";
	
	Rect3i nonPML = mNonPMLRegion;
	Rect3i clipPMLDir(-1, -1, -1, 1, 1, 1);
	
	for (int kk = 0; kk < m_nnz; kk++)
	for (int jj = 0; jj < m_nny; jj++)
	for (int ii = 0; ii < m_nnx; ii++)
	if (!nonPML.encloses(ii,jj,kk))
	{
		Vector3i pPML(ii,jj,kk);
		Vector3i nearestNonPML(clip(nonPML, pPML));
		Vector3i pmlOffset(nearestNonPML - pPML);
		Vector3i pmlDir(-clip(clipPMLDir, pmlOffset));
		
		Paint* q = (*this)(nearestNonPML);
		Paint* p = Paint::getPMLPaint(q, pmlDir);
		(*this)(pPML) = p;
	}
}

void VoxelizedGrid::
calculateMaterialIndices()
{
	// This must be done separately for each octant.
	
	for (int nn = 0; nn < 8; nn++)
	{
		//LOG << "Starting side " << nn << "\n";
		Map<Paint*, long> materialIndices;
		Vector3i offset = YU::halfCellOffset(nn);
		//LOG << "Offset is " << offset << "\n";
		//LOG << "Size is " << m_nnx << " " << m_nny << " " << m_nnz << "\n";
		for (int kk = offset[2]; kk < m_nnz; kk += 2)
		for (int jj = offset[1]; jj < m_nny; jj += 2)
		for (int ii = offset[0]; ii < m_nnx; ii += 2)
		{
			Paint* p = (*this)(ii,jj,kk);
			if (materialIndices.count(p) == 0)
			{
				materialIndices[p] = 1;
				mMaterialIndexHalfCells[ii+m_nnx*jj+m_nnx*m_nny*kk] = 0;
				//LOG << "Starting material " << hex << p << dec << "\n";
			}
			else
			{
				mMaterialIndexHalfCells[ii+m_nnx*jj+m_nnx*m_nny*kk] =
					materialIndices[p];
				materialIndices[p]++;
			}
		}
	}
	/*
	for (int kk = 0; kk < m_nnz; kk++)
	{
		cout << "z = " << kk << "\n";
		for (int jj = m_nny-1; jj >= 0; jj--)
		{
			for (int ii = 0; ii < m_nnx; ii++)
			{
				cout << setw(6) <<
					mMaterialIndexHalfCells[ii + m_nnx*jj + m_nnx*m_nny*kk];
			}
			cout << "\n";
		}
	}
	*/
}


void VoxelizedGrid::
calculateHuygensSymmetries(const GridDescription & gridDesc)
{
	LOG << "Calculating Huygens surface symmetries.\n";
	
	const vector<HuygensSurfaceDescPtr> & surfaces =
		gridDesc.getHuygensSurfaces();
	
	mHuygensRegionSymmetries.resize(surfaces.size());
	
	for (unsigned int nn = 0; nn < surfaces.size(); nn++)
	{
		mHuygensRegionSymmetries[nn] = 
			huygensSymmetry(*surfaces[nn]);
		
		LOG << "Surface " << nn << " has bounds " <<
			surfaces[nn]->getDestHalfRect() << " and symmetries "
			<< mHuygensRegionSymmetries[nn] << "\n";
	}
}

Vector3i VoxelizedGrid::
huygensSymmetry(const HuygensSurfaceDescription & surf)
{
	int ii, jj, kk;
	
	Vector3i symmetries(1,1,1);
	
	Vector3i o = surf.getDestHalfRect().p1; // origin
	Vector3i p = surf.getDestHalfRect().p2; // and opposite corner
	Vector3i dim = surf.getDestHalfRect().size();
	const set<Vector3i> & omittedSides = surf.getOmittedSides();
	
	for (int side_i = 0; side_i < 3; side_i++)
	{
		int side_j = (side_i+1)%3;
		int side_k = (side_i+2)%3;
		Vector3i e1 = -YU::cardinalDirection(2*side_i);
		Vector3i e2 = -YU::cardinalDirection( (2*side_i+2)%6 );
		Vector3i e3 = -YU::cardinalDirection( (2*side_i+4)%6 );
		Mat3i m = sMatWithCols(e1,e2,e3);
		
		// 1.  Check the "front" and "back" sides (the sides perpendicular
		// to vector e1).
		if (!omittedSides.count(e1) && !omittedSides.count(-e1))
		{
			for (jj = 0; jj < dim[side_j]; jj++)
			for (kk = 0; kk < dim[side_k]; kk++)
			{
				Vector3i frontSide( (o + jj*e2 + kk*e3) );
				Vector3i backSide( frontSide + (dim[side_i]*e1) );
				if ( (*this)(frontSide) != (*this)(backSide) )
					symmetries[side_i] = 0;
			}
		}
		
		// 2.  Check the "j" sides.  All materials in stripes in the i direction
		// should be the same unless the side is omitted.
		if (!omittedSides.count(e2))
		{
			Vector3i sideOrigin = o + dim[side_j]*e2;
			for (kk = 0; kk < dim[side_k]; kk++)
			{
				Paint* matchMe = (*this)( (sideOrigin+kk*e3) );
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != (*this)(
						(sideOrigin+ii*e1 + kk*e3) ))
						symmetries[side_i] = 0;
			}
		}
		if (!omittedSides.count(-e2))
		{
			Vector3i sideOrigin = o;
			for (kk = 0; kk < dim[side_k]; kk++)
			{
				Paint* matchMe = (*this)((sideOrigin+kk*e3) );
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != (*this)(
						(sideOrigin+ii*e1 + kk*e3) ))
						symmetries[side_i] = 0;
			}
		}

		// 3.  Check the "k" sides.  All materials in stripes in the i direction
		// should be the same unless the side is omitted.
		if (!omittedSides.count(e3))
		{
			Vector3i sideOrigin = o + dim[side_k]*e3;
			for (jj = 0; jj < dim[side_j]; jj++)
			{
				Paint* matchMe = (*this)((sideOrigin+jj*e2));
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != (*this)(
						(sideOrigin+ii*e1 + jj*e2) ))
						symmetries[side_i] = 0;
			}
		}
		if (!omittedSides.count(-e3))
		{
			Vector3i sideOrigin = o;
			for (jj = 0; jj < dim[side_j]; jj++)
			{
				Paint* matchMe = (*this)((sideOrigin+jj*e2));
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != (*this)(
						(sideOrigin+ii*e1 + jj*e2) ))
						symmetries[side_i] = 0;
			}
		}
	} // foreach sideNum
	
	return symmetries;
}

#pragma mark *** Paint Instructions ***


void VoxelizedGrid::
paintBlock(const GridDescription & gridDesc,
	const Block & instruction)
{
	Paint* paint = Paint::getPaint(instruction.getMaterial());
	
	Rect3i halfCells;
	if (instruction.getFillStyle() == kPECStyle)
	{
		halfCells = instruction.getYeeRect();
		halfCells = halfCells*2;
		halfCells.p2 += Vector3i(1,1,1);
	}
	else if (instruction.getFillStyle() == kPMCStyle)
	{
		halfCells = instruction.getYeeRect();
		halfCells.p1 -= Vector3i(1,1,1);
	}
	else if (instruction.getFillStyle() == kHalfCellStyle)
		halfCells = instruction.getHalfRect();
	else
		assert(!"Unknown fill style!");
	
	halfCells.p1 = clip(gridDesc.getHalfCellBounds(), halfCells.p1);
	halfCells.p2 = clip(gridDesc.getHalfCellBounds(), halfCells.p2);
	halfCells = halfCells;
	
	for (int kk = halfCells.p1[2]; kk <= halfCells.p2[2]; kk++)
	for (int jj = halfCells.p1[1]; jj <= halfCells.p2[1]; jj++)
	for (int ii = halfCells.p1[0]; ii <= halfCells.p2[0]; ii++)
		(*this)(ii,jj,kk) = paint;
}

void VoxelizedGrid::
paintKeyImage(const GridDescription & gridDesc,
	const KeyImage & instruction)
{
	Rect3i yeeRect = instruction.getYeeRect();
	vector<ColorKey> keys = instruction.getKeys();
	Vector3i u1 = instruction.getRowDirection();
	Vector3i u2 = -instruction.getColDirection();
	Vector3i u3 = cross(u1, u2);
	assert(norm2(u3) == 1);
	int nUp = abs(dot(u3, yeeRect.p2-yeeRect.p1)) + 1;
	
	Mat3i matrixImgToGrid = sMatWithCols(u1,u2,u3);
		
	// this will select the correct corner of the fill rect regardless of the
	// direction of the image unit vectors.  This point corresponds to the
	// origin of the image (top-left pixel of the .bmp or .*).
	Vector3i fillOrigin = clip(yeeRect,
		Vector3i(-100000*u1 + -100000*u2 + -100000*u3) );
	
	// now the image!
	Magick::Image keyImage;
	try {
		keyImage.read(instruction.getImageFileName());
		for (unsigned int nKey = 0; nKey < keys.size(); nKey++)
		{
			FillStyle style = keys[nKey].getFillStyle();
			Paint* paint = Paint::getPaint(keys[nKey].getMaterial());
			
			// iterate over rows and columns of the image; extrude into grid
			for (unsigned int row = 0; row < keyImage.rows(); row++)
			for (unsigned int col = 0; col < keyImage.columns(); col++)
			{
				Vector3i color = sConvertColor(
					keyImage.pixelColor(col, row)); // column is x in img
				
				if (color == keys[nKey].getColor())
				{
					Vector3i p = fillOrigin +
						matrixImgToGrid*Vector3i(col, row, 0);
					
					for (int up = 0; up < nUp; up++)
					{
						Vector3i pGrid = p + u3*up;
						if (style == kPECStyle)
							paintPEC(paint, pGrid[0], pGrid[1], pGrid[2]);
						else
							paintPMC(paint, pGrid[0], pGrid[1], pGrid[2]);
					} 
				}
			}
		}
	} catch (Magick::Exception & magExc)
 	{
		cerr << "Error reading KeyImage.  Maybe a file type issue?\n";
		cerr << "Caught ImageMagick exception " << magExc.what() << endl;
		LOG << "Exception logged.  Exiting.\n";
		exit(1);
	}
}

void VoxelizedGrid::
paintHeightMap(const GridDescription & gridDesc,
	const HeightMap & instruction)
{
	Rect3i yeeRect = instruction.getYeeRect();
	Vector3i u1 = instruction.getRowDirection();
	Vector3i u2 = -instruction.getColDirection();
	Vector3i u3 = instruction.getUpDirection();
	
	int nUp = abs(dot(u3, yeeRect.p2-yeeRect.p1)) + 1;
	
	Mat3i matrixImgToGrid = sMatWithCols(u1,u2,u3);
	
	// this will select the correct corner of the fill rect regardless of the
	// direction of the image unit vectors.  This point corresponds to the
	// origin of the image (top-left pixel of the .bmp or .*).
	Vector3i fillOrigin = clip(yeeRect,
		Vector3i(-100000*u1 + -100000*u2 + -100000*u3) );
	
	// now the image!
	Magick::Image heightMap;
	try {
		heightMap.read(instruction.getImageFileName());
		FillStyle style = instruction.getFillStyle();
		Paint* paint = Paint::getPaint(instruction.getMaterial());
		
		// iterate over rows and columns of the image; extrude into grid
		for (unsigned int row = 0; row < heightMap.rows(); row++)
		for (unsigned int col = 0; col < heightMap.columns(); col++)
		{
			Magick::ColorGray color = heightMap.pixelColor(col, row);
			int height = int(double(nUp)*color.shade());
			
			Vector3i p = fillOrigin + matrixImgToGrid*Vector3i(col, row, 0);
			for (int up = 0; up < height; up++)
			{
				Vector3i pGrid = p + u3*up;
				if (style == kPECStyle)
					paintPEC(paint, pGrid[0], pGrid[1], pGrid[2]);
				else
					paintPMC(paint, pGrid[0], pGrid[1], pGrid[2]);
			}
		}
	} catch (Magick::Exception & magExc) {
		cerr << "Error reading HeightMap.  Maybe a file type issue?\n";
		cerr << "Caught ImageMagick exception " << magExc.what() << endl;
		LOG << "Exception logged.  Exiting.\n";
		exit(1);
	}
}


void VoxelizedGrid::
paintEllipsoid(const GridDescription & gridDesc,
	const Ellipsoid & instruction)
{
	int ii, jj, kk, iYee, jYee, kYee;
	LOG << "Warning: this probably has bugs in it.\n";
	Paint* paint = Paint::getPaint(instruction.getMaterial());
	
	if (instruction.getFillStyle() == kPECStyle ||
		instruction.getFillStyle() == kPMCStyle)
	{
		//halfCells = instruction.getYeeRect();
		//halfCells.p2 += Vector3i(1,1,1);
		Rect3i yeeRect = instruction.getYeeRect();
		Vector3i centerTimesTwo = yeeRect.p1 + yeeRect.p2;
		Vector3i extent = yeeRect.p2 - yeeRect.p1 + Vector3i(1,1,1);
		Vector3d radii = 0.5*Vector3d(extent[0], extent[1], extent[2]);
		Vector3d center = 0.5*Vector3d(centerTimesTwo[0], centerTimesTwo[1],
			centerTimesTwo[2]);
		
		Rect3i fillRect(clip(gridDesc.getYeeBounds(), yeeRect.p1),
			clip(gridDesc.getYeeBounds(), yeeRect.p2));
		
		for (kYee = fillRect.p1[2]; kYee <= fillRect.p2[2]; kYee++)
		for (jYee = fillRect.p1[1]; jYee <= fillRect.p2[1]; jYee++)
		for (iYee = fillRect.p1[0]; iYee <= fillRect.p2[0]; iYee++)
		{
			Vector3d v( Vector3d(iYee,jYee,kYee) - center );
			
			v[0] /= radii[0];
			v[1] /= radii[1];
			v[2] /= radii[2];
			
			if (norm2(v) <= 1.00001) // give it room for error
			if (instruction.getFillStyle() == kPECStyle)
				paintPEC(paint, iYee, jYee, kYee);
			else
				paintPEC(paint, iYee, jYee, kYee);
		}
	}
	else if (instruction.getFillStyle() == kHalfCellStyle)
	{
		Rect3i halfCells = instruction.getYeeRect();
		halfCells.p2 += Vector3i(1,1,1);
		halfCells = halfCells;
		
		Vector3i extent = halfCells.p2 - halfCells.p1 + Vector3i(1,1,1);
		Vector3i center2 = halfCells.p1 + halfCells.p2;
		Vector3d radii = 0.5*Vector3d(extent[0], extent[1], extent[2]);
		Vector3d center = 0.5*Vector3d(center2[0], center2[1], center2[2]);
		
		halfCells.p1 = clip(gridDesc.getHalfCellBounds(),
			halfCells.p1);
		halfCells.p2 = clip(gridDesc.getHalfCellBounds(),
			halfCells.p2);
		
		if (instruction.getFillStyle() == kHalfCellStyle)
		{
			for (kk = halfCells.p1[2]; kk <= halfCells.p2[2]; kk++)
			for (jj = halfCells.p1[1]; jj <= halfCells.p2[1]; jj++)
			for (ii = halfCells.p1[0]; ii <= halfCells.p2[0]; ii++)
			{
				Vector3d v( Vector3d(ii,jj,kk) - center );
				v[0] /= radii[0];
				v[1] /= radii[1];
				v[2] /= radii[2];
				
				if (norm2(v) <= 1.00001) // give it room for error
					(*this)(ii,jj,kk) = paint;
			}
		}
	}		
}

void VoxelizedGrid::
paintCopyFrom(const GridDescription & gridDesc,
	const CopyFrom & instruction,
	const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids)
{
	const VoxelizedGrid & grid2 = *voxelizedGrids[instruction.getGrid()];
	Rect3i copyFrom = instruction.getSourceHalfRect();
	Rect3i copyTo = instruction.getDestHalfRect();
	int nn;
	
	for (nn = 0; nn < 3; nn++)
	if (copyFrom.size(nn) != 0)
	{
		if (copyFrom.size(nn) != copyTo.size(nn))
		throw(Exception("Error: copy from region must be same size as "
			"copy to region or have size 0"));
		if (copyFrom.p1[nn]%2 != copyTo.p1[nn]%2)
		throw(Exception("Error: copy from region must start on same half-cell"
			" octant as copy to region or have size 0 (both should start on"
			" even indices or on odd indices)"));
		// it's sufficient to check p1 and not p2 because we already know
		// that the dimensions are equal.
	}
	
	// This matrix will project a coordinate in the dest grid onto the source
	// grid if the source grid has fewer dimensions.
	Mat3i matrix(0);
	matrix(0,0) = (copyFrom.size(0) != 0);
	matrix(1,1) = (copyFrom.size(1) != 0);
	matrix(2,2) = (copyFrom.size(2) != 0);
	
	Rect3i copyTo2;
	copyTo2.p1 = clip(gridDesc.getHalfCellBounds(), copyTo.p1);
	copyTo2.p2 = clip(gridDesc.getHalfCellBounds(), copyTo.p2);
	
	Vector3i p;
	Vector3i q;
	for (p[2] = copyTo2.p1[2]; p[2] <= copyTo2.p2[2]; p[2]++)
	for (p[1] = copyTo2.p1[1]; p[1] <= copyTo2.p2[1]; p[1]++)
	for (p[0] = copyTo2.p1[0]; p[0] <= copyTo2.p2[0]; p[0]++)
	{
		q = copyFrom.p1 + matrix*(p - copyTo.p1);
		Paint* plainPaint = Paint::getParentPaint(grid2(q));
		(*this)(p) = plainPaint;
	}
}


void VoxelizedGrid::
paintExtrude(const GridDescription & gridDesc, const Extrude & instruction)
{
	int ii,jj,kk;
	Rect3i fill = instruction.getExtrudeTo();
	Rect3i from = instruction.getExtrudeFrom();
	
	for (kk = fill.p1[2]; kk <= fill.p2[2]; kk++)
	for (jj = fill.p1[1]; jj <= fill.p2[1]; jj++)
	for (ii = fill.p1[0]; ii <= fill.p2[0]; ii++)
	{
		Vector3i q(maxval(minval(Vector3i(ii,jj,kk), from.p2), from.p1));
		(*this)(ii,jj,kk) = (*this)(q);
	}
}

void VoxelizedGrid::
paintHuygensSurface(const HuygensSurfaceDescription & surf)
{
	int ii, jj, kk;
	for (int sideNum = 0; sideNum < 6; sideNum++)
	{
		Vector3i side(YU::cardinalDirection(sideNum));
		if (!surf.getOmittedSides().count(side))
		{
			NeighborBufferDescPtr nb = surf.getBuffers()[sideNum];
			assert(nb != 0L);
			
			Rect3i innerHalfRect = YU::edgeOfRect(
				surf.getDestHalfRect(), sideNum);
			Rect3i outerHalfRect = (innerHalfRect + side);
			
			// Paint the inside
			for (kk = innerHalfRect.p1[2]; kk <= innerHalfRect.p2[2]; kk++)
			for (jj = innerHalfRect.p1[1]; jj <= innerHalfRect.p2[1]; jj++)
			for (ii = innerHalfRect.p1[0]; ii <= innerHalfRect.p2[0]; ii++)
			{
				Paint* p = Paint::getCurlBufferedPaint( (*this)(ii,jj,kk),
					sideNum, nb);
				(*this)(ii,jj,kk) = p;
			}
			
			//LOG << "Huygens inner " << innerHalfRect << "\n";
			
			// Paint the outside
			int oppositeSideNum = (sideNum % 2) ? (sideNum-1) : (sideNum+1);
			for (kk = outerHalfRect.p1[2]; kk <= outerHalfRect.p2[2]; kk++)
			for (jj = outerHalfRect.p1[1]; jj <= outerHalfRect.p2[1]; jj++)
			for (ii = outerHalfRect.p1[0]; ii <= outerHalfRect.p2[0]; ii++)
			{
				Paint* q = Paint::getCurlBufferedPaint( (*this)(ii,jj,kk),
					oppositeSideNum, nb);
				(*this)(ii,jj,kk) = q;
			}
			
			//LOG << "Huygens outer " << outerHalfRect << "\n"; 

		}
	}
}

#pragma mark *** Accessor and grid paint methods ***

Paint* & VoxelizedGrid::
operator() (int ii, int jj, int kk)
{
	return mMaterialHalfCells[(ii+m_nnx)%m_nnx + 
		((jj+m_nny)%m_nny)*m_nnx +
		((kk+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* VoxelizedGrid::
operator() (int ii, int jj, int kk) const
{
	return mMaterialHalfCells[(ii+m_nnx)%m_nnx + 
		((jj+m_nny)%m_nny)*m_nnx +
		((kk+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* & VoxelizedGrid::
operator() (const Vector3i & pp)
{
	return mMaterialHalfCells[(pp[0]+m_nnx)%m_nnx + 
		((pp[1]+m_nny)%m_nny)*m_nnx +
		((pp[2]+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* VoxelizedGrid::
operator() (const Vector3i & pp) const
{
	return mMaterialHalfCells[(pp[0]+m_nnx)%m_nnx + 
		((pp[1]+m_nny)%m_nny)*m_nnx +
		((pp[2]+m_nnz)%m_nnz)*m_nnx*m_nny];
}

void VoxelizedGrid::
paintPEC(Paint* paint, int iYee, int jYee, int kYee)
{
	/*
	assert(iYee >= 0 && iYee < m_nx);
	assert(jYee >= 0 && jYee < m_ny);
	assert(kYee >= 0 && kYee < m_nz);
	*/
	for (int kk = 0; kk < 2; kk++)
	for (int jj = 0; jj < 2; jj++)
	for (int ii = 0; ii < 2; ii++)
		(*this)(2*iYee+ii, 2*jYee+jj, 2*kYee+kk) = paint;
}

void VoxelizedGrid::
paintPMC(Paint* paint, int iYee, int jYee, int kYee)
{
	/*
	assert(iYee >= 0 && iYee < m_nx);
	assert(jYee >= 0 && jYee < m_ny);
	assert(kYee >= 0 && kYee < m_nz);
	*/
	for (int kk = 0; kk < 2; kk++)
	for (int jj = 0; jj < 2; jj++)
	for (int ii = 0; ii < 2; ii++)
		(*this)(2*iYee-ii, 2*jYee-jj, 2*kYee-kk) = paint;
}



std::ostream &
operator<< (std::ostream & out, const VoxelizedGrid & grid)
{
	const Map<Paint, PaintPtr> & paints = Paint::getPalette();
	Map<Paint*, char> code;
	char curChar = '!';
	for (Map<Paint,PaintPtr>::const_iterator itr = paints.begin();
		itr != paints.end(); itr++)
	{
		Paint p = itr->first;
		PaintPtr pp = itr->second;
		code[itr->second] = curChar;
		
		out << curChar << ": " << p << endl;
		do {
			curChar++;
		} while (curChar == '+' || curChar == '-' || curChar == '|');
	}
	code[0L] = ' ';
	
	int ii,jj,kk;
	for (kk = 0; kk < grid.m_nnz; kk++)
	{
		// Print z slice number
		out << "\n";
		out << "\nz = " << kk << "\n";
		
		// Print x marks
		out << "     ";
		for (ii = 0; ii < grid.m_nnx; ii += 10)
			out << "|" << setw(9) << std::left << ii;
		
		// Print line at top of grid
		out << "\n     +";
		for (ii = 0; ii < grid.m_nnx; ii++)
			out << "-";
		out << "+";
		
		// Print the grid
		for (jj = grid.m_nny-1; jj >= 0; jj--)
		{
			out << "\n" << setw(5) << std::left << jj << "|";
			for (ii = 0; ii < grid.m_nnx; ii++)
			{
				out << code[grid(ii,jj,kk)];
			}
			out << "|" << setw(5) << std::right << jj;
		}
		// Print line at bottom of grid
		out << "\n     +";
		for (ii = 0; ii < grid.m_nnx; ii++)
			out << "-";
		out << "+";
		
		// Print x positions again
		out << "\n     ";
		for (ii = 0; ii < grid.m_nnx; ii += 10)
			out << "|" << setw(9) << std::left << ii;
	}
	
	return out;
}

#pragma mark *** Static stuff ***

//  This helper function exists because when an image includes color
//
//  0x112233
//
//  ImageMagick will represent it with high-res quanta multiplied by **257**,
//
//  0x111122223333
//
//  which will not be the same as the quantum we get programmatically from
//
//  Color("#112233")
//
//  which will be the 256x,
//
//  0x110022003300
//
//  hence not the same as the value from the file.  We can use the provided
//  macros for scaling quanta to do away with this whole mess, and so I'll
//  use my own Vector3i to represent a color.
//
//  (Why: multiplying by 257 fills up the dynamic range completely.)
//
Vector3i sConvertColor(const Magick::Color & inColor)
{
    Vector3i outColor;
    
#if (MagickLibVersion == 0x618)
    outColor[0] = Magick::ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = Magick::ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = Magick::ScaleQuantumToChar(inColor.greenQuantum());
#else
    outColor[0] = MagickLib::ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = MagickLib::ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = MagickLib::ScaleQuantumToChar(inColor.greenQuantum());
#endif

    return outColor;
}


static Mat3i sMatWithCols(Vector3i v1, Vector3i v2, Vector3i v3)
{
	Mat3i m;
	for (int row = 0; row < 3; row++)
	{
		m(row,0) = v1[row];
		m(row,1) = v2[row];
		m(row,2) = v3[row];
	}
	return m;
}



