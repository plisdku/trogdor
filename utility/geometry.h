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

#include "vmlib.hh"

typedef vmlib::SVec<3,int> Vector3i;
typedef vmlib::SVec<3,double> Vector3d;
typedef vmlib::SVec<3,bool> Vector3b;
typedef vmlib::SVec<3,float> Vector3f;
typedef vmlib::SMat<3,float> Mat3f;
typedef vmlib::SMat<3,int> Mat3i;
typedef vmlib::SMat<3,bool> Mat3b;

namespace vmlib {

template <unsigned DIM, typename T>
bool operator< (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs);

template <unsigned DIM, typename T>
bool operator> (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs);

template <unsigned DIM, typename T>
bool operator== (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs);

template <unsigned DIM, typename T>
bool operator!= (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs);

template <unsigned DIM, typename T>
vmlib::SVec<DIM,T> operator% (const vmlib::SVec<DIM,T>& lhs, T rhs);

template <unsigned DIM, typename T>
vmlib::SVec<DIM,T>
operator% (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T> & rhs);

} // namespace vmlib



template <unsigned DIM, typename T>
bool vec_lt(const vmlib::SVec<DIM,T>& lhs, const T & rhs);

template <unsigned DIM, typename T>
bool vec_gt(const vmlib::SVec<DIM,T>& lhs, const T & rhs);

template <unsigned DIM, typename T>
bool vec_le(const vmlib::SVec<DIM,T>& lhs, const T & rhs);

template <unsigned DIM, typename T>
bool vec_ge(const vmlib::SVec<DIM,T>& lhs, const T & rhs);

template <unsigned DIM, typename T>
bool vec_lt(const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs);

template <unsigned DIM, typename T>
bool vec_gt(const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs);

template <unsigned DIM, typename T>
bool vec_le(const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs);

template <unsigned DIM, typename T>
bool vec_ge(const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs);

template <unsigned DIM, typename T>
vmlib::SVec<DIM,T> dominantComponent(const vmlib::SVec<DIM,T> & arg);

template <typename T>
vmlib::SVec<3,T>
cross(const vmlib::SVec<3,T> & lhs, const vmlib::SVec<3,T>& rhs);

template <typename T>
int
parity(const vmlib::SVec<3,T> & arg);



template<typename T>
class Rect
{
public:
	Rect();
	Rect(T i0, T j0, T k0, T i1, T j1, T k1);
	Rect(const vmlib::SVec<3,T> & p0, const  vmlib::SVec<3,T> & p1);
	Rect(const Rect<T> & copyMe);
	
	T size(unsigned int dim) const;
	vmlib::SVec<3,T> size() const;
	
	bool
	encloses(const Rect<T> & inRect) const;
	
	bool
	encloses(const vmlib::SVec<3,T> & inPt) const;
	
	bool
	encloses(T x, T y, T z) const;
	
	bool
	intersects(const Rect<T> & inRect) const;
	
	int
	numNonSingularDims() const;
	
	vmlib::SVec<3,T> p1;
	vmlib::SVec<3,T> p2; // must be indexwise < p1.
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
operator * (const vmlib::SMat<3,T> & lhs, const Rect<T> & rhs);

template<typename T>
Rect<T>
operator + (const Rect<T> & lhs, const vmlib::SVec<3,T> & rhs);

template<typename T>
Rect<T>
operator - (const Rect<T> & lhs, const vmlib::SVec<3,T> & rhs);

template<typename T>
Rect<T>
operator + (const vmlib::SVec<3,T> & lhs, const Rect<T> & rhs);

template<typename T>
Rect<T>
operator - (const vmlib::SVec<3,T> & lhs, const Rect<T> & rhs);

template<typename T>
Rect<T>
inset( const Rect<T> & inRect, T dx0, T dy0, T dz0, T dx1, T dy1, T dz1);

template<typename T>
vmlib::SVec<3,T>
clip( const Rect<T> & clipRect, const vmlib::SVec<3,T> & v );

template<typename T>
Rect<T>
clip( const Rect<T> & rectToClip, const Rect<T> & clipRect);

template<typename T>
bool operator<(const Rect<T> & lhs, const Rect<T> & rhs);

template<typename T>
bool operator>(const Rect<T> & lhs, const Rect<T> & rhs);




template<typename T>
vmlib::SMat<3,T> matWithCols(const vmlib::SVec<3,T> & v1,
	const vmlib::SVec<3,T> & v2, const vmlib::SVec<3,T> & v3);



template<typename T, typename S>
void mapPoint(const Rect<T> & fromRect, const vmlib::SVec<3,T> & fromPoint,
    const Rect<T> & toRect, vmlib::SVec<3,T> & toPoint);
    
template<typename T, typename S>
void mapPoint(const Rect<T> & fromRect, int fromX, int fromY, int fromZ,
    const Rect<T> & toRect, int & toX, int & toY, int & toZ);

template<typename T>
struct OrientedRect
{
	OrientedRect();
	OrientedRect(const Rect<T> & r, const vmlib::SVec<3,T> & n) :
		rect(r),
		normal(n)
	{}
	
	Rect<T> rect;
	vmlib::SVec<3,T> normal;
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
