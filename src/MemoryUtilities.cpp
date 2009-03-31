/*
 *  MemoryUtilities.cpp
 *  TROGDOR
 *
 *  Created by Paul Hansen on 3/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "MemoryUtilities.h"
#include <cassert>

MemoryBuffer::
MemoryBuffer(unsigned long length, unsigned long stride) :
	mLength(length),
	mStride(stride)
{
}

MemoryBuffer::
MemoryBuffer(const MemoryBuffer & copyMe) :
	mLength(copyMe.mLength),
	mStride(copyMe.mStride)
{
}


BufferPointer::
BufferPointer(const MemoryBuffer & buffer) :
	mBuffer(buffer),
	mOffset(0)
{
}

BufferPointer::
BufferPointer(const MemoryBuffer & buffer, unsigned long offset) :
	mBuffer(buffer),
	mOffset(offset)
{
}

BufferPointer::
BufferPointer(const BufferPointer & copyMe) :
	mBuffer(copyMe.mBuffer),
	mOffset(copyMe.mOffset)
{
}

void BufferPointer::
setOffset(unsigned long offset)
{
	assert(offset % mBuffer.mStride == 0);
	assert(offset < mBuffer.mLength);
	mOffset = offset;
}


BufferPointer
operator + (const BufferPointer & lhs, unsigned long rhs)
{
	return BufferPointer(lhs.getBuffer(), lhs.getOffset() + rhs);
}

BufferPointer
operator - (const BufferPointer & lhs, unsigned long rhs)
{
	assert(lhs.getOffset() >= rhs);
	return BufferPointer(lhs.getBuffer(), lhs.getOffset() - rhs);
}

BufferPointer &
operator += (BufferPointer & lhs, unsigned long rhs)
{
	lhs.setOffset(lhs.getOffset() + rhs);
	return lhs;
}

BufferPointer &
operator -= (BufferPointer & lhs, unsigned long rhs)
{
	assert(lhs.getOffset() > rhs);
	lhs.setOffset(lhs.getOffset() - rhs);
	return lhs;
}




