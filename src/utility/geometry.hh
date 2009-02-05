/*
 *  geometry.cpp
 *  vmlibtest
 *
 *  Created by Paul Hansen on 2/8/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef _GEOMETRY_

#include "geometry.h"

#pragma mark *** Vector ***

namespace vmlib {

template <unsigned DIM, typename T>
bool operator< (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (unsigned int ii = 0; ii < DIM; ii++)
	{
		if (lhs[ii] < rhs[ii])
			return 1;
		else if (lhs[ii] > rhs[ii])
			return 0;
	}
	return 0;
}

template <unsigned DIM, typename T>
bool operator> (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (unsigned int ii = 0; ii < DIM; ii++)
	{
		if (lhs[ii] > rhs[ii])
			return 1;
		else if (lhs[ii] < rhs[ii])
			return 0;
	}
	return 0;
}

template <unsigned DIM, typename T>
bool operator== (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (unsigned int ii = 0; ii < DIM; ii++)
	{
		if (lhs[ii] != rhs[ii])
			return 0;
	}
	return 1;
}

template <unsigned DIM, typename T>
bool operator!= (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (unsigned int ii = 0; ii < DIM; ii++)
	{
		if (lhs[ii] != rhs[ii])
			return 1;
	}
	return 0;
}

template <unsigned DIM, typename T>
vmlib::SVec<DIM,T> operator% (const vmlib::SVec<DIM,T>& lhs, T rhs)
{
	vmlib::SVec<DIM,T> retval(lhs);
	for (unsigned int ii = 0; ii < DIM; ii++)
		retval[ii] %= rhs;
	return retval;
}

} // namespace vmlib

/*
template <unsigned DIM, typename T>
bool operator<= (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (int ii = 0; ii < DIM; ii++)
	{
		if (lhs[ii] >= rhs[ii])
			return 1;
		else if (lhs[ii] <= rhs[ii])
			return 0;
	}
	return 0;
}

template <unsigned DIM, typename T>
bool operator>= (const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (int ii = 0; ii < DIM; ii++)
	{
		if (lhs[ii] >= rhs[ii])
			return 1;
		else if (lhs[ii] <= rhs[ii])
			return 0;
	}
	return 0;
}
*/


template <unsigned DIM, typename T>
bool vec_lt(const vmlib::SVec<DIM,T>& lhs, const T& rhs)
{
	for (int ii = 0; ii < DIM; ii++)
		if (lhs[ii] >= rhs)
			return 0;
	return 1;
}

template <unsigned DIM, typename T>
bool vec_gt(const vmlib::SVec<DIM,T>& lhs, const T& rhs)
{
	for (int ii = 0; ii < DIM; ii++)
		if (lhs[ii] <= rhs)
			return 0;
	return 1;
}

template <unsigned DIM, typename T>
bool vec_le(const vmlib::SVec<DIM,T>& lhs, const T& rhs)
{
	for (int ii = 0; ii < DIM; ii++)
		if (lhs[ii] > rhs)
			return 0;
	return 1;
}

template <unsigned DIM, typename T>
bool vec_ge(const vmlib::SVec<DIM,T>& lhs, const T& rhs)
{
	for (int ii = 0; ii < DIM; ii++)
		if (lhs[ii] < rhs)
			return 0;
	return 1;
}

template <unsigned DIM, typename T>
bool vec_lt(const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (int ii = 0; ii < DIM; ii++)
		if (lhs[ii] >= rhs[ii])
			return 0;
	return 1;
}

template <unsigned DIM, typename T>
bool vec_gt(const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (int ii = 0; ii < DIM; ii++)
		if (lhs[ii] <= rhs[ii])
			return 0;
	return 1;
}

template <unsigned DIM, typename T>
bool vec_le(const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (unsigned int ii = 0; ii < DIM; ii++)
		if (lhs[ii] > rhs[ii])
			return 0;
	return 1;
}

template <unsigned DIM, typename T>
bool vec_ge(const vmlib::SVec<DIM,T>& lhs, const vmlib::SVec<DIM,T>& rhs)
{
	for (unsigned int ii = 0; ii < DIM; ii++)
		if (lhs[ii] < rhs[ii])
			return 0;
	return 1;
}

template <unsigned DIM, typename T>
vmlib::SVec<DIM,T> dominantComponent(const vmlib::SVec<DIM,T> & arg)
{
	T maxabs = normi(arg);
	
	vmlib::SVec<DIM,T> retval = 0*arg;
	
	for (int ii = 0; ii < DIM; ii++)
	{
		if (abs(arg[ii]) == maxabs)
		{
			retval[ii] = arg[ii];
			return retval;
		}
	}
	return retval;
}

template <typename T>
vmlib::SVec<3,T>
cross(const vmlib::SVec<3,T> & lhs, const vmlib::SVec<3,T>& rhs)
{
	return vmlib::SVec<3,T>( lhs[1]*rhs[2] - lhs[2]*rhs[1], 
		lhs[2]*rhs[0] - lhs[0]*rhs[2],
		lhs[0]*rhs[1] - lhs[1]*rhs[0]);
}

template <typename T>
int
parity(const vmlib::SVec<3,T> & arg)
{
	return (arg[0]%2) + 2*(arg[1]%2) + 4*(arg[2]%2);
}

#pragma mark *** Rect ***


template<typename T>
Rect<T>::Rect() :
    p1(0,0,0),
    p2(0,0,0)
{
}

template<typename T>
Rect<T>::Rect(T i0, T j0, T k0, T i1, T j1, T k1) :
    p1(i0, j0, k0),
    p2(i1, j1, k1)
{
}

template<typename T>
Rect<T>::Rect( const vmlib::SVec<3,T> & inP1,
    const vmlib::SVec<3,T> & inP2 ) :
	p1(inP1), p2(inP2)
{
}

template<typename T>
Rect<T>::Rect(const Rect<T> & copyMe) :
    p1(copyMe.p1),
    p2(copyMe.p2)
{
}

template<typename T>
T Rect<T>::size(unsigned int dim) const
{
	return p2[dim] - p1[dim];
}

template<typename T>
vmlib::SVec<3,T> Rect<T>::size() const
{
	return p2 - p1;
}

template<typename T>
bool Rect<T>::encloses(const Rect<T> & inRect) const
{
    return (encloses(inRect.p1) && encloses(inRect.p2));
}

template<typename T>
bool Rect<T>::encloses(const vmlib::SVec<3,T> & inPt) const
{
    if ( inPt[0] >= p1[0] && inPt[0] <= p2[0] &&
         inPt[1] >= p1[1] && inPt[1] <= p2[1] &&
         inPt[2] >= p1[2] && inPt[2] <= p2[2] )
         return 1;
    return 0;
}

template<typename T>
bool Rect<T>::encloses(T x, T y, T z) const
{
    if ( x >= p1[0] && x <= p2[0] &&
         y >= p1[1] && y <= p2[1] &&
         z >= p1[2] && z <= p2[2] )
         return 1;
    return 0;
}

template<typename T>
bool Rect<T>::intersects(const Rect<T> & inRect) const
{
	
    /*return ( p2[0] >= inRect.p1[0] && p1[0] <= inRect.p2[0] &&
             p2[1] >= inRect.p1[1] && p1[1] <= inRect.p2[1] &&
             p2[2] >= inRect.p1[2] && p1[2] <= inRect.p2[2] );
	*/
	
	return (vec_ge(p2, inRect.p1) && vec_le(p1, inRect.p2));
	
	//p2 >= inRect.p1 && p1 <= inRect.p2);
}

template<typename T>
int Rect<T>::numNonSingularDims() const
{
    int ndims = 0;
    if (p1[0] != p2[0])
        ndims++;
    if (p1[1] != p2[1])
        ndims++;
    if (p1[2] != p2[2])
        ndims++;
    
    return ndims;
}


#pragma mark *** Other functions ***


template<typename T>
Rect<T>
operator * (const Rect<T> & lhs, const T rhs)
{
    return Rect<T>(lhs.p1*rhs, lhs.p2*rhs);
}

template<typename T>
Rect<T>
operator / (const Rect<T> & lhs, const T rhs)
{
    return Rect<T>(lhs.p1/rhs, lhs.p2/rhs);
}

template<typename T>
Rect<T>
operator * (const T scalar, const Rect<T> & rhs)
{
    return Rect<T>(rhs.p1*scalar, rhs.p2*scalar);
}

template<typename T>
Rect<T> &
operator *= (Rect<T> & lhs, const T scalar)
{
    lhs.p1 *= scalar;
    lhs.p2 *= scalar;
    return lhs;
}

template<typename T>
Rect<T> &
operator /= (Rect<T> & lhs, const T scalar)
{
    lhs.p1 /= scalar;
    lhs.p2 /= scalar;
    return lhs;
}

template<typename T>
Rect<T>
inset( const Rect<T> & inRect, T dx0, T dx1, T dy0, T dy1, T dz0, T dz1 )
{
    Rect<T> out(inRect);
    out.p1[0] += dx0;
    out.p1[1] += dy0;
    out.p1[2] += dz0;
    out.p2[0] -= dx1;
    out.p2[1] -= dy1;
    out.p2[2] -= dz1;
    return out;
}
                   
template<typename T>
vmlib::SVec<3,T>  clip(const Rect<T> & clipRect, const vmlib::SVec<3,T> & v)
{
    vmlib::SVec<3,T> out(v);
    if (out[0] < clipRect.p1[0])
        out[0] = clipRect.p1[0];
    else if (out[0] > clipRect.p2[0])
        out[0] = clipRect.p2[0];
    
    if (out[1] < clipRect.p1[1])
        out[1] = clipRect.p1[1];
    else if (out[1] > clipRect.p2[1])
        out[1] = clipRect.p2[1];
    
    if (out[2] < clipRect.p1[2])
        out[2] = clipRect.p1[2];
    else if (out[2] > clipRect.p2[2])
        out[2] = clipRect.p2[2];
    
    return out;
}


template<typename T>
bool operator<(const Rect<T> & lhs, const Rect<T> & rhs)
{
    if (lhs.p1 < rhs.p1)
        return 1;
    else if (lhs.p1 > rhs.p1)
        return 0;
    else if (lhs.p2 < rhs.p2)
        return 1;
    return 0;
}


template<typename T>
bool operator>(const Rect<T> & lhs, const Rect<T> & rhs)
{
    if (lhs.p1 > rhs.p1)
        return 1;
    else if (lhs.p1 < rhs.p1)
        return 0;
    else if (lhs.p2 > rhs.p2)
        return 1;
    return 0;
}

template<typename T, typename S>
void mapPoint(const Rect<T> & fromRect, const vmlib::SVec<3,T> & fromPoint,
    const Rect<T> & toRect, vmlib::SVec<3,T> & toPoint)
{
    toPoint[0] = (fromPoint[0] - fromRect.p1[0]) + toRect.p1[0];
    toPoint[1] = (fromPoint[1] - fromRect.p1[1]) + toRect.p1[1];
    toPoint[2] = (fromPoint[2] - fromRect.p1[2]) + toRect.p1[2];
}

template<typename T, typename S>
void mapPoint(const Rect<T> & fromRect, int fromX, int fromY, int fromZ,
    const Rect<T> & toRect, int & toX, int & toY, int & toZ)
{
    toX = (fromX - fromRect.p1[0]) + toRect.p1[0];
    toY = (fromY - fromRect.p1[1]) + toRect.p1[1];
    toZ = (fromZ - fromRect.p1[2]) + toRect.p1[2];
}


template<typename T>
std::ostream & operator<<(std::ostream & str, const Rect<T> & rect)
{
	str << "[" << rect.p1 << ", " << rect.p2 << "]";
	return str;
}

template<typename T>
std::istream & operator>>(std::istream & str, Rect<T> & rect)
{
	str >> rect.p1[0] >> rect.p1[1] >> rect.p1[2]
		>> rect.p2[0] >> rect.p2[1] >> rect.p2[2];
	return str;
}

template<typename T>
std::ostream & operator<<(std::ostream & str, const OrientedRect<T> & orect)
{
	str << orect.rect << ", " << orect.normal;
	return str;
}

#endif
