/*
 *  GridWalker.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/23/06.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _GRIDWALKER_
#define _GRIDWALKER_

#include "IndexStencil.h"
#include "StructureGrid.h"
#include "Pointer.h"
#include "geometry.h"

#include "RunlineType.h"

#include <set>
#include <string>
#include <bitset>

class GridWalker
{
public:
    GridWalker(StructureGridPtr grid, int ii0, int jj0, int kk0);
    ~GridWalker();
    
    void beginRow();
    bool hasRunline() { return mHasRunline; }
    void startNewRunline(const MaterialType & inType, int ii, int jj, int kk);
    void incrementLength();
    
    void step(int ii, int jj, int kk);
    bool needNewRunline();
    RunlineType& getRunline();
	
private:
    void getStencil(IndexStencil & stencil, int ii, int jj, int kk) const;
    
    void validateRunline();
	

#pragma mark *** Private data ***
private:

    StructureGridPtr mGrid;
    
    bool mIsTFSF;
    
    int m_aux_nx;
    int m_aux_ny;
    int m_aux_nz;
    
	Map<int, int> mTagForTFSFTag;
    Map<int, int> mMaterialIndexForTag;
    
    bool mHasRunline;
    Pointer<RunlineType> mRunline;
    int mMaterial;
	
	std::bitset<6> mMatTFSFFlags;
	std::bitset<6> mUseXYZFlags;
	
	std::bitset<6> mComparisonXYZFlags;  // == ~mMatTFSFFlags & mUseXYZFlags
	
	//unsigned int mMatTFSFFlags;
	//unsigned int mUseXYZFlags;
	    
    //  these are the coordinates of the walker's current step in the grid.
    int m_ii;
    int m_jj;
    int m_kk;
    
    IndexStencil mNeighborIndices;
    
	std::vector<Vector3i> mCardinals;
    
};


#endif
