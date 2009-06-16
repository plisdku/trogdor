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
    //const Map<Vector3i, Map<string, string> > & gridPMLParams,
    const GridDescription & gridDesc,
	Paint* parentPaint)
{
	assert(parentPaint != 0L);
    
    MaterialDelegatePtr matDel;
	const MaterialDescPtr bulkMaterial = parentPaint->getBulkMaterial();
	string materialClass(bulkMaterial->getModelName());
	string materialName(bulkMaterial->getName());
    const Map<Vector3i, Map<string, string> > & gridPMLParams(
        gridDesc.getPMLParams());
    Map<Vector3i, Map<string, string> > pmlParams;
    
    //LOG << "Hey, grid pml: \n";
    //LOGMORE << gridPMLParams << endl;
    
    // This creates the map of PML parameters, first consulting the material's
    // parameters and secondarily the grid's default parameters and the global
    // default parameters.  In the usual case neither map will have anything,
    // because I will be confident in the parameters that I build in to the
    // program.  (-: (-: (-:
    if (parentPaint->isPML())
    {
        pmlParams = defaultPMLParams();
        const Map<Vector3i, Map<string, string> > & matParams =
            bulkMaterial->getPMLParams();
        
        for (int sideNum = 0; sideNum < 6; sideNum++)
        {
            Vector3i dir = cardinalDirection(sideNum);
            
            map<string, string>::const_iterator itr;
            for (itr = pmlParams[dir].begin(); itr != pmlParams[dir].end();
                itr++)
            {
                if (matParams.count(dir) && matParams[dir].count(itr->first))
                    pmlParams[dir][itr->first] =
                        matParams[dir][itr->first];
                else if (gridPMLParams.count(dir) &&
                    gridPMLParams[dir].count(itr->first))
                    pmlParams[dir][itr->first] =
                        gridPMLParams[dir][itr->first];
            }
        }
    }
    
	//LOG << "Getting delegate for " << *parentPaint << ".\n"; 
    
	if (materialClass == "StaticDielectric")
	{
        if (parentPaint->isPML())
            matDel = MaterialDelegatePtr(new StaticDielectricPMLDelegate(
                pmlParams));
        else
            matDel = MaterialDelegatePtr(new StaticDielectricDelegate);
	}
    else if (materialClass == "StaticLossyDielectric")
    {
        matDel = MaterialDelegatePtr(new StaticLossyDielectricDelegate);
    }
	else if (materialClass == "DrudeMetal")
	{
        matDel = MaterialDelegatePtr(new DrudeModel1Delegate(bulkMaterial));
	}
	else if (materialClass == "PerfectConductor")
	{
        matDel = MaterialDelegatePtr(new PerfectConductorDelegate);
	}
	else
    {
        LOG << "Using default (silly) delegate.\n";
        matDel = MaterialDelegatePtr(new SimpleBulkMaterialDelegate);
    }
    matDel->setParentPaint(parentPaint);
    
    return matDel;
}

Map<Vector3i, Map<string, string> > MaterialFactory::
defaultPMLParams()
{
    Map<string, string> allDirectionsDefault;
    Map<Vector3i, Map<string, string> > params;
    
    allDirectionsDefault["sigma"] =
        "(d^3)*0.8*4/(((mu0/eps0)^0.5)*dx)";
    allDirectionsDefault["alpha"] =
        "d*3e8*eps0/(50*dx)";
    allDirectionsDefault["kappa"] =
        "1 + (5-1)*(d^3)";
    /*
    allDirectionsDefault["kappa"] = "d";
    allDirectionsDefault["alpha"] = "d";
    allDirectionsDefault["sigma"] = "d"; // I'll take L to be the PML thickness
    */
    
    for (int sideNum = 0; sideNum < 6; sideNum++)
        params[cardinalDirection(sideNum)] = allDirectionsDefault;
    
    return params;
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
setNumCellsE(int fieldDir, int numCells)
{
}

void MaterialDelegate::
setNumCellsH(int fieldDir, int numCells)
{
}


void MaterialDelegate::
setPMLHalfCells(int pmlDir, Rect3i halfCellsOnSide,
    const GridDescription & gridDesc)
{
}

void MaterialDelegate::
setNumCellsOnPMLFaceE(int fieldDir, int faceNum, int numCells)
{
}
void MaterialDelegate::
setNumCellsOnPMLFaceH(int fieldDir, int faceNum, int numCells)
{
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
    // The start point of the runline *may* be outside the grid, *if* we are
    // performing calculations on ghost points!  This may happen in data-push
    // adjoint update equations.  In any case, usually the wrap does nothing.
    assert(vec_eq(vp.getGridHalfCells().p1, 0));
    Vector3i gridNumHalfCells = vp.getGridHalfCells().size() + 1;
    Vector3i wrappedStart = (mStartPoint+gridNumHalfCells) % gridNumHalfCells;
    assert(halfCellIndex(wrappedStart) == halfCellIndex(mStartPoint));
    Rect3i pmlYeeCells = rectHalfToYee(
        vp.getPMLHalfCells(mStartPaint->getPMLDirections()),
        halfCellIndex(wrappedStart));
    mCurrentRunline.pmlDepthIndex = vecHalfToYee(wrappedStart) - pmlYeeCells.p1;
    
    // TODO: test that this gives the correct PML if we wrapped to the far
    // side of the grid somehow.  (What am I talking about???)
    
    /*
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
    */
    
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
    
    // Notably differing from non-PML, the PML materials can't wrap to a new
    // y or z coordinate because that screws up the indexing into their
    // update constants.
    if (oldPos[1] != newPos[1] || oldPos[2] != newPos[2])
        return 0;
    
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
    /*
    str << hex << rl.fi << " " << rl.gj[0] << " " << rl.gj[1] << " "
        << rl.gk[0] << " " << rl.gk[1] << " " << dec << rl.pmlIndex[0]
        << " " << rl.pmlIndex[1] << " " << rl.pmlIndex[2] << " " << rl.length;
    */
    
    str << hex << rl.fi << ": " << MemoryBuffer::identify(rl.fi) << "\n";
    str << hex << rl.gj[0] << ": " << MemoryBuffer::identify(rl.gj[0]) << "\n";
    str << hex << rl.gj[1] << ": " << MemoryBuffer::identify(rl.gj[1]) << "\n";
    str << hex << rl.gk[0] << ": " << MemoryBuffer::identify(rl.gk[0]) << "\n";
    str << hex << rl.gk[1] << ": " << MemoryBuffer::identify(rl.gk[1]) << "\n";
    str << dec << rl.pmlIndex[0] << ", " << rl.pmlIndex[1] << ", "
        << rl.pmlIndex[2] << " length " << rl.length << "\n";
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























