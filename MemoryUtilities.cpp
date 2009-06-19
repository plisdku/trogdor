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
#include <sstream>

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

bool MemoryBuffer::
includes(float const* ptr) const
{
    long offset = long(ptr) - long(mHeadPointer);
    offset /= sizeof(float);
    
    if (offset >= 0 && offset/mStride < mLength && offset%mStride == 0)
        return 1;
    return 0;
}

string MemoryBuffer::
identify(float const * ptr)
{
    set<MemoryBuffer*>::const_iterator itr;
    for (itr = sAllBuffers.begin(); itr != sAllBuffers.end(); itr++)
    {
        //LOG << (*itr)->getDescription() << "\n";
        if ((*itr)->includes(ptr))
        {
            ostringstream str;
            str << (*itr)->getDescription() << " offset " <<
                (long(ptr)-long((*itr)->getHeadPointer()))/sizeof(float);
            return str.str();
        }
    }
    return "Pointer not covered!";
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
    assert(mOffset >= 0 && mOffset < mBuffer->getLength());
}

BufferPointer::
BufferPointer(const BufferPointer & copyMe) :
	mBuffer(copyMe.mBuffer),
	mOffset(copyMe.mOffset)
{
    if (mBuffer != 0)
        assert(mOffset >= 0 && mOffset < mBuffer->getLength());
}

float* BufferPointer::
getPointer() const
{
    assert(mBuffer->getHeadPointer() != 0L);
    return mBuffer->getHeadPointer() + mOffset;
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



