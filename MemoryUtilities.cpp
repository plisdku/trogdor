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
#include <iomanip>

using namespace std;

set<MemoryBuffer*> MemoryBuffer::sAllBuffers;

MemoryBuffer::
MemoryBuffer() :
	mLength(0),
	mStride(1),
	mDescription("Empty buffer"),
    mHeadPointer(0L)
{
    sAllBuffers.insert(this);
}

MemoryBuffer::
MemoryBuffer(unsigned long length, unsigned long stride) :
	mLength(length),
	mStride(stride),
	mDescription(""),
    mHeadPointer(0L)
{
    sAllBuffers.insert(this);
}

MemoryBuffer::
MemoryBuffer(const string & inDescription, unsigned long length,
	unsigned long stride) :
	mLength(length),
	mStride(stride),
	mDescription(inDescription),
    mHeadPointer(0L)
{
    sAllBuffers.insert(this);
}

MemoryBuffer::
MemoryBuffer(const MemoryBuffer & copyMe) :
	mLength(copyMe.mLength),
	mStride(copyMe.mStride),
	mDescription(copyMe.mDescription),
    mHeadPointer(copyMe.mHeadPointer)
{
    sAllBuffers.insert(this);
}

MemoryBuffer::
~MemoryBuffer()
{
    sAllBuffers.erase(this);
}

void MemoryBuffer::
setHeadPointer(float* ptr)
{
    //LOG << "Setting head ptr for " << hex << this << dec << " to "
    //    << hex << ptr << dec << "\n";
    mHeadPointer = ptr;
}

ostream &
operator<<(std::ostream & str, const MemoryBuffer & buffer)
{
    str << hex << buffer.getHeadPointer() << dec << ": length " <<
        buffer.getLength() << " stride " << buffer.getStride() << " (" <<
        buffer.getDescription() << ")";
    return str;
}


BufferPointer::
BufferPointer() :
	mBuffer(0L),
	mOffset(0)
{
}

BufferPointer::
BufferPointer(const MemoryBuffer & buffer) :
	mBuffer(&buffer),
	mOffset(0)
{
}

BufferPointer::
BufferPointer(const MemoryBuffer & buffer, unsigned long offset) :
	mBuffer(&buffer),
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
	assert(offset % mBuffer->mStride == 0);
	assert(offset < mBuffer->mLength);
	mOffset = offset;
}


BufferPointer
operator + (const BufferPointer & lhs, unsigned long rhs)
{
	return BufferPointer(*lhs.getBuffer(), lhs.getOffset() + rhs);
}

BufferPointer
operator - (const BufferPointer & lhs, unsigned long rhs)
{
	assert(lhs.getOffset() >= rhs);
	return BufferPointer(*lhs.getBuffer(), lhs.getOffset() - rhs);
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

std::ostream &
operator << (std::ostream & str, const BufferPointer & buff)
{
	if (buff.mBuffer != 0L)
		str << buff.mBuffer->getDescription();
	else
		str << "(null buffer)";
	str << " offset " << buff.mOffset;
	return str;
}


