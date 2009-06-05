/*
 *  MaterialBoss.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 4/8/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "MaterialBoss.h"
#include "SimulationDescription.h"

#include "VoxelizedPartition.h"
#include "VoxelGrid.h"
#include "PartitionCellCount.h"
#include "Paint.h"
#include "Log.h"
#include "YeeUtilities.h"
#include "CalculationPartition.h"

#include "StaticDielectric.h"
#include "StaticDielectricPML.h"
#include "StaticLossyDielectric.h"
#include "DrudeModel1.h"
#include "PerfectConductor.h"

using namespace std;
using namespace YeeUtilities;

MaterialDelegatePtr MaterialFactory::
getDelegate(const VoxelGrid & vg, const PartitionCellCountPtr cg, 
	Paint* parentPaint)
{
	assert(parentPaint != 0L);
	//LOG << "Delegate for " << *parentPaint << endl;
	
	//PaintType type = parentPaint->getType();
	const MaterialDescPtr bulkMaterial = parentPaint->getBulkMaterial();
	string materialClass(bulkMaterial->getModelName());
	string materialName(bulkMaterial->getName());
	
	//LOG << "Getting delegate for " << *parentPaint << ".\n"; 
    
	if (materialClass == "StaticDielectric")
	{
        if (parentPaint->isPML())
            return MaterialDelegatePtr(new StaticDielectricPMLDelegate);
        else
            return MaterialDelegatePtr(new StaticDielectricDelegate);
	}
    else if (materialClass == "StaticLossyDielectric")
    {
        //if (parentPaint->isPML())
        //    return MaterialDelegatePtr(new StaticLossyDielectricPMLDelegate);
        //else
            return MaterialDelegatePtr(new StaticLossyDielectricDelegate);
    }
	else if (materialClass == "DrudeMetal")
	{
        //if (parentPaint->isPML())
        //    return MaterialDelegatePtr(new DrudeModel1PMLDelegate);
        //else
            return MaterialDelegatePtr(new DrudeModel1Delegate(bulkMaterial));
	}
	else if (materialClass == "PerfectConductor")
	{
        return MaterialDelegatePtr(new PerfectConductorDelegate);
	}
	
	
	LOG << "Using default (silly) delegate.\n";
    
	if (parentPaint->isPML())
		return MaterialDelegatePtr(new SimpleBulkPMLMaterialDelegate);
	return MaterialDelegatePtr(new SimpleBulkMaterialDelegate);
}

Material::
Material()
{
}

Material::
~Material()
{
}

void Material::
allocateAuxBuffers()
{
}

MaterialDelegate::
MaterialDelegate()
{
}

MaterialDelegate::
~MaterialDelegate()
{
}

void MaterialDelegate::
setNumCells(int octant, int number)
{
	//LOG << "Default: octant " << octant << ", num " << number << "\n";
}

void MaterialDelegate::
setNumCellsOnPMLFace(int octant, int faceNum, int number)
{
	//LOG << "Default: octant " << octant << ", face " << faceNum << ", num "
	//	<< number << "\n";
}

void MaterialDelegate::
setPMLDepth(int octant, int faceNum, int number)
{
	//LOG << "Default: octant " << octant << ", face " << faceNum << ", num "
	//	<< number << "\n";
}


#pragma mark *** Simple Bulk Material ***

SimpleBulkMaterialDelegate::
SimpleBulkMaterialDelegate() :
	MaterialDelegate()
{
}

void SimpleBulkMaterialDelegate::
startRunline(const VoxelizedPartition & vp, const Vector3i & startPos)
{
	int nSide;
	const VoxelGrid & voxelGrid(vp.getVoxelGrid());
	const PartitionCellCount & cellCountGrid(*vp.getIndices());
	
	mStartPoint = startPos;
	mStartPaint = voxelGrid(startPos);
	
	// Set the mask (which directions to check)
	int fieldDirection = octantFieldDirection(startPos);
	int dir_j = (fieldDirection+1)%3;
	int dir_k = (fieldDirection+2)%3;
	
	// Set the start neighbor indices, check buffers, store pointers
	BufferPointer bp[6]; // we won't fill all of these, but it makes things easy
	for (nSide = 0; nSide < 6; nSide++)
	{
		mStartNeighborIndices[nSide] = vp.linearYeeIndex(
			vp.wrap(startPos + cardinalDirection(nSide)));
		
		if (nSide/2 == fieldDirection)
			mUsedNeighborIndices[nSide] = 0;
		else if (mStartPaint->hasCurlBuffer(nSide))
		{
			mUsedNeighborIndices[nSide] = 0;
			bp[nSide] = vp.fieldPointer(mStartPaint->getCurlBuffer(nSide),
				mStartPoint+cardinalDirection(nSide));
		}
		else
		{
			mUsedNeighborIndices[nSide] = 1;
			bp[nSide] = vp.fieldPointer(mStartPoint+cardinalDirection(nSide));
		}
	}
	mCurrentRunline.f_i = vp.fieldPointer(startPos);
	mCurrentRunline.f_j[0] = bp[2*dir_j];
	mCurrentRunline.f_j[1] = bp[2*dir_j+1];
	mCurrentRunline.f_k[0] = bp[2*dir_k];
	mCurrentRunline.f_k[1] = bp[2*dir_k+1];
	mCurrentRunline.auxIndex = cellCountGrid(startPos);
	mCurrentRunline.length = 1;
	
	//LOG << "Field direction " << fieldDirection << " ui " << ui << " uj "
	//	<< uj << " uk " << uk << "\n";
	
	
	LOG << "Start runline:\n";
	LOGMORE << "start " << mStartPoint << "\n";
	LOGMORE << "aux " << mCurrentRunline.auxIndex << "\n";
	LOGMORE << "f_i " << mCurrentRunline.f_i << "\n";
	LOGMORE << "f_j " << mCurrentRunline.f_j[0] << " "
		<< mCurrentRunline.f_j[1] << "\n";
	LOGMORE << "f_k " << mCurrentRunline.f_k[0] << " "
		<< mCurrentRunline.f_k[1] << "\n";
	
}

bool SimpleBulkMaterialDelegate::
canContinueRunline(const VoxelizedPartition & vp, const Vector3i & oldPos,
	const Vector3i & newPos, Paint* newPaint) const
{
	for (int nSide = 0; nSide < 6; nSide++)
	if (mUsedNeighborIndices[nSide])
	{
		int index = vp.linearYeeIndex(vp.wrap(
            newPos + cardinalDirection(nSide)));
		if (mStartNeighborIndices[nSide] + mCurrentRunline.length != index)
			return 0;
	}
	else
	{
		if (newPaint->getCurlBuffer(nSide) != mStartPaint->getCurlBuffer(nSide))
			return 0;
	}
	return 1;
}

void SimpleBulkMaterialDelegate::
continueRunline(const Vector3i & newPos)
{
	mCurrentRunline.length++;
}

void SimpleBulkMaterialDelegate::
endRunline()
{
	int field = octantFieldNumber(mStartPoint);
	mRunlines[field].push_back(SBMRunlinePtr(new SBMRunline(mCurrentRunline)));
}

void SimpleBulkMaterialDelegate::
printRunlines(std::ostream & out) const
{
	for (int field = 0; field < 6; field++)
	{
		out << "Field " << field << "\n";
		for (unsigned int rr = 0; rr < mRunlines[field].size(); rr++)
		{
			out << rr << ": length " << mRunlines[field][rr]->length <<
				" aux " << mRunlines[field][rr]->auxIndex << "\n";
			out << "\t" << mRunlines[field][rr]->f_i << "\n";
			out << "\t" << mRunlines[field][rr]->f_j[0] << "\n";
			out << "\t" << mRunlines[field][rr]->f_j[1] << "\n";
			out << "\t" << mRunlines[field][rr]->f_k[0] << "\n";
			out << "\t" << mRunlines[field][rr]->f_k[1] << "\n";
		}
	}
}

MaterialPtr SimpleBulkMaterialDelegate::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    cerr << "You shouldn't be here.  Overload for your material.";
    exit(1);
}


#pragma mark *** Simple Bulk PML Material ***

SimpleBulkPMLMaterialDelegate::
SimpleBulkPMLMaterialDelegate() :
	MaterialDelegate()
{
}

void SimpleBulkPMLMaterialDelegate::
startRunline(const VoxelizedPartition & vp, const Vector3i & startPos)
{
	int nSide;
	const VoxelGrid & voxelGrid(vp.getVoxelGrid());
	const PartitionCellCount & cellCountGrid(*vp.getIndices());
	
	mStartPoint = startPos;
	mStartPaint = voxelGrid(startPos);
	
	// Set the mask (which directions to check)
	int fieldDirection = octantFieldDirection(startPos);
	int dir_j = (fieldDirection+1)%3;
	int dir_k = (fieldDirection+2)%3;
	
	// Set the start neighbor indices, check buffers, store pointers
	BufferPointer bp[6]; // we won't fill all of these, but it makes things easy
	for (nSide = 0; nSide < 6; nSide++)
	{
		mStartNeighborIndices[nSide] = vp.linearYeeIndex(
			vp.wrap(startPos + cardinalDirection(nSide)));
		
		if (nSide/2 == fieldDirection)
			mUsedNeighborIndices[nSide] = 0;
		else if (mStartPaint->hasCurlBuffer(nSide))
		{
			mUsedNeighborIndices[nSide] = 0;
			bp[nSide] = vp.fieldPointer(mStartPaint->getCurlBuffer(nSide),
				mStartPoint+cardinalDirection(nSide));
		}
		else
		{
			mUsedNeighborIndices[nSide] = 1;
			bp[nSide] = vp.fieldPointer(mStartPoint+cardinalDirection(nSide));
		}
	}
	mCurrentRunline.f_i = vp.fieldPointer(startPos);
	mCurrentRunline.f_j[0] = bp[2*dir_j];
	mCurrentRunline.f_j[1] = bp[2*dir_j+1];
	mCurrentRunline.f_k[0] = bp[2*dir_k];
	mCurrentRunline.f_k[1] = bp[2*dir_k+1];
	mCurrentRunline.auxIndex = cellCountGrid(startPos);
	mCurrentRunline.length = 1;
	
	// PML aux stuff
	Rect3i gridHalfCells = vp.getGridHalfCells();
    assert(vec_eq(gridHalfCells.p1, 0));  // never hurts to be sure.
	Vector3i gridSize = gridHalfCells.size() + Vector3i(1,1,1);
	Vector3i pmlDir = mStartPaint->getPMLDirections();
	mPMLRect = vp.getPMLHalfCells(pmlDir);
	
    // The start point of the runline *may* be outside the grid, *if* we are
    // performing calculations on ghost points!  This may happen in data-push
    // adjoint update equations.  In any case, usually the wrap does nothing.
	Vector3i wrappedStartPoint = Vector3i(mStartPoint + gridSize) % gridSize;
	mCurrentRunline.pmlDepthIndex = wrappedStartPoint/2 - mPMLRect.p1/2;
    
    // IF THE PML IS ON A NEGATIVE (e.g. left or bottom) SIDE:
    //  the pml depth index is 0 at the highest-conductivity point
    // IF THE PML IS ON A POSITIVE (e.g. right or top) SIDE:
    //  the pml depth index is 0 at the lowest-conductivity point, which is
    //  considered to be a physical distance of dx or dx/2 into the PML.
    // This index is in Yee cells.
    
	/*
	LOG << "runline for PML in " << mPMLRect << "\n";
	LOGMORE << "dir " << pmlDir << "\n";
	LOGMORE << "start " << mStartPoint << "\n";
	LOGMORE << "grid size " << gridSize << "\n";
	LOGMORE << "wrap start " << wrappedStartPoint << "\n";
	LOGMORE << "depth " << mCurrentRunline.pmlDepthIndex << "\n";
	*/
}

bool SimpleBulkPMLMaterialDelegate::
canContinueRunline(const VoxelizedPartition & vp, const Vector3i & oldPos,
	const Vector3i & newPos, Paint* newPaint) const
{
	for (int nSide = 0; nSide < 6; nSide++)
	if (mUsedNeighborIndices[nSide])
	{
		int index = vp.linearYeeIndex(vp.wrap(
            newPos + cardinalDirection(nSide)));
		if (mStartNeighborIndices[nSide] + mCurrentRunline.length != index)
			return 0;
	}
	else
	{
		if (newPaint->getCurlBuffer(nSide) != mStartPaint->getCurlBuffer(nSide))
			return 0;
	}
	return 1;
}

void SimpleBulkPMLMaterialDelegate::
continueRunline(const Vector3i & newPos)
{
	mCurrentRunline.length++;
}

void SimpleBulkPMLMaterialDelegate::
endRunline()
{
	int field = octantFieldNumber(mStartPoint);
	mRunlines[field].push_back(SBPMRunlinePtr(
		new SBPMRunline(mCurrentRunline)));
}

void SimpleBulkPMLMaterialDelegate::
printRunlines(std::ostream & out) const
{
	for (int field = 0; field < 6; field++)
	{
		out << "Field " << field << "\n";
		for (unsigned int rr = 0; rr < mRunlines[field].size(); rr++)
		{
			out << rr << ": length " << mRunlines[field][rr]->length <<
				" aux " << mRunlines[field][rr]->auxIndex <<
				" pml depth";
			for (int nn = 0; nn < 3; nn++)
				out << " " << mRunlines[field][rr]->pmlDepthIndex[nn];
			out << "\n";
			out << "\t" << mRunlines[field][rr]->f_i << "\n";
			out << "\t" << mRunlines[field][rr]->f_j[0] << "\n";
			out << "\t" << mRunlines[field][rr]->f_j[1] << "\n";
			out << "\t" << mRunlines[field][rr]->f_k[0] << "\n";
			out << "\t" << mRunlines[field][rr]->f_k[1] << "\n";
		}
	}
}

MaterialPtr SimpleBulkPMLMaterialDelegate::
makeCalcMaterial(const VoxelizedPartition & vp, const CalculationPartition & cp)
    const
{
    cerr << "You shouldn't be here.  Overload for your material.";
    exit(1);
}


SimpleRunline::
SimpleRunline(const SBMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
}


SimplePMLRunline::
SimplePMLRunline(const SBPMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
    pmlIndex[0] = setupRunline.pmlDepthIndex[0];
    pmlIndex[1] = setupRunline.pmlDepthIndex[1];
    pmlIndex[2] = setupRunline.pmlDepthIndex[2];
}


SimpleAuxRunline::
SimpleAuxRunline(const SBMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
    auxIndex = setupRunline.auxIndex;
}


SimpleAuxPMLRunline::
SimpleAuxPMLRunline(const SBPMRunline & setupRunline) :
    fi(setupRunline.f_i.getPointer()),
    length(setupRunline.length)
{
    gj[0] = setupRunline.f_j[0].getPointer();
    gj[1] = setupRunline.f_j[1].getPointer();
    gk[0] = setupRunline.f_k[0].getPointer();
    gk[1] = setupRunline.f_k[1].getPointer();
    auxIndex = setupRunline.auxIndex;
    pmlIndex[0] = setupRunline.pmlDepthIndex[0];
    pmlIndex[1] = setupRunline.pmlDepthIndex[1];
    pmlIndex[2] = setupRunline.pmlDepthIndex[2];
}


ostream &
operator<<(std::ostream & str, const SimpleRunline & rl)
{
    /*
    str << hex << rl.fi << " " << rl.gj[0] << " " << rl.gj[1] << " "
        << rl.gk[0] << " " << rl.gk[1] << " " << dec << rl.length;
    */
    
    str << hex << rl.fi << ": " << MemoryBuffer::identify(rl.fi) << "\n";
    str << hex << rl.gj[0] << ": " << MemoryBuffer::identify(rl.gj[0]) << "\n";
    str << hex << rl.gj[1] << ": " << MemoryBuffer::identify(rl.gj[1]) << "\n";
    str << hex << rl.gk[0] << ": " << MemoryBuffer::identify(rl.gk[0]) << "\n";
    str << hex << rl.gk[1] << ": " << MemoryBuffer::identify(rl.gk[1]) << "\n";
    return str;
}

ostream &
operator<<(std::ostream & str, const SimplePMLRunline & rl)
{
    str << hex << rl.fi << " " << rl.gj[0] << " " << rl.gj[1] << " "
        << rl.gk[0] << " " << rl.gk[1] << " " << dec << rl.pmlIndex[0]
        << " " << rl.pmlIndex[1] << " " << rl.pmlIndex[2] << " " << rl.length;
    return str;
}

ostream &
operator<<(std::ostream & str, const SimpleAuxRunline & rl)
{
    str << hex << rl.fi << " " << rl.gj[0] << " " << rl.gj[1] << " "
        << rl.gk[0] << " " << rl.gk[1] << " " << dec << rl.auxIndex
        << " " << rl.length;
    return str;
}

ostream &
operator<<(std::ostream & str, const SimpleAuxPMLRunline & rl)
{
    str << hex << rl.fi << " " << rl.gj[0] << " " << rl.gj[1] << " "
        << rl.gk[0] << " " << rl.gk[1] << " " << dec << rl.auxIndex << " "
        << rl.pmlIndex[0] << " " << rl.pmlIndex[1] << " " << rl.pmlIndex[2]
        << " " << rl.length;
    return str;
}























