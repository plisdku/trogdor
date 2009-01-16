/*
 *  RunlineType.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/22/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _RUNLINETYPE_
#define _RUNLINETYPE_

#include "MaterialType.h"
#include "Pointer.h"
#include "IndexStencil.h"
#include <string>

class RunlineType
{
public:
    //RunlineType();
	
    RunlineType(const MaterialType & inType, int inMaterialIndex,
		int inSelfIndex, int ii0, int jj0, int kk0,
		const IndexStencil & inNeighbors);
	
    RunlineType(const MaterialType & inType, int inMaterialIndex,
		int inSelfIndex, int ii0, int jj0, int kk0,
		const IndexStencil & inNeighbors,
		const IndexStencil & inBufferNeighbors);
	
    virtual ~RunlineType();
    
    void setLength(int length) { mLength = length; }
    
    int getLength() const { return mLength; }
    int materialIndex() const { return mMaterialIndex; }
    int getSelfIndex() const { return mSelfIndex; }
    
    int get_ii0() const { return m_ii0; }
    int get_jj0() const { return m_jj0; }
    int get_kk0() const { return m_kk0; }
    
    const IndexStencil & getNeighbors() const { return mNeighbors; }
	const IndexStencil & getBufferNeighbors() const { return mBufferNeighbors; }
    //const IndexStencil & getAuxNeighbors() const { return mAuxNeighbors; }
    
    const MaterialType & getType() const { return mType; }
    
    bool isTFSF() const { return mType.isTFSF(); }
    
    void print(std::ostream & ostr) const;
    
private:
    /*
    void dealloc();
    void alloc(int stencilSize);
    void clear();
    */
    
    MaterialType mType;
    int mLength;
    int mMaterialIndex;
    
    int mSelfIndex;
    
    IndexStencil mNeighbors;
    IndexStencil mBufferNeighbors;
    
    int m_ii0;
    int m_jj0;
    int m_kk0;
};

typedef Pointer<RunlineType> RunlineTypePtr;


#endif
