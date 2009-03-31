/*
 *  MemoryUtilities.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 3/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MEMORYUTILITIES_
#define _MEMORYUTILITIES_

//class MemoryBuffer;
class BufferPointer;

class MemoryBuffer
{
public:
	MemoryBuffer(unsigned long length, unsigned long stride = 1);
	MemoryBuffer(const MemoryBuffer & copyMe);
	
	unsigned long getLength() const { return mLength; }
	unsigned long getStride() const { return mStride; }
	
private:
	unsigned long mLength;
	unsigned long mStride;
	
	friend class BufferPointer;
};

class BufferPointer
{
public:
	BufferPointer(const MemoryBuffer & buffer);
	BufferPointer(const MemoryBuffer & buffer, unsigned long offset);
	BufferPointer(const BufferPointer & copyMe);
	
	unsigned long getOffset() const { return mOffset; }
	const MemoryBuffer & getBuffer() const { return mBuffer; }
	
	void setOffset(unsigned long offset);
	
	
private:
	const MemoryBuffer & mBuffer;
	unsigned long mOffset;
};

BufferPointer
operator + (const BufferPointer & lhs, unsigned long rhs);

BufferPointer
operator - (const BufferPointer & lhs, unsigned long rhs);

BufferPointer &
operator += (BufferPointer & lhs, unsigned long rhs);

BufferPointer &
operator -= (BufferPointer & lhs, unsigned long rhs);



#endif
