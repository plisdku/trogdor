/*
 *  IndexStencil.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/27/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _INDEXSTENCIL_
#define _INDEXSTENCIL_

#include <iostream>
#include <bitset>

class IndexStencil
{
public:
    IndexStencil();
    virtual ~IndexStencil();
    IndexStencil & operator=(const IndexStencil & rhs);
    bool operator==(const IndexStencil &rhs);
	
	// redacted
    //void clear();
    
	int getIndex(int faceNumber) const;
	void setIndex(int faceNumber, int index);
	
    int getXIndex(int i) const; // { return mXIndices[i]; }
    int getYIndex(int i) const; // { return mYIndices[i]; }
    int getZIndex(int i) const; // { return mZIndices[i]; }
    
    void setXIndex(int i, int index); // { mXIndices[i] = index; }
    void setYIndex(int i, int index); // { mYIndices[i] = index; }
    void setZIndex(int i, int index); // { mZIndices[i] = index; }
    
    //bool compareShifted(int myShiftCells, const IndexStencil & s2);
    unsigned int compareShiftedOldish(int myShiftCells, const IndexStencil & s2);
	
	bool compareToIncremented(const IndexStencil & firstStencil,
		int increment, const std::bitset<6> & mask);

    void print(std::ostream & str) const;
private:
    
	int mIndices[6];
	
    int mXIndices[2];
    int mYIndices[2];
    int mZIndices[2];
};


#endif

