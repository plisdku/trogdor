/*
 *  StructureGrid.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/19/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "StructureGrid.h"

#include "SetupTFSFBufferSet.h"

#include "StreamFromString.h"

using namespace std;




StructureGrid::
StructureGrid(int in_nnx, int in_nny, int in_nnz, Rect3i activeRegion, Rect3i roi) :
    mIndexForMaterial(),
	mMaterialForIndex(),
	mCharForIndex(),
	mIndexForChar(),
	mNextMaterialIndex(0),
	mNextMaterialChar(0),
	mTagGrid(0L),
	m_nnx(in_nnx),
    m_nny(in_nny),
    m_nnz(in_nnz),
	m_nx((in_nnx+1)/2),
	m_ny((in_nny+1)/2),
	m_nz((in_nnz+1)/2),
    m_roi(roi),
    mActiveRegion(activeRegion)
{
    mNextMaterialIndex = 1;     //  index zero is reserved for nuthin' (undef)
    mNextMaterialChar = '!';    //  the first printable char after Space
    //mNextMaterialChar = 'A';
    mTagGrid = new int[m_nnx*m_nny*m_nnz];
    
    for (unsigned int n = 0; n < m_nnx*m_nny*m_nnz; n++)
        mTagGrid[n] = 0;
    
    
    //   Unknown materials will print as empty space.
    mCharForIndex[0] = ' ';
    mIndexForChar[' '] = 0;
}

StructureGrid::
~StructureGrid()
{
    LOGF << "destructor.\n";
    delete [] mTagGrid;
}


// this is such disgusting design. -- PCH June 2 2008
// even worse now. -- PCH June 5 2008
void StructureGrid::
getUnmodifiedMaterialsFrom(const StructureGrid & otherGrid)
{
	map<MaterialType, int>::const_iterator itr;
	
	for (itr = otherGrid.mIndexForMaterial.begin();
		itr != otherGrid.mIndexForMaterial.end();
		itr++)
	{
		const MaterialType & mat = (*itr).first;
		int index = (*itr).second;
		
		if (mat.isUnmodified())
		{
			mIndexForMaterial[mat] = index;
			mMaterialForIndex[index] = mat;
			mCharForIndex[index] = otherGrid.mCharForIndex[index];
			mIndexForChar[mCharForIndex[index]] = 
				otherGrid.mIndexForChar[mCharForIndex[index]];
			
			if (mNextMaterialIndex < index + 1)
				mNextMaterialIndex = index+1;
			
			if (mNextMaterialChar < mCharForIndex[index]+1)
				mNextMaterialChar = mCharForIndex[index]+1;
		}
	}
}


Rect3i StructureGrid::
getBounds () const
{
    return Rect3i(0, 0, 0, m_nnx-1, m_nny-1, m_nnz-1);
}

const Rect3i & StructureGrid::
getRegionOfInterest() const
{
    return m_roi;
}

const Rect3i & StructureGrid::
getActiveRegion() const
{
    return mActiveRegion;
}

int StructureGrid::
materialIndex(string & name)
{
    MaterialType type(name);
    if (mIndexForMaterial.count(type) == 0)
    {
        mIndexForMaterial[type] = mNextMaterialIndex;
        mMaterialForIndex[mNextMaterialIndex] = type;
        
        while (mIndexForChar.count(mNextMaterialChar) != 0)
            mNextMaterialChar++;
        mCharForIndex[mNextMaterialIndex] = mNextMaterialChar;
        mIndexForChar[mNextMaterialChar] = mNextMaterialIndex;
        
        mNextMaterialIndex++;
        mNextMaterialChar++;
    }
    return mIndexForMaterial[type];
}

int StructureGrid::
materialIndex(string & name, char suggestedSymbol)
{
    MaterialType type(name);
    if (mIndexForMaterial.count(type) == 0)
    {
        mIndexForMaterial[type] = mNextMaterialIndex;
        mMaterialForIndex[mNextMaterialIndex] = type;
        
        char matSymbol = suggestedSymbol;
        while (mIndexForChar.count(matSymbol) != 0)
            matSymbol++;
        mCharForIndex[mNextMaterialIndex] = matSymbol;
        mIndexForChar[matSymbol] = mNextMaterialIndex;
        
        mNextMaterialIndex++;
    }
    return mIndexForMaterial[type];

}

int StructureGrid::
allocMaterialIndex(MaterialType type)
{
    assert(mIndexForMaterial.count(type) == 0);
    
    mIndexForMaterial[type] = mNextMaterialIndex;
    mMaterialForIndex[mNextMaterialIndex] = type;
    
    while (mIndexForChar.count(mNextMaterialChar) != 0)
        mNextMaterialChar++;
    mCharForIndex[mNextMaterialIndex] = mNextMaterialChar;
    mIndexForChar[mNextMaterialChar] = mNextMaterialIndex;
    
    //LOG << "Allocating material char '" << mNextMaterialChar << "', hex "
    //    << hex << (int)mNextMaterialChar << dec << ".\n";
    
    mNextMaterialIndex++;
    mNextMaterialChar++;
    
    return mIndexForMaterial[type];
}

void StructureGrid::
setMaterialType(const MaterialType & type, int ii, int jj, int kk)
{
	int index;
	if (mIndexForMaterial.count(type) == 0)
		index = allocMaterialIndex(type);
	else
		index = mIndexForMaterial[type];
	
	material(ii,jj,kk) = index;
}


int StructureGrid::
getMaterialIndex(const std::string & name) const
{
	MaterialType type(name);
	if (mIndexForMaterial.count(type) != 0)
		return mIndexForMaterial[MaterialType(name)];
	else
		return -1;
}


const MaterialType & StructureGrid::
getMaterialType(int index) const
{
    if (mMaterialForIndex.count(index) != 0)
        return mMaterialForIndex[index];
	else
	{
		cerr << "Cannot find material with index " << index << "\n";
		LOG << "Error in getMaterial().\n";
		assert(0);
		exit(1);
	}
}


const MaterialType & StructureGrid::
getMaterialType(int ii, int jj, int kk) const
{
    int index = material(ii,jj,kk);
    if (mMaterialForIndex.count(index) != 0)
        return mMaterialForIndex[index];
	else
	{
		cerr << "Cannot get material type for " << ii << " " << jj << " "
			<< kk << " with tag " << index << "\n";
		LOG << "Error in getMaterialType().\n";
		assert(0);
		exit(1);
	}
}

char StructureGrid::
getMaterialChar(int materialIndex) const
{
    return mCharForIndex[materialIndex];
}


void StructureGrid::
setPECMaterialCube(int index, int i, int j, int k)
{
    //  yee cell center
    //int ii = 2*i+1;
    //int jj = 2*j+1;
    //int kk = 2*k+1;
    
    for (int ii = 2*i; ii <= 2*i+2; ii++)
    for (int jj = 2*j; jj <= 2*j+2; jj++)
    for (int kk = 2*k; kk <= 2*k+2; kk++)
    {
        materialWrapped(ii, jj, kk) = index;
    }
}

void StructureGrid::
setPMCMaterialCube(int index, int i, int j, int k)
{
    //  yee cell center
    //int ii = 2*i;
    //int jj = 2*j;
    //int kk = 2*k;
    
    for (int ii = 2*i-1; ii <= 2*i+1; ii++)
    for (int jj = 2*j-1; jj <= 2*j+1; jj++)
    for (int kk = 2*k-1; kk <= 2*k+1; kk++)
    {
        materialWrapped(ii, jj, kk) = index;
    }
}


//  Material array access
int& StructureGrid::
materialWrapped(int ii, int jj, int kk)
{
    if (ii < 0)
        ii += m_nnx;
    else if (ii >= m_nnx)
        ii -= m_nnx;
    if (jj < 0)
        jj += m_nny;
    else if (jj >= m_nny)
        jj -= m_nny;
    if (kk < 0)
        kk += m_nnz;
    else if (kk >= m_nnz)
        kk -= m_nnz;
    
    return mTagGrid[ii + m_nnx*jj + m_nnx*m_nny*kk];
}

int StructureGrid::
materialWrapped(int ii, int jj, int kk) const
{
    if (ii < 0)
        ii += m_nnx;
    else if (ii >= m_nnx)
        ii -= m_nnx;
    if (jj < 0)
        jj += m_nny;
    else if (jj >= m_nny)
        jj -= m_nny;
    if (kk < 0)
        kk += m_nnz;
    else if (kk >= m_nnz)
        kk -= m_nnz;
    
    return mTagGrid[ii + m_nnx*jj + m_nnx*m_nny*kk];
}

int& StructureGrid::
material(int ii, int jj, int kk)
{
    return mTagGrid[ii + m_nnx*jj + m_nnx*m_nny*kk];
}

int StructureGrid::
material(int ii, int jj, int kk) const
{
    return mTagGrid[ii + m_nnx*jj + m_nny*m_nnx*kk];
}

int StructureGrid::
yeeIndex(int ii, int jj, int kk) const
{
    assert(ii >= 0);
    assert(jj >= 0);
    assert(kk >= 0);
    assert(ii < m_nnx);
    assert(jj < m_nny);
    assert(kk < m_nnz);
    int index = ii/2 + (jj/2)*m_nx + (kk/2)*m_nx*m_ny;
    assert(index < m_nx*m_ny*m_nz);
    
    return index;
}

int StructureGrid::
nnx() const
{
    return m_nnx;
}

int StructureGrid::
nny() const
{
    return m_nny;
}

int StructureGrid::
nnz() const
{
    return m_nnz;
}

int StructureGrid::
nx() const
{
    return m_nx;
}

int StructureGrid::
ny() const
{
    return m_ny;
}

int StructureGrid::
nz() const
{
    return m_nz;
}


vmlib::SMat<3,bool> StructureGrid::
getSymmetries(const Rect3i & boundingRect) const
{
	vmlib::SMat<3,bool> symmetries;
	
	Vector3i unitVectors[3] = { Vector3i(1,0,0), Vector3i(0,1,0),
		Vector3i(0,0,1) };
	
	for (int rayDir = 0; rayDir < 3; rayDir++)
	{
		//LOG << "Starting rayDir = " << rayDir << endl;
		Vector3i & e_i = unitVectors[rayDir];
		Vector3i & e_j = unitVectors[(rayDir+1)%3];
		Vector3i & e_k = unitVectors[(rayDir+2)%3];
		const Vector3i & o = boundingRect.p1;
		
		//LOG << "Unit vectors are " << e_i << "\n" << e_j << "\n" << e_k << endl;
		//LOG << "Origin is " << o << endl;
		
		int ni = dot(boundingRect.p2 - boundingRect.p1, e_i) + 1;
		int nj = dot(boundingRect.p2 - boundingRect.p1, e_j) + 1;
		int nk = dot(boundingRect.p2 - boundingRect.p1, e_k) + 1;
		
		// Check ray-normal faces (the "front" and "back")
		bool faceSymmetrical = 1;
		Vector3i p, q;
		for (int jj = 0; jj < nj; jj++)
		for (int kk = 0; kk < nk; kk++)
		{
			p = o + jj*e_j + kk*e_k;
			q = p + (ni-1)*e_i;
			if (material(p[0], p[1], p[2]) != material(q[0], q[1], q[2]))
				faceSymmetrical = 0;
		}
		symmetries(rayDir, rayDir) = faceSymmetrical;
				
		// Check ray+1 faces (the "sides")
		bool sideSymmetrical = 1;
		for (int kk = 0; kk < nk-1; kk++)
		{
			Vector3i rayStart1 = o + kk*e_k;
			Vector3i rayStart2 = o + kk*e_k + (nj-1)*e_j;  // "the other side"
			
			int mat1 = material(rayStart1[0], rayStart1[1], rayStart1[2]);
			int mat2 = material(rayStart2[0], rayStart2[1], rayStart2[2]);
			
			for (int ii = 0; ii < ni-1; ii++)
			{
				p = rayStart1 + ii*e_i;
				q = rayStart2 + ii*e_i;
				if (material(p[0], p[1], p[2]) != mat1)
					sideSymmetrical = 0;
				if (material(q[0], q[1], q[2]) != mat2)
					sideSymmetrical = 0;
			}
		}
		symmetries(rayDir, (rayDir+1)%3) = sideSymmetrical;
		
		// Check ray+2 faces (the "other sides")
		sideSymmetrical = 1;
		for (int jj = 0; jj < nj-1; jj++)
		{
			Vector3i rayStart1 = o + jj*e_j;
			Vector3i rayStart2 = o + jj*e_j + (nk-1)*e_k;  // "the other side"
			
			int mat1 = material(rayStart1[0], rayStart1[1], rayStart1[2]);
			int mat2 = material(rayStart2[0], rayStart2[1], rayStart2[2]);
			
			for (int ii = 0; ii < ni-1; ii++)
			{
				p = rayStart1 + ii*e_i;
				q = rayStart2 + ii*e_i;
				if (material(p[0], p[1], p[2]) != mat1)
					sideSymmetrical = 0;
				if (material(q[0], q[1], q[2]) != mat2)
					sideSymmetrical = 0;
			}
		}
		symmetries(rayDir, (rayDir+2)%3) = sideSymmetrical;
	}
	
	return symmetries;
}


void StructureGrid::
print(ostream & str) const
{
	int ii;
    for (map<int, char>::const_iterator itr = mCharForIndex.begin();
         itr != mCharForIndex.end(); itr++)
    if ( (*itr).second != ' ')
    {
        MaterialType type = mMaterialForIndex[(*itr).first];
        
        str << (*itr).first << ", " << (*itr).second << ": " << type.getName();
        
        if ( type.isPML() )
            str << " (PML " << type.getDirection() << ")";
        else if (type.getLinkType() == kTFType)
		{
			str << " (TF,";
			for (ii = 0; ii < 6; ii++)
				str << " " << type.getBufferIndex(ii);
			str << ")";
		}
        else if (type.getLinkType() == kSFType)
		{
			str << " (SF,";
			for (ii = 0; ii < 6; ii++)
				str << " " << type.getBufferIndex(ii);
			str << ")";
		}
        
        str << "\n";
    }
    
    for (int kk = 0; kk < m_nnz; kk++)
    {
        str << "Z = " << kk << "\n";
        for (int jj = m_nny-1; jj >= 0; jj--)
        {
            if (jj % 10 == 0)
                str << setw(5) << std::left << jj;
            else
                str << "     ";
            for (ii = 0; ii < m_nnx; ii++)
            {
                if (ii%10 == 0)
                    str << " ";
                str << mCharForIndex[material(ii,jj,kk)];
            }
            str << "\n";
        }
        str << "\n";
    }
}

void StructureGrid::
printMaterialTypes(std::ostream & str) const
{
    str << "Types:\n";
    map<MaterialType, int>::const_iterator itr;
    for (itr = mIndexForMaterial.begin(); itr != mIndexForMaterial.end(); itr++)
    {
        str << "Index " << (*itr).second << " " << (*itr).first.getName()
            << ": PML = " << (*itr).first.isPML() << ", TFSF = "
            << (*itr).first.isTFSF() << " ";
        if ((*itr).first.isPML())
        {
            str << "( PML direction " << (*itr).first.getDirection() << ")\n";
        }
        else
            str << "\n";
    }
}


bool StructureGrid::
areCellsFilled() const
{
	bool allFilled = 1;
	// Determine if every cell in the grid has a defined material in it.
	/*
	for (int kk = mActiveRegion.p1[2]; kk <= mActiveRegion.p2[2]; kk++)
	for (int jj = mActiveRegion.p1[1]; jj <= mActiveRegion.p2[1]; jj++)
	for (int ii = mActiveRegion.p1[0]; ii <= mActiveRegion.p2[0]; ii++)
	*/
	for (int kk = m_roi.p1[2]; kk <= m_roi.p2[2]; kk++)
	for (int jj = m_roi.p1[1]; jj <= m_roi.p2[1]; jj++)
	for (int ii = m_roi.p1[0]; ii <= m_roi.p2[0]; ii++)
	if (material(ii,jj,kk) <= 0)
	{
		LOG << "No material at halfcell " << ii << " " << jj << " " << kk
			<< "\n";
		allFilled = 0;
	}
	return allFilled;
}



/*
void StructureGrid::
assertSafeTFSFRegions(const vector<TFSFRegion> & inTFRegions,
                      const vector<TFSFRegion> & inSFRegions)
{
    //  1.  Make sure none of the TF regions are adjacent/overlap.
    //  2.  Make sure none of the SF regions are adjacent/overlap.
    //  3.  Make sure none of the TF/SF regions are adjacent/overlap.
    //  4.  Make sure none of the TF/SF regions touch the PML.
    //  5.  Make sure TF/SF regions are appropriately sized and aligned.
    //
    //  Precondition: we know that the ROI is enclosed by the active region,
    //  and that the active region is enclosed within the grid.
    
    vector<TFSFRegion>::const_iterator itr1;
    vector<TFSFRegion>::const_iterator itr2;
    Rect3i fatRegion;
    Rect3i otherRegion;
    Rect3i region;
    
    //  1.
    for (itr1 = inTFRegions.begin(); itr1 != inTFRegions.end(); itr1++)
    for (itr2 = itr1+1; itr2 != inTFRegions.end(); itr2++)
    {
        otherRegion = (*itr2).getMainRect();
        fatRegion = inset((*itr1).getMainRect(), -1, -1, -1, -1, -1, -1);
        assert(!fatRegion.intersects( otherRegion ));
    }
    
    //  2.
    for (itr1 = inSFRegions.begin(); itr1 != inSFRegions.end(); itr1++)
    for (itr2 = itr1+1; itr2 != inSFRegions.end(); itr2++)
    {
        otherRegion = (*itr2).getMainRect();
        fatRegion = inset((*itr1).getMainRect(), -1, -1, -1, -1, -1, -1);
        assert(!fatRegion.intersects( otherRegion ));
    }
    
    //  3.
    for (itr1 = inTFRegions.begin(); itr1 != inTFRegions.end(); itr1++)
    for (itr2 = inSFRegions.begin(); itr2 != inSFRegions.end(); itr2++)
    {
        otherRegion = (*itr2).getMainRect();
        fatRegion = inset((*itr1).getMainRect(), -1, -1, -1, -1, -1, -1);
        assert(!fatRegion.intersects( otherRegion ));
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
    
    for (itr1 = inTFRegions.begin(); itr1 != inTFRegions.end(); itr1++)
    {
        region = (*itr1).getMainRect();
        
        log.prefix(__LINE__) << mActiveRegion << "\n";
        log.prefix(__LINE__) << region << "\n";
        assert(mActiveRegion.encloses(region));
        assert(m_roi.p1[0] < region.p1[0] || m_roi.p1[0] == mActiveRegion.p1[0]);
        assert(m_roi.p1[1] < region.p1[1] || m_roi.p1[1] == mActiveRegion.p1[1]);
        assert(m_roi.p1[2] < region.p1[2] || m_roi.p1[2] == mActiveRegion.p1[2]);
        assert(m_roi.p2[0] > region.p2[0] || m_roi.p2[0] == mActiveRegion.p2[0]);
        assert(m_roi.p2[1] > region.p2[1] || m_roi.p2[1] == mActiveRegion.p2[1]);
        assert(m_roi.p2[2] > region.p2[2] || m_roi.p2[2] == mActiveRegion.p2[2]);
    }
    
    for (itr1 = inSFRegions.begin(); itr1 != inSFRegions.end(); itr1++)
    {
        region = (*itr1).getMainRect();
        
        assert(mActiveRegion.encloses(region));
        assert(m_roi.p1[0] < region.p1[0] || m_roi.p1[0] == mActiveRegion.p1[0]);
        assert(m_roi.p1[1] < region.p1[1] || m_roi.p1[1] == mActiveRegion.p1[1]);
        assert(m_roi.p1[2] < region.p1[2] || m_roi.p1[2] == mActiveRegion.p1[2]);
        assert(m_roi.p2[0] > region.p2[0] || m_roi.p2[0] == mActiveRegion.p2[0]);
        assert(m_roi.p2[1] > region.p2[1] || m_roi.p2[1] == mActiveRegion.p2[1]);
        assert(m_roi.p2[2] > region.p2[2] || m_roi.p2[2] == mActiveRegion.p2[2]);
    }
    
    //  5.  TFSF main rects must begin and end on the same field components
    //      as their auxiliary rects.  The auxiliary rects may either be the
    //      exact same size as the main field rects in all dimensions OR have
    //      singleton dimensions (reducing the grid to 1D or 2D, effectively).
    //      If the auxiliary rects have singleton dimensions, it is necessary
    //      that the auxiliary grid is also singleton, but that cannot be
    //      checked for here.
    
    Rect3i mainRect, auxRect;
    for (itr1 = inTFRegions.begin(); itr1 != inTFRegions.end(); itr1++)
    {
        mainRect = (*itr1).getMainRect();
        auxRect = (*itr1).getAuxRect();
        
        //  check alignments
        assert(mainRect.p1[0]%2 == auxRect.p1[0]%2);
        assert(mainRect.p1[1]%2 == auxRect.p1[1]%2);
        assert(mainRect.p1[2]%2 == auxRect.p1[2]%2);
        assert(mainRect.p2[0]%2 == auxRect.p2[0]%2);
        assert(mainRect.p2[1]%2 == auxRect.p2[1]%2);
        assert(mainRect.p2[2]%2 == auxRect.p2[2]%2);
        
        //  check dimensions
        assert(mainRect.p2[0] - mainRect.p1[0] == auxRect.p2[0] - auxRect.p1[0] ||
               auxRect.p2[0] == auxRect.p1[0]+1);
        assert(mainRect.p2[1] - mainRect.p1[1] == auxRect.p2[1] - auxRect.p1[1] ||
               auxRect.p2[1] == auxRect.p1[1]+1);
        assert(mainRect.p2[2] - mainRect.p1[2] == auxRect.p2[2] - auxRect.p1[2] ||
               auxRect.p2[2] == auxRect.p1[2]+1);
    }
    
    for (itr1 = inSFRegions.begin(); itr1 != inSFRegions.end(); itr1++)
    {
        mainRect = (*itr1).getMainRect();
        auxRect = (*itr1).getAuxRect();
        
        //  check alignments
        assert(mainRect.p1[0]%2 == auxRect.p1[0]%2);
        assert(mainRect.p1[1]%2 == auxRect.p1[1]%2);
        assert(mainRect.p1[2]%2 == auxRect.p1[2]%2);
        assert(mainRect.p2[0]%2 == auxRect.p2[0]%2);
        assert(mainRect.p2[1]%2 == auxRect.p2[1]%2);
        assert(mainRect.p2[2]%2 == auxRect.p2[2]%2);
        
        //  check dimensions
        assert(mainRect.p2[0] - mainRect.p1[0] == auxRect.p2[0] - auxRect.p1[0] ||
               auxRect.p2[0] == auxRect.p1[0]+1);
        assert(mainRect.p2[1] - mainRect.p1[1] == auxRect.p2[1] - auxRect.p1[1] ||
               auxRect.p2[1] == auxRect.p1[1]+1);
        assert(mainRect.p2[2] - mainRect.p1[2] == auxRect.p2[2] - auxRect.p1[2] ||
               auxRect.p2[2] == auxRect.p1[2]+1);
    }
}
*/
/*
void StructureGrid::
insertSFRegions(const std::vector<TFSFRegion> & inSFRegions)
{
    
}
*/

/*
void StructureGrid::
getOrientedOuterBoundary( set<OrientedPoint3i> & outSet,
                          const Rect3i & inRegion,
                          const Rect3i & clipRect ) const
{
    int ii, jj, kk;
    
    Rect3i boundary = inset(inRegion, -1, -1, -1, -1, -1, -1);
	Vector3i direction;
    
    
    //  Create the set of ALL points just outside inRegion.
    //  This is done by the slowest way possible, but somehow it doesn't
    //  bother me to do this just once.
    
    set <Vector3i> outerPoints;
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
    //  For all those within clipRect, put them into outSet along with the
    //  direction from inRegion (the surface normal, more or less).
    for ( set<Vector3i>::iterator iPt = outerPoints.begin();
          iPt != outerPoints.end(); iPt++ )
    {
        if (clipRect.encloses(*iPt))
            outSet.insert(OrientedPoint3i(*iPt, *iPt - clip(inRegion, *iPt)));
    }
}
*/
