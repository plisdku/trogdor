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
#include "MaterialBoss.h"

using namespace std;
using namespace YeeUtilities;

#pragma mark *** VoxelizedGrid ***

VoxelizedGrid::
VoxelizedGrid(const GridDescription & gridDesc, 
	const Map<GridDescPtr, VoxelizedGridPtr> & voxelizedGrids) :
	mVoxels(gridDesc.getNumYeeCells(), gridDesc.getNonPMLRegion()),
	mPMLFaceIndices(6)
{
	LOG << "VoxelizedGrid()\n";
	
	mNumYeeCells = gridDesc.getNumYeeCells();
	mNumHalfCells = gridDesc.getNumHalfCells();
	m_nx = mNumYeeCells[0];
	m_ny = mNumYeeCells[1];
	m_nz = mNumYeeCells[2];
	m_nnx = mNumHalfCells[0];
	m_nny = mNumHalfCells[1];
	m_nnz = mNumHalfCells[2];
	mNonPMLRegion = gridDesc.getNonPMLRegion();
	mCalcRegion = gridDesc.getCalcRegion();
	mOriginYee = gridDesc.getOriginYee();
	
	LOG << "nonPML " << mNonPMLRegion << "\n";
	LOG << "calc " << mCalcRegion << "\n";
	LOG << "origin " << mOriginYee << "\n";
	
	paintFromAssembly(gridDesc, voxelizedGrids);
	calculateHuygensSymmetries(gridDesc);
	paintFromHuygensSurfaces(gridDesc);
	paintFromCurrentSources(gridDesc);
	
	mVoxels.overlayPML();
	
	// 1.  Paint in basic materials
	//	This includes marking boundaries as boundaries, subcell parts as subcell
	//	parts, etc.  Everything must be marked as the right *type* in this
	//	stage.  Nothing should be put into the PML here.  After painting,
	//  determine the material indices in a second pass, and write variable
	//  params like space-varying permittivity, etc.  (At least leave room.)
	// 2.  Calculate symmetries of Huygens surface interiors and paint edges
	// 3.  Paint PML
	
	calculateMaterialIndices();
	
	loadSpaceVaryingData();
	
	generateRunlines();
}

bool VoxelizedGrid::
hasPML(int faceNum) const
{
	LOG << "Using non-parallel-friendly method.\n";
	assert(faceNum >= 0);
	assert(faceNum < 6);
	
	if (faceNum%2 == 0) // low side
	{
		if (mNonPMLRegion.p1[faceNum/2] <= 0)
			return 0;
	}
	else
	{
		if (mNonPMLRegion.p2[faceNum/2] >= mNumHalfCells[faceNum/2]-1)
			return 0;
	}
	return 1;
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
				mVoxels.paintBlock(gridDesc, 
					(const Block&)*instructions[nn]);
				break;
			case kKeyImageType:
				mVoxels.paintKeyImage(gridDesc,
					(const KeyImage&)*instructions[nn]);
				break;
			case kHeightMapType:
				mVoxels.paintHeightMap(gridDesc,
					(const HeightMap&)*instructions[nn]);
				break;
			case kEllipsoidType:
				mVoxels.paintEllipsoid(gridDesc,
					(const Ellipsoid&)*instructions[nn]);
				break;
			case kCopyFromType:
				mVoxels.paintCopyFrom(gridDesc,
					(const CopyFrom&)*instructions[nn],
					voxelizedGrids[((const CopyFrom&)*instructions[nn])
						.getGrid()]->mVoxels);
				break;
			case kExtrudeType:
				mVoxels.paintExtrude(gridDesc,
					(const Extrude&)*instructions[nn]);
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
		mVoxels.overlayHuygensSurface(*surfaces[nn]);
}

void VoxelizedGrid::
paintFromCurrentSources(const GridDescription & gridDesc)
{
	LOG << "Painting from current sources.  (Doing nothing.)\n";
}


void VoxelizedGrid::
calculateMaterialIndices()
{
	// This must be done separately for each octant.
	
	Rect3i cheesyEverywhereRect(0,0,0,m_nnx-1, m_nny-1, m_nnz-1);
	mCentralIndices = CellCountGridPtr(new CellCountGrid(mVoxels,
		cheesyEverywhereRect));
	
	cout << *mCentralIndices << endl;
	
	for (int nFace = 0; nFace < 6; nFace++)
	if (hasPML(nFace))
	{
		LOG << "Face number " << nFace << " of 6...\n";
		Rect3i face = edgeOfRect(mNonPMLRegion, nFace);
		if (nFace % 2 == 0) // low-side face
			face.p2 -= cardinalDirection(nFace);
		else // high-side face
			face.p1 -= cardinalDirection(nFace);
		
		assert(mNonPMLRegion.encloses(face));
		//mPMLFaces[nFace] = face;
		
		LOG << "PML face is " << face << endl;
		
		mPMLFaceIndices[nFace] = CellCountGridPtr(new CellCountGrid(mVoxels,
			face));
		
		//cout << *mPMLFaceIndices[nFace] << endl;
	}
	else
	{
		LOG << "Face number " << nFace << " of 6 has no PML.\n";
	}
	
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
		Vector3i e1 = -cardinalDirection(2*side_i);
		Vector3i e2 = -cardinalDirection( (2*side_i+2)%6 );
		Vector3i e3 = -cardinalDirection( (2*side_i+4)%6 );
		Mat3i m = matWithCols(e1,e2,e3);
		
		// 1.  Check the "front" and "back" sides (the sides perpendicular
		// to vector e1).
		if (!omittedSides.count(e1) && !omittedSides.count(-e1))
		{
			for (jj = 0; jj < dim[side_j]; jj++)
			for (kk = 0; kk < dim[side_k]; kk++)
			{
				Vector3i frontSide( (o + jj*e2 + kk*e3) );
				Vector3i backSide( frontSide + (dim[side_i]*e1) );
				if ( mVoxels(frontSide) != mVoxels(backSide) )
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
				Paint* matchMe = mVoxels( (sideOrigin+kk*e3) );
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != mVoxels(
						(sideOrigin+ii*e1 + kk*e3) ))
						symmetries[side_i] = 0;
			}
		}
		if (!omittedSides.count(-e2))
		{
			Vector3i sideOrigin = o;
			for (kk = 0; kk < dim[side_k]; kk++)
			{
				Paint* matchMe = mVoxels((sideOrigin+kk*e3) );
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != mVoxels(
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
				Paint* matchMe = mVoxels((sideOrigin+jj*e2));
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != mVoxels(
						(sideOrigin+ii*e1 + jj*e2) ))
						symmetries[side_i] = 0;
			}
		}
		if (!omittedSides.count(-e3))
		{
			Vector3i sideOrigin = o;
			for (jj = 0; jj < dim[side_j]; jj++)
			{
				Paint* matchMe = mVoxels((sideOrigin+jj*e2));
				for (ii = 0; ii < dim[side_i]; ii++)
					if (matchMe != mVoxels(
						(sideOrigin+ii*e1 + jj*e2) ))
						symmetries[side_i] = 0;
			}
		}
	} // foreach sideNum
	
	return symmetries;
}

void VoxelizedGrid::
loadSpaceVaryingData()
{
	LOG << "Not loading anything yet.\n";
}

void VoxelizedGrid::
generateRunlines()
{
	// Provide a RunlineEncoder for each uniquely-updating Paint
	
	// Walk the grid (ONCE only would be splendid) and step the appropriate
	// encoders.  (Where do setup runlines go?)
	
	LOG << "Check it out, we're sticking to the calc region.  I'm not sure "
		"yet precisely how to use this in the MPI context—work it out later.\n";
	
	for (int fieldNum = 0; fieldNum < 6; fieldNum++)
	{
		LOG << "Runlines for offset " << halfCellFieldOffset(fieldNum) << "\n";
		genRunlinesInOctant(halfCellIndex(halfCellFieldOffset(fieldNum)));
	} 
	
}

void VoxelizedGrid::
genRunlinesInOctant(int octant)
{
	//	 First task: generate a starting half-cell in the correct octant.
	// The loops may still end by not exceeding mCalcRegion.p2—this works fine.
	Vector3i offset = halfCellOffset(octant);
	Vector3i p1 = mCalcRegion.p1;
	for (int nn = 0; nn < 3; nn++)
	if (p1[nn] % 2 != offset[nn])
		p1[nn]++;
	LOG << "Calc region " << mCalcRegion << endl;
	LOG << "Runlines in octant " << octant << " at start " << p1 << endl;
	
	Map<Paint*,long> allNumCells = mCentralIndices->getAllNumCells(octant);
	Map<Paint*, MaterialDelegatePtr> delegates;
	
	for (map<Paint*, long>::const_iterator itr = allNumCells.begin();
		itr != allNumCells.end(); itr++)
	{
		delegates[itr->first] = NewMaterialFactory::getDelegate(mVoxels,
			*mCentralIndices, mPMLFaceIndices, itr->first);
	}
	MaterialDelegate* material; // unsafe pointer for speed in this case.
	
	// If there is a current runline
	//	Ask the MaterialDelegate whether the current cell belongs to it
	//	YES: keep on loopin'
	//	NO: end runline and begin new runline
	//
	// Therafter, if there is not a current runline
	//	Begin new runline
	
	bool needNewRunline = 1;
	Vector3i x(p1), lastX(p1);
	Paint *xPaint, *xParentPaint = 0L, *lastXParentPaint = 0L;
	for (x[2] = p1[2]; x[2] <= mCalcRegion.p2[2]; x[2] += 2)
	for (x[1] = p1[1]; x[1] <= mCalcRegion.p2[1]; x[1] += 2)
	for (x[0] = p1[0]; x[0] <= mCalcRegion.p2[0]; x[0] += 2)
	{
		xPaint = mVoxels(x);
		xParentPaint = Paint::getParentPaint(xPaint);
		if (!needNewRunline)
		{
			if (xParentPaint == lastXParentPaint &&
				material->canContinueRunline(lastX, x, xPaint)) // kludge
				material->continueRunline(x);
			else
			{
				material->endRunline();
				needNewRunline = 1;
			}
		}
		if (needNewRunline)
		{
			material = delegates[xParentPaint];
			material->startRunline(x, xPaint);
			needNewRunline = 0;
		}
		lastX = x;
		lastXParentPaint = xParentPaint;
	}
	material->endRunline();  // DO NOT FORGET THIS
}


std::ostream &
operator<< (std::ostream & out, const VoxelizedGrid & grid)
{
	out << grid.mVoxels;
	return out;
}


