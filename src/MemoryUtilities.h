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

#include <string>
#include <iostream>
#include "Pointer.h"
//class MemoryBuffer;
class BufferPointer;

class MemoryBuffer
{
public:
	MemoryBuffer();
	MemoryBuffer(unsigned long length, unsigned long stride = 1);
	MemoryBuffer(const std::string & inDescription, 
		unsigned long length, unsigned long stride = 1);
	MemoryBuffer(const MemoryBuffer & copyMe);
	
	unsigned long getLength() const { return mLength; }
	unsigned long getStride() const { return mStride; }
	const std::string & getDescription() const { return mDescription; }
	void setDescription(const std::string & inDesc) { mDescription = inDesc; }
	
private:
	unsigned long mLength;
	unsigned long mStride;
	std::string mDescription;
	
	friend class BufferPointer;
};
typedef Pointer<MemoryBuffer> MemoryBufferPtr;

class BufferPointer
{
public:
	BufferPointer();
	BufferPointer(const MemoryBuffer & buffer);
	BufferPointer(const MemoryBuffer & buffer, unsigned long offset);
	BufferPointer(const BufferPointer & copyMe);
	
	unsigned long getOffset() const { return mOffset; }
	const MemoryBuffer * getBuffer() const { return mBuffer; }
	
	void setOffset(unsigned long offset);
	
	friend std::ostream & operator << (std::ostream & str,
		const BufferPointer & buff);
private:
	const MemoryBuffer* mBuffer;
	unsigned long mOffset;
};
std::ostream & operator << (std::ostream & str, const BufferPointer & buff);

BufferPointer
operator + (const BufferPointer & lhs, unsigned long rhs);

BufferPointer
operator - (const BufferPointer & lhs, unsigned long rhs);

BufferPointer &
operator += (BufferPointer & lhs, unsigned long rhs);

BufferPointer &
operator -= (BufferPointer & lhs, unsigned long rhs);



#endif
