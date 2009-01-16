/*
 *  Runline.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/29/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#ifndef _RUNLINE_
#define _RUNLINE_

#include "RunlineType.h"
#include "Fields.h"

#include <cassert>

class TFSFBufferSet;

class Runline
{
public:
    Runline(const RunlineType & inRunline, Fields & inFields);
    Runline(const RunlineType & inRunline, Fields & inFields,
		TFSFBufferSet & inBuffer);
    Runline(const Runline & copyMe);
    
    const Runline & operator=(const Runline & rhs);
    
    ~Runline();
    
    /*
    void setLength(int inLength);
    void setAuxIndex1(int inAuxIndex);
    void setAuxIndex2(int inAuxIndex);
    void setAuxIndex3(int inAuxIndex);
    void setXFields(const float* inFieldXNeg, const float* inFieldXPos);
    void setYFields(const float* inFieldYNeg, const float* inFieldYPos);
    void setZFields(const float* inFieldZNeg, const float* inFieldZPos);
    void setXCorrection(const float* inCorrection);
    void setYCorrection(const float* inCorrection);
    void setZCorrection(const float* inCorrection);
    */
	
	void setAuxIndex(int whichIndex, int inValue)
		{ mAuxIndex[whichIndex] = inValue; }
	int getAuxIndex(int whichIndex) { return mAuxIndex[whichIndex]; }
	
    
    void setAuxIndex1(int inAuxIndex) { mAuxIndex[0] = inAuxIndex; }
    void setAuxIndex2(int inAuxIndex) { mAuxIndex[1] = inAuxIndex; }
    void setAuxIndex3(int inAuxIndex) { mAuxIndex[2] = inAuxIndex; }
    
    int getLength() const           { return mLength; }
    int getIndex() const            { return mIndex; }
    int getAuxIndex1() const        { return mAuxIndex[0]; }
    int getAuxIndex2() const        { return mAuxIndex[1]; }
    int getAuxIndex3() const        { return mAuxIndex[2]; }
    
    float* const field0() const  { return mField0; }
    
    
    //  Field accessors.  USAGE:
    //
    //      for Ex, access Hy(k-1) with field1Neg()
    //              access Hy(k+1) with field1Pos()
    //              access Hz(k-1) with field2Neg()
    //              access Hz(k+1) with field2Pos()
    //
    //  To access the correction terms, use the aux functions:
    //
    //      for Ex, access extra Hy(k-1) with field1NegAux()
    //
    //  et cetera.  These functions have undefined results for runlines of
    //  non-TFSF materials.
    
    const float* field1Neg() const { return mNeighbors_j[0]; }
    const float* field1Pos() const { return mNeighbors_j[1]; }
    
    const float* field2Neg() const { return mNeighbors_k[0]; }
    const float* field2Pos() const { return mNeighbors_k[1]; }
    
    const float* field1NegAux() const {
        assert(mAuxNeighbors);
        return mAuxNeighbors[0];
    }
    const float* field1PosAux() const {
        assert(mAuxNeighbors);
        return mAuxNeighbors[1];
    }
    
    const float* field2NegAux() const {
        assert(mAuxNeighbors);
        return mAuxNeighbors[2];
    }
    const float* field2PosAux() const {
        assert(mAuxNeighbors);
        return mAuxNeighbors[3];
    }
    
    void printNeighbors() const;
	
	void assertZero() const;
    
private:
    int mLength;
    int mIndex;         //  cell index in its material
	int mAuxIndex[3];   //  user-defined
    
    float* mField0;                 //  pointer to the field to update
    const float* mNeighbors_j[2];    //  for Ex, this is Hy
    const float* mNeighbors_k[2];    //  for Ex, this is Hz
    
    const float* mAuxNeighbors[4];  //  just for debuggery
    //const float** mAuxNeighbors;    //  for TFSF corrected materials, this is
                                    //  a pointer to a float*[8].  And cattle.
    
    //  Yes, friends, you heard that right, cattle.  Man cows and woman cows,
    //  boy cows and girl cows, baby cows of as-yet undetermined gender -- they
    //  are all here.
public:
    int m_ii0, m_jj0, m_kk0;  // for debugging
private:
};


#endif
