/*
 *  RunlineType.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/22/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *  $Date::                              $:  Date of last commit
 *  $Id::                                $:  Id of last commit
 *
 */

#include "RunlineType.h"

#include "SetupTFSFBufferSet.h"

using namespace std;
/*
RunlineType::
RunlineType() :
    mLength(0),
    mMaterialIndex(0),
    mSelfIndex(0),
    m_ii0(0),
    m_jj0(0),
    m_kk0(0)
{
    //alloc(mStencilSize);
    //clear();
}
*/

RunlineType::
RunlineType(const MaterialType & inType, int inMaterialIndex,
    int inSelfIndex, int ii0, int jj0, int kk0,
    const IndexStencil & inNeighbors) :
    mType(inType),
    mLength(0),
    mMaterialIndex(inMaterialIndex),
    mSelfIndex(inSelfIndex),
    mNeighbors(inNeighbors),
	mBufferNeighbors(),
    m_ii0(ii0),
    m_jj0(jj0),
    m_kk0(kk0)
{
    //alloc(mStencilSize);
    //clear();
}

RunlineType::
RunlineType(const MaterialType & inType, int inMaterialIndex,
    int inSelfIndex, int ii0, int jj0, int kk0,
    const IndexStencil & inNeighbors,
	const IndexStencil & inBufferNeighbors) :
    mType(inType),
    mLength(0),
    mMaterialIndex(inMaterialIndex),
    mSelfIndex(inSelfIndex),
    mNeighbors(inNeighbors),
	mBufferNeighbors(inBufferNeighbors),
    m_ii0(ii0),
    m_jj0(jj0),
    m_kk0(kk0)
{
    //alloc(mStencilSize);
    //clear();
}

RunlineType::
~RunlineType()
{
}


void RunlineType::
print(std::ostream & ostr) const
{
    ostr << "(" << m_ii0 << " " << m_jj0 << " " << m_kk0 << ") length ";
    ostr << mLength << endl;
}

