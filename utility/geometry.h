/*
 *  geometry.h
 *  vmlibtest
 *
 *  Created by Paul Hansen on 2/8/08.
 *  Copyright 2008 Stanford University. All rights reserved.
 *
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:
 *
 */

#ifndef _GEOMETRY_
#define _GEOMETRY_

#include "VectorMatrix.h"

typedef Matrix3i Mat3i;
typedef Matrix3f Mat3f;
typedef Matrix3d Mat3d;
typedef Matrix3b Mat3b;

template<typename T>
class Rect
{
public:
	Rect();
	Rect(T i0, T j0, T k0, T i1, T j1, T k1);
	Rect(const Vector3<T> & p0, const Vector3<T> & p1);
	Rect(const Rect<T> & copyMe);
	
    // size is calculated as p2-p1
	T size(unsigned int dim) const;
	Vector3<T> size() const;
    
    // num is calculated as p2-p1+1 (so, "inclusive" size suitable for ints)
    T num(unsigned int dim) const;
    Vector3<T> num() const;
	
	bool
	encloses(const Rect<T> & inRect) const;
	
	bool
	encloses(const Vector3<T> & inPt) const;
	
	bool
	encloses(T x, T y, T z) const;
	
	bool
	intersects(const Rect<T> & inRect) const;
	
	int
	numNonSingularDims() const;
	
	Vector3<T> p1;
	Vector3<T> p2; // must be indexwise < p1.
};

typedef Rect<int> Rect3i;
typedef Rect<double> Rect3d;

template<typename T>
Rect<T>
operator * (const Rect<T> & lhs, const T rhs);

template<typename T>
Rect<T>
operator / (const Rect<T> & lhs, const T rhs);

template<typename T>
Rect<T>
operator * (const T scalar, const Rect<T> & rhs);

template<typename T>
Rect<T> &
operator *= (Rect<T> & lhs, const T scalar);

template<typename T>
Rect<T> &
operator /= (Rect<T> & lhs, const T scalar);

template<typename T>
Rect<T>
operator * (const Matrix3<T> & lhs, const Rect<T> & rhs);

template<typename T>
Rect<T>
operator + (const Rect<T> & lhs, const Vector3<T> & rhs);

template<typename T>
Rect<T>
operator - (const Rect<T> & lhs, const Vector3<T> & rhs);

template<typename T>
Rect<T>
operator + (const Vector3<T> & lhs, const Rect<T> & rhs);

template<typename T>
Rect<T>
operator - (const Vector3<T> & lhs, const Rect<T> & rhs);

template<typename T>
Rect<T>
inset( const Rect<T> & inRect, T dx0, T dy0, T dz0, T dx1, T dy1, T dz1);

template<typename T>
Vector3<T>
clip( const Rect<T> & clipRect, const Vector3<T> & v );

template<typename T>
Rect<T>
clip( const Rect<T> & rectToClip, const Rect<T> & clipRect);

template<typename T>
Rect<T>
cyclicPermute(const Rect<T> & r, unsigned int nn);

template<typename T>
bool operator<(const Rect<T> & lhs, const Rect<T> & rhs);

template<typename T>
bool operator>(const Rect<T> & lhs, const Rect<T> & rhs);

template<typename T>
struct OrientedRect
{
	OrientedRect();
	OrientedRect(const Rect<T> & r, const Vector3<T> & n) :
		rect(r),
		normal(n)
	{}
	
	Rect<T> rect;
	Vector3<T> normal;
};
typedef OrientedRect<int> OrientedRect3i;
typedef OrientedRect<double> OrientedRect3d;


#pragma mark *** Input/Output ***

template<typename T>
std::ostream & operator<<(std::ostream & str, const Rect<T> & rect);

template<typename T>
std::istream & operator>>(std::istream & str, Rect<T> & rect);

template<typename T>
std::ostream & operator<<(std::ostream & str, const OrientedRect<T> & orect);

#include "geometry.hh"

#endif
