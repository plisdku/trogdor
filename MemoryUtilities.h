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
#include <set>
#include "Pointer.h"

class BufferPointer;

class MemoryBuffer
{
public:
	MemoryBuffer();
	MemoryBuffer(unsigned long length, unsigned long stride = 1);
	MemoryBuffer(const std::string & inDescription, 
		unsigned long length, unsigned long stride = 1);
	MemoryBuffer(const MemoryBuffer & copyMe);
    ~MemoryBuffer();
	
	unsigned long length() const { return mLength; }
	unsigned long stride() const { return mStride; }
	const std::string & description() const { return mDescription; }
	void setDescription(const std::string & inDesc) { mDescription = inDesc; }
    
    void setHeadPointer(float* ptr);
    float* headPointer() const { return mHeadPointer; }
	
    static const std::set<MemoryBuffer*> & allBuffers()
        { return sAllBuffers; }
    
    bool includes(float const* ptr) const;
    
    static std::string identify(float const* ptr);
    
    MemoryBuffer & operator=(const MemoryBuffer & rhs);
private:
	unsigned long mLength;
	unsigned long mStride;
	std::string mDescription;
    
    float* mHeadPointer;
    
    static std::set<MemoryBuffer*> sAllBuffers;
	
	friend class BufferPointer;
};
typedef Pointer<MemoryBuffer> MemoryBufferPtr;
std::ostream & operator<<(std::ostream & str, const MemoryBuffer & buffer);


class BufferPointer
{
public:
	BufferPointer();
	BufferPointer(const MemoryBuffer & buffer);
	BufferPointer(const MemoryBuffer & buffer, unsigned long offset);
	BufferPointer(const BufferPointer & copyMe);
	
	unsigned long offset() const { return mOffset; }
	const MemoryBuffer * buffer() const { return mBuffer; }
    float* pointer() const;// { return mBuffer->headPointer() + mOffset; }
	
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
