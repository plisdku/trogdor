/*
 *  SetupGrid.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/14/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SetupGrid.h"

#include "SetupMaterialModel.h"
#include "SetupSource.h"
#include "SetupOutput.h"
#include "SetupInput.h"
#include "SetupLink.h"
#include "SetupTFSFBufferSet.h"
#include "SetupTFSFSource.h"

#include "GridWalker.h"
#include "ValidateSetupAttributes.h"
#include "StreamFromString.h"
#include "ObjFile.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <sstream>

#include <Magick++.h>
using namespace Magick;

using namespace std;

static Vector3i unitVectorFromString(string axisString);


SetupGrid::
SetupGrid(const TiXmlElement* gridElem,
    string & inName, int nnx, int nny, int nnz, Rect3i inActiveRegion,
    Rect3i inRegionOfInterest, bool doSaveBoundaries) :
    mName(inName),
    mActiveRegion(inActiveRegion),
    m_roi(inRegionOfInterest),
	m_nnx(nnx), m_nny(nny), m_nnz(nnz),
	m_nx((nnx+1)/2), m_ny((nny+1)/2), m_nz((nnz+1)/2),
	mStructureGrid(new StructureGrid(nnx, nny, nnz, inActiveRegion, 
		inRegionOfInterest)),
	mOutputs(),
	mInputs(),
	mSources(),
	mLinks(),
	mTFSFSources(),
	mAFPSources(),
	mBuffers(),
	mMaterials()
{	
	mStructureGrid = StructureGridPtr(new StructureGrid( nnx, nny, nnz,
		inActiveRegion, inRegionOfInterest) );
    
    //  Quick sanity check for the parameters of the grid.
    bool valid = 1;
    if (!getBounds().encloses(m_roi))
    {
        valid = 0;
        LOG << "ERROR: region of interest is bigger than the grid.\n";
    }
    if (!getBounds().encloses(mActiveRegion))
    {
        valid = 0;
        LOG << "ERROR: active region is bigger than the grid.\n";
    }
    if (!getBounds().encloses(m_roi))
    {
        valid = 0;
        LOG << "ERROR: region of interest extends outside active region.\n";
    }
    if (!valid)
    {
        cerr << "Error: StructureGrid bounds are invalid.\n";
        cerr << " - The active region must be contained within the grid.\n";
        cerr << " - The region of interest must be contained within the active"
             << " region.\n";
        cerr << " ( grid = " << getBounds() << "\n";
        cerr << "   activeRegion = " << mActiveRegion
             << "\n";
        cerr << "   roi = " << m_roi
             << "\n(line " << gridElem->Row() << ".)\n";
        exit(1);
    }
    
    readSetupMaterials(gridElem);
	readSetupSources(gridElem);
	readSetupTFSFSources(gridElem);
	readSetupOutputs(gridElem);
	readSetupInputs(gridElem);
	readSetupLinks(gridElem);
    readSetupAssembly(gridElem); // includes links and TF/SF
    
	if (!mStructureGrid->areCellsFilled())
	{
		cerr << "Some cells in " << mName << " are not filled.  Quitting."
			<< endl;
		LOG << "Died." << endl;
		exit(1);
	}	
	
	if (doSaveBoundaries)
		saveMaterialBoundariesBeta();
    
    insertPML();
	
    
    if (m_nnx <= 100 && m_nny <= 100 && m_nnz <= 10)
        mStructureGrid->print(cout);
}

SetupGrid::
SetupGrid(const SetupGrid & parentGrid,
	string inName,
	const Rect3i & copyFromRect, 
	const Rect3i & copyToRect,
	const Rect3i & boundingRectAndActiveRegion,
	const Rect3i & regionOfInterest,
	SetupTFSFSourcePtr auxSource) :
	mName(inName),
	mActiveRegion(boundingRectAndActiveRegion),
	m_roi(regionOfInterest),
	m_nnx(boundingRectAndActiveRegion.size(0)+1),
	m_nny(boundingRectAndActiveRegion.size(1)+1),
	m_nnz(boundingRectAndActiveRegion.size(2)+1),
	m_nx((m_nnx+1)/2), m_ny((m_nny+1)/2), m_nz((m_nnz+1)/2),
	mStructureGrid(new StructureGrid(m_nnx, m_nny, m_nnz, 
		boundingRectAndActiveRegion, regionOfInterest)),
	mOutputs(),
	mInputs(),
	mSources(),
	mLinks(),
	mTFSFSources(),
	mAFPSources(),
	mBuffers(),
	mMaterials(parentGrid.mMaterials)
	
{
	//LOG << "Interesting rects: \n";
	//LOGMORE << "copy from: " << copyFromRect << "\n";
	//LOGMORE << "copy to: " << copyToRect << "\n";
	//LOGMORE << "bounding etc.: " << boundingRectAndActiveRegion << "\n";
	//LOGMORE << "roi: " << regionOfInterest << "\n";
	
	mStructureGrid->getUnmodifiedMaterialsFrom(*(parentGrid.mStructureGrid));
	
	mTFSFSources.push_back(auxSource);
	
	// To make sure material boundaries come out right, E-field materials have
	// to match to E-field materials and H to H.  But if the grid has collapsed
	// in a dimension, we're guaranteed that the materials are all the same
	// in the original (clone, parent) grid in that direction, so we don't need
	// to impose this stringent %2 requirement.  In fact it may come back to
	// haunt you if you ever source a plane wave into a region that happens to
	// start on the "wrong" half-cell.
	//
	// Additionally, if the idea is to clone a single material through the
	// entire grid, the copyFromRect will be a single half-cell.  In this
	// case it is also unnecessary to do the %2 check.
	for (int nn = 0; nn < 3; nn++)
	if (copyToRect.p2[nn] - copyToRect.p1[nn] > 1 &&
		copyFromRect.p2[nn] > copyFromRect.p1[nn])
		assert(copyFromRect.p1[nn] % 2 == copyToRect.p1[nn] % 2);
	
	// Clone the materials in the clone box.  This prolly leaves some room
	// around the edges.
	Vector3i xx;  // position in child grid (copy to here)
	Vector3i yy;  // position in parent grid (copy from here)
	Vector3i displacement;
	Vector3i cloneSpan = copyFromRect.p2 - copyFromRect.p1 + 1;
	for (xx[0] = copyToRect.p1[0]; xx[0] <= copyToRect.p2[0]; xx[0]++)
	for (xx[1] = copyToRect.p1[1]; xx[1] <= copyToRect.p2[1]; xx[1]++)
	for (xx[2] = copyToRect.p1[2]; xx[2] <= copyToRect.p2[2]; xx[2]++)
	{
		displacement = xx - copyToRect.p1;
		displacement[0] = displacement[0] % cloneSpan[0];
		displacement[1] = displacement[1] % cloneSpan[1];
		displacement[2] = displacement[2] % cloneSpan[2];
		yy = copyFromRect.p1 + displacement;
		//LOG << " xx " << xx << " yy " << yy << " was " << 
		//	parentGrid.mStructureGrid->material(yy[0], yy[1], yy[2]) << "\n";
		mStructureGrid->material(xx[0], xx[1], xx[2]) = 
			parentGrid.mStructureGrid->material(yy[0], yy[1], yy[2]);
	}
	
	// Now fill in around the edges.
	for (xx[0] = mActiveRegion.p1[0]; xx[0] <= mActiveRegion.p2[0]; xx[0]++)
	for (xx[1] = mActiveRegion.p1[1]; xx[1] <= mActiveRegion.p2[1]; xx[1]++)
	for (xx[2] = mActiveRegion.p1[2]; xx[2] <= mActiveRegion.p2[2]; xx[2]++)
	{
		Vector3i p = clip(copyToRect, xx);
		mStructureGrid->material(xx[0],xx[1],xx[2]) =
			mStructureGrid->material(p[0], p[1], p[2]);
	}
	
	//mStructureGrid->print(cout);
	
	insertPML();
	
	//mStructureGrid->printMaterialTypes(cout);
	
	//assert(0);
	
    if (m_nnx <= 100 && m_nny <= 100 && m_nnz <= 10)
        mStructureGrid->print(cout);
}

SetupGrid::
~SetupGrid()
{
}

void SetupGrid::
addSource(SetupSourcePtr src)
{
	mSources.push_back(src);
}

void SetupGrid::
addOutput(SetupOutputPtr output)
{
	mOutputs.push_back(output);
}

void SetupGrid::
addAFPSource(SetupTFSFSourcePtr src)
{
	//	 First fill in the symmetries of this source region.  We need to do
	//	this now, before the StructureGrid gets all mungleld up with TFSF-
	//	modified materials.
	src->cacheGridSymmetries(mStructureGrid->getSymmetries(src->getTFRect()));
	mAFPSources.push_back(src);
}


void SetupGrid::
makeLinkBuffers(const Map<string, SetupGridPtr> & grids)
{
	LOGF << "Making link buffers.\n";
	
	for (int n = 0; n < mLinks.size(); n++)
	{
		SetupLinkPtr link = mLinks[n];
		
		if (link->getLinkType() != kTFType)
		{
			cerr << "Error: only TF type links are supported now.\n";
			LOGF << "Link error occurred here.\n";
			exit(1);
		}
		
		SetupGridPtr auxGrid = grids[link->getSourceGridName()];
		
		SetupTFSFBufferSet* pBuffer = new SetupTFSFBufferSet(*link, auxGrid,
			mActiveRegion);
		SetupTFSFBufferSetPtr buffer(pBuffer);
		mBuffers.push_back(buffer);
	}
}


void SetupGrid::
makeAFPBuffers()
{
	LOGF << "Making " << mAFPSources.size() << " AFP buffers.\n";
	
	for (int n = 0; n < mAFPSources.size(); n++)
	{
		SetupTFSFSourcePtr src = mAFPSources[n];
		
		SetupTFSFBufferSet* pBuffer = new SetupTFSFBufferSet(*src,
			mActiveRegion);
		SetupTFSFBufferSetPtr buffer(pBuffer);
		mBuffers.push_back(buffer);
	}
}

vmlib::SMat<3,bool> SetupGrid::
getSymmetries(const Rect3i & boundingRect) const
{
	return mStructureGrid->getSymmetries(boundingRect);
}

Vector3b SetupGrid::
getPeriodicDimensions(const Rect3i & boundingRect) const
{
	Vector3b periodicFlags(0, 0, 0);
	
	if (boundingRect.p2[0] - boundingRect.p1[0] == m_nnx-1)
		periodicFlags[0] = 1;
	if (boundingRect.p2[1] - boundingRect.p1[1] == m_nny-1)
		periodicFlags[1] = 1;
	if (boundingRect.p2[2] - boundingRect.p1[2] == m_nnz-1)
		periodicFlags[2] = 1;
	
	return periodicFlags;
}

Vector3b SetupGrid::
getSingularDimensions() const
{
	Vector3b singulars(m_nnx == 2, m_nny == 2, m_nnz == 2);
	return singulars;
}


void SetupGrid::
insertTFSF()
{
    TFSFType innerType, outerType;
	int ii, jj, kk;
	int material;
	MaterialType matType;
	vector<Vector3i> cardinals;
	cardinals.push_back(Vector3i(-1,0,0));
	cardinals.push_back(Vector3i(1,0,0));
	cardinals.push_back(Vector3i(0,-1,0));
	cardinals.push_back(Vector3i(0,1,0));
	cardinals.push_back(Vector3i(0,0,-1));
	cardinals.push_back(Vector3i(0,0,1));
	
    for (int n = 0; n < mBuffers.size(); n++)
    {
        if (mBuffers[n]->getTFSFType() == kTFType)
        {
            innerType = kTFType;
            outerType = kSFType;
        }
        else
        {
            innerType = kSFType;
            outerType = kTFType;
        }
        
        //SetupLinkPtr link = mLinks[n];
		SetupTFSFBufferSetPtr buffer = mBuffers[n];
        const Rect3i & tfRect = buffer->getTFRect();
        
		// A TFSF cell is labeled by a pointer to the buffer set and then on
		// each side by the indices of the buffers it is corrected on.
		
		// For each side of the buffer region that is not clipped by the
		// active region
		
		// determine the correction directions (which correction flags to use)
		// iterate over all cells on the outer boundary
		//		flag their types as TFSF
		//		get their current material types and add an appropriate flag
		//		save the new material types if necessary
		//	iterate over all cells on the inner boundary
		//		flag their types as TFSF
		//		get their current material types and add an appropriate flag
		//		save the new material types if necessary
		//  The inner boundary step may result in unused material types.
		
		int lowBufferIndex, highBufferIndex;
		int lowSide, highSide;
		for (int ndir = 0; ndir < 3; ndir++)
		{
			//	 Each direction has two sides, a positive and a negative.
			//	 Consequently there are two outer rects and two inner rects.
			//Vector3i & v1 = cardinals[2*ndir];
			//Vector3i & v2 = cardinals[2*ndir + 1];
			
			lowBufferIndex = 2*ndir;
			highBufferIndex = 2*ndir+1;
			lowSide = lowBufferIndex;
			highSide = highBufferIndex;
			
			//unsigned int lowFlag = 1 << 2*ndir;
			//unsigned int highFlag = 1 << (2*ndir+1);
			
			// Get the inner rects.  These will be on the SF side of a normal
			// TFSF boundary.
			Rect3i in1(buffer->getInnerBufferRect(lowBufferIndex));
			Rect3i in2(buffer->getInnerBufferRect(highBufferIndex));
			
			// Get the outer rects.  These will be on the TF side of a normal
			// TFSF boundary.
			Rect3i out1(buffer->getOuterBufferRect(lowBufferIndex));
			Rect3i out2(buffer->getOuterBufferRect(highBufferIndex));
			
			//LOG << in1 << "\n" << in2 << "\n" << out1 << "\n" << out2 << "\n";
			
			// Mark the outside cells.  Check if the edge of the outer rect is
			// in the active region.  (This is how the original algorithm
			// worked.)
			if (mActiveRegion.encloses(out1) && !buffer->omits(lowBufferIndex))
			{
				for (ii = out1.p1[0]; ii <= out1.p2[0]; ii++)
				for (jj = out1.p1[1]; jj <= out1.p2[1]; jj++)
				for (kk = out1.p1[2]; kk <= out1.p2[2]; kk++)
				{
					material = mStructureGrid->material(ii,jj,kk);
					if (material != 0)
					{
						matType = mStructureGrid->getMaterialType(material);
						matType.linkType() = outerType;
						matType.setBufferIndex(highSide, lowBufferIndex);
						//matType.bufferFlags() |= highFlag;
						matType.setBuffer(buffer);
						mStructureGrid->setMaterialType(matType, ii, jj, kk);
					}
				}
				
				for (ii = in1.p1[0]; ii <= in1.p2[0]; ii++)
				for (jj = in1.p1[1]; jj <= in1.p2[1]; jj++)
				for (kk = in1.p1[2]; kk <= in1.p2[2]; kk++)
				{
					material = mStructureGrid->material(ii,jj,kk);
					if (material != 0)
					{
						matType = mStructureGrid->getMaterialType(material);
						matType.linkType() = innerType;
						matType.setBufferIndex(lowSide, lowBufferIndex);
						//matType.bufferFlags() |= lowFlag;
						matType.setBuffer(buffer);
						mStructureGrid->setMaterialType(matType, ii, jj, kk);
					}
				}
			} // if (mActiveRegion.encloses(out1))
			else
				assert(buffer->omits(lowBufferIndex));
						
			if (mActiveRegion.encloses(out2) && !buffer->omits(highBufferIndex))
			{
				for (ii = out2.p1[0]; ii <= out2.p2[0]; ii++)
				for (jj = out2.p1[1]; jj <= out2.p2[1]; jj++)
				for (kk = out2.p1[2]; kk <= out2.p2[2]; kk++)
				{
					material = mStructureGrid->material(ii,jj,kk);
					if (material != 0)
					{
						matType = mStructureGrid->getMaterialType(material);
						matType.linkType() = outerType;
						matType.setBufferIndex(lowSide, highBufferIndex);
						//matType.bufferFlags() |= lowFlag;
						matType.setBuffer(buffer);
						mStructureGrid->setMaterialType(matType, ii, jj, kk);
					}
				}
				
				for (ii = in2.p1[0]; ii <= in2.p2[0]; ii++)
				for (jj = in2.p1[1]; jj <= in2.p2[1]; jj++)
				for (kk = in2.p1[2]; kk <= in2.p2[2]; kk++)
				{
					material = mStructureGrid->material(ii,jj,kk);
					if (material != 0)
					{
						matType = mStructureGrid->getMaterialType(material);
						matType.linkType() = innerType;
						matType.setBufferIndex(highSide, highBufferIndex);
						//matType.bufferFlags() |= highFlag;
						matType.setBuffer(buffer);
						mStructureGrid->setMaterialType(matType, ii, jj, kk);
					}
				}
			} // if (mActiveRegion.encloses(out2))
			else
				assert(buffer->omits(highBufferIndex));
			
		} // for ndir = 0:2
    } // foreach buffer
	
	//LOG << "Printing grid after TFSF insertion.\n";
	//mStructureGrid->print(cout);
}



const string & SetupGrid::
getName() const
{
    return mName;
}


Rect3i SetupGrid::
getBounds() const
{
    return Rect3i(0, 0, 0, m_nnx-1, m_nny-1, m_nnz-1);
}

Rect3d SetupGrid::
getYeeBounds() const
{
    return Rect3d(0, 0.5*m_nnx - 1, 0, 0.5*m_nny - 1, 0, 0.5*(m_nnz-1));
}

Rect3i SetupGrid::
getActiveRegion() const
{
    return mActiveRegion;
}

Rect3i SetupGrid::
getRegionOfInterest() const
{
    return m_roi;
}


const vector<SetupTFSFBufferSetPtr> & SetupGrid::
getBuffers() const
{
	return mBuffers;
}


const vector<SetupSourcePtr> & SetupGrid::
getSources() const
{
    return mSources;
}

const Map<string, SetupMaterialPtr> & SetupGrid::
getMaterials() const
{
    return mMaterials;
}

const std::vector<SetupTFSFSourcePtr> & SetupGrid::
getTFSFSources() const
{
	return mTFSFSources;
}

Pointer< vector<RunlineType> > SetupGrid::
makeMaterialRunlines() const
{
    //  Allocate the runlines to return
    vector<RunlineType> * ptr = new vector<RunlineType>;
    Pointer<vector<RunlineType> > runlines(ptr);
    
    LOGF << "Making runlines... " << flush;
    makeRunlines(*runlines, mActiveRegion,
        1, 0, 0);
    LOGFMORE << "(1, 0, 0)... " << flush;
    makeRunlines(*runlines, mActiveRegion,
        0, 1, 0);
    LOGFMORE << "(0, 1, 0)... " << flush;
    makeRunlines(*runlines, mActiveRegion,
        0, 0, 1);
    LOGFMORE << "(0, 0, 1)... " << flush;
    makeRunlines(*runlines, mActiveRegion,
        1, 1, 0);
    LOGFMORE << "(1, 1, 0)... " << flush;
    makeRunlines(*runlines, mActiveRegion,
        0, 1, 1);
    LOGFMORE << "(0, 1, 1)... " << flush;
    makeRunlines(*runlines, mActiveRegion,
        1, 0, 1);
    LOGFMORE << "(1, 0, 1)... done." << endl;
    
	/*
    for (int n = 0; n < runlines->size(); n++)
    {
        (*runlines)[n].print(cout);
    }
	*/
    
    
    return runlines;
}

const vector<SetupOutputPtr> & SetupGrid::
getOutputs() const
{
    return mOutputs;
}

const vector<SetupInputPtr> & SetupGrid::
getInputs() const
{
    return mInputs;
}

void SetupGrid::
addLink(SetupLinkPtr link)
{
	mLinks.push_back(link);
}


void SetupGrid::
writeAFPRequests(float dx, float dy, float dz, float dt, int numT) const
{
	// 1.  Simulation parameters
	// 2.  Source information (params, bounds)
	// 3.  Materials and parameters
	// 4.  Material sample (1D or 2D)
	// 5.  Required sample locations: Ex, Ey, Ez, Hx, Hy, Hz
	
	for (int nn = 0; nn < mAFPSources.size(); nn++)
	{
		const Pointer<SetupTFSFSource> ss = mAFPSources[nn];
		
		ostringstream str;
		str << "SOURCE_" << mName << "_" << nn << ".m";
		
		ofstream file;
		file.open(str.str().c_str());
		
		file << "clear afp;\n";
		file << "afp.dxdydzdt = [" << dx << " " << dy << " " << dz << " " << dt
			<< "];\n";
		file << "afp.numT = [" << numT << "];\n";
		
		file << "afp.class = '" << ss->getClass() << "';\n";
		
		file << "afp.tfRect = " << ss->getTFRect() << ";\n";
		if (ss->getFormula() != "")
			file << "afp.formula = '" << ss->getFormula() << "';\n";
		if (ss->getInputFile() != "")
			file << "afp.inputFile = '" << ss->getInputFile() << "';\n";
		
		if (ss->getClass() == "PlaneWave")
		{
			file << "afp.polarization = " << ss->getPolarization() << ";\n";
			file << "afp.sourceSymmetries = " << ss->getSymmetries() << ";\n";
			file << "afp.direction = " << ss->getDirection() << ";\n";
		}
		
		// Now write the materials and their parameters.
		int ii, jj, kk;
		int numMaterials = 0;
		Rect3i sampleRect = ss->getTFRect();
		const vmlib::SMat<3,bool> & gridSymmetries =
			ss->getCachedGridSymmetries();
		Vector3b sourceSymmetries = ss->getSymmetries();
		Vector3b periodicDimensions = getPeriodicDimensions(sampleRect);
		Vector3b combinedSymmetries(0,0,0);
		
		LOGF << "Grid symmetries are " << gridSymmetries << endl;
		LOGF << "Source symmetries are " << sourceSymmetries << endl;
		LOGF << "Periodic dimensions are " << periodicDimensions << endl;
		
		for (int nSym = 0; nSym < 3; nSym++)
		{
			// There are two conditions under which a dimension can be dropped.
			//	 1.  If the sourceRect has full X symmetry (which means that
			//		gridSymmetries(nSym,:) == 1) then the X direction collapses.
			//	 2.  If the sourceRect has X symmetry on some faces, but
			//      the remaining faces don't need TFSF correction, then that
			//		is good enough.  That condition is usually the same as
			//		having a periodic dimension.  (Warning: this is a kludge.)
			
			Vector3b symmetryOrPeriodicity(
				periodicDimensions[0] || gridSymmetries(nSym,0),
				periodicDimensions[1] || gridSymmetries(nSym,1),
				periodicDimensions[2] || gridSymmetries(nSym,2) );
			
			if (minval(symmetryOrPeriodicity) == 1) // if vector == (1 1 1)
				combinedSymmetries[nSym] = 1;
		}
		
		/*
		for (int nSym = 0; nSym < 3; nSym++)
		{
			combinedSymmetries[nSym] = gridSymmetries(nSym,0) &&
				gridSymmetries(nSym,1) && gridSymmetries(nSym,2);
		}
		*/
		
		LOGF << "Combined symmetries are " << combinedSymmetries << endl;
		
		for (int nn = 0; nn < 3; nn++)
		if (combinedSymmetries[nn])
			sampleRect.p2[nn] = sampleRect.p1[nn];
		
		// Determine all the materials in this chunk of grid.
		Map<int, int> tagToParent;
		Map<int, int> parentToIndex;
		vector<int> materials;
		for (kk = sampleRect.p1[2]; kk <= sampleRect.p2[2]; kk++)
		for (jj = sampleRect.p1[1]; jj <= sampleRect.p2[1]; jj++)
		for (ii = sampleRect.p1[0]; ii <= sampleRect.p2[0]; ii++)
		{
			int mat = mStructureGrid->material(ii,jj,kk);
			int parentTag;
			//cout << mat << "\n";
			if (tagToParent.count(mat) == 0)
			{
				const MaterialType & matType =
					mStructureGrid->getMaterialType(mat);
				if (matType.isTFSF())
					parentTag = mStructureGrid->getMaterialIndex(
						matType.getName());
				else
					parentTag = mat;
				
				tagToParent[mat] = parentTag;
			}
			
			parentTag = tagToParent[mat];
			if (parentToIndex.count(parentTag) == 0)
			{
				parentToIndex[parentTag] = numMaterials;
				materials.push_back(parentTag);
				numMaterials++;
			}
		}
		// Write the material descriptions
		for (int mm = 0; mm < numMaterials; mm++)
		{
			const MaterialType & matType = mStructureGrid->getMaterialType(
				materials[mm]);
			const SetupMaterialPtr setupMat = mMaterials[matType.getName()];
			file << "afp.materials{" << mm+1 << "}.class = '" << 
				setupMat->getClass() << "';\n";
			file << "afp.materials{" << mm+1 << "}.name = '" <<
				setupMat->getName() << "';\n";
			
			const Map<string, string> & params = setupMat->getParameters();
			Map<string, string>::const_iterator itr;
			for (itr = params.begin(); itr != params.end(); itr++)
			{
				file << "afp.materials{" << mm+1 << "}." << itr->first
					<< " = " << itr->second << ";\n";
			}
		}
		// Write the grid
		file << "afp.sampleBounds = " << sampleRect << ";\n";
		file << "afp.grid = [";
		for (kk = sampleRect.p1[2]; kk <= sampleRect.p2[2]; kk++)
		{
			file << "...\n";
			for (jj = sampleRect.p1[1]; jj <= sampleRect.p2[1]; jj++)
			{
				file << "\t";
				for (ii = sampleRect.p1[0]; ii <= sampleRect.p2[0]; ii++)
				{
					int mat = mStructureGrid->material(ii,jj,kk);
					int matInd = parentToIndex[tagToParent[mat]];
					file << matInd+1 << " ";
				}
				file << "...\n";
			}
		}
		file << "];\n";
		
		// Write the buffer points required, first in half cells, then in meters
		SetupTFSFBufferSet temporaryBuffer(*ss, mActiveRegion);
		
		// Make a list of parities of Ex, Ey, Ez, Hx, Hy and Hz
		vector<Vector3i> fieldParities;
		fieldParities.push_back(Vector3i(1,0,0));
		fieldParities.push_back(Vector3i(0,1,0));
		fieldParities.push_back(Vector3i(0,0,1));
		fieldParities.push_back(Vector3i(0,1,1));
		fieldParities.push_back(Vector3i(1,0,1));
		fieldParities.push_back(Vector3i(1,1,0));
		vector<string> fieldNames;
		fieldNames.push_back("Ex");
		fieldNames.push_back("Ey");
		fieldNames.push_back("Ez");
		fieldNames.push_back("Hx");
		fieldNames.push_back("Hy");
		fieldNames.push_back("Hz");
		
		int ff, gg;
		for (ff = 0; ff < 6; ff++)
		{
			file << "afp.yee"+fieldNames[ff] << " = [ ...\n";
			
			for (gg = 0; gg < 6; gg++)  // iterate over sides (-x, +x, etc)
			if (!temporaryBuffer.omits(gg))
			{
				Rect3i yeeBounds = temporaryBuffer.getYeeBufferRect(gg,
					fieldParities[ff]);
				for (kk = yeeBounds.p1[2]; kk <= yeeBounds.p2[2]; kk++)
				for (jj = yeeBounds.p1[1]; jj <= yeeBounds.p2[1]; jj++)
				for (ii = yeeBounds.p1[0]; ii <= yeeBounds.p2[0]; ii++)
					file << "\t[" << ii << ", " << jj << ", " << kk << "]; ...\n";
			}
			file << "];\n";
		}
		
		for (ff = 0; ff < 6; ff++)
		{
			file << "afp.pos"+fieldNames[ff] << " = [ ...\n";
			
			for (gg = 0; gg < 6; gg++)  // iterate over sides (-x, +x, etc)
			if (!temporaryBuffer.omits(gg))
			{
				Rect3i yeeBounds = temporaryBuffer.getYeeBufferRect(gg,
					fieldParities[ff]);
				for (kk = yeeBounds.p1[2]; kk <= yeeBounds.p2[2]; kk++)
				for (jj = yeeBounds.p1[1]; jj <= yeeBounds.p2[1]; jj++)
				for (ii = yeeBounds.p1[0]; ii <= yeeBounds.p2[0]; ii++)
				{
					file << "\t[" << dx*(ii+0.5*fieldParities[ff][0]) << ", "
						<< dy*(jj+0.5*fieldParities[ff][1]) << ", "
						<< dz*(kk+0.5*fieldParities[ff][2]) << "]; ...\n";
				}
			}
			file << "];\n";
		}
		
		file << "% end auto-generated file \n";
		
		file.close();
	}
}

void SetupGrid::
saveOutputCrossSections() const
{
    // iterate over SetupOutputs
    // find 2D outputs
    // spit out image overlays
    
	const StructureGrid & grid = *mStructureGrid;
	
    for (int nn = 0; nn < mOutputs.size(); nn++)
    {
        SetupOutputPtr output = mOutputs[nn];
        Rect3i bounds;
        
        if (output->getParameters().count("region") != 0)
        {
            output->getParameters()["region"] >> bounds;
            
			//LOG << "bounds are " << bounds << endl;
			
            if (bounds.numNonSingularDims() == 2)
            {
				//LOG << "printing.\n";
                int nCols, nRows;
                Vector3i row, col, origin, rowStart;
                
                // In order to draw the image in right-handed coordinates,
                // so the image as drawn looks like a cross-section through
                // xyz space, I set the "origin" to the top-left corner of
                // the image and proceed in scanline order.
                
                if (bounds.p1[0] == bounds.p2[0])
                {
                    nCols = bounds.p2[1] - bounds.p1[1] + 1;
                    nRows = bounds.p2[2] - bounds.p1[2] + 1;
                    
                    origin = Vector3i(bounds.p1[0], bounds.p1[1], bounds.p2[2]);
                    row = Vector3i(0,1,0);
                    col = Vector3i(0,0,-1);
                }
                else if (bounds.p1[1] == bounds.p2[1])
                {
                    nCols = bounds.p2[2] - bounds.p1[2] + 1;
                    nRows = bounds.p2[0] - bounds.p1[0] + 1;
                    
                    origin = Vector3i(bounds.p2[0], bounds.p1[1], bounds.p1[2]);
                    row = Vector3i(0,0,1);
                    col = Vector3i(-1,0,0);
                }
                else
                {
                    nCols = bounds.p2[0] - bounds.p1[0] + 1;
                    nRows = bounds.p2[1] - bounds.p1[1] + 1;
                    
                    origin = Vector3i(bounds.p1[0], bounds.p2[1], bounds.p1[2]);
                    row = Vector3i(1,0,0);
                    col = Vector3i(0,-1,0);
                }
                
                // zip-a-dee-doo-dah.
                Image image(Geometry(nCols, nRows), ColorRGB(1.0, 1.0, 1.0));
                
                Vector3i yeeComponent(0,0,0);
                string field = output->getParameters()["field"]; // if there...
                if (field == "ex")
                    yeeComponent = Vector3i(1,0,0);
                else if (field == "ey")
                    yeeComponent = Vector3i(0,1,0);
                else if (field == "ez")
                    yeeComponent = Vector3i(0,0,1);
                else if (field == "hx")
                    yeeComponent = Vector3i(0,1,1);
                else if (field == "hy")
                    yeeComponent = Vector3i(1,0,1);
                else if (field == "hz")
                    yeeComponent = Vector3i(1,1,0);
                
                // I am not writing edge pixels at the edge of the zone because
                // I am a first-rate juggler and acrobat extraordinnaire.
                Vector3i here;
                Vector3i v1, v2, v3, v4; // this is cheezy.
                for (int rr = 1; rr < nRows-2; rr++)
                {
                    //  rowStart and here are in half cells
                    rowStart = 2*origin + 2*rr*col + yeeComponent;
                    for (int cc = 1; cc < nCols-2; cc++)
                    {
                        int mat;
                        here = rowStart + 2*cc*row;
                        v1 = here + 2*col;
                        v2 = here - 2*col;
                        v3 = here + 2*row;
                        v4 = here - 2*row;
                        
                        mat = grid.material(here[0], here[1], here[2]);
                        
                        if (mat != grid.material(v1[0], v1[1], v1[2]) ||
                            mat != grid.material(v2[0], v2[1], v2[2]) ||
                            mat != grid.material(v3[0], v3[1], v3[2]) ||
                            mat != grid.material(v4[0], v4[1], v4[2]) )
                        {
                            image.pixelColor(cc, rr, "Black");
                        }
                    }
                }
                
                image.write(output->getFilePrefix()+".bmp");
            }
        }
    }
}


void SetupGrid::
saveMaterialBoundariesBeta() const
{
	ObjFile objFile;
	map<string, SetupMaterialPtr>::const_iterator itr;
	int ii,jj,kk;
	
	ostringstream foutname;
	foutname << getName() << "_faces.obj";
	ofstream fout(foutname.str().c_str());
	
	const StructureGrid& grid = *mStructureGrid;
	
	// We'll write the materials one at a time.
	for (itr = mMaterials.begin(); itr != mMaterials.end(); itr++)
	{
		string name = (*itr).first;
		SetupMaterialPtr mat = (*itr).second;
		int index = grid.getMaterialIndex(name);
		int lastMat, curMat;
		
		LOGF << "Considering material " << name << endl;
		
		//fout << name << endl;
		
		vector<OrientedRect3i> xFaces;
		vector<OrientedRect3i> yFaces;
		vector<OrientedRect3i> zFaces;
		
		// Do all the face determination stuff
		{
		// X-normal faces
		//	a.  Top layer
		ii = m_roi.p1[0];
		for (kk = m_roi.p1[2]; kk <= m_roi.p2[2]; kk++)
		for (jj = m_roi.p1[1]; jj <= m_roi.p2[1]; jj++)
		{
			curMat = grid.material(ii, jj, kk);
			if (curMat == index)
			{
				xFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii,jj+1,kk+1),
					Vector3i(-1, 0, 0)) );
			}
		} 
		//	b.  Bottom layer
		ii = m_roi.p2[0];
		for (kk = m_roi.p1[2]; kk <= m_roi.p2[2]; kk++)
		for (jj = m_roi.p1[1]; jj <= m_roi.p2[1]; jj++)
		{
			curMat = grid.material(ii, jj, kk);
			if (curMat == index)
			{
				xFaces.push_back( OrientedRect3i(
					Rect3i(ii+1,jj,kk,ii+1,jj+1,kk+1),
					Vector3i(1,0,0) ));
			}
		}
		//	c.  Middle (careful about X direction for-loop limits)
		for (ii = m_roi.p1[0]+1; ii <= m_roi.p2[0]; ii++)
		for (kk = m_roi.p1[2]; kk <= m_roi.p2[2]; kk++)
		for (jj = m_roi.p1[1]; jj <= m_roi.p2[1]; jj++)
		{
			lastMat = grid.material(ii-1, jj, kk);
			curMat = grid.material(ii, jj, kk);
			
			if (lastMat == index && curMat != index)
			{
				xFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii,jj+1,kk+1),
					Vector3i(1, 0, 0)) );
			}
			else if (lastMat != index && curMat == index)
			{
				xFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii,jj+1,kk+1),
					Vector3i(-1,0,0) ));
			}
		}
		
		// Y-normal faces
		//	a.  Top layer
		jj = m_roi.p1[1];
		for (ii = m_roi.p1[0]; ii <= m_roi.p2[0]; ii++)
		for (kk = m_roi.p1[2]; kk <= m_roi.p2[2]; kk++)
		{
			curMat = grid.material(ii, jj, kk);
			if (curMat == index)
			{
				yFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj,kk+1),
					Vector3i(0, -1, 0) ));
			}
		} 
		//	b.  Bottom layer
		jj = m_roi.p2[1];
		for (ii = m_roi.p1[0]; ii <= m_roi.p2[0]; ii++)
		for (kk = m_roi.p1[2]; kk <= m_roi.p2[2]; kk++)
		{
			curMat = grid.material(ii, jj, kk);
			if (curMat == index)
			{
				yFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj+1,kk,ii+1,jj+1,kk+1),
					Vector3i(0, 1, 0) ));
			}
		}
		//	c.  Middle (careful about X direction for-loop limits)
		for (jj = m_roi.p1[1]+1; jj <= m_roi.p2[1]; jj++)
		for (ii = m_roi.p1[0]; ii <= m_roi.p2[0]; ii++)
		for (kk = m_roi.p1[2]; kk <= m_roi.p2[2]; kk++)
		{
			lastMat = grid.material(ii, jj-1, kk);
			curMat = grid.material(ii, jj, kk);
			
			if (lastMat == index && curMat != index)
			{
				yFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj,kk+1),
					Vector3i(0, 1, 0) ));
			}
			else if (lastMat != index && curMat == index)
			{
				yFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj,kk+1),
					Vector3i(0, -1, 0) ));
			}
		}
		
		// Z-normal faces
		//	a.  Top layer
		kk = m_roi.p1[2];
		for (jj = m_roi.p1[1]; jj <= m_roi.p2[1]; jj++)
		for (ii = m_roi.p1[0]; ii <= m_roi.p2[0]; ii++)
		{
			curMat = grid.material(ii, jj, kk);
			if (curMat == index)
			{
				zFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj+1,kk),
					Vector3i(0, 0, -1) ));
			}
		} 
		//	b.  Bottom layer
		kk = m_roi.p2[2];
		for (jj = m_roi.p1[1]; jj <= m_roi.p2[1]; jj++)
		for (ii = m_roi.p1[0]; ii <= m_roi.p2[0]; ii++)
		{
			curMat = grid.material(ii, jj, kk);
			if (curMat == index)
			{
				zFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk+1,ii+1,jj+1,kk+1),
					Vector3i(0, 0, 1) ));
			}
		}
		//	c.  Middle (careful about X direction for-loop limits)
		for (kk = m_roi.p1[2]+1; kk <= m_roi.p2[2]; kk++)
		for (jj = m_roi.p1[1]; jj <= m_roi.p2[1]; jj++)
		for (ii = m_roi.p1[0]; ii <= m_roi.p2[0]; ii++)
		{
			lastMat = grid.material(ii, jj, kk-1);
			curMat = grid.material(ii, jj, kk);
			
			if (lastMat == index && curMat != index)
			{
				zFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj+1,kk),
					Vector3i(0, 0, 1) ));
			}
			else if (lastMat != index && curMat == index)
			{
				zFaces.push_back( OrientedRect3i(
					Rect3i(ii,jj,kk,ii+1,jj+1,kk),
					Vector3i(0, 0, -1) ));
			}
		}
		}
		// Now try to consolidate a little bit.		
		list<OrientedRect3i> xCompressed;
		list<OrientedRect3i> yCompressed;
		list<OrientedRect3i> zCompressed;
		
		consolidateRects(xFaces, xCompressed, 0);
		consolidateRects(yFaces, yCompressed, 1);
		consolidateRects(zFaces, zCompressed, 2);
		
		/*
		ostream_iterator<OrientedRect3i> fout_itr(fout, "\n");
		copy(xCompressed.begin(), xCompressed.end(), fout_itr);
		copy(yCompressed.begin(), yCompressed.end(), fout_itr);
		copy(zCompressed.begin(), zCompressed.end(), fout_itr);
		*/
		
		objFile.appendGroup(name, xCompressed);
		objFile.appendGroup(name, yCompressed);
		objFile.appendGroup(name, zCompressed);
	}
	
	objFile.write(fout, 0.5);  // The scale factor 0.5 undoes half-cells.
	fout.close();
}


void SetupGrid::
print(std::ostream & str) const
{
	mStructureGrid->print(str);
}

#pragma mark *** Private methods ***

void SetupGrid::
readSetupSources(const TiXmlElement* gridElem)
{
    LOGF << "readSetupMaterialsSourcesOutputs()..." << endl;
    
    const TiXmlElement* sourceElem;
    const TiXmlElement* linkElem;
    Map<string, string> attribs;
    Map<string, string> params;
    string errorMessage;
    
    
    //  read sources
    
    sourceElem = gridElem->FirstChildElement("Source");
    while (sourceElem)
    {
        attribs = getAttributes(sourceElem);
        params = getAttributes(sourceElem->FirstChildElement("Params"));
		
		if (attribs.count("class"))
		{
			cerr << "Warning: converting v4.4.1 source.  Validation of\n"
				"source attributes and parameters will be limited.\n";
			convertLegacySource(attribs, params);
		}
        else if (!validateSetupSource(attribs, errorMessage, sourceElem->Row()))
        {
            cerr << errorMessage;
            exit(1);
        }
        
		string formula;
		string filename;
		Field field;
		Vector3d polarization;
		Rect3i region;
		
		if (attribs.count("formula"))
			formula = attribs["formula"];
		if (attribs.count("file"))
			attribs["file"] >> filename;
		if (attribs.count("field"))
		{
			if (attribs["field"] == "electric")
				field = kElectric;
			else if (attribs["field"] == "magnetic")
				field = kMagnetic;
			else
			{
				assert(!"Need to convert old source format to new.");
			}
		}
		if (attribs.count("polarization"))
			attribs["polarization"] >> polarization;
		if (attribs.count("region"))
			attribs["region"] >> region;
		
		SetupSource* pSetSrc = new SetupSource(formula, filename, field,
			polarization, region, params);
		
		// This is the old-style constructor.
		/*
        SetupSource* pSetSrc = new SetupSource(attribs["formula"],
			attribs["formula"], params);
		*/
		
        mSources.push_back(SetupSourcePtr(pSetSrc));
        
        LOGF << "Reading source." << endl;
        
        sourceElem = sourceElem->NextSiblingElement("Source");
    }
}

void SetupGrid::
convertLegacySource(Map<string, string> & attribs,
	Map<string, string> & params)
{
	Map<string, string> oldAttribs(attribs);
	Map<string, string> oldParams(params);
	attribs.clear();
	params.clear();
	
	const string srcClass = oldAttribs["class"];
	
	assert(oldParams.count("extent"));
	attribs["region"] = oldParams["extent"];
	
	assert(oldParams.count("field"));
	string fieldNom = oldParams["field"];
	
	if (fieldNom[0] == 'e')
		attribs["field"] = "electric";
	else
		attribs["field"] = "magnetic";
    
    if (fieldNom == "ex")
		attribs["polarization"] = "1 0 0";
    else if (fieldNom == "ey")
		attribs["polarization"] = "0 1 0";
    else if (fieldNom == "ez")
		attribs["polarization"] = "0 0 1";
    else if (fieldNom == "hx")
		attribs["polarization"] = "1 0 0";
    else if (fieldNom == "hy")
		attribs["polarization"] = "0 1 0";
    else if (fieldNom == "hz")
		attribs["polarization"] = "0 0 1";
    else
    {
        cerr << "Invalid field: " << fieldNom << endl;
        exit(1);
    }
		
	if (srcClass == "GaussianSource")
	{
		assert(oldParams.count("amplitude"));
		assert(oldParams.count("sigma"));
		assert(oldParams.count("center"));
		
		params["A"] = oldParams["amplitude"];
		params["sigma"] = oldParams["sigma"];
		params["t0"] = oldParams["center"];
		
		attribs["formula"] = "A*exp(-1*((t-t0)/sigma)^2)";
	}
	else if (srcClass == "RaisedCosineSource")
	{
		cerr << "Warning: deprecated RaisedCosineSource is being replaced with "
			<< "a deprecated GaussianSource.\n";
		
		assert(oldParams.count("amplitude"));
		assert(oldParams.count("fullWidth"));
		assert(oldParams.count("center"));
		
		params["A"] = oldParams["amplitude"];
		params["sigma"] = oldParams["fullWidth"];
		params["t0"] = oldParams["center"];
		
		attribs["formula"] = "A*exp(-1*((t-t0)/sigma)^2)";
		
		/*
		params["A"] = oldParams["amplitude"];
		params["fW"] = oldParams["fullWidth"];
		params["t0"] = oldParams["center"];
		
		attribs["formula"] = "A*0.5*(1.0+cos(2*pi*(t-t0)/fW))";
		*/
	}
	else if (srcClass == "ModulatedGaussianSource")
	{
		assert(oldParams.count("amplitude"));
		assert(oldParams.count("sigma"));
		assert(oldParams.count("center"));
		assert(oldParams.count("omegaCenter"));
		
		params["A"] = oldParams["amplitude"];
		params["sigma"] = oldParams["sigma"];
		params["t0"] = oldParams["center"];
		params["w0"] = oldParams["omegaCenter"];
		
		attribs["formula"] = "A*sin((t-t0)*w0)*exp(-1*((t-t0)/sigma)^2)";
	}
	else if (srcClass == "RampedSineSource")
	{
		assert(oldParams.count("amplitude"));
		assert(oldParams.count("omega"));
		assert(oldParams.count("rampCycles"));
		
		
		// I'm going to use a logistic function for the ramp in this
		// approximation, since it's simple.  The ramp half-length will be
		// defined as the time it takes to go from ramp value of 0.25 to
		// 0.75.  The logistic function is
		//
		// L(x) = 1/(1+exp(-2*k*x))
		//
		// which goes from 1/4 to 3/4 in the interval kx = -0.549 to 0.549.
		//
		// If x goes from -1/4 rampCycles to 1/4 rampCycles (in timesteps),
		// then k = 0.549/(1/4 rampTime) =~ 2.2/rampTime.  rampTime will be
		// rampCycles*2*pi/omega.  So, k = 2.2*omega / (2*pi*rampCycles).
		//
		// The ramp-up will be translated so the source begins at value 0.001.
		// Then the x translation will be x0 = ln(999)/(2*k).
		
		float rampCycles;
		oldParams["rampCycles"] >> rampCycles;
		float omega;
		oldParams["omega"] >> omega;

		float k = 2.2*omega/(2*M_PI*rampCycles);
		float x0 = log(999)/(2*k);
		
		params["A"] = oldParams["amplitude"];
		params["omega"] = oldParams["omega"];
		
		ostringstream x0str;
		x0str << x0;
		params["t0"] = x0str.str();
		
		ostringstream kstr;
		kstr << k;
		params["k"] = kstr.str();
		//LOG << "k is " << k << " as '" << kstr.str() << "'.\n";
		
		if (oldParams.count("phaseDegrees"))
			params["phi"] = oldParams["phaseDegrees"];
		else
			params["phi"] = "0.0";
		
		attribs["formula"] =
			"A*sin(omega*(t-t0) + 2*pi*phi/180)/(1 + exp(-2*k*(t-t0)))";
	}
	else
	{
		LOG << "Source type error.\n";
		cerr << "Cannot convert source of type '" << srcClass << "'; class"
			" must be one of GaussianSource, RampedSineSource, "
			"ModulatedGaussianSource, or RaisedCosineSource.\n";
		assert(0);
		exit(1);
	}
	
}


void SetupGrid::
readSetupLinks(const TiXmlElement* gridElem)
{
	const TiXmlElement* linkElem;
    Map<string, string> attribs;
    Map<string, string> params;
	string errorMessage;

    //  read links
        linkElem = gridElem->FirstChildElement("Link");
    while (linkElem)
    {
        attribs = getAttributes(linkElem);
        
        if (!validateSetupLink(attribs, errorMessage, linkElem->Row()))
        {
            cerr << errorMessage;
            exit(1);
        }
        
        params = getAttributes(linkElem->FirstChildElement("Params"));
        
        TFSFType linkType;
        string srcName;
        
        if (attribs["type"] == "TF")
            linkType = kTFType;
        else if (attribs["type"] == "SF")
            linkType = kSFType;
        else
        {
            cerr << "Error: link attribute must be TF or SF, not '"
                 << attribs["type"] << "' (line " << linkElem->Row() << ").\n";
            exit(1);
        }
        
        Rect3i sourceRect, destRect;
        
        if (attribs.count("sourceRect"))
        {
            attribs["sourceRect"] >> sourceRect;
            sourceRect = 2*sourceRect;
            sourceRect.p2 += Vector3i(1,1,1);
        }
        else
            attribs["fineSourceRect"] >> sourceRect;
        
        if (attribs.count("destRect"))
        {
            attribs["destRect"] >> destRect;
            destRect = 2*destRect;
            destRect.p2 += Vector3i(1,1,1);
        }
        else
            attribs["fineDestRect"] >> destRect;
        
        srcName = attribs["sourceGrid"];
        
        SetupLink* pLink = new SetupLink(linkType,
            srcName, sourceRect, destRect);
        mLinks.push_back(SetupLinkPtr(pLink));
        
		set<Vector3i> omitSides;
		
        const TiXmlElement* omitElem = linkElem->FirstChildElement("OmitSide");
        while (omitElem)
        {
            Vector3i omitSide;
            omitElem->GetText() >> omitSide;
            pLink->omitSide(omitSide);
			omitSides.insert(omitSide);
            omitElem = omitElem->NextSiblingElement("OmitSide");
        }
		
		/*
		// New block for buffer
		vector<Vector3i> cardinals;
		cardinals.push_back(Vector3i(1,0,0));
		cardinals.push_back(Vector3i(-1,0,0));
		cardinals.push_back(Vector3i(0,1,0));
		cardinals.push_back(Vector3i(0,-1,0));
		cardinals.push_back(Vector3i(0,0,1));
		cardinals.push_back(Vector3i(0,0,-1));
		
		for (int nn = 1; nn < cardinals.size(); nn++)
		if (!omitSides.count(cardinals[nn]))
		{
			SetupBuffer* pBuffer = SetupBuffer::newGridBuffer(
				srcName, sourceRect, destRect, cardinals[nn]);
			mBuffers.push_back(SetupBufferPtr(pBuffer));
		}
		
		// end buffer block
		*/
        
        linkElem = linkElem->NextSiblingElement("Link");
    }
    
    if (!validateTFSFRegions(errorMessage, 2))
    {
        cerr << errorMessage;
        cerr << "(children of Grid element at line " << gridElem->Row() << ").";
        cerr << "\n";
        exit(1);
    }

}

void SetupGrid::
readSetupOutputs(const TiXmlElement* gridElem)
{
    const TiXmlElement* outputElem;
    Map<string, string> attribs;
    Map<string, string> params;
	string errorMessage;

    //  read outputs
    
    outputElem = gridElem->FirstChildElement("Output");
    while (outputElem)
    {
        attribs = getAttributes(outputElem);
        
        if (!validateSetupOutput(attribs, errorMessage, outputElem->Row()))
        {
            cerr << errorMessage;
            exit(1);
        }
        
        params = getAttributes(outputElem->FirstChildElement("Params"));
        
        if (attribs.count("filePrefix") == 0)
        {
            cerr << "Error: Outputs must have a filePrefix attribute, e.g.\n";
            cerr << "   <Output filePrefix=\"Ez_trace\">\n";
            cerr << "(" << " line " << outputElem->Row() << ".)";
            cerr << endl;
            exit(1);
        }
        
        if (attribs.count("class") == 0)
        {
            cerr << "Error: Outputs must have a class attribute, e.g.\n";
            cerr << "   <Output class=\"OneFieldOutput\">\n";
            cerr << "(" << " line " << outputElem->Row() << ".)";
            cerr << endl;
            exit(1);
        }
        
        int period = 1;
        if (attribs.count("period"))
            attribs["period"] >> period;
        
        SetupOutput* pSetOut = new SetupOutput(attribs["filePrefix"],
                                               attribs["class"],
                                               period,
                                               params);
        mOutputs.push_back(SetupOutputPtr(pSetOut));
        
        outputElem = outputElem->NextSiblingElement("Output");
    }
}

void SetupGrid::
readSetupMaterials(const TiXmlElement* gridElem)
{
    LOGF << "readSetupMaterialsSourcesOutputs()..." << endl;
    
    const TiXmlElement* materialElem;
    Map<string, string> attribs;
    Map<string, string> params;
    string errorMessage;
    
    //  read material models
    materialElem = gridElem->FirstChildElement("Material");
    while (materialElem)
    {
        attribs = getAttributes(materialElem);
        
        if (!validateSetupMaterial(attribs, errorMessage, materialElem->Row()))
        {
            cerr << errorMessage;
            exit(1);
        }
        
        params = getAttributes(materialElem->FirstChildElement("Params"));
        
        SetupMaterialModel* pMat = new SetupMaterialModel(attribs["name"],
                                                          attribs["class"],
                                                          params);
        mMaterials[attribs["name"]] = SetupMaterialPtr(pMat);
        
        materialElem = materialElem->NextSiblingElement("Material");
    }
}

void SetupGrid::
readSetupTFSFSources(const TiXmlElement* gridElem)
{
	const TiXmlElement* sourceElem;
    Map<string, string> attribs;
    Map<string, string> params;
	string errorMessage;
    //  read links
    
    sourceElem = gridElem->FirstChildElement("TFSFSource");
    while (sourceElem)
    {
        attribs = getAttributes(sourceElem);
        
        if (!validateSetupTFSFSource(attribs, errorMessage, sourceElem->Row()))
        {
            cerr << errorMessage;
            exit(1);
        }
        
        params = getAttributes(sourceElem->FirstChildElement("Params"));
                
        Rect3i tfRect;
        Vector3d direction;
		
        if (attribs.count("TFRect"))
        {
            attribs["TFRect"] >> tfRect;
            tfRect = 2*tfRect;
            tfRect.p2 += Vector3i(1,1,1);
        }
        else
            attribs["fineTFRect"] >> tfRect;
		
		attribs["direction"] >> direction;
		
		SetupTFSFSource* pTFSFSource = new SetupTFSFSource(attribs["class"],
			tfRect, direction, params);
		mTFSFSources.push_back(SetupTFSFSourcePtr(pTFSFSource));
		
        const TiXmlElement* omitElem = sourceElem->FirstChildElement("OmitSide");
        while (omitElem)
        {
            Vector3i omitSide;
            omitElem->GetText() >> omitSide;
            pTFSFSource->omitSide(omitSide);
            omitElem = omitElem->NextSiblingElement("OmitSide");
        }
        sourceElem = sourceElem->NextSiblingElement("TFSFSource");
    }
    /*
    if (!validateTFSFRegions(errorMessage, 2))
    {
        cerr << errorMessage;
        cerr << "(children of Grid element at line " << gridElem->Row() << ").";
        cerr << "\n";
        exit(1);
    }
	*/
}

void SetupGrid::
readSetupInputs(const TiXmlElement* gridElem)
{
	const TiXmlElement* inputElem;
    Map<string, string> attribs;
    Map<string, string> params;
	string errorMessage;
	
    // read inputs
        
    inputElem = gridElem->FirstChildElement("Input");
    while (inputElem)
    {
        attribs = getAttributes(inputElem);
        
        if (!validateSetupInput(attribs, errorMessage, inputElem->Row()))
        {
            cerr << errorMessage;
            exit(1);
        }
        
        params = getAttributes(inputElem->FirstChildElement("Params"));
                        
        SetupInput* pSetIn = new SetupInput(attribs["filePrefix"],
                                               attribs["class"],
                                               params);
        mInputs.push_back(SetupInputPtr(pSetIn));
        
        inputElem = inputElem->NextSiblingElement("Input");
    }

}void SetupGrid::
readSetupAssembly(const TiXmlElement* gridElem)
{
    string errorMessage;
    LOGF << "readSetupAssembly()..." << endl;
    
    const TiXmlElement* assemblyElem = gridElem->FirstChildElement("Assembly");
    
    if (!assemblyElem)
    {
        cerr << "Warning: no Assembly elements for StructureGrid.\n";
        return;
    }
    
    //  Step 1/3: Assemble the grid by loading auxiliary files or reading
    //  ascii data.
    const TiXmlElement* elem = assemblyElem->FirstChildElement();
    while (elem)
    {
        if (elem->ValueStr() == "Block")
        {
            LOGF << "Block assembly." << endl;
            assembleFromBlock(elem);
        }
		else if (elem->ValueStr() == "Ellipsoid")
		{
			LOGF << "Ellipsoid assembly." << endl;
			assembleFromEllipsoid(elem);
		}
        else if (elem->ValueStr() == "Ascii")
        {
            LOGF << "Ascii assembly." << endl;
            assembleFromAscii(elem);
        }
        else if (elem->ValueStr() == "KeyImage")
        {
            LOGF << "KeyImage assembly." << endl;
            assembleFromKeyImage(elem);
        }
        else if (elem->ValueStr() == "HeightMap")
        {
            LOGF << "HeightMap assembly." << endl;
            assembleFromHeightMap(elem);
        }
        elem = elem->NextSiblingElement();
    }
    LOGF << "readSetupAssembly() done." << endl;
}

void SetupGrid::
assembleFromBlock(const TiXmlElement* elem)
{
    Map<string, string> asciiAttribs;
    Rect3i fillRect;
    Rect3i halfCellRect;
    string materialName;
    string errorMessage;
    FillStyle style = kPECStyle;
    
    Map<char, string> tags;
    
    //  READ 1 of 1: attributes
    asciiAttribs = getAttributes(elem);
    if (!validateSetupAssemblyBlock(asciiAttribs, errorMessage, elem->Row()))
    {
        cerr << errorMessage;
        exit(1);
    }
    
    if (asciiAttribs.count("fillRect"))
    {
        asciiAttribs["fillRect"] >> fillRect;
        halfCellRect = 2*fillRect;
        halfCellRect.p2 += Vector3i(1,1,1);
        
        if (!getBounds().encloses(halfCellRect))
        {
            cerr << "Error: fillRect extends outside the grid:\n";
            cerr << "  fillRect = " << fillRect << "\n";
            cerr << "  grid     = " << getYeeBounds() << "\n";
            cerr << "(line " << elem->Row() << ".)\n";
            exit(1);
        }
    
        materialName = asciiAttribs["material"];
        
        if (asciiAttribs.count("fillStyle"))
        {
            if (asciiAttribs["fillStyle"] == "PMCStyle")
                style = kPMCStyle;
            else if (asciiAttribs["fillStyle"] == "PECStyle")
                style = kPECStyle;
            else
            {
                cerr << "Error: invalid fill style " << asciiAttribs["fillStyle"]
                    << "\n";
                cerr << "(line " << elem->Row() << ".)\n";
                exit(1);
            }
        }
        
        int materialIndex = mStructureGrid->materialIndex(materialName);
        
        for (int i = fillRect.p1[0]; i <= fillRect.p2[0]; i++)
        for (int j = fillRect.p1[1]; j <= fillRect.p2[1]; j++)
        for (int k = fillRect.p1[2]; k <= fillRect.p2[2]; k++)
        if (style == kPECStyle)
            mStructureGrid->setPECMaterialCube(materialIndex, i, j, k);
        else
            mStructureGrid->setPMCMaterialCube(materialIndex, i, j, k);
    }
    else if (asciiAttribs.count("fineFillRect"))
    {
        asciiAttribs["fineFillRect"] >> halfCellRect;
        if (!getBounds().encloses(halfCellRect))
        {
            cerr << "Error: fineFillRect extends outside the grid:\n";
            cerr << "  fineFillRect = " << halfCellRect << "\n";
            cerr << "  grid (half cells) = " << getBounds() << "\n";
            cerr << "(line " << elem->Row() << ".)\n";
            exit(1);
        }
        
        materialName = asciiAttribs["material"];
        int materialIndex = mStructureGrid->materialIndex(materialName);
        
        for (int ii = halfCellRect.p1[0]; ii <= halfCellRect.p2[0]; ii++)
        for (int jj = halfCellRect.p1[1]; jj <= halfCellRect.p2[1]; jj++)
        for (int kk = halfCellRect.p1[2]; kk <= halfCellRect.p2[2]; kk++)
            mStructureGrid->material(ii, jj, kk) = materialIndex;
    }
    else
        assert(!"Why does the Block have neither fillRect nor fineFillRect?");
}



void SetupGrid::
assembleFromAscii(const TiXmlElement* elem)
{
    const TiXmlElement* tagElem;
    const TiXmlElement* dataElem;
    Map<string, string> asciiAttribs;
    Rect3i fillRect;
    Rect3i halfCellRect;
    string data;
    string errorMessage;
    
    Map<char, string> tags;
    vector<char> tagsInOrder;
    vector<FillStyle> fillStylesInOrder;
    
    // -----------------------------
    //  READ from XML tree
    
    //  READ 1 of 3: attributes
    asciiAttribs = getAttributes(elem);
    if (!validateSetupAssemblyAscii(asciiAttribs, errorMessage, elem->Row()))
    {
        cerr << errorMessage;
        exit(1);
    }
    
    asciiAttribs["fillRect"] >> fillRect;
    halfCellRect = 2*fillRect;
    halfCellRect.p2 += Vector3i(1,1,1);
    if (!getBounds().encloses(halfCellRect))
    {
        cerr << "Error: fillRect extends outside the grid:\n";
        cerr << "  fillRect = " << fillRect << "\n";
        cerr << "  grid     = " << getYeeBounds() << "\n";
        cerr << "(line " << elem->Row() << ".)\n";
        exit(1);
    }
    
    //  READ 2 OF 3: Tag elements
    tagElem = elem->FirstChildElement("Tag");
    if (!tagElem)
    {
        cerr << "Ascii elements must have at least one tag element, e.g.\n";
        cerr << "  <Ascii>\n";
        cerr << "      <Tag symbol=\"f\" material=\"Free Space\" />\n";
        cerr << "      <Data>f</Data>\n";
        cerr << "  </Ascii>\n";
        cerr << "(line " << tagElem->Row() << ".)\n";
        exit(1);
    }
    while (tagElem)
    {
        Map<string, string> attribs = getAttributes(tagElem);
        if (attribs.count("symbol") == 0 || attribs.count("material") == 0)
        {
            cerr << "Tag elements must have symbol and material attributes, ";
            cerr << "e.g.\n";
            cerr << "   <Tag symbol=\"f\" material=\"Free Space\" />\n";
            cerr << "(line " << tagElem->Row() << ".)\n";
            exit(1);
        }
        if (attribs["symbol"].length() > 1)
        {
            cerr << "Symbol elements must be a single character.\n";
            cerr << "(line " << tagElem->Row() << ".)\n";
            exit(1);
        }
        
        if (attribs.count("fillStyle"))
        {
            if (attribs["fillStyle"] == "PMCStyle")
                fillStylesInOrder.push_back(kPMCStyle);
            else if (attribs["fillStyle"] == "PECStyle")
                fillStylesInOrder.push_back(kPECStyle);
            else
            {
                cerr << "Error: invalid fill style " << attribs["fillStyle"]
                    << "\n";
                cerr << "(line " << elem->Row() << ".)\n";
                exit(1);
            }
        }
        else
            fillStylesInOrder.push_back(kPECStyle);
        
        tags[attribs["symbol"][0]] = attribs["material"];
        tagsInOrder.push_back(attribs["symbol"][0]);
        
        tagElem = tagElem->NextSiblingElement("Tag");
    }
    
    //  READ 3 of 3: Data element
    dataElem = elem->FirstChildElement("Data");
    if (!dataElem)
    {
        cerr << "Ascii elements must have at least one Data element, e.g.\n";
        cerr << "  <Ascii>\n";
        cerr << "      <Tag symbol=\"f\" material=\"Free Space\" />\n";
        cerr << "      <Data>f</Data>\n";
        cerr << "  </Ascii>\n";
        cerr << "(line " << tagElem->Row() << ".)\n";
        exit(1);
    }
    else
    {
        data = dataElem->GetText();
        if (dataElem->NextSiblingElement("Data"))
        {
            cerr << "Warning: only the first Data element is valid.\n";
            cerr << "(line " << dataElem->NextSiblingElement("Data")->Row();
            cerr << "\n";
        }
    }
    
    // -----------------------------
    //  WRITE to StructureGrid
    //
    //  The filling rules specify that ASCII elements are painted to the grid
    //  in order of their material tag listings, so that listing free space
    //  before listing PEC will guarantee that the PEC ends up with proper
    //  boundary conditions.
    
    istringstream str(data);
    vector<string> rows;
    
    string row;
    while (getline(str, row))
    {
        //  For each line of data, strip out all the spaces and tabs, then save.
        while (row.find(" ") != string::npos)
            row.replace(row.find(" "), 1, "");
        while (row.find("\t") != string::npos)
            row.replace(row.find("\t"), 1, "");
        rows.push_back(row);
        //LOG << row << "\n";
    }
    assert(rows.size() != 0);
    reverse(rows.begin(), rows.end());  // flip into cartesian ordering
    
    //  Verify that each row of data has the same length now.
    int nCols = rows[0].length();
    int nRows = rows.size();
    for (int ii = 0; ii < rows.size(); ii++)
    {
        if (rows[ii].length() != nCols)
        {
            cerr << "Error: all rows of Ascii data must be the same length.\n";
            cerr << "(near line " << dataElem->Row() << ".)\n";
            exit(1);
        }
    }
    
    //LOG << "nRows " << nRows << " nCols " << nCols << "\n";
    
    //  Write into the grid.  This is tricky, because the data will repeat
    //  as much as necessary to fill in the FillRect.  Algorithm:
    //
    //  Define the axis unit vectors u1, u2, and u3
    //  For each row in rows[]
    //      
    
    //  define unit vectors
    Vector3i u1 = unitVectorFromString(asciiAttribs["row"]);
    Vector3i u2 = unitVectorFromString(asciiAttribs["column"]);
    Vector3i u3 = cross(u1, u2);
    if (u3 == Vector3i(0,0,0))
    {
        cerr << "Error: data row and column axes must be orthogonal and of";
        cerr << " the form\n";
        cerr << " \"x\", \"y\", \"z\", \"-x\", \"-y\", or \"-z\"\n";
        cerr << "u1 = " << u1 << "\n";
        cerr << "u2 = " << u2 << "\n";
        cerr << "u3 = " << u3 << "\n";
        cerr << "(line " << elem->Row() << ".)\n";
        exit(1);
    }
    
    //LOG << "Axes: " << u1 << " " << u2 << " " << u3 << endl;
    
    //  Iterate over the fill region
    //  n1, n2, n3 are the number of cells along the first, second and third
    //  axes in the StructureGrid.  That is: how many cells do we fill along
    //  u1, u2 and u3?  Then fillDiagonal just goes crosswise across the fill
    //  region from minimum corner to maximum corner.  It's just used to find
    //  the startPoint.
    int n1 = abs(dot(u1, fillRect.p2-fillRect.p1)) + 1;
    int n2 = abs(dot(u2, fillRect.p2-fillRect.p1)) + 1;
    int n3 = abs(dot(u3, fillRect.p2-fillRect.p1)) + 1;
    Vector3i fillDiagonal = fillRect.p2 - fillRect.p1; // n.b.: might be zero
    Vector3i startPoint; // the origin of the ascii data mapped to the grid.
    
    startPoint = fillRect.p1;
    if ( dot(u1, fillDiagonal) < 0 )
        startPoint -= (n1-1)*u1;
    if ( dot(u2, fillDiagonal) < 0 )
        startPoint -= (n2-1)*u2;
    if ( dot(u3, fillDiagonal) < 0 )
        startPoint -= (n3-1)*u3;
    
    //  While getting a material index for the material, we also suggest
    //  a symbol to the StructureGrid so its print() output will most
    //  closely resemble the Ascii input.  Do that here.
    Map<char, int> indexMap;
    for (map<char,string>::iterator itr = tags.begin();
         itr != tags.end(); itr++)
    {
        indexMap[(*itr).first] = mStructureGrid->materialIndex(
                                    (*itr).second, (*itr).first);
    }
    
    // Write into the grid in order of the tags, so boundary conditions can be
    // handled correctly.
    for (int tagN = 0; tagN < tagsInOrder.size(); tagN++)
    {
        char curAsciiTag = tagsInOrder[tagN];
        FillStyle style = fillStylesInOrder[tagN];
        
        //LOG << "tag " << curAsciiTag << " style is PEC? " << 
        //    (style == kPECStyle) << endl;
        
        for (int i2 = 0; i2 < n2; i2++) // in order over elements of rows[]
        {
            Vector3i gridRowStart = startPoint + u2*i2;
            
            for (int i1 = 0; i1 < n1; i1++) // in order over chars of rows[i2]
            {
                int matTag;
                char asciiTag;
                
                asciiTag = rows[i2%nRows][i1%nCols];
                if (asciiTag == curAsciiTag)  // each material is done in order.
                {
                    Vector3i uu = gridRowStart + i1*u1;
                    matTag = indexMap[asciiTag]; // unknown tags will map to 0
                    
                    for (int i3 = 0; i3 < n3; i3++) //  in order over the extra axis
                    {
                        Vector3i vv = uu + i3*u3;
                        if (style == kPECStyle)
                            mStructureGrid->setPECMaterialCube(matTag, vv[0],
                                vv[1], vv[2]);
                        else
                            mStructureGrid->setPMCMaterialCube(matTag, vv[0],
                                vv[1], vv[2]);
                    }
                }
            }
        }
    }
    
    for (map<char, int>::iterator itr = indexMap.begin();
         itr != indexMap.end(); itr++)
    {
        if ( (*itr).second == 0)
            cerr << "Warning, unknown material '" << (*itr).first << "'\n";
    }
}

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
Vector3i sConvertColor(const Color & inColor)
{
    Vector3i outColor;
    
#if (MagickLibVersion == 0x618)
    outColor[0] = ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = ScaleQuantumToChar(inColor.greenQuantum());
#else
    outColor[0] = MagickLib::ScaleQuantumToChar(inColor.redQuantum());
    outColor[1] = MagickLib::ScaleQuantumToChar(inColor.blueQuantum());
    outColor[2] = MagickLib::ScaleQuantumToChar(inColor.greenQuantum());
#endif

    return outColor;
}

void SetupGrid::
assembleFromKeyImage(const TiXmlElement* elem)
{
    const TiXmlElement* tagElem;
    Map<string, string> imageAttribs;
    Rect3i fillRect;
    Rect3i halfCellRect;
    string filename;
    string errorMessage;
    Image keyImage;
    
    Map<Vector3i, string> tags;   // tags[Vector3i(255, 255, 255)] = "Uranium"
    //                            from <Tag color="#FFFFFF" material="Uranium"/>
    vector<Vector3i> tagsInOrder;
    vector<FillStyle> fillStylesInOrder;
    
    // -----------------------------
    //  READ from XML tree
    
    //  READ 1 of 3: attributes
    imageAttribs = getAttributes(elem);
    if (!validateSetupAssemblyKeyImage(imageAttribs, errorMessage, elem->Row()))
    {
        cerr << errorMessage;
        exit(1);
    }
    
    imageAttribs["fillRect"] >> fillRect;
    halfCellRect = 2*fillRect;
    halfCellRect.p2 += Vector3i(1,1,1);
    if (!getBounds().encloses(halfCellRect))
    {
        cerr << "Error: fillRect extends outside the grid:\n";
        cerr << "  fillRect = " << fillRect << "\n";
        cerr << "  grid     = " << getYeeBounds() << "\n";
        cerr << "(line " << elem->Row() << ".)\n";
        exit(1);
    }
    
    //  READ 2 OF 3: Tag elements
    tagElem = elem->FirstChildElement("Tag");
    if (!tagElem)
    {
        cerr << "KeyImage elements must have at least one tag element, e.g.\n";
        cerr << "  <KeyImage>\n";
        cerr << "      <Tag color=\"#FFFFFF\" material=\"Free Space\" />\n";
        cerr << "  </KeyImage>\n";
        cerr << "(line " << tagElem->Row() << ".)\n";
        exit(1);
    }
    while (tagElem)
    {
        Map<string, string> attribs = getAttributes(tagElem);
        if (attribs.count("color") == 0 || attribs.count("material") == 0)
        {
            cerr << "Tag elements must have color and material attributes, ";
            cerr << "e.g.\n";
            cerr << "   <Tag color=\"#FFFFFF\" material=\"Free Space\" />\n";
            cerr << "(line " << tagElem->Row() << ".)\n";
            exit(1);
        }
        tags[sConvertColor(Color(attribs["color"]))] = attribs["material"];
        tagsInOrder.push_back(sConvertColor(Color(attribs["color"])));
        
        if (attribs.count("fillStyle"))
        {
            if (attribs["fillStyle"] == "PMCStyle")
                fillStylesInOrder.push_back(kPMCStyle);
            else if (attribs["fillStyle"] == "PECStyle")
                fillStylesInOrder.push_back(kPECStyle);
            else
            {
                cerr << "Error: invalid fill style " << attribs["fillStyle"]
                    << "\n";
                cerr << "(line " << elem->Row() << ".)\n";
                exit(1);
            }
        }
        else
            fillStylesInOrder.push_back(kPECStyle);
        
        //LOG << attribs["material"] << " is color " << attribs["color"] <<
        //    " represented by " << sConvertColor(Color(attribs["color"])) <<
        //    ".\n";
        
        tagElem = tagElem->NextSiblingElement("Tag"); 
    }
    
    
    filename = imageAttribs["file"];
    
    try
    {
        keyImage.read(filename);
        
        LOGF << "Read the image " << filename << endl;
        //  Enough goofing around.
        
        //  define unit vectors.  NOTE that u2 is REVERSED because the image
        //  columns begin at the TOP as is the graphics convention.
        Vector3i u1 = unitVectorFromString(imageAttribs["row"]);
        Vector3i u2 = -unitVectorFromString(imageAttribs["column"]);
        Vector3i u3 = cross(u1, u2);
        if (u3 == Vector3i(0,0,0))
        {
            cerr << "Error: data row and column axes must be orthogonal and of";
            cerr << " the form\n";
            cerr << " \"x\", \"y\", \"z\", \"-x\", \"-y\", or \"-z\"\n";
            cerr << "u1 = " << u1 << "\n";
            cerr << "u2 = " << u2 << "\n";
            cerr << "u3 = " << u3 << "\n";
            cerr << "(line " << elem->Row() << ".)\n";
            exit(1);
        }
        //LOG << "Axes: " << u1 << " " << u2 << " " << u3 << endl;
        
        //  iterate over the fill region
        int n1 = abs(dot(u1, fillRect.p2-fillRect.p1)) + 1;
        int n2 = abs(dot(u2, fillRect.p2-fillRect.p1)) + 1;
        int n3 = abs(dot(u3, fillRect.p2-fillRect.p1)) + 1;
        Vector3i fillDiagonal = fillRect.p2 - fillRect.p1; // n.b.: mb zero
        Vector3i startPoint; // the origin of the image data mapped to the grid.
        
        startPoint = fillRect.p1;
        if ( dot(u1, fillDiagonal) < 0 )
            startPoint -= (n1-1)*u1;
        if ( dot(u2, fillDiagonal) < 0 )
            startPoint -= (n2-1)*u2;
        if ( dot(u3, fillDiagonal) < 0 )
            startPoint -= (n3-1)*u3;
        
        //LOG << "n1 = " << n1 << " n2 = " << n2 << " n3 = " << n3 << "\n";
        //LOG << "startPoint = " << startPoint << "\n";
        
        //  n1 is the column index
        //  n2 is the row index
        
        if (keyImage.columns() % n1 != 0)
        {
            cerr << "Warning: key image has " << keyImage.columns() <<
                " columns but fill rect has " << n1 << " columns, so the"
                " key image will not tile an integral number of times.\n";
        }
        if (keyImage.rows() % n2 != 0)
        {
            cerr << "Warning: key image has " << keyImage.rows() << 
                " rows but fill rect has " << n2 << " rows, so the"
                " key image will not tile an integral number of times.\n";
        }
        
        Map<Vector3i, int> indexMap;
        for (map<Vector3i,string>::iterator itr = tags.begin();
             itr != tags.end(); itr++)
        {
            indexMap[(*itr).first] = mStructureGrid->materialIndex(
                (*itr).second);
        }
        
        for (int tagN = 0; tagN < tagsInOrder.size(); tagN++)
        {
            Vector3i curColor = tagsInOrder[tagN];
            FillStyle style = fillStylesInOrder[tagN];
                
            for (int column = 0; column < n1; column++)
            {
                Vector3i gridRowStart = startPoint + u1*column;
                //LOG << "row start " << gridRowStart << "\n";
                
                for (int row = 0; row < n2; row++)
                {
                    int matTag;
                    Vector3i hereColor;
                    
                    Vector3i uu = gridRowStart + row*u2;
                    
                    hereColor = sConvertColor(keyImage.pixelColor(column, row));
                    
                    if (hereColor == curColor)
                    {
                        
                        if (indexMap.count(hereColor) != 0)
                        {
                            matTag = indexMap[hereColor];
                            
                            for (int zed = 0; zed < n3; zed++)
                            {
                                Vector3i vv = uu + zed*u3;
                                
                                if (style == kPECStyle)
                                    mStructureGrid->setPECMaterialCube(matTag,
                                        vv[0], vv[1], vv[2]);
                                else
                                    mStructureGrid->setPMCMaterialCube(matTag,
                                        vv[0], vv[1], vv[2]);
                                //mStructureGrid->material(vv[0], vv[1], vv[2]) = matTag;
                                //LOG << "writing at " << vv << "\n";
                            }
                        }
                        else
                        {
                            LOG << "No index tag for " << hereColor << "\n";
                        }
                    }
                }
            }
        }

    }
    catch (Exception & exception)
    {
        cerr << "Error reading KeyImage.  Maybe a file type issue?\n";
        cerr << "Caught exception " << exception.what() << endl;
        LOG << "Exception logged.  Exiting.\n";
        exit(1);
    }
}



void SetupGrid::
assembleFromHeightMap(const TiXmlElement* elem)
{
    string errorMessage;
    Rect3i fillRect;
    Rect3i halfCellRect;
    string filename;
    string materialName;
    Image heightImage;
    int materialTag;
    FillStyle style;
    
    Map<string, string> attribs = getAttributes(elem);
    
    if (!validateSetupAssemblyHeightMap(attribs, errorMessage, elem->Row()))
    {
        cerr << errorMessage;
        exit(1);
    }
    
        
    attribs["fillRect"] >> fillRect;
    halfCellRect = 2*fillRect;
    halfCellRect.p2 += Vector3i(1,1,1);
    if (!getBounds().encloses(halfCellRect))
    {
        cerr << "Error: fillRect extends outside the grid:\n";
        cerr << "  fillRect = " << fillRect << "\n";
        cerr << "  grid     = " << getYeeBounds() << "\n";
        cerr << "(line " << elem->Row() << ".)\n";
        exit(1);
    }
    
    filename = attribs["file"];
    materialName = attribs["material"];
    materialTag = mStructureGrid->materialIndex(materialName);
    
    if (attribs.count("fillStyle"))
    {
        if (attribs["fillStyle"] == "PMCStyle")
            style = kPMCStyle;
        else if (attribs["fillStyle"] == "PECStyle")
            style = kPECStyle;
        else
        {
            cerr << "Error: invalid fill style " << attribs["fillStyle"]
                << "\n";
            cerr << "(line " << elem->Row() << ".)\n";
            exit(1);
        }
    }
    else
        style = kPECStyle;
    
    try
    {
        heightImage.read(filename);
        
        LOGF << "Read the image " << filename << endl;        
        //  Enough goofing around.
        
        //  define unit vectors.  NOTE that u2 is REVERSED because the image
        //  columns begin at the TOP as is the graphics convention.
        Vector3i u1 = unitVectorFromString(attribs["row"]);
        Vector3i u2 = -unitVectorFromString(attribs["column"]);
        Vector3i u3 = unitVectorFromString(attribs["up"]);
        if (dot(u1, u2) != 0 || dot(u2, u3) != 0 ||
            dot(u1, u3) != 0)
        {
            cerr << "Error: data row, column, and up axes must be orthogonal";
            cerr << " and of the form\n";
            cerr << " \"x\", \"y\", \"z\", \"-x\", \"-y\", or \"-z\"\n";
            cerr << "u1 = " << u1 << "\n";
            cerr << "u2 = " << u2 << "\n";
            cerr << "u3 = " << u3 << "\n";
            cerr << "(line " << elem->Row() << ".)\n";
            exit(1);
        }
        //LOG << "Axes: " << u1 << " " << u2 << " " << u3 << endl;
        
        //  iterate over the fill region
        int n1 = abs(dot(u1, fillRect.p2-fillRect.p1)) + 1;
        int n2 = abs(dot(u2, fillRect.p2-fillRect.p1)) + 1;
        int n3 = abs(dot(u3, fillRect.p2-fillRect.p1)) + 1;
        Vector3i fillDiagonal = fillRect.p2 - fillRect.p1; // n.b.: mb zero
        Vector3i startPoint; // the origin of the image data mapped to the grid.
        
        startPoint = fillRect.p1;
        if ( dot(u1, fillDiagonal) < 0 )
            startPoint -= (n1-1)*u1;
            //startPoint[0] = fillRect.p2[0];
        if ( dot(u2, fillDiagonal) < 0 )
            startPoint -= (n2-1)*u2;
            //startPoint[1] = fillRect.p2[1];
        if ( dot(u3, fillDiagonal) < 0 )
            startPoint -= (n3-1)*u3;
            //startPoint[2] = fillRect.p2[2];
        
        //LOG << "n1 = " << n1 << " n2 = " << n2 << " n3 = " << n3 << "\n";
        
        //  n1 is the column index
        //  n2 is the row index
        
        if (heightImage.columns() % n1 != 0)
        {
            cerr << "Warning: key image has " << heightImage.columns() <<
                " columns but fill rect has " << n1 << " columns, so the"
                " key image will not tile an integral number of times.\n";
        }
        
        if (heightImage.rows() % n2 != 0)
        {
            cerr << "Warning: key image has " << heightImage.rows() << 
                " rows but fill rect has " << n2 << " rows, so the"
                " key image will not tile an integral number of times.\n";
        }
        
        for (int column = 0; column < n1; column++)
        {
            Vector3i gridRowStart = startPoint + u1*column;
            //LOG << "row start " << gridRowStart << "\n";
            
            for (int row = 0; row < n2; row++)
            {
                int height;
                Vector3i uu = gridRowStart + row*u2;
                ColorGray hereColor;
                //LOG << "up column start " << uu << "\n";
                
                //  Question: does this return a grayscale color even if the
                //  image is RGB?  I don't know.
                hereColor = heightImage.pixelColor(column, row);
                
                height = int(double(n3)*hereColor.shade());
                
                //LOG << "coord " << uu << " height " << height << endl;
                /*
                LOG << "at coord " << uu << " to write " << string(
                    keyImage.pixelColor(column%keyImage.columns(),
                    row%keyImage.rows())) << "\n";
                */
                
                for (int zed = 0; zed < height; zed++)
                {
                    Vector3i vv = uu + zed*u3;
                    if (style == kPECStyle)
                        mStructureGrid->setPECMaterialCube(materialTag, vv[0],
                            vv[1], vv[2]);
                    else
                        mStructureGrid->setPMCMaterialCube(materialTag, vv[0],
                            vv[1], vv[2]);
                    //mStructureGrid->material(vv[0], vv[1], vv[2]) = materialTag;
                }
            }
        }

    }
    catch (Exception & exception)
    {
        cerr << "Error reading HeightMap.  Maybe a file type issue?\n";
        cerr << "Caught exception " << exception.what() << endl;
        LOG << "Exception logged.  Exiting.\n";
        exit(1);
    }
}


void SetupGrid::
assembleFromEllipsoid(const TiXmlElement* elem)
{
    Map<string, string> asciiAttribs;
    Rect3i fillRect;
    Rect3i halfCellRect;
    string materialName;
    string errorMessage;
    FillStyle style = kPECStyle;
    
    Map<char, string> tags;
    
    //  READ 1 of 1: attributes
    asciiAttribs = getAttributes(elem);
    if (!validateSetupAssemblyEllipsoid(asciiAttribs, errorMessage,  
		elem->Row()))
    {
        cerr << errorMessage;
        exit(1);
    }
    
    if (asciiAttribs.count("fillRect"))
    {
        asciiAttribs["fillRect"] >> fillRect;
        halfCellRect = 2*fillRect;
        halfCellRect.p2 += Vector3i(1,1,1);
        
        if (!getBounds().encloses(halfCellRect))
        {
            cerr << "Error: fillRect extends outside the grid:\n";
            cerr << "  fillRect = " << fillRect << "\n";
            cerr << "  grid     = " << getYeeBounds() << "\n";
            cerr << "(line " << elem->Row() << ".)\n";
            exit(1);
        }
    
        materialName = asciiAttribs["material"];
        
        if (asciiAttribs.count("fillStyle"))
        {
            if (asciiAttribs["fillStyle"] == "PMCStyle")
                style = kPMCStyle;
            else if (asciiAttribs["fillStyle"] == "PECStyle")
                style = kPECStyle;
            else
            {
                cerr << "Error: invalid fill style " << asciiAttribs["fillStyle"]
                    << "\n";
                cerr << "(line " << elem->Row() << ".)\n";
                exit(1);
            }
        }
        
        int materialIndex = mStructureGrid->materialIndex(materialName);
        
		Vector3i extent = fillRect.p2 - fillRect.p1 + Vector3i(1,1,1);
		Vector3i center2 = fillRect.p2 + fillRect.p1;
		Vector3d radii = 0.5*Vector3d(extent[0], extent[1], extent[2]);
		Vector3d center = 0.5*Vector3d(center2[0], center2[1], center2[2]);
		
        for (int i = fillRect.p1[0]; i <= fillRect.p2[0]; i++)
        for (int j = fillRect.p1[1]; j <= fillRect.p2[1]; j++)
        for (int k = fillRect.p1[2]; k <= fillRect.p2[2]; k++)
		{
			Vector3d v( Vector3d(i,j,k) - center );
			v[0] /= radii[0];
			v[1] /= radii[1];
			v[2] /= radii[2];
			
			//LOG << v << " has absSquared " << norm2(v) << endl;
			
			if (norm2(v) <= 1.0001)  // give it a little room for error
			{
				if (style == kPECStyle)
					mStructureGrid->setPECMaterialCube(materialIndex, i, j, k);
				else
					mStructureGrid->setPMCMaterialCube(materialIndex, i, j, k);
			}
		}
    }
    else if (asciiAttribs.count("fineFillRect"))
    {
        asciiAttribs["fineFillRect"] >> halfCellRect;
        if (!getBounds().encloses(halfCellRect))
        {
            cerr << "Error: fineFillRect extends outside the grid:\n";
            cerr << "  fineFillRect = " << halfCellRect << "\n";
            cerr << "  grid (half cells) = " << getBounds() << "\n";
            cerr << "(line " << elem->Row() << ".)\n";
            exit(1);
        }
        
        materialName = asciiAttribs["material"];
        int materialIndex = mStructureGrid->materialIndex(materialName);
		
		Vector3i extent = halfCellRect.p2 - halfCellRect.p1 + Vector3i(1,1,1);
		Vector3i center2(halfCellRect.p1 + halfCellRect.p2);
		Vector3d radii = 0.5*Vector3d(extent[1], extent[2], extent[3]);
		Vector3d center = 0.5*Vector3d(center2[1], center2[2], center2[3]);
		
        for (int ii = halfCellRect.p1[0]; ii <= halfCellRect.p2[0]; ii++)
        for (int jj = halfCellRect.p1[1]; jj <= halfCellRect.p2[1]; jj++)
        for (int kk = halfCellRect.p1[2]; kk <= halfCellRect.p2[2]; kk++)
		{
			Vector3d v( Vector3d(ii,jj,kk) - center );
			v[0] /= radii[0];
			v[1] /= radii[1];
			v[2] /= radii[2];
			
			if (norm2(v) <= 1.0001)  // give it a little room for error
				mStructureGrid->material(ii,jj,kk) = materialIndex;
		}
    }
    else
        assert(!"Why does the Ellipsoid have neither fillRect nor fineFillRect?");
}


void SetupGrid::
insertPML()
{
    set<OrientedPoint3i> innermostPML;
    set<OrientedPoint3i>::iterator iInPML;
    
    typedef pair<int, Vector3i> PMLType;  // (material, absorption direction)
    set<PMLType> pmlTypes;
    set<PMLType>::iterator iPMLTypes;
    
    Map<PMLType, int> pmlTags;
    
    //  1.  Get oriented outer boundary of the region of interest.  This is the
    //  set of all cells just outside the region of interest, clipped to the
    //  active region.
    getOrientedOuterBoundary(innermostPML, m_roi, mActiveRegion);
    
    
    //  2.  Assemble the set of PML types for the innermost PML cells, and mark
    //  them in the grid.
    for (iInPML = innermostPML.begin(); iInPML != innermostPML.end(); iInPML++)
    {
        Vector3i p = (*iInPML).first;   //  boundary cell (in PML)
        Vector3i v = (*iInPML).second; //  boundary cell normal (outward)
        Vector3i q = p - v;             //  boundary cell (in region of interest)
        int mat = mStructureGrid->material(q[0], q[1], q[2]);
        PMLType type(mat, v);
        
        //cout << "Cell " << p[0] << " " << p[1] << " " << p[2] << "\n";
        //cout << "Dir " << v[0] << " " << v[1] << " " << v[2] << "\n";
        //cout << "Match " << q[0] << " " << q[1] << " " << q[2] << "\n";
        //cout << "\n";
        
        //  If this is the first time we have needed this PML type, create
        //  a new MaterialType for it and register the appropriate tag in
        //  pmlTypes so we can refer to it later.
        
        if (mat != 0) // mat == 0 for material models not in the XML file!
        if (pmlTypes.count(type) == 0)
        {
            MaterialType parentMaterial = mStructureGrid->getMaterialType(mat);
            MaterialType myPML(parentMaterial, v);
            
            //mStructureGrid->printMaterialTypes(cout);
            
            pmlTypes.insert(type);
            pmlTags[type] = mStructureGrid->allocMaterialIndex(myPML);
            
            //LOG << "Inserting PML type for matID "
            //    << mat << " with tag " << mCharForIndex[pmlTags[type]] 
            //    << " (hex " << hex << int(mCharForIndex[pmlTags[type]]) 
            //    << dec << ")\n";
        }
        //  The whole innermost PML layer must have its material tags set to
        //  the appropriate PML material tag.
        mStructureGrid->material(p[0], p[1], p[2]) = pmlTags[type];
    }
    
    //  3.  All PML cells not in the innermost layer take the material tag
    //  of their nearest boundary PML type.  We do this the slow way: iterate
    //  over the ENTIRE grid to find the handful of PML cells at the edge.
    Rect3i innerPMLRect = inset(m_roi, -1, -1, -1, -1, -1, -1);
    for (int ii = mActiveRegion.p1[0]; ii <= mActiveRegion.p2[0]; ii++)
    for (int jj = mActiveRegion.p1[1]; jj <= mActiveRegion.p2[1]; jj++)
    for (int kk = mActiveRegion.p1[2]; kk <= mActiveRegion.p2[2]; kk++)
    {
        if (!m_roi.encloses(ii,jj,kk)) //  if cell is in the PML region
        {
            //  Clip the point (ii,jj,kk) to the innermost PML region to get
            //  the right material tag.
            Vector3i p = clip(innerPMLRect, Vector3i(ii,jj,kk));
            mStructureGrid->material(ii,jj,kk) =
                mStructureGrid->material(p[0], p[1], p[2]);
        }
    }
}


bool SetupGrid::
validateTFSFRegions(string & outErrorMessage, int differenceOrder) const
{
    //  1.  Make sure none of the TF/SF regions are adjacent/overlap.
    //  4.  Make sure none of the TF/SF regions touch the PML.
    //  5.  Make sure TF/SF regions are appropriately sized and aligned.
    //
    //  Precondition: we know that the ROI is enclosed by the active region,
    //  and that the active region is enclosed within the grid.
    
    bool valid = 1;
    vector<SetupLinkPtr>::const_iterator itr1;
    vector<SetupLinkPtr>::const_iterator itr2;
    Rect3i fatRegion;
    Rect3i otherRegion;
    Rect3i region;
    
    ostringstream str;
    
    //  1.
    for (itr1 = mLinks.begin(); itr1 != mLinks.end(); itr1++)
    for (itr2 = itr1+1; itr2 != mLinks.end(); itr2++)
    {
        otherRegion = (*itr2)->getDestRect();
		fatRegion = inset((*itr1)->getDestRect(), -1, -1, -1, -1, -1, -1);
		
		if (fatRegion.intersects( otherRegion ))
        {
            valid = 0;
            str << "Error: overlapping link regions:\n";
            str << "    " << (*itr1)->getDestRect() << "\n";
            str << "    " << (*itr2)->getDestRect() << "\n";
        }
    }
    
    //  4.  No touching the PML.  This is a tricky one, because if the ROI
    //  shares an edge with the active region, it's ok to push the TF or SF
    //  region right up to that edge.
    //
    //  The demarcated TF or SF region is an inner boundary.  One cell out
    //  from here is the abutting SF or TF region.  This is the region that
    //  must not overlap any PML; hence the demarcated TF or SF region itself
    //  must be *fully inside* the region of interest on all sides that have
    //  PML cells.
    //
    //  You are not expected to understand this.
    
    for (itr1 = mLinks.begin(); itr1 != mLinks.end(); itr1++)
    {
        region = (*itr1)->getDestRect();
        
        //LOG << mActiveRegion << "\n";
        //LOG << region << "\n";
        
        if (!( (mActiveRegion.encloses(region))
            && (m_roi.p1[0] < region.p1[0] || m_roi.p1[0] == mActiveRegion.p1[0])
            && (m_roi.p1[1] < region.p1[1] || m_roi.p1[1] == mActiveRegion.p1[1])
            && (m_roi.p1[2] < region.p1[2] || m_roi.p1[2] == mActiveRegion.p1[2])
            && (m_roi.p2[0] > region.p2[0] || m_roi.p2[0] == mActiveRegion.p2[0])
            && (m_roi.p2[1] > region.p2[1] || m_roi.p2[1] == mActiveRegion.p2[1])
            && (m_roi.p2[2] > region.p2[2] || m_roi.p2[2] == mActiveRegion.p2[2])) )
        {
            valid = 0;
            str << "Error: link regions must not overlap or touch PML.\n";
            str << "    " << region << "\n";
        }
    }
    
    //  5.  TFSF main rects must begin and end on the same field components
    //      as their auxiliary rects.  The auxiliary rects may either be the
    //      exact same size as the main field rects in all dimensions OR have
    //      singleton dimensions (reducing the grid to 1D or 2D, effectively).
    //      If the auxiliary rects have singleton dimensions, it is necessary
    //      that the auxiliary grid is also singleton, but that cannot be
    //      checked for here.
    
    Rect3i mainRect, auxRect;
    for (itr1 = mLinks.begin(); itr1 != mLinks.end(); itr1++)
    {
        mainRect = (*itr1)->getDestRect();
        auxRect = (*itr1)->getSourceRect();
        
        //  check alignments
        if (!( (mainRect.p1[0]%2 == auxRect.p1[0]%2)
            && (mainRect.p1[1]%2 == auxRect.p1[1]%2)
            && (mainRect.p1[2]%2 == auxRect.p1[2]%2)
            && (mainRect.p2[0]%2 == auxRect.p2[0]%2)
            && (mainRect.p2[1]%2 == auxRect.p2[1]%2)
            && (mainRect.p2[2]%2 == auxRect.p2[2]%2) ))
        {
            valid = 0;
            str << "Error: a link's source and destination rects must be ";
            str << "aligned on all sides to within n cells.\n";
            str << "    " << mainRect << "\n";
            str << "    " << auxRect << "\n";
        }
        
        //  check dimensions
        if (! ( (mainRect.p2[0] - mainRect.p1[0] == auxRect.p2[0] - auxRect.p1[0] ||
                 auxRect.p2[0] == auxRect.p1[0]+1)
             && (mainRect.p2[1] - mainRect.p1[1] == auxRect.p2[1] - auxRect.p1[1] ||
                 auxRect.p2[1] == auxRect.p1[1]+1)
             && (mainRect.p2[2] - mainRect.p1[2] == auxRect.p2[2] - auxRect.p1[2] ||
                 auxRect.p2[2] == auxRect.p1[2]+1) ) )
        {
            valid = 0;
            str << "Error: a link's source rect must have singular dimensions ";
            str << "or equal dimensions to the destination rect.\n";
            str << "    " << mainRect << "\n";
            str << "    " << auxRect << "\n";
        }
    }
    
    if (!valid)
        outErrorMessage = str.str();
    
    return valid;
}



void SetupGrid::
makeRunlines(vector<RunlineType> & outRunlines, Rect3i clipRect, int field_ii,
	int field_jj, int field_kk) const
{
    //  The purpose of ii0 et al. is to get the runline off on the right
    //  subcell.  Now use them to line up the edges of the clip region
    //  with the runlines.
    //LOG << "Field coords " << field_ii << " " << field_jj << " " << field_kk 
    //    << " clip to ";
    int ii0, jj0, kk0;
    ii0 = (field_ii % 2 == clipRect.p1[0] % 2) ? clipRect.p1[0] : clipRect.p1[0]+1;
    jj0 = (field_jj % 2 == clipRect.p1[1] % 2) ? clipRect.p1[1] : clipRect.p1[1]+1;
    kk0 = (field_kk % 2 == clipRect.p1[2] % 2) ? clipRect.p1[2] : clipRect.p1[2]+1;
    LOGFMORE << ii0 << " " << jj0 << " " << kk0 << "\n";
    
    MaterialType type;
    
    //  Utilize a "walker" to abstract away the uglies of building runlines.
    //  It marches down the grid one row at a time in the +x direction,
    //  returning runlines when they have been completed.
    GridWalker walker(mStructureGrid, ii0, jj0, kk0);
    
    for (int kk = kk0; kk <= clipRect.p2[2]; kk += 2)
    for (int jj = jj0; jj <= clipRect.p2[1]; jj += 2)
    {
        //  Begin a runline.  Get the current material, then begin a runline and
		//  start marching.
        type = mStructureGrid->getMaterialType(ii0, jj, kk);
                
        walker.beginRow();
        for (int ii = ii0; ii <= clipRect.p2[0]; ii += 2)
        {
            //  The walker keeps track of a few things -- current position in
            //  the main and linked grids, and how many cells of the current
            //  material have been seen.  This function updates this.
            walker.step(ii,jj,kk);
            
            if (walker.needNewRunline())
            {
                if (walker.hasRunline())
                {
                    outRunlines.push_back(walker.getRunline());
                    type = mStructureGrid->getMaterialType(ii, jj, kk);
                }
                
                walker.startNewRunline(type, ii, jj, kk);
            }
            
            walker.incrementLength();
        }
        outRunlines.push_back(walker.getRunline());
    }
}

#pragma mark *** Non-class methods ***


Map<string, string> getAttributes(const TiXmlElement* elem)
{
    Map<string, string> attribs;
    if (elem)
    {
        const TiXmlAttribute* attrib = elem->FirstAttribute();
        while (attrib)
        {
            attribs[attrib->Name()] = attrib->Value();
            attrib = attrib->Next();
        }
    }
    return attribs;
}

void SetupGrid::
getOrientedOuterBoundary( set<OrientedPoint3i> & outSet,
                          const Rect3i & inRegion,
                          const Rect3i & clipRect ) const
{
    int ii, jj, kk;
    
    Rect3i boundary = inset(inRegion, -1, -1, -1, -1, -1, -1);
    Vector3i direction;
    
    /*
    cout << "region " << inRegion.p1[0] << " " << inRegion.p1.y
         << " " << inRegion.p1[2] << " " << inRegion.p2[0] << " "
         << inRegion.p2[1] << " " << inRegion.p2[2] << "\n";
    */
    
    //  Create the set of ALL points just outside inRegion.
    //  This is done by the slowest way possible, but somehow it doesn't
    //  bother me to do this just once.
    
    set <Vector3i> outerPoints;
	
	int ii0, ii1, jj0, jj1, kk0, kk1;
	ii0 = boundary.p1[0];
	ii1 = boundary.p2[0];
	jj0 = boundary.p1[1];
	jj1 = boundary.p2[1];
	kk0 = boundary.p1[2];
	kk1 = boundary.p2[2];
	
	assert(vec_le(boundary.p1, boundary.p2));
	
	// speed factor 33.3% (spent in set insertion)
	// feb 29 2008
    for (ii = ii0; ii <= ii1; ii++)
    for (jj = jj0; jj <= jj1; jj++)
	{
		outerPoints.insert(Vector3i(ii,jj,kk0));
		outerPoints.insert(Vector3i(ii,jj,kk1));
	}
	
    for (ii = ii0; ii <= ii1; ii++)
    for (kk = kk0; kk <= kk1; kk++)
	{
		outerPoints.insert(Vector3i(ii,jj0,kk));
		outerPoints.insert(Vector3i(ii,jj1,kk));
	}
	
    for (jj = jj0; jj <= jj1; jj++)
    for (kk = kk0; kk <= kk1; kk++)
	{
		outerPoints.insert(Vector3i(ii0,jj,kk));
		outerPoints.insert(Vector3i(ii1,jj,kk));
	}
	
	// speed factor 7.7% (spent in set insertion)
	// feb 29 2008
	/*
    for (ii = ii0; ii <= ii1; ii++)
    for (jj = jj0; jj <= jj1; jj++)
    for (kk = kk0; kk <= kk1; kk++)
    {
        if (ii == ii0 || ii == ii1 ||
            jj == jj0 || jj == jj1 ||
            kk == kk0 || kk == kk1)
        {
            outerPoints.insert(Vector3i(ii,jj,kk));
        }
    }
	*/
	
	
	// speed factor 2.2% (spent in set insertion)
	// feb 29 2008
	/*
    for (ii = boundary.p1[0]; ii <= boundary.p2[0]; ii++)
    for (jj = boundary.p1[1]; jj <= boundary.p2[1]; jj++)
    for (kk = boundary.p1[2]; kk <= boundary.p2[2]; kk++)
    {
        if (ii == boundary.p1[0] || ii == boundary.p2[0] ||
            jj == boundary.p1[1] || jj == boundary.p2[1] ||
            kk == boundary.p1[2] || kk == boundary.p2[2])
        {
            outerPoints.insert(Vector3i(ii,jj,kk));
        }
    }
	*/
	
    //  For all those within clipRect, put them into outSet along with the
    //  direction from inRegion (the surface normal, more or less).
    for ( set<Vector3i>::iterator iPt = outerPoints.begin();
          iPt != outerPoints.end(); iPt++ )
    {
        if (clipRect.encloses(*iPt))
            outSet.insert(OrientedPoint3i(*iPt, *iPt - clip(inRegion, *iPt)));
    }
}


void SetupGrid::
consolidateRects(const vector<OrientedRect3i> & smallRects,
	list<OrientedRect3i> & bigRects, int normalIndex) const
{
	int nn;
	int ii = normalIndex;
	int jj = (ii+1)%3;
	int kk = (ii+2)%3;
	list<OrientedRect3i>::iterator itr, nextItr;
	
	if (smallRects.size() == 0)
		return;
	
	// First step: consolidate along the jj direction.  This is easy bkoz
	// small rects are not adjacent along j unless they actually at adjacent
	// indices.
	Rect3i curRect = smallRects[0].rect;
	Vector3i curNormal = smallRects[0].normal;
	for (nn = 1; nn < smallRects.size(); nn++)
	{
		const Rect3i & nextRect = smallRects[nn].rect;
		const Vector3i & nextNormal = smallRects[nn].normal;
		if (curRect.p1[ii] == nextRect.p1[ii] &&
			curRect.p2[jj] == nextRect.p1[jj] &&
			curRect.p1[kk] == nextRect.p1[kk] &&
			curRect.p2[kk] == nextRect.p2[kk] &&
			curNormal == nextNormal)
		{
			curRect.p2[jj] = nextRect.p2[jj];
		}
		else
		{
			bigRects.push_back(OrientedRect3i(curRect,curNormal));
			curRect = nextRect;
			curNormal = nextNormal;
		}
	}
	bigRects.push_back(OrientedRect3i(curRect, curNormal));
	
	// Second step: consolidate along the kk direction.  This is a little harder
	// bkoz there may be several non-abutting rects between mergeable rects.
	
	
	//LOG << "Now consolidate.\n";
	for (itr = bigRects.begin(); itr != bigRects.end(); itr++)
	{
		nextItr = itr;
		nextItr++;
		
		// While there IS another rect, while the rects are coplanar, and
		// while the rects might be adjacent in the k direction, continue.
		while (nextItr != bigRects.end() && 
			nextItr->rect.p1[ii] == itr->rect.p1[ii] &&
			nextItr->rect.p1[kk] <= itr->rect.p2[kk])
		{
			//LOG << *itr << " vs " << *nextItr << "\n";
			if (itr->rect.p2[kk] == nextItr->rect.p1[kk] &&
				itr->rect.p1[jj] == nextItr->rect.p1[jj] &&
				itr->rect.p2[jj] == nextItr->rect.p2[jj] &&
				itr->normal == nextItr->normal)
			{
				itr->rect.p2[kk] = nextItr->rect.p2[kk];  // the merge step
				bigRects.erase(nextItr++);  // this only works for list<>.
			}
			else
				nextItr++;
		}
	}
}


#pragma mark *** Local static functions ***

Vector3i unitVectorFromString(string axisString)
{
    Vector3i out(0,0,0);
    
    if (axisString == "x")
        out = Vector3i(1,0,0);
    else if (axisString == "-x")
        out = Vector3i(-1,0,0);
    else if (axisString == "y")
        out = Vector3i(0,1,0);
    else if (axisString == "-y")
        out = Vector3i(0, -1 ,0);
    else if (axisString == "z")
        out = Vector3i(0,0,1);
    else if (axisString == "-z")
        out = Vector3i(0,0,-1);
    else
        LOG << "Warning: bad axis string '" << axisString << "'.\n";
    return out;
}




