/*
 *  Pointer.h
 *  TROGDOR
 *
 *  Created by Paul Hansen on 6/14/06.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _POINTER_
#define _POINTER_

#include "Log.h"
#include <iomanip>
#include <cassert>
#include "Map.h"


template <typename T>
class Pointer
{
public:
    Pointer() : mPtr(0L) { }
    explicit Pointer(T* inPtr);
	
	// this is a bad idea!  (see explanation below in implementation.)
	/*
	template <typename S>
	explicit Pointer(const Pointer<S> & rhs);
	*/
	
    ~Pointer();
    
    Pointer(const Pointer<T>& src);
    Pointer<T>& operator=(const Pointer<T>& rhs);
    Pointer<T>& operator=(int nullShouldBeZero);
	
	// not sure this is a good idea
	//template <typename T2>
	//Pointer<T>& operator=(T2* rhs);
    
    T& operator*();
    const T& operator*() const;
    T* operator->();
    const T* operator->() const;
    //operator long() const { return long(mPtr); } // to test for nullness
	operator T*() const { return mPtr; } // for the odd desmartinization need
	
    int refcount() const;
	static const Map<T*,int> & getMap() { return mReferenceCounts; }
    
    template<typename T1, typename T2>
    friend bool operator<(const Pointer<T1> & lhs, const Pointer<T2> & rhs);
    
    static void print();
protected:
    T* mPtr;
    
    static Map<T*, int> mReferenceCounts;
    
    void decrementPointer();
};

template<typename T>
Map<T*, int> Pointer<T>::mReferenceCounts;

template<typename T1, typename T2>
bool operator<(const Pointer<T1> & lhs, const Pointer<T2> & rhs)
{
    return (lhs.mPtr < rhs.mPtr);
}

template <typename T>
Pointer<T>::
Pointer(T* inPtr) :
    mPtr(inPtr)
{
    if (mPtr != 0L)
    {
        assert(mReferenceCounts[inPtr] == 0);
        mReferenceCounts[inPtr]++;
    }
    //LOG << "constructor " << std::hex << inPtr << std::dec << " refcount "
    //    << mReferenceCounts[inPtr] << "\n";
    //LOG << "total pointers: " << mReferenceCounts.size() << std::endl;
}
// this is a bad idea because the new pointer maintains its own ref count, 
// and so on casts, you can end up deallocating things twice
/*
template<typename T>
template<typename S>
Pointer<T>::
Pointer(const Pointer<S> & rhs) :
	mPtr((T*)&(*rhs))
{
	if (mPtr != 0L)
	{
		mReferenceCounts[mPtr]++;
	}
}*/


template <typename T>
Pointer<T>::
~Pointer()
{
    //LOG << "destructor " << std::hex << mPtr << std::dec << " refcount first ";
    //LOGMORE << mReferenceCounts[mPtr] << "...\n";
    decrementPointer();
}

template <typename T>
Pointer<T>::
Pointer(const Pointer<T> & src)
    : mPtr(src.mPtr)
{
    if (mPtr != 0L)
        mReferenceCounts[mPtr]++;
    //LOG << "copy constructor " << std::hex << mPtr << std::dec
    //    << " (new refcount ";
    //LOGMORE << mReferenceCounts[mPtr] << ")\n";
    //LOG << "total pointers: " << mReferenceCounts.size() << std::endl;
}

template <typename T>
Pointer<T>& Pointer<T>::
operator=(const Pointer<T> & rhs)
{
    //LOG << "total pointers: " << mReferenceCounts.size() << std::endl;
    
    //LOG << "operator " << std::hex << mPtr << " = " << rhs.mPtr << std::dec
    //    << " refcounts " << mReferenceCounts[mPtr] << " and ";
    //LOGMORE << mReferenceCounts[rhs.mPtr] << "\n" << std::flush;
    if ( &rhs == this )
    {
        //LOG << "case 1...\n";
        return *this;
    }
    if ( rhs.mPtr == mPtr )
    {
        //LOG << "case 2, pointers " << std::hex << mPtr << std::dec << " ...\n";
        return *this;
    }
    
    decrementPointer();
    
    mPtr = rhs.mPtr;
    if (mPtr != 0L)
        mReferenceCounts[mPtr]++;
    //LOG << "operator= " << std::hex << mPtr << std::dec
    //    << " (new refcount ";
    //LOGMORE << mReferenceCounts[mPtr] << ")\n";
    
    return *this;
}

template <typename T>
Pointer<T>& Pointer<T>::
operator=(int nullShouldBeZero)
{
    assert(!nullShouldBeZero && "Pointer = int only valid for NULL pointer.");
    
    *this = Pointer<T>(0L);
    return *this;
}
/*
template <typename T>
template <typename T2>
Pointer<T>& Pointer<T>::
operator=(T2* rhs)
{
	if (mPtr == rhs)
	{
		return *this;
	}
	decrementPointer();
	mPtr = rhs;
	if (mPtr != 0L)
		mReferenceCounts[mPtr]++;
	return *this;
}
*/
template <typename T>
T& Pointer<T>::operator*()
{
    if (mPtr == 0L)
    {
        std::cerr << "Can't dereference a NULL pointer!\n";
        std::exit(1);
    }
    return (*mPtr);
}

template <typename T>
T* Pointer<T>::operator->()
{
    if (mPtr == 0L)
    {
        assert(!"Can't dereference a NULL pointer!\n");
        /*
        std::cerr << "Can't dereference a NULL pointer!\n";
        std::exit(1);*/
    }
    return (mPtr);
}

template <typename T>
const T& Pointer<T>::operator*() const
{
    if (mPtr == 0L)
    {
        assert(!"Can't dereference a NULL pointer!\n");
        /*
        std::cerr << "Can't dereference a NULL pointer!\n";
        std::exit(1);*/
    }
    return (*mPtr);
}

template<typename T>
int Pointer<T>::refcount() const
{
    return mReferenceCounts[mPtr];
}


template <typename T>
const T* Pointer<T>::operator->() const
{
    if (mPtr == 0L)
    {
        assert(!"Can't dereference a NULL pointer!\n");
        /*
        std::cerr << "Can't dereference a NULL pointer!\n";
        std::exit(1);*/
    }
    return (mPtr);
}

template <typename T>
void Pointer<T>::decrementPointer()
{
    if (mPtr != 0L)
    {
        int & count = mReferenceCounts[mPtr];
        count--;
        if (count == 0)
        {
            //LOG << "decrementing and deleting " << std::hex
            //    << mPtr << std::dec << std::endl;
            delete mPtr;
        }
        else if (count > 0)
        {
            //LOG << "decrementing " << std::hex << mPtr << std::dec
            //    << " (new refcount " << count << ")\n";
        }
        else
        {
            //LOG << "Dagnabbit!  " << std::hex << mPtr << std::dec
            //    << " Refcount " << count << "\n";
            assert(!"Bicycle race.");
        }
        assert(count == mReferenceCounts[mPtr]);
    }
    
    //LOG << "total pointers: " << mReferenceCounts.size() << std::endl;
}

template <typename T>
void Pointer<T>::print()
{
    LOG << mReferenceCounts.size() << " pointer records.\n";
    /*
    Map<T*, int>::iterator itr;
    for (itr = mReferenceCounts.begin(); itr != mReferenceCounts.end(); itr++)
    {
        LOG << "pointer " << std::hex << (*itr).first << std::dec << " count "
            << (*itr).second << "\n";
    }*/
}


#endif

