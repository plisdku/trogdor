/*
 *  VoxelizedPartition.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "VoxelizedPartition.h"

#include "SimulationDescription.h"
#include "Log.h"
#include "YeeUtilities.h"
#include "MaterialBoss.h"

using namespace std;
using namespace YeeUtilities;

#pragma mark *** VoxelizedPartition ***

VoxelizedPartition::
VoxelizedPartition(const GridDescription & gridDesc, 
	const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids,
	Rect3i partitionBounds, Rect3i calcRegion) :
	mVoxels(partitionBounds, gridDesc.getNonPMLRegion()),
	//mPMLFaceIndices(6),
	mPartitionBounds(partitionBounds),
	mCalcRegion(calcRegion)
{
	LOG << "VoxelizedPartition()\n";
	
	mNonPMLRegion = gridDesc.getNonPMLRegion();
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
	
	createMaterialDelegates();
	
	loadSpaceVaryingData();
	
	generateRunlines();
}

bool VoxelizedPartition::
partitionHasPML(int faceNum) const
{
	//LOG << "Using non-parallel-friendly method.\n";
	assert(faceNum >= 0);
	assert(faceNum < 6);
	
	if (faceNum%2 == 0) // low side
	{
		if (mNonPMLRegion.p1[faceNum/2] <= mPartitionBounds.p1[faceNum/2])
			return 0;
	}
	else
	{
		if (mNonPMLRegion.p2[faceNum/2] >= mPartitionBounds.p2[faceNum/2])
			return 0;
	}
	return 1;
}

Rect3i VoxelizedPartition::
getPMLRegionOnFace(int faceNum) const
{
	//LOG << "Using non-parallel-friendly method.\n";
	assert(faceNum >= 0);
	assert(faceNum < 6);
	
	Rect3i pmlRegion(mPartitionBounds);
	
	if (faceNum%2 == 0) // low side
	{
		pmlRegion.p2[faceNum/2] = mNonPMLRegion.p1[faceNum/2]-1;
	}
	else
	{
		pmlRegion.p1[faceNum/2] = mNonPMLRegion.p2[faceNum/2]+1;
	}
	return pmlRegion;
}


void VoxelizedPartition::
paintFromAssembly(const GridDescription & gridDesc,
	const Map<GridDescPtr, VoxelizedPartitionPtr> & voxelizedGrids)
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

void VoxelizedPartition::
paintFromHuygensSurfaces(const GridDescription & gridDesc)
{
	LOG << "Painting from Huygens surfaces.\n";
	
	const vector<HuygensSurfaceDescPtr> & surfaces =
		gridDesc.getHuygensSurfaces();
	
	for (unsigned int nn = 0; nn < surfaces.size(); nn++)
		mVoxels.overlayHuygensSurface(*surfaces[nn]);
}

void VoxelizedPartition::
paintFromCurrentSources(const GridDescription & gridDesc)
{
	LOG << "Painting from current sources.  (Doing nothing.)\n";
}


void VoxelizedPartition::
calculateMaterialIndices()
{
	// This must be done separately for each octant.
	
	mCentralIndices = CellCountGridPtr(new CellCountGrid(mVoxels,
		mPartitionBounds));
	
	cout << *mCentralIndices << endl;
	
	/*
	for (int nFace = 0; nFace < 6; nFace++)
	if (partitionHasPML(nFace))
	{
		//LOG << "Face number " << nFace << " of 6...\n";
		Rect3i face = edgeOfRect(mNonPMLRegion, nFace);
		if (nFace % 2 == 0) // low-side face
			face.p2 -= cardinalDirection(nFace);
		else // high-side face
			face.p1 -= cardinalDirection(nFace);
		
		assert(mNonPMLRegion.encloses(face));
		//mPMLFaces[nFace] = face;
		
		//LOG << "PML face is " << face << endl;
		
		
		mPMLFaceIndices[nFace] = CellCountGridPtr(new CellCountGrid(mVoxels,
			face));
		
		
		//cout << *mPMLFaceIndices[nFace] << endl;
	}
	else
	{
		LOG << "Face number " << nFace << " of 6 has no PML.\n";
	}
	*/
	
}

void VoxelizedPartition::
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

Vector3i VoxelizedPartition::
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

void VoxelizedPartition::
createMaterialDelegates()
{
	set<Paint*> allPaints = mCentralIndices->getCurlBufferParentPaints();
	
	// Cache PML rects
	vector<Rect3i> pmlRects;
	for (int nn = 0; nn < 6; nn++)
		pmlRects.push_back(getPMLRegionOnFace(nn));
	
	LOG << "Iterating over paints...\n";
	for (set<Paint*>::iterator itr = allPaints.begin(); itr != allPaints.end();
		itr++)
	{
		Paint* p = *itr;
		if (mDelegates.count(p) == 0)
		{
			mDelegates[p] = NewMaterialFactory::getDelegate(
				mVoxels, mCentralIndices, /*mPMLFaceIndices,*/ p);
		}
		MaterialDelegate & mat = *mDelegates[p];
		
		LOGMORE << hex << p << dec << ":\n";
		LOGMORE << *p << "\n";
		
		for (int octant = 0; octant < 8; octant++)
		{
			long numCellsInOctant = mCentralIndices->getNumCells(p, octant);
			
			// This count suffices for bulk materials
			mat.setNumCells(octant, numCellsInOctant);
			
			// This fills in information for PMLs
			if (p->isPML()) // this condition just speeds things up
			{
				Paint* nonPML = Paint::getParentPaint(p);
				for (int faceNum = 0; faceNum < 6; faceNum++)
				if (partitionHasPML(faceNum))
				{
					mat.setPMLDepth(octant, faceNum,
						rectHalfToYee(pmlRects[faceNum], octant).size(faceNum/2)+1);
					//mat.setNumCellsOnPMLFace(octant, faceNum,
					//	mPMLFaceIndices[faceNum]->getNumCells(nonPML, octant));
				}
			}
		}
	}
}

void VoxelizedPartition::
loadSpaceVaryingData()
{
	LOG << "Material delegates need to provide temporary space!\n";
	LOGMORE << "Not loading anything yet.\n";
}

void VoxelizedPartition::
generateRunlines()
{
	// Provide a RunlineEncoder for each uniquely-updating Paint
	
	// Walk the grid (ONCE only would be splendid) and step the appropriate
	// encoders.  (Where do setup runlines go?)
	
	LOG << "Check it out, we're sticking to the calc region.  I'm not sure "
		"yet precisely how to use this in the MPI context—work it out later.\n";
		
	for (int fieldNum = 0; fieldNum < 6; fieldNum++)
	{
		// Remember to set up the buffers here!
		LOG << "Runlines for offset " << halfCellFieldOffset(fieldNum) << "\n";
		genRunlinesInOctant(halfCellIndex(halfCellFieldOffset(fieldNum)));
	} 
	
}

void VoxelizedPartition::
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
		xParentPaint = Paint::retrieveCurlBufferParentPaint(xPaint);
		if (!needNewRunline)
		{
			if (xParentPaint == lastXParentPaint &&
				material->canContinueRunline(mVoxels, *mCentralIndices,
					/*mPMLFaceIndices,*/ lastX, x, xPaint)) // kludge
				material->continueRunline(x);
			else
			{
				material->endRunline();
				needNewRunline = 1;
			}
		}
		if (needNewRunline)
		{
			material = mDelegates[xParentPaint];
			material->startRunline(mVoxels, *mCentralIndices,
				/*mPMLFaceIndices,*/ x);
			needNewRunline = 0;
		}
		lastX = x;
		lastXParentPaint = xParentPaint;
	}
	material->endRunline();  // DO NOT FORGET THIS
}


std::ostream &
operator<< (std::ostream & out, const VoxelizedPartition & grid)
{
	out << grid.mVoxels;
	return out;
}


