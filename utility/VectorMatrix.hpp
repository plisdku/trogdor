/*
 *  VectorMatrix.cpp
 *  MyVectorMatrix
 *
 *  Created by Paul Hansen on 5/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 */

#ifdef _VECTORMATRIX_

#include <cassert>

template<typename T>
static T smax(T lhs, T rhs)
{
    return rhs > lhs ? rhs : lhs;
}

template<typename T>
static T smin(T lhs, T rhs)
{
    return lhs < rhs ? lhs : rhs;
}

template<typename T>
static T sabs(T val)
{
    return val < 0 ? -val : val;
}

#pragma mark *** VECTOR ***

template<typename T>
Vector3<T>::
Vector3()
{
    v[0] = v[1] = v[2] = 0;
}

template<typename T>
Vector3<T>::
Vector3(const Vector3<T> & copyMe)
{
    v[0] = copyMe[0];
    v[1] = copyMe[1];
    v[2] = copyMe[2];
}

template<typename T>
template<typename T2>
Vector3<T>::
Vector3(const Vector3<T2> & copyMe )
{
    v[0] = copyMe[0];
    v[1] = copyMe[1];
    v[2] = copyMe[2];
}

template<typename T>
Vector3<T>::
Vector3(T v0, T v1, T v2)
{
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
}

template<typename T>
template<typename T2>
Vector3<T>::
Vector3( T2 v0, T2 v1, T2 v2 )
{
    v[0] = v0;
    v[1] = v1;
    v[2] = v2;
}

template<typename T, typename S>
Vector3<T> operator+(const Vector3<T> & lhs, const Vector3<S> & rhs)
{
    return Vector3<T>(lhs[0]+rhs[0], lhs[1]+rhs[1], lhs[2]+rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator-(const Vector3<T> & lhs, const Vector3<S> & rhs)
{
    return Vector3<T>(lhs[0]-rhs[0], lhs[1]-rhs[1], lhs[2]-rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator*(const Vector3<T> & lhs, const Vector3<S> & rhs)
{
    return Vector3<T>(lhs[0]*rhs[0], lhs[1]*rhs[1], lhs[2]*rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator/(const Vector3<T> & lhs, const Vector3<S> & rhs)
{
    return Vector3<T>(lhs[0]/rhs[0], lhs[1]/rhs[1], lhs[2]/rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator%(const Vector3<T> & lhs, const Vector3<S> & rhs)
{
    return Vector3<T>(lhs[0]%rhs[0], lhs[1]%rhs[1], lhs[2]%rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator+(const Vector3<T> & lhs, S rhs)
{
    return Vector3<T>(lhs[0]+rhs, lhs[1]+rhs, lhs[2]+rhs);
}

template<typename T, typename S>
Vector3<T> operator-(const Vector3<T> & lhs, S rhs)
{
    return Vector3<T>(lhs[0]-rhs, lhs[1]-rhs, lhs[2]-rhs);
}

template<typename T, typename S>
Vector3<T> operator*(const Vector3<T> & lhs, S rhs)
{
    return Vector3<T>(lhs[0]*rhs, lhs[1]*rhs, lhs[2]*rhs);
}

template<typename T, typename S>
Vector3<T> operator/(const Vector3<T> & lhs, S rhs)
{
    return Vector3<T>(lhs[0]/rhs, lhs[1]/rhs, lhs[2]/rhs);
}

template<typename T, typename S>
Vector3<T> operator%(const Vector3<T> & lhs, S rhs)
{
    return Vector3<T>(lhs[0]%rhs, lhs[1]%rhs, lhs[2]%rhs);
}


template<typename T, typename S>
Vector3<T> operator+(S lhs, const Vector3<T> & rhs)
{
    return Vector3<T>(lhs+rhs[0], lhs+rhs[1], lhs+rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator-(S lhs, const Vector3<T> & rhs)
{
    return Vector3<T>(lhs-rhs[0], lhs-rhs[1], lhs-rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator*(S lhs, const Vector3<T> & rhs)
{
    return Vector3<T>(lhs*rhs[0], lhs*rhs[1], lhs*rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator/(S lhs, const Vector3<T> & rhs)
{
    return Vector3<T>(lhs/rhs[0], lhs/rhs[1], lhs/rhs[2]);
}

template<typename T, typename S>
Vector3<T> operator%(S lhs, const Vector3<T> & rhs)
{
    return Vector3<T>(lhs%rhs[0], lhs%rhs[1], lhs%rhs[2]);
}


template<typename T>
Vector3<T> & Vector3<T>::
operator+=(const Vector3<T> & rhs)
{
    v[0] += rhs.v[0];
    v[1] += rhs.v[1];
    v[2] += rhs.v[2];
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator-=(const Vector3<T> & rhs)
{
    v[0] -= rhs.v[0];
    v[1] -= rhs.v[1];
    v[2] -= rhs.v[2];
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator*=(const Vector3<T> & rhs)
{
    v[0] *= rhs.v[0];
    v[1] *= rhs.v[1];
    v[2] *= rhs.v[2];
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator/=(const Vector3<T> & rhs)
{
    v[0] /= rhs.v[0];
    v[1] /= rhs.v[1];
    v[2] /= rhs.v[2];
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator%=(const Vector3<T> & rhs)
{
    v[0] %= rhs.v[0];
    v[1] %= rhs.v[1];
    v[2] %= rhs.v[2];
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator+=(const T & rhs)
{
    v[0] += rhs;
    v[1] += rhs;
    v[2] += rhs;
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator-=(const T & rhs)
{
    v[0] -= rhs;
    v[1] -= rhs;
    v[2] -= rhs;
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator*=(const T & rhs)
{
    v[0] *= rhs;
    v[1] *= rhs;
    v[2] *= rhs;
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator/=(const T & rhs)
{
    v[0] /= rhs;
    v[1] /= rhs;
    v[2] /= rhs;
    return *this;
}

template<typename T>
Vector3<T> & Vector3<T>::
operator%=(const T & rhs)
{
    v[0] %= rhs;
    v[1] %= rhs;
    v[2] %= rhs;
    return *this;
}

template<typename T>
Vector3<T> operator+(const Vector3<T> & rhs)
{
    return rhs;
}

template<typename T>
Vector3<T> operator-(const Vector3<T> & rhs)
{
    return Vector3<T>(-rhs[0], -rhs[1], -rhs[2]);
}

template<typename T>
bool operator==(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    return (lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2]);
}

template<typename T>
bool operator!=(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    return !(lhs == rhs);
}

template<typename T>
bool operator<(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    if (lhs[0] < rhs[0]) return 1;
    else if (lhs[0] > rhs[0]) return 0;
    if (lhs[1] < rhs[1]) return 1;
    else if (lhs[1] > rhs[1]) return 0;
    if (lhs[2] < rhs[2]) return 1;
    
    return 0;
    //else if (lhs[2] > rhs[2]) return 0;
}

template<typename T>
bool operator>(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    if (lhs[0] > rhs[0]) return 1;
    else if (lhs[0] < rhs[0]) return 0;
    if (lhs[1] > rhs[1]) return 1;
    else if (lhs[1] < rhs[1]) return 0;
    if (lhs[2] > rhs[2]) return 1;
    
    return 0;
    //else if (lhs[2] < rhs[2]) return 0;
}

template<typename T>
bool operator<=(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    return !(lhs > rhs);
}

template<typename T>
bool operator>=(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    return !(lhs < rhs);
}


template<typename T>
T dot(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    return lhs[0]*rhs[0] + lhs[1]*rhs[1] + lhs[2]*rhs[2];
}

template<typename T>
Vector3<T> cross(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    return Vector3<T>(lhs[1]*rhs[2] - lhs[2]*rhs[1],
        lhs[2]*rhs[0] - lhs[0]*rhs[2],
        lhs[0]*rhs[1] - lhs[1]*rhs[0]);
}

template<typename T>
T norm(const Vector3<T> & v)
{
    return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
}

template<typename T>
T norm1(const Vector3<T> & v)
{
    return smax(smax(sabs(v[0]), sabs(v[1])), sabs(v[2]));
}

template<typename T>
T norm2(const Vector3<T> & v)
{
    return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

template<typename T>
Vector3<T> dominantComponent(const Vector3<T> & rhs)
{
    T a2, b2, c2;
    a2 = rhs[0]*rhs[0];
    b2 = rhs[1]*rhs[1];
    c2 = rhs[2]*rhs[2];
    
    if (a2 > b2 && a2 > c2)
        return Vector3<T>(rhs[0], 0, 0);
    else if (b2 > c2)
        return Vector3<T>(0, rhs[1], 0);
    return Vector3<T>(0, 0, rhs[2]);
}


template <typename T, typename S>
bool vec_lt(const Vector3<T>& lhs, const S & rhs)
{
    return (lhs[0] < rhs && lhs[1] < rhs && lhs[2] < rhs);
}

template <typename T, typename S>
bool vec_gt(const Vector3<T>& lhs, const S & rhs)
{
    return (lhs[0] > rhs && lhs[1] > rhs && lhs[2] > rhs);
}

template <typename T, typename S>
bool vec_le(const Vector3<T>& lhs, const S & rhs)
{
    return (lhs[0] <= rhs && lhs[1] <= rhs && lhs[2] <= rhs);
}

template <typename T, typename S>
bool vec_ge(const Vector3<T>& lhs, const S & rhs)
{
    return (lhs[0] >= rhs && lhs[1] >= rhs && lhs[2] >= rhs);
}

template <typename T, typename S>
bool vec_lt(const Vector3<T>& lhs, const Vector3<S>& rhs)
{
    return (lhs[0] < rhs[0] && lhs[1] < rhs[1] && lhs[2] < rhs[2]);
}

template <typename T, typename S>
bool vec_gt(const Vector3<T>& lhs, const Vector3<S>& rhs)
{
    return (lhs[0] > rhs[0] && lhs[1] > rhs[1] && lhs[2] > rhs[2]);
}

template <typename T, typename S>
bool vec_le(const Vector3<T>& lhs, const Vector3<S>& rhs)
{
    return (lhs[0] <= rhs[0] && lhs[1] <= rhs[1] && lhs[2] <= rhs[2]);
}

template <typename T, typename S>
bool vec_ge(const Vector3<T>& lhs, const Vector3<S>& rhs)
{
    return (lhs[0] >= rhs[0] && lhs[1] >= rhs[1] && lhs[2] >= rhs[2]);
}


template<typename T>
Vector3<T> vec_max(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    return Vector3<T>(smax(lhs[0], rhs[0]), smax(lhs[1], rhs[1]),
        smax(lhs[2], rhs[2]));
}

template<typename T>
Vector3<T> vec_min(const Vector3<T> & lhs, const Vector3<T> & rhs)
{
    return Vector3<T>(smin(lhs[0], rhs[0]), smin(lhs[1], rhs[1]),
        smin(lhs[2], rhs[2]));
}

template<typename T>
Vector3<T> vec_max(const Vector3<T> & lhs, T rhs)
{
    return Vector3<T>(smax(lhs[0], rhs), smax(lhs[1], rhs), smax(lhs[2], rhs));
}

template<typename T>
Vector3<T> vec_min(const Vector3<T> & lhs, T rhs)
{
    return Vector3<T>(smin(lhs[0], rhs), smin(lhs[1], rhs), smin(lhs[2], rhs));
}







template<typename T>
std::ostream & operator<<(std::ostream & str, const Vector3<T> & rhs)
{
    str << "(" << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ")";
    return str;
}

template<typename T>
std::istream & operator>>(std::istream & str, Vector3<T> & rhs)
{
    str >> rhs[0] >> rhs[1] >> rhs[2];
    return str;
}




#pragma mark *** MATRIX ***



template<typename T>
Matrix3<T>::
Matrix3()
{
    m[0] = m[1] = m[2] = m[3] = m[4] = m[5] = m[6] = m[7] = m[8] = 0;
}

template<typename T>
Matrix3<T>::
Matrix3(const Matrix3<T> & copyMe)
{
    m[0] = copyMe.m[0];
    m[1] = copyMe.m[1];
    m[2] = copyMe.m[2];
    m[3] = copyMe.m[3];
    m[4] = copyMe.m[4];
    m[5] = copyMe.m[5];
    m[6] = copyMe.m[6];
    m[7] = copyMe.m[7];
    m[8] = copyMe.m[8];
}

template<typename T>
template<typename T2>
Matrix3<T>::
Matrix3(const Matrix3<T2> & copyMe )
{
    m[0] = copyMe[0];
    m[1] = copyMe[1];
    m[2] = copyMe[2];
    m[3] = copyMe[3];
    m[4] = copyMe[4];
    m[5] = copyMe[5];
    m[6] = copyMe[6];
    m[7] = copyMe[7];
    m[8] = copyMe[8];
}

template<typename T>
template<typename S>
Matrix3<T>::
Matrix3(S m00, S m01, S m02, S m10, S m11, S m12, S m20, S m21, S m22)
{
    m[0] = m00;
    m[1] = m01;
    m[2] = m02;
    m[3] = m10;
    m[4] = m11;
    m[5] = m12;
    m[6] = m20;
    m[7] = m21;
    m[8] = m22;
}

template<typename T>
Matrix3<T> Matrix3<T>::
eye()
{
    return Matrix3<T>(1,0,0,0,1,0,0,0,1);
}

template<typename T>
template<typename T2>
Matrix3<T> Matrix3<T>::
withColumns(const Vector3<T2> & c1, const Vector3<T2> & c2,
    const Vector3<T> & c3)
{
    return Matrix3<T>(
        c1[0],c2[0],c3[0],
        c1[1],c2[1],c3[1],
        c1[2],c2[2],c3[2]);
}

template<typename T>
template<typename T2>
Matrix3<T> Matrix3<T>::
withRows(const Vector3<T2> & c1, const Vector3<T2> & c2,
    const Vector3<T> & c3)
{
    return Matrix3<T>(
        c1[0],c1[1],c1[2],
        c2[0],c2[1],c2[2],
        c3[0],c3[1],c3[2]);
}

template<typename T>
template<typename T2>
Matrix3<T> Matrix3<T>::
diagonal(const Vector3<T2> & d)
{
    return Matrix3<T>(
        d[0], 0,    0,
        0,    d[1], 0,
        0,    0,    d[2]);
}

template<typename T>
template<typename T2>
Matrix3<T> Matrix3<T>::
diagonal(T2 d)
{
    return Matrix3<T>(
        d, 0, 0,
        0, d, 0,
        0, 0, d);
}

template<typename T>
Matrix3<T> Matrix3<T>::
cyclicPermutation()
{
    return Matrix3<T>(
        0, 0, 1,
        1, 0, 0,
        0, 1, 0);
}

template<typename T>
Matrix3<T> transpose(const Matrix3<T> & rhs)
{
    return Matrix3<T>(
        rhs[0], rhs[3], rhs[6],
        rhs[1], rhs[4], rhs[7],
        rhs[2], rhs[5], rhs[8]);
}

template<typename T>
T determinant(const Matrix3<T> & rhs)
{
    return rhs[0]*(rhs[4]*rhs[8] - rhs[7]*rhs[5])
        - rhs[1]*(rhs[3]*rhs[8] - rhs[6]*rhs[5])
        + rhs[2]*(rhs[3]*rhs[7] - rhs[6]*rhs[4]);
}

template<typename T>
Matrix3<T> inverse(const Matrix3<T> & rhs)
{
    // http://www.dr-lex.be/random/matrix_inv.html;
    return Matrix3<T>(
        rhs(2,2)*rhs(1,1)-rhs(2,1)*rhs(1,2),
        -(rhs(2,2)*rhs(0,1)-rhs(2,1)*rhs(0,2)),
        rhs(1,2)*rhs(0,1)-rhs(1,1)*rhs(0,2),
        -(rhs(2,2)*rhs(1,0)-rhs(2,0)*rhs(1,2)),
        rhs(2,2)*rhs(0,0)-rhs(2,0)*rhs(0,2),
        -(rhs(1,2)*rhs(0,0)-rhs(1,0)*rhs(0,2)),
        rhs(2,1)*rhs(1,0)-rhs(2,0)*rhs(1,1),
        -(rhs(2,1)*rhs(0,0)-rhs(2,0)*rhs(0,1)),
        rhs(1,1)*rhs(0,0)-rhs(1,0)*rhs(0,1)
        ) / determinant(rhs);
}

template<typename T, typename S>
Matrix3<T> operator+(const Matrix3<T> & lhs, const Matrix3<S> & rhs)
{
    return Matrix3<T>(lhs[0]+rhs[0], lhs[1]+rhs[1], lhs[2]+rhs[2],
        lhs[3]+rhs[3], lhs[4]+rhs[4], lhs[5]+rhs[5], lhs[6]+rhs[6],
        lhs[7]+rhs[7], lhs[8]+rhs[8]);
}

template<typename T, typename S>
Matrix3<T> operator-(const Matrix3<T> & lhs, const Matrix3<S> & rhs)
{
    return Matrix3<T>(lhs[0]-rhs[0], lhs[1]-rhs[1], lhs[2]-rhs[2],
        lhs[3]-rhs[3], lhs[4]-rhs[4], lhs[5]-rhs[5], lhs[6]-rhs[6],
        lhs[7]-rhs[7], lhs[8]-rhs[8]);
}

template<typename T, typename S>
Matrix3<T> operator*(const Matrix3<T> & lhs, const Matrix3<S> & rhs)
{
    return Matrix3<T>(
        lhs[0]*rhs[0]+lhs[1]*rhs[3]+lhs[2]*rhs[6],
        lhs[0]*rhs[1]+lhs[1]*rhs[4]+lhs[2]*rhs[7],
        lhs[0]*rhs[2]+lhs[1]*rhs[5]+lhs[2]*rhs[8],
        lhs[3]*rhs[0]+lhs[4]*rhs[3]+lhs[5]*rhs[6],
        lhs[3]*rhs[1]+lhs[4]*rhs[4]+lhs[5]*rhs[7],
        lhs[3]*rhs[2]+lhs[4]*rhs[5]+lhs[5]*rhs[8],
        lhs[6]*rhs[0]+lhs[7]*rhs[3]+lhs[8]*rhs[6],
        lhs[6]*rhs[1]+lhs[7]*rhs[4]+lhs[8]*rhs[7],
        lhs[6]*rhs[2]+lhs[7]*rhs[5]+lhs[8]*rhs[8]);
        
}

template<typename T, typename S>
Matrix3<T> operator%(const Matrix3<T> & lhs, const Matrix3<S> & rhs)
{
    return Matrix3<T>(lhs[0]%rhs[0], lhs[1]%rhs[1], lhs[2]%rhs[2],
        lhs[3]%rhs[3], lhs[4]%rhs[4], lhs[5]%rhs[5], lhs[6]%rhs[6],
        lhs[7]%rhs[7], lhs[8]%rhs[8]);
}

template<typename T, typename S>
Vector3<S> operator*(const Matrix3<T> & lhs, const Vector3<S> & rhs)
{
    return Vector3<S>(
        lhs[0]*rhs[0]+lhs[1]*rhs[1]+lhs[2]*rhs[2],
        lhs[3]*rhs[0]+lhs[4]*rhs[1]+lhs[5]*rhs[2],
        lhs[6]*rhs[0]+lhs[7]*rhs[1]+lhs[8]*rhs[2]);
}

template<typename T, typename S>
Vector3<S> operator*(const Vector3<S> & lhs, const Matrix3<T> & rhs)
{
    return Vector3<S>(
        lhs[0]*rhs[0]+lhs[1]*rhs[3]+lhs[2]*rhs[6],
        lhs[0]*rhs[1]+lhs[1]*rhs[4]+lhs[2]*rhs[7],
        lhs[0]*rhs[2]+lhs[1]*rhs[5]+lhs[2]*rhs[8]);
}


template<typename T, typename S>
Matrix3<T> operator+(const Matrix3<T> & lhs, S & rhs)
{
    return Matrix3<T>(lhs[0]+rhs, lhs[1]+rhs, lhs[2]+rhs, lhs[3]+rhs,
        lhs[4]+rhs, lhs[5]+rhs, lhs[6]+rhs, lhs[7]+rhs, lhs[8]+rhs);
}

template<typename T, typename S>
Matrix3<T> operator-(const Matrix3<T> & lhs, S & rhs)
{
    return Matrix3<T>(lhs[0]-rhs, lhs[1]-rhs, lhs[2]-rhs, lhs[3]-rhs,
        lhs[4]-rhs, lhs[5]-rhs, lhs[6]-rhs, lhs[7]-rhs, lhs[8]-rhs);
}

template<typename T, typename S>
Matrix3<T> operator*(const Matrix3<T> & lhs, S rhs)
{
    return Matrix3<T>(lhs[0]*rhs, lhs[1]*rhs, lhs[2]*rhs, lhs[3]*rhs,
        lhs[4]*rhs, lhs[5]*rhs, lhs[6]*rhs, lhs[7]*rhs, lhs[8]*rhs);
}

template<typename T, typename S>
Matrix3<T> operator/(const Matrix3<T> & lhs, S rhs)
{
    return Matrix3<T>(lhs[0]/rhs, lhs[1]/rhs, lhs[2]/rhs, lhs[3]/rhs,
        lhs[4]/rhs, lhs[5]/rhs, lhs[6]/rhs, lhs[7]/rhs, lhs[8]/rhs);
}



template<typename T, typename S>
Matrix3<T> operator+(S lhs, const Matrix3<T> rhs)
{
    return Matrix3<T>(rhs[0]+lhs, rhs[1]+lhs, rhs[2]+lhs, rhs[3]+lhs,
        rhs[4]+lhs, rhs[5]+lhs, rhs[6]+lhs, rhs[7]+lhs, rhs[8]+lhs);
}

template<typename T, typename S>
Matrix3<T> operator-(S lhs, const Matrix3<T> rhs)
{
    return Matrix3<T>(rhs[0]-lhs, rhs[1]-lhs, rhs[2]-lhs, rhs[3]-lhs,
        rhs[4]-lhs, rhs[5]-lhs, rhs[6]-lhs, rhs[7]-lhs, rhs[8]-lhs);
}

template<typename T, typename S>
Matrix3<T> operator*(S lhs, const Matrix3<T> rhs)
{
    return Matrix3<T>(rhs[0]*lhs, rhs[1]*lhs, rhs[2]*lhs, rhs[3]*lhs,
        rhs[4]*lhs, rhs[5]*lhs, rhs[6]*lhs, rhs[7]*lhs, rhs[8]*lhs);
}

template<typename T, typename S>
Matrix3<T> operator/(S lhs, const Matrix3<T> rhs)
{
    return Matrix3<T>(rhs[0]/lhs, rhs[1]/lhs, rhs[2]/lhs, rhs[3]/lhs,
        rhs[4]/lhs, rhs[5]/lhs, rhs[6]/lhs, rhs[7]/lhs, rhs[8]/lhs);
}




template<typename T>
Matrix3<T> operator+(const Matrix3<T> & rhs)
{
    return rhs;
}

template<typename T>
Matrix3<T> operator-(const Matrix3<T> & rhs)
{
    return Matrix3<T>(-rhs[0], -rhs[1], -rhs[2], -rhs[3], -rhs[4], -rhs[5],
        -rhs[6], -rhs[7], -rhs[8]);
}

template<typename T>
bool operator==(const Matrix3<T> & lhs, const Matrix3<T> & rhs)
{
    return (lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] &&
        lhs[3] == rhs[3] && lhs[4] == rhs[4] && lhs[5] == rhs[5] &&
        lhs[6] == rhs[6] && lhs[7] == rhs[7] && lhs[8] == rhs[8]);
}

template<typename T>
bool operator!=(const Matrix3<T> & lhs, const Matrix3<T> & rhs)
{
    return !(lhs == rhs);
}

template<typename T>
bool operator<(const Matrix3<T> & lhs, const Matrix3<T> & rhs)
{
    if (lhs[0] < rhs[0]) return 1;
    else if (lhs[0] > rhs[0]) return 0;
    if (lhs[1] < rhs[1]) return 1;
    else if (lhs[1] > rhs[1]) return 0;
    if (lhs[2] < rhs[2]) return 1;
    else if (lhs[2] > rhs[2]) return 0;
    if (lhs[3] < rhs[3]) return 1;
    else if (lhs[3] > rhs[3]) return 0;
    if (lhs[4] < rhs[4]) return 1;
    else if (lhs[4] > rhs[4]) return 0;
    if (lhs[5] < rhs[5]) return 1;
    else if (lhs[5] > rhs[5]) return 0;
    if (lhs[6] < rhs[6]) return 1;
    else if (lhs[6] > rhs[6]) return 0;
    if (lhs[7] < rhs[7]) return 1;
    else if (lhs[7] > rhs[7]) return 0;
    if (lhs[8] < rhs[8]) return 1;
    return 0;
    //else if (lhs[8] > rhs[8]) return 0;
}

template<typename T>
bool operator>(const Matrix3<T> & lhs, const Matrix3<T> & rhs)
{
    if (lhs[0] > rhs[0]) return 1;
    else if (lhs[0] < rhs[0]) return 0;
    if (lhs[1] > rhs[1]) return 1;
    else if (lhs[1] < rhs[1]) return 0;
    if (lhs[2] > rhs[2]) return 1;
    else if (lhs[2] < rhs[2]) return 0;
    if (lhs[3] > rhs[3]) return 1;
    else if (lhs[3] < rhs[3]) return 0;
    if (lhs[4] > rhs[4]) return 1;
    else if (lhs[4] < rhs[4]) return 0;
    if (lhs[5] > rhs[5]) return 1;
    else if (lhs[5] < rhs[5]) return 0;
    if (lhs[6] > rhs[6]) return 1;
    else if (lhs[6] < rhs[6]) return 0;
    if (lhs[7] > rhs[7]) return 1;
    else if (lhs[7] < rhs[7]) return 0;
    if (lhs[8] > rhs[8]) return 1;
    return 0;
}

template<typename T>
bool operator<=(const Matrix3<T> & lhs, const Matrix3<T> & rhs)
{
    return !(lhs > rhs);
}
template<typename T>
bool operator>=(const Matrix3<T> & lhs, const Matrix3<T> & rhs)
{
    return !(lhs < rhs);
}

template<typename T>
std::ostream & operator<<(std::ostream & str, const Matrix3<T> & rhs)
{
    str << "[ " << rhs[0] << ", " << rhs[1] << ", " << rhs[2] << ",\n";
    str << "  " << rhs[3] << ", " << rhs[4] << ", " << rhs[5] << ",\n";
    str << "  " << rhs[6] << ", " << rhs[7] << ", " << rhs[8] << "]";
    return str;
}

template<typename T>
std::istream & operator>>(std::istream & str, Matrix3<T> & rhs)
{
    str >> rhs[0] >> rhs[1] >> rhs[2] >> rhs[3] >> rhs[4] >> rhs[5]
        >> rhs[6] >> rhs[7] >> rhs[8];
    return str;
}


#endif