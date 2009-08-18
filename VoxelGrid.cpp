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
#include "HuygensSurface.h"
#include "CurrentSource.h"

#include "Map.h"
#include <vector>
using namespace std;
#include <Magick++.h>
static Vector3i sConvertColor(const Magick::Color & inColor);

#include "YeeUtilities.h"
using namespace YeeUtilities;


VoxelGrid::
VoxelGrid(Rect3i voxelizedBounds, Rect3i gridHalfCells, Rect3i nonPML) :
	mAllocRegion(voxelizedBounds),
	mGridHalfCells(gridHalfCells),
	mNonPMLRegion(nonPML)
{	
	m_nnx = mAllocRegion.size(0)+1;
	m_nny = mAllocRegion.size(1)+1;
	m_nnz = mAllocRegion.size(2)+1;
	
	assert(m_nnx%2 == 0);
	assert(m_nny%2 == 0);
	assert(m_nnz%2 == 0);
	
	m_nx = m_nnx/2;
	m_ny = m_nny/2;
	m_nz = m_nnz/2;
	
//	LOG << "Non PML is " << nonPML << endl;
	
    long allocSize = m_nnx*m_nny*m_nnz;
    if (mMaterialHalfCells.max_size() < allocSize)
    {
        cerr << "Warning: VoxelGrid is going to attempt to allocate a "
            << m_nnx << "x" << m_nny << "x" << m_nnz << " cell array with "
            "std::vector; the total size is " << allocSize << " and the vector"
            " maximum size is " << mMaterialHalfCells.max_size() << ", so this"
            " will likely fail." << endl;
    }
	mMaterialHalfCells.resize(m_nnx*m_nny*m_nnz);
}


#pragma mark *** Paint Instructions ***


void VoxelGrid::
paintBlock(const GridDescription & gridDesc,
	const Block & instruction)
{
	Paint* paint = Paint::getPaint(instruction.material());
	
	Rect3i halfCells;
	if (instruction.fillStyle() == kPECStyle)
	{
		halfCells = yeeToHalf(instruction.yeeRect());
		halfCells.p2 += Vector3i(1,1,1);
	}
	else if (instruction.fillStyle() == kPMCStyle)
	{
		halfCells = yeeToHalf(instruction.yeeRect());
		halfCells.p1 -= Vector3i(1,1,1);
	}
	else if (instruction.fillStyle() == kHalfCellStyle)
		halfCells = instruction.halfRect();
	else
		assert(!"Unknown fill style!");
	
	// Clipping explicitly is unnecessary and even undesirable because of grid-
	// level wraparound.
	//halfCells = clip(mAllocRegion, halfCells);
	//LOG << "Painting half cells " << halfCells << "\n";
    
	for (int kk = halfCells.p1[2]; kk <= halfCells.p2[2]; kk++)
	for (int jj = halfCells.p1[1]; jj <= halfCells.p2[1]; jj++)
	for (int ii = halfCells.p1[0]; ii <= halfCells.p2[0]; ii++)
		paintHalfCell(paint, ii, jj, kk);
		//(*this)(ii,jj,kk) = paint;
}

void VoxelGrid::
paintKeyImage(const GridDescription & gridDesc,
	const KeyImage & instruction)
{
	Rect3i yeeRect = instruction.yeeRect();
	vector<ColorKey> keys = instruction.keys();
	Vector3i u1 = instruction.rowDirection();
	Vector3i u2 = -instruction.columnDirection();
	Vector3i u3 = cross(u1, u2);
	assert(norm2(u3) == 1);
	int nUp = abs(dot(u3, yeeRect.p2-yeeRect.p1)) + 1;
	
	Mat3i matrixImgToGrid(Mat3i::withColumns(u1,u2,u3));
		
	// this will select the correct corner of the fill rect regardless of the
	// direction of the image unit vectors.  This point corresponds to the
	// origin of the image (top-left pixel of the .bmp or .*).
	Vector3i fillOrigin = clip(yeeRect,
		Vector3i(-100000*u1 + -100000*u2 + -100000*u3) );
	
	// now the image!
	Magick::Image keyImage;
	try {
		keyImage.read(instruction.imageFileName());
		for (unsigned int nKey = 0; nKey < keys.size(); nKey++)
		{
			FillStyle style = keys[nKey].fillStyle();
			Paint* paint = Paint::getPaint(keys[nKey].material());
			
			// iterate over rows and columns of the image; extrude into grid
			for (unsigned int row = 0; row < keyImage.rows(); row++)
			for (unsigned int col = 0; col < keyImage.columns(); col++)
			{
				Vector3i color = sConvertColor(
					keyImage.pixelColor(col, row)); // column is x in img
				
				if (color == keys[nKey].color())
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
	Rect3i yeeRect = instruction.yeeRect();
	Vector3i u1 = instruction.rowDirection();
	Vector3i u2 = -instruction.columnDirection();
	Vector3i u3 = instruction.upDirection();
	
	int nUp = abs(dot(u3, yeeRect.p2-yeeRect.p1)) + 1;
	
	Mat3i matrixImgToGrid(Mat3i::withColumns(u1,u2,u3));
	
	// this will select the correct corner of the fill rect regardless of the
	// direction of the image unit vectors.  This point corresponds to the
	// origin of the image (top-left pixel of the .bmp or .*).
	Vector3i fillOrigin = clip(yeeRect,
		Vector3i(-100000*u1 + -100000*u2 + -100000*u3) );
	
	// now the image!
	Magick::Image heightMap;
	try {
		heightMap.read(instruction.imageFileName());
		FillStyle style = instruction.fillStyle();
		Paint* paint = Paint::getPaint(instruction.material());
		
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
	Paint* paint = Paint::getPaint(instruction.material());
	
	if (instruction.fillStyle() == kPECStyle ||
		instruction.fillStyle() == kPMCStyle)
	{
		//halfCells = instruction.yeeRect();
		//halfCells.p2 += Vector3i(1,1,1);
		Rect3i yeeRect = instruction.yeeRect();
		Vector3i centerTimesTwo = yeeRect.p1 + yeeRect.p2;
		Vector3i extent = yeeRect.p2 - yeeRect.p1 + Vector3i(1,1,1);
		Vector3d radii = 0.5*Vector3d(extent[0], extent[1], extent[2]);
		Vector3d center = 0.5*Vector3d(centerTimesTwo[0], centerTimesTwo[1],
			centerTimesTwo[2]);
		
		// Don't clip: we clip in the paint routines.
		//Rect3i fillRect(clip(gridDesc.yeeBounds(), yeeRect.p1),
		//	clip(gridDesc.yeeBounds(), yeeRect.p2));
		Rect3i fillRect(yeeRect);
		
		for (kYee = fillRect.p1[2]; kYee <= fillRect.p2[2]; kYee++)
		for (jYee = fillRect.p1[1]; jYee <= fillRect.p2[1]; jYee++)
		for (iYee = fillRect.p1[0]; iYee <= fillRect.p2[0]; iYee++)
		{
			Vector3d v( Vector3d(iYee,jYee,kYee) - center );
			
			v[0] /= radii[0];
			v[1] /= radii[1];
			v[2] /= radii[2];
			
			if (norm2(v) <= 1.00001) // give it room for error
			if (instruction.fillStyle() == kPECStyle)
				paintPEC(paint, iYee, jYee, kYee);
			else
				paintPEC(paint, iYee, jYee, kYee);
		}
	}
	else if (instruction.fillStyle() == kHalfCellStyle)
	{
		Rect3i halfCells = instruction.yeeRect();
		halfCells.p2 += Vector3i(1,1,1);
		halfCells = halfCells;
		
		Vector3i extent = halfCells.p2 - halfCells.p1 + Vector3i(1,1,1);
		Vector3i center2 = halfCells.p1 + halfCells.p2;
		Vector3d radii = 0.5*Vector3d(extent[0], extent[1], extent[2]);
		Vector3d center = 0.5*Vector3d(center2[0], center2[1], center2[2]);
		
		// Don't clip: we clip in the paint routines
		//halfCells = clip(halfCells, mAllocRegion);
				
		if (instruction.fillStyle() == kHalfCellStyle)
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
	Rect3i copyFrom = instruction.sourceHalfRect();
	Rect3i copyTo = instruction.destHalfRect();
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
	Mat3i matrix;
	matrix(0,0) = (copyFrom.size(0) != 0);
	matrix(1,1) = (copyFrom.size(1) != 0);
	matrix(2,2) = (copyFrom.size(2) != 0);
	
	// Don't clip here, we clip in the paint routines
	//Rect3i copyTo2 = clip(copyTo, mAllocRegion);
	Rect3i copyTo2(copyTo);
	
	Vector3i p;
	Vector3i q;
	for (p[2] = copyTo2.p1[2]; p[2] <= copyTo2.p2[2]; p[2]++)
	for (p[1] = copyTo2.p1[1]; p[1] <= copyTo2.p2[1]; p[1]++)
	for (p[0] = copyTo2.p1[0]; p[0] <= copyTo2.p2[0]; p[0]++)
	{
		q = copyFrom.p1 + matrix*(p - copyTo.p1);
		Paint* plainPaint = grid2(q)->withoutModifications();
		(*this)(p) = plainPaint;
	}
}


void VoxelGrid::
paintExtrude(const GridDescription & gridDesc, const Extrude & instruction)
{
	int ii,jj,kk;
	Rect3i fill = instruction.extrudeTo();
	Rect3i from = instruction.extrudeFrom();
	
	// These two clipping steps here are why the Extrude instruction is not
	// enabled for MPI.
	fill = clip(fill, mAllocRegion);
	from = clip(from, mAllocRegion);
	
	for (kk = fill.p1[2]; kk <= fill.p2[2]; kk++)
	for (jj = fill.p1[1]; jj <= fill.p2[1]; jj++)
	for (ii = fill.p1[0]; ii <= fill.p2[0]; ii++)
	{
		Vector3i q(vec_max(vec_min(Vector3i(ii,jj,kk), from.p2), from.p1));
		(*this)(ii,jj,kk) = (*this)(q);
	}
}

#pragma mark *** Overlay methods ***

void VoxelGrid::
overlayHuygensSurface(const HuygensSurface & surf)
{
	int ii, jj, kk;
    
    //cout << *this << "\n";
	for (int sideNum = 0; sideNum < 6; sideNum++)
	{
        if (surf.hasBuffer(sideNum))
		{
			NeighborBufferPtr nb = surf.buffer(sideNum);
			assert(nb != 0L);
			
			Rect3i innerHalfRect = edgeOfRect(surf.halfCells(), sideNum);
			Rect3i outerHalfRect = (innerHalfRect + cardinal(sideNum));
			
			// Paint the inside
			for (kk = innerHalfRect.p1[2]; kk <= innerHalfRect.p2[2]; kk++)
			for (jj = innerHalfRect.p1[1]; jj <= innerHalfRect.p2[1]; jj++)
			for (ii = innerHalfRect.p1[0]; ii <= innerHalfRect.p2[0]; ii++)
			{
				Paint* p = (*this)(ii,jj,kk)->withCurlBuffer(sideNum, nb);
				paintHalfCell(p, ii, jj, kk);
			}
			
			//LOG << "Huygens inner " << innerHalfRect << "\n";
			
			// Paint the outside
			int oppositeSideNum = (sideNum % 2) ? (sideNum-1) : (sideNum+1);
			for (kk = outerHalfRect.p1[2]; kk <= outerHalfRect.p2[2]; kk++)
			for (jj = outerHalfRect.p1[1]; jj <= outerHalfRect.p2[1]; jj++)
			for (ii = outerHalfRect.p1[0]; ii <= outerHalfRect.p2[0]; ii++)
			{
				Paint* q = (*this)(ii,jj,kk)->withCurlBuffer(oppositeSideNum,
					nb);
				paintHalfCell(q, ii, jj, kk);
			}
			
			//LOG << "Huygens outer " << outerHalfRect << "\n"; 

		}
	}
    
    //cout << *this << "\n";
    //LOG << "Done.\n";
}

void VoxelGrid::
overlayCurrentSource(const SetupCurrentSource & current)
{
//	LOG << "Overlaying current source.\n";
    Vector3i p;
    int fieldDirection;
    Rect3i yeeCells;
    int rr;
    
    const CurrentSourceDescription & description = *current.description();
    
    // Paint the source for electric currents
    for (fieldDirection = 0; fieldDirection < 3; fieldDirection++)
    if (description.sourceCurrents().whichJ()[fieldDirection] != 0)
    {
        for (rr = 0; rr < description.regions().size(); rr++)
        {
            yeeCells = description.regions()[rr].yeeCells();
            for (p[2] = yeeCells.p1[2]; p[2] <= yeeCells.p2[2]; p[2]++)
            for (p[1] = yeeCells.p1[1]; p[1] <= yeeCells.p2[1]; p[1]++)
            for (p[0] = yeeCells.p1[0]; p[0] <= yeeCells.p2[0]; p[0]++)
            {
                Paint* paint = (*this)(yeeToHalf(p, octantE(fieldDirection)))
                    ->withCurrentSource(current.description());
                paintHalfCell(paint, yeeToHalf(p, octantE(fieldDirection)));
            }
        }
    }
    
    // Paint the source for magnetic currents
    for (fieldDirection = 0; fieldDirection < 3; fieldDirection++)
    if (description.sourceCurrents().whichK()[fieldDirection] != 0)
    {
        for (rr = 0; rr < description.regions().size(); rr++)
        {
            yeeCells = description.regions()[rr].yeeCells();
            for (p[2] = yeeCells.p1[2]; p[2] <= yeeCells.p2[2]; p[2]++)
            for (p[1] = yeeCells.p1[1]; p[1] <= yeeCells.p2[1]; p[1]++)
            for (p[0] = yeeCells.p1[0]; p[0] <= yeeCells.p2[0]; p[0]++)
            {
                Paint* paint = (*this)(yeeToHalf(p, octantH(fieldDirection)))
                    ->withCurrentSource(current.description());
                paintHalfCell(paint, yeeToHalf(p, octantH(fieldDirection)));
            }
        }
    }
}

void VoxelGrid::
overlayPML()
{
	//LOG << "Overlaying PML.  Using slow algorithm.\n";
	//LOGMORE << "Bounds " << mAllocRegion << "\n";
	//LOGMORE << "nonPML " << mNonPMLRegion << "\n";
	
    /*
	LOG << "Warning: this will not result in correct behavior if the current "
		"node needs to paint the PML from the opposite side of the whole grid"
		" and if the PML is not in the same direction.  This can be avoided "
		"by always painting a half-cell of PEC or PMC at the grid edge, I "
		"believe, but I don't know that I would count on this behavior.\n";
	*/
	Rect3i clipPMLDir(-1, -1, -1, 1, 1, 1);
	Rect3i & mGrid = mGridHalfCells;
	
    const bool USE_FASTER_METHOD = 1;
    
    Vector3i pp;
    if (USE_FASTER_METHOD == 0)
    {
        for (pp[2] = mGrid.p1[2]; pp[2] <= mGrid.p2[2]; pp[2]++)
        for (pp[1] = mGrid.p1[1]; pp[1] <= mGrid.p2[1]; pp[1]++)
        for (pp[0] = mGrid.p1[0]; pp[0] <= mGrid.p2[0]; pp[0]++)
        if (!mNonPMLRegion.encloses(pp))
        {
            // pmlDir points from the PML to the non PML region, and all
            // components are 1, -1 or 0.
            Vector3i pmlDir(clip(clipPMLDir, 
                Vector3i(pp - clip(mNonPMLRegion, pp))));
            
            // this is like paintHalfCell except it tags PML.  there's some fancy
            // footwork to account for PML that is wrapping around the grid in
            // MPI contexts, should that ever occur.
            paintPML(pmlDir, pp);
        }
    }
    else
    {
        // 1.  +X and -X faces
        for (pp[2] = mGrid.p1[2]; pp[2] <= mGrid.p2[2]; pp[2]++)
        for (pp[1] = mGrid.p1[1]; pp[1] <= mGrid.p2[1]; pp[1]++)
        for (pp[0] = mGrid.p1[0]; pp[0] < mNonPMLRegion.p1[0]; pp[0]++)
        {
            Vector3i pmlDir(clip(clipPMLDir,  pp - clip(mNonPMLRegion, pp)));
            paintPML(pmlDir, pp);
        }
        
        for (pp[2] = mGrid.p1[2]; pp[2] <= mGrid.p2[2]; pp[2]++)
        for (pp[1] = mGrid.p1[1]; pp[1] <= mGrid.p2[1]; pp[1]++)
        for (pp[0] = mNonPMLRegion.p2[0]+1; pp[0] <= mGrid.p2[0]; pp[0]++)
        {
            Vector3i pmlDir(clip(clipPMLDir,  pp - clip(mNonPMLRegion, pp)));
            paintPML(pmlDir, pp);
        }
        
        // 2.  +Y and -Y faces, limited to the non-X-PML regions
        for (pp[2] = mGrid.p1[2]; pp[2] <= mGrid.p2[2]; pp[2]++)
        for (pp[1] = mGrid.p1[1]; pp[1] < mNonPMLRegion.p1[1]; pp[1]++)
        for (pp[0] = mNonPMLRegion.p1[0]; pp[0] <= mNonPMLRegion.p2[0]; pp[0]++)
        {
            Vector3i pmlDir(clip(clipPMLDir,  pp - clip(mNonPMLRegion, pp)));
            paintPML(pmlDir, pp);
        }
        
        for (pp[2] = mGrid.p1[2]; pp[2] <= mGrid.p2[2]; pp[2]++)
        for (pp[1] = mNonPMLRegion.p2[1]+1; pp[1] <= mGrid.p2[1]; pp[1]++)
        for (pp[0] = mNonPMLRegion.p1[0]; pp[0] <= mNonPMLRegion.p2[0]; pp[0]++)
        {
            Vector3i pmlDir(clip(clipPMLDir,  pp - clip(mNonPMLRegion, pp)));
            paintPML(pmlDir, pp);
        }
        
        // 3.  +Z and -Z faces, limited to non-X-PML and non-Y-PML regions
        for (pp[2] = mGrid.p1[2]; pp[2] < mNonPMLRegion.p1[2]; pp[2]++)
        for (pp[1] = mNonPMLRegion.p1[1]; pp[1] <= mNonPMLRegion.p2[1]; pp[1]++)
        for (pp[0] = mNonPMLRegion.p1[0]; pp[0] <= mNonPMLRegion.p2[0]; pp[0]++)
        {
            Vector3i pmlDir(clip(clipPMLDir,  pp - clip(mNonPMLRegion, pp)));
            paintPML(pmlDir, pp);
        }
        
        for (pp[2] = mNonPMLRegion.p2[2]+1; pp[2] <= mGrid.p2[2]; pp[2]++)
        for (pp[1] = mNonPMLRegion.p1[1]; pp[1] <= mNonPMLRegion.p2[1]; pp[1]++)
        for (pp[0] = mNonPMLRegion.p1[0]; pp[0] <= mNonPMLRegion.p2[0]; pp[0]++)
        {
            Vector3i pmlDir(clip(clipPMLDir,  pp - clip(mNonPMLRegion, pp)));
            paintPML(pmlDir, pp);
        }
    }
}

#pragma mark *** Accessor and grid paint methods ***


Paint* & VoxelGrid::
operator() (int ii, int jj, int kk)
{
	ii = ii - mAllocRegion.p1[0];
	jj = jj - mAllocRegion.p1[1];
	kk = kk - mAllocRegion.p1[2];
	return mMaterialHalfCells[(ii+m_nnx)%m_nnx + 
		((jj+m_nny)%m_nny)*m_nnx +
		((kk+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* VoxelGrid::
operator() (int ii, int jj, int kk) const
{
	ii = ii - mAllocRegion.p1[0];
	jj = jj - mAllocRegion.p1[1];
	kk = kk - mAllocRegion.p1[2];
	return mMaterialHalfCells[(ii+m_nnx)%m_nnx + 
		((jj+m_nny)%m_nny)*m_nnx +
		((kk+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* & VoxelGrid::
operator() (const Vector3i & pp)
{
	Vector3i qq(pp - mAllocRegion.p1);
	return mMaterialHalfCells[(qq[0]+m_nnx)%m_nnx + 
		((qq[1]+m_nny)%m_nny)*m_nnx +
		((qq[2]+m_nnz)%m_nnz)*m_nnx*m_nny];
}

Paint* VoxelGrid::
operator() (const Vector3i & pp) const
{
	Vector3i qq(pp - mAllocRegion.p1);
	return mMaterialHalfCells[(qq[0]+m_nnx)%m_nnx + 
		((qq[1]+m_nny)%m_nny)*m_nnx +
		((qq[2]+m_nnz)%m_nnz)*m_nnx*m_nny];
}

void VoxelGrid::
paintHalfCell(Paint* paint, int ii, int jj, int kk)
{
	//	 Paint the cell itself
	if (mAllocRegion.encloses(ii,jj,kk))
	{
		(*this)(ii,jj,kk) = paint;
	}
	
	// Now grid-symmetrical paints
	for (int transDir = 0; transDir < 6; transDir++)
	{
		Vector3i translation(cardinal(transDir)*
			(mGridHalfCells.size(transDir/2)+1));
		if (mAllocRegion.encloses(ii+translation[0], jj+translation[1],
			kk+translation[2]))
		{
			(*this)(ii+translation[0], jj+translation[1], kk+translation[2])
				 = paint;
		}
	}
}

void VoxelGrid::
paintHalfCell(Paint* paint, const Vector3i & pp)
{
    paintHalfCell(paint, pp[0], pp[1], pp[2]);
}

void VoxelGrid::
paintPEC(Paint* paint, int iYee, int jYee, int kYee)
{
	for (int kk = 0; kk < 2; kk++)
	for (int jj = 0; jj < 2; jj++)
	for (int ii = 0; ii < 2; ii++)
		paintHalfCell(paint, 2*iYee+ii, 2*jYee+jj, 2*kYee+kk);
}

void VoxelGrid::
paintPMC(Paint* paint, int iYee, int jYee, int kYee)
{
	for (int kk = 0; kk < 2; kk++)
	for (int jj = 0; jj < 2; jj++)
	for (int ii = 0; ii < 2; ii++)
		paintHalfCell(paint, 2*iYee-ii, 2*jYee-jj, 2*kYee-kk);
}

void VoxelGrid::
paintPML(Vector3i pmlDir, Vector3i pp)
{
	//	 Paint the cell itself
	if (mAllocRegion.encloses(pp))
		(*this)(pp) = (*this)(pp)->withPML(pmlDir);
	
	// Now grid-symmetrical paints
	for (int transDir = 0; transDir < 6; transDir++)
	{
		Vector3i qq(pp + cardinal(transDir)*
			(mGridHalfCells.size(transDir/2)+1));
		
		if (mAllocRegion.encloses(qq))
		{
			(*this)(qq) = (*this)(qq)->withPML(pmlDir);
		}
	}
}

void VoxelGrid::
clear()
{
    mMaterialHalfCells.resize(0);
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
static Vector3i sConvertColor(const Magick::Color & inColor)
{
    Vector3i outColor;
    

#if (MagickLibVersion == 0x618)
    outColor[0] = ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = ScaleQuantumToChar(inColor.greenQuantum());
#elif (MagickLibVersion == 0x628)
    outColor[0] = MagickLib::ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = MagickLib::ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = MagickLib::ScaleQuantumToChar(inColor.greenQuantum());
#else
    outColor[0] = MagickCore::ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = MagickCore::ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = MagickCore::ScaleQuantumToChar(inColor.greenQuantum());
#endif
    return outColor;
}


