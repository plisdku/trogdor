/*
 *  VoxelGrid.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/7/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "VoxelGrid.h"
#include "SimulationDescription.h"

#include "Map.h"
#include <vector>
using namespace std;
#include <Magick++.h>
static Vector3i sConvertColor(const Magick::Color & inColor);

#include "YeeUtilities.h"
using namespace YeeUtilities;


VoxelGrid::
VoxelGrid(Vector3i numYeeCells, Rect3i nonPML)
{
	mNonPMLRegion = nonPML;
	m_nx = numYeeCells[0];
	m_ny = numYeeCells[1];
	m_nz = numYeeCells[2];
	m_nnx = 2*m_nx;
	m_nny = 2*m_ny;
	m_nnz = 2*m_nz;
	
	mMaterialHalfCells.resize(m_nnx*m_nny*m_nnz);
}


#pragma mark *** Paint Instructions ***


void VoxelGrid::
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

void VoxelGrid::
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
	
	Mat3i matrixImgToGrid = matWithCols(u1,u2,u3);
		
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

void VoxelGrid::
paintHeightMap(const GridDescription & gridDesc,
	const HeightMap & instruction)
{
	Rect3i yeeRect = instruction.getYeeRect();
	Vector3i u1 = instruction.getRowDirection();
	Vector3i u2 = -instruction.getColDirection();
	Vector3i u3 = instruction.getUpDirection();
	
	int nUp = abs(dot(u3, yeeRect.p2-yeeRect.p1)) + 1;
	
	Mat3i matrixImgToGrid = matWithCols(u1,u2,u3);
	
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


void VoxelGrid::
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

void VoxelGrid::
paintCopyFrom(const GridDescription & gridDesc,
	const CopyFrom & instruction,
	const VoxelGrid & grid2)
{
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


void VoxelGrid::
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

#pragma mark *** Overlay methods ***

void VoxelGrid::
overlayHuygensSurface(const HuygensSurfaceDescription & surf)
{
	int ii, jj, kk;
	for (int sideNum = 0; sideNum < 6; sideNum++)
	{
		Vector3i side(cardinalDirection(sideNum));
		if (!surf.getOmittedSides().count(side))
		{
			NeighborBufferDescPtr nb = surf.getBuffers()[sideNum];
			assert(nb != 0L);
			
			Rect3i innerHalfRect = edgeOfRect(
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

void VoxelGrid::
overlayCurrentSource(const int & currentSource)
{
	LOG << "Overlaying current source.\n";
}

void VoxelGrid::
overlayPML()
{
	LOG << "Overlaying PML.  Using slow algorithm.\n";
	
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


#pragma mark *** Accessor and grid paint methods ***


Paint* & VoxelGrid::
operator() (int ii, int jj, int kk)
{
	return mMaterialHalfCells[(ii+m_nnx)%m_nnx + 
		((jj+m_nny)%m_nny)*m_nnx +
		((kk+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* VoxelGrid::
operator() (int ii, int jj, int kk) const
{
	return mMaterialHalfCells[(ii+m_nnx)%m_nnx + 
		((jj+m_nny)%m_nny)*m_nnx +
		((kk+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* & VoxelGrid::
operator() (const Vector3i & pp)
{
	return mMaterialHalfCells[(pp[0]+m_nnx)%m_nnx + 
		((pp[1]+m_nny)%m_nny)*m_nnx +
		((pp[2]+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* VoxelGrid::
operator() (const Vector3i & pp) const
{
	return mMaterialHalfCells[(pp[0]+m_nnx)%m_nnx + 
		((pp[1]+m_nny)%m_nny)*m_nnx +
		((pp[2]+m_nnz)%m_nnz)*m_nnx*m_nny];
}

void VoxelGrid::
paintPEC(Paint* paint, int iYee, int jYee, int kYee)
{
	for (int kk = 0; kk < 2; kk++)
	for (int jj = 0; jj < 2; jj++)
	for (int ii = 0; ii < 2; ii++)
		(*this)(2*iYee+ii, 2*jYee+jj, 2*kYee+kk) = paint;
}

void VoxelGrid::
paintPMC(Paint* paint, int iYee, int jYee, int kYee)
{
	for (int kk = 0; kk < 2; kk++)
	for (int jj = 0; jj < 2; jj++)
	for (int ii = 0; ii < 2; ii++)
		(*this)(2*iYee-ii, 2*jYee-jj, 2*kYee-kk) = paint;
}



std::ostream &
operator<< (std::ostream & out, const VoxelGrid & grid)
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


