/*
 *  StructureGrid.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/19/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _STRUCTUREGRID_
#define _STRUCTUREGRID_

#include "SetupMaterialModel.h"
#include "MaterialType.h"

#include "Pointer.h"
#include "geometry.h"

#include <vector>

class StructureGrid
{
public:
    StructureGrid(int nnx, int nny, int nnz, Rect3i activeRegion, Rect3i roi);
    ~StructureGrid();
    
	void getUnmodifiedMaterialsFrom(const StructureGrid & otherGrid);
	
    Rect3i getBounds() const;
    const Rect3i & getRegionOfInterest() const;
    const Rect3i & getActiveRegion() const;
    
    //  These methods will allocate a new material index and char (screen tag)
    //  if the given material is not yet present in the StructureGrid.
    //  That is, these are getters AND allocators, like map::operator[]
    int materialIndex(std::string & name);
    int materialIndex(std::string & name, char suggestedSymbol);
    int allocMaterialIndex(MaterialType type);
	
	//	Helper function to make new MaterialType when setting a cell, if needed.
	void setMaterialType(const MaterialType & type, int ii, int jj, int kk);
	
	int getMaterialIndex(const std::string & name) const;
    
    char getMaterialChar(int materialIndex) const;
    
    const MaterialType & getMaterialType(int index) const;
    const MaterialType & getMaterialType(int ii, int jj, int kk) const;
    
    // fill the cell and enforce PEC boundaries
    void setPECMaterialCube(int index, int i, int j, int k);
    
    void setPMCMaterialCube(int index, int i, int j, int k);
    
    int& materialWrapped(int ii, int jj, int kk);
    int materialWrapped(int ii, int jj, int kk) const;
    
    int& material(int ii, int jj, int kk);
    int material(int ii, int jj, int kk) const;
    int yeeIndex(int ii, int jj, int kk) const;
    int nnx() const;
    int nny() const;
    int nnz() const;
    int nx() const;
    int ny() const;
    int nz() const;
	
	// Symmetry is determined for the Jth face in the Ith direction by casting
	// rays along the I axis through the faces normal to axis J and seeing if 
	// all the cells encountered have the same material type.  Thus the matrix
	// is indexed as M(I,J) or M(ray direction, face normal), where true means
	// symmetrical and false means asymmetrical.
	vmlib::SMat<3,bool> getSymmetries(const Rect3i & boundingRect) const;
    
    void print(std::ostream & str) const;
    void printMaterialTypes(std::ostream & str) const;
	
	bool areCellsFilled() const;
    
private:
    
    //  Methods for insertion of TF, SF and PML areas.
    //void assertSafeTFSFRegions(const std::vector<TFSFRegion> & inTFRegions,
    //                          const std::vector<TFSFRegion> & inSFRegions);
    
    
    //  Two-way map for materials and tags
    Map<MaterialType, int> mIndexForMaterial;
    Map<int, MaterialType> mMaterialForIndex;
    Map<int, char> mCharForIndex;
    Map<char, int> mIndexForChar;
    
    int mNextMaterialIndex;
    char mNextMaterialChar;
    
    //  3D double-cell array for material tags.  (The Data.)
    int* mTagGrid;
    int m_nnx;
    int m_nny;
    int m_nnz;
    int m_nx;
    int m_ny;
    int m_nz;
    
    Rect3i m_roi;
    Rect3i mActiveRegion;
    
};

typedef Pointer<StructureGrid> StructureGridPtr;


#endif

