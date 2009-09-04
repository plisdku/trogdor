/*
 *  geometry.cpp
 *  vmlibtest
 *
 *  Created by Paul Hansen on 2/8/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef _GEOMETRY_

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
Rect<T>::Rect( const Vector3<T> & inP1,
    const Vector3<T> & inP2 ) :
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
Vector3<T> Rect<T>::size() const
{
	return p2 - p1 ;
}

template<typename T>
T Rect<T>::volume() const
{
    return size(0)*size(1)*size(2);
}

template<typename T>
T Rect<T>::num(unsigned int dim) const
{
	return p2[dim] - p1[dim] + 1;
}

template<typename T>
Vector3<T> Rect<T>::num() const
{
	return p2 - p1 + 1;
}

template<typename T>
T Rect<T>::count() const
{
    return num(0)*num(1)*num(2);
}


template<typename T>
bool Rect<T>::encloses(const Rect<T> & inRect) const
{
    return (encloses(inRect.p1) && encloses(inRect.p2));
}

template<typename T>
bool Rect<T>::encloses(const Vector3<T> & inPt) const
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
	return (vec_ge(p2, inRect.p1) && vec_le(p1, inRect.p2));
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
operator * (const Matrix3<T> & lhs, const Rect<T> & rhs)
{
    return Rect<T>(lhs*rhs.p1, lhs*rhs.p2);
}


template<typename T>
Rect<T>
operator + (const Rect<T> & lhs, const Vector3<T> & rhs)
{
	return Rect<T>(lhs.p1 + rhs, lhs.p2 + rhs);
}

template<typename T>
Rect<T>
operator - (const Rect<T> & lhs, const Vector3<T> & rhs)
{
	return Rect<T>(lhs.p1 - rhs, lhs.p2 - rhs);
}

template<typename T>
Rect<T>
operator + (const Vector3<T> & lhs, const Rect<T> & rhs)
{
	return Rect<T>(rhs.p1 + lhs, rhs.p2 + lhs);
}

template<typename T>
Rect<T>
operator - (const Vector3<T> & lhs, const Rect<T> & rhs)
{
	return Rect<T>(lhs - rhs.p1, lhs - rhs.p2);
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
Vector3<T>  clip(const Rect<T> & clipRect, const Vector3<T> & v)
{
    Vector3<T> out(v);
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
Rect<T> clip( const Rect<T> & rectToClip, const Rect<T> & clipRect)
{
	Rect<T> out( clip(clipRect, rectToClip.p1), clip(clipRect, rectToClip.p2) );
	return out;
}

template<typename T>
Rect<T> cyclicPermute(const Rect<T> & r, unsigned int nn)
{
    return Rect<T>(cyclicPermute(r.p1,nn), cyclicPermute(r.p2,nn));
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


template<typename T>
OrientedRect<T> cyclicPermute(const OrientedRect<T> & r, unsigned int nn)
{
    return OrientedRect<T>(cyclicPermute(r.rect,nn),
        cyclicPermute(r.normal,nn));
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
