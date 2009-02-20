/*
 *  VooxelizedGrid.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "VoxelizedGrid.h"

#include "SimulationDescription.h"
#include "Log.h"

using namespace std;

#pragma mark *** Paint ***

Paint::
Paint(PaintType inType) :
	mType(inType),
	mPMLDirection(0,0,0),
	mCurrentBufferIndex(-1),
	mCurlBufferIndices(6,-1)
{
}

Paint Paint::
pml( const Paint & baseMaterial, Vector3i direction )
{
	return Paint(kBulkPaintType);
}

Paint Paint::
currentSource( const Paint & baseMaterial, 
	int currentSourceShouldHaveDescriptionTypeTooEventually )
{
	return Paint(kBulkPaintType);
}

Paint Paint::
huygensSurface( const Paint & baseMaterial, int bufferSide,
	int bufferNumberOrSomeOtherID )
{
	return Paint(kBulkPaintType);
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
	else if (lhs.mPMLDirection < rhs.mPMLDirection)
		return 1;
	else if (lhs.mPMLDirection > rhs.mPMLDirection)
		return 0;
	else if (lhs.mCurrentBufferIndex < rhs.mCurrentBufferIndex)
		return 1;
	else if (lhs.mCurrentBufferIndex > rhs.mCurrentBufferIndex)
		return 0;
	else if (lhs.mCurlBufferIndices < rhs.mCurlBufferIndices)
		return 1;
	else if (lhs.mCurlBufferIndices > rhs.mCurlBufferIndices)
		return 0;
	
	return 0;
}

#pragma mark *** VoxelizedGrid ***

VoxelizedGrid::
VoxelizedGrid(const GridDescription & gridDesc, 
	const Map<string, VoxelizedGridPtr> & voxelizedGrids,
	Mat3i orientation)
{
	LOG << "VoxelizedGrid()\n";
	
	Vector3i nxyz = orientation*gridDesc.getNumYeeCells();
	Vector3i nnxyz = orientation*gridDesc.getNumHalfCells();
	m_nx = nxyz[0]; m_ny = nxyz[1]; m_nz = nxyz[2];
	m_nnx = nnxyz[0]; m_nny = nnxyz[1]; m_nnz = nnxyz[2];
	mNonPMLRegion = orientation*gridDesc.getNonPMLRegion();
	mCalcRegion = orientation*gridDesc.getCalcRegion();
	
	paintFromAssembly(gridDesc, voxelizedGrids, orientation);
	paintFromHuygensSurfaces(gridDesc, orientation);
	paintFromCurrentSources(gridDesc, orientation);
	paintPML();
	
	// 1.  Paint in basic materials
	//	This includes marking boundaries as boundaries, subcell parts as subcell
	//	parts, etc.  Everything must be marked as the right *type* in this
	//	stage.  Nothing should be put into the PML here.  After painting,
	//  determine the material indices in a second pass, and write variable
	//  params like space-varying permittivity, etc.  (At least leave room.)
	// 2.  Calculate symmetries of Huygens surface interiors and paint edges
	// 3.  Paint PML
	
	// Question: do I need to "paint" current sources into the grid here?  I
	// guess so, since they'll probably end up buffer-based.
}

void VoxelizedGrid::
paintFromAssembly(const GridDescription & gridDesc,
	const Map<string, VoxelizedGridPtr> & voxelizedGrids,
	Mat3i orientation)
{
	LOG << "Painting from assembly.\n";
	
	const vector<MaterialDescPtr> materials;
	assert(!"There are no materials to paint from!  Get from Simulation.");
	//const vector<MaterialDescPtr> & materials = gridDesc.getMaterials();
	const vector<InstructionPtr> & instructions = gridDesc.getAssembly()->
		getInstructions();
	
	for (unsigned int nn = 0; nn < instructions.size(); nn++)
	{
		switch(instructions[nn]->getType())
		{
			case kBlockType:
				paintBlock(gridDesc, (const Block&)*instructions[nn]);
				break;
			case kKeyImageType:
				paintKeyImage(gridDesc, (const KeyImage&)*instructions[nn]);
				break;
			case kHeightMapType:
				paintHeightMap(gridDesc, (const HeightMap&)*instructions[nn]);
				break;
			case kEllipsoidType:
				paintEllipsoid(gridDesc, (const Ellipsoid&)*instructions[nn]);
				break;
			case kCopyFromType:
				paintCopyFrom(gridDesc, (const CopyFrom&)*instructions[nn],
					voxelizedGrids);
				break;
			default:
				throw(Exception("Unknown instruction type."));
				break;
		}
	}
	
}

void VoxelizedGrid::
paintFromHuygensSurfaces(const GridDescription & gridDesc, Mat3i orientation)
{
	LOG << "Painting from Huygens surfaces.\n";
}

void VoxelizedGrid::
paintFromCurrentSources(const GridDescription & gridDesc, Mat3i orientation)
{
	LOG << "Painting from current sources.\n";
}

void VoxelizedGrid::
paintPML()
{
	LOG << "Painting PML.\n";
}


#pragma mark *** Paint Instructions ***


void VoxelizedGrid::
paintBlock(const GridDescription & gridDesc,
	const Block & instruction)
{
	//unsigned short paintNum = getPaint(instruction->getMaterial());
	if (instruction.getFillStyle() == kPECStyle)
	{
	}
	else if (instruction.getFillStyle() == kPMCStyle)
	{
	}
	else if (instruction.getFillStyle() == kHalfCellStyle)
	{
	}
	else
		assert(!"Unknown fill style!");
}

void VoxelizedGrid::
paintKeyImage(const GridDescription & gridDesc,
	const KeyImage & instruction)
{
}

void VoxelizedGrid::
paintHeightMap(const GridDescription & gridDesc,
	const HeightMap & instruction)
{
}

void VoxelizedGrid::
paintEllipsoid(const GridDescription & gridDesc,
	const Ellipsoid & instruction)
{
}

void VoxelizedGrid::
paintCopyFrom(const GridDescription & gridDesc,
	const CopyFrom & instruction,
	const Map<std::string, VoxelizedGridPtr> & voxelizedGrids)
{
}







