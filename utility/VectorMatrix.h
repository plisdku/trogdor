/*
 *  VectorMatrix.h
 *  MyVectorMatrix
 *
 *  Created by Paul Hansen on 5/18/09.
 *  Copyright 2009 Stanford University. All rights reserved.
 *
 *  THESE VECTORS ETC. DO NOT BEHAVE CORRECTLY FOR COMPLEX NUMBERS.
 *  There are no complex conjugations anywhere, e.g. in dot products and in
 *  absolute values/norms.
 */

#ifndef _VECTORMATRIX_
#define _VECTORMATRIX_


#include <iostream>
#include <cmath>

template<typename T>
class Vector3;
typedef Vector3<int> Vector3i;
typedef Vector3<double> Vector3d;
typedef Vector3<float> Vector3f;
typedef Vector3<bool> Vector3b;

template<typename T>
class Matrix3;
typedef Matrix3<int> Matrix3i;
typedef Matrix3<double> Matrix3d;
typedef Matrix3<float> Matrix3f;
typedef Matrix3<bool> Matrix3b;


template<typename T>
class Vector3
{
public:
    Vector3();
    Vector3(const Vector3<T> & copyMe);
    
    template<typename T2>
    explicit Vector3( const Vector3<T2> & copyMe );
    
    Vector3(T v0, T v1, T v2);
    
    template<typename T2>
    explicit Vector3( T2 v0, T2 v1, T2 v2 );
    
    T & operator[](unsigned int n)
        { return v[n]; }
    const T & operator[](unsigned int n) const
        { return v[n]; }
    
    Vector3<T> & operator+=(const Vector3<T> & rhs);
    Vector3<T> & operator-=(const Vector3<T> & rhs);
    Vector3<T> & operator*=(const Vector3<T> & rhs);
    Vector3<T> & operator/=(const Vector3<T> & rhs);
    Vector3<T> & operator%=(const Vector3<T> & rhs);
    
    Vector3<T> & operator+=(const T & rhs);
    Vector3<T> & operator-=(const T & rhs);
    Vector3<T> & operator*=(const T & rhs);
    Vector3<T> & operator/=(const T & rhs);
    Vector3<T> & operator%=(const T & rhs);
private:
    T v[3];
};


template<typename T, typename S>
Vector3<T> operator+(const Vector3<T> & lhs, const Vector3<S> & rhs);
template<typename T, typename S>
Vector3<T> operator-(const Vector3<T> & lhs, const Vector3<S> & rhs);
template<typename T, typename S>
Vector3<T> operator*(const Vector3<T> & lhs, const Vector3<S> & rhs);
template<typename T, typename S>
Vector3<T> operator/(const Vector3<T> & lhs, const Vector3<S> & rhs);
template<typename T, typename S>
Vector3<T> operator%(const Vector3<T> & lhs, const Vector3<S> & rhs);

template<typename T, typename S>
Vector3<T> operator+(const Vector3<T> & lhs, S rhs);
template<typename T, typename S>
Vector3<T> operator-(const Vector3<T> & lhs, S rhs);
template<typename T, typename S>
Vector3<T> operator*(const Vector3<T> & lhs, S rhs);
template<typename T, typename S>
Vector3<T> operator/(const Vector3<T> & lhs, S rhs);
template<typename T, typename S>
Vector3<T> operator%(const Vector3<T> & lhs, S rhs);

template<typename T, typename S>
Vector3<T> operator+(S lhs, const Vector3<T> & rhs);
template<typename T, typename S>
Vector3<T> operator-(S lhs, const Vector3<T> & rhs);
template<typename T, typename S>
Vector3<T> operator*(S lhs, const Vector3<T> & rhs);
template<typename T, typename S>
Vector3<T> operator/(S lhs, const Vector3<T> & rhs);
template<typename T, typename S>
Vector3<T> operator%(S lhs, const Vector3<T> & rhs);

template<typename T>
Vector3<T> operator+(const Vector3<T> & rhs);
template<typename T>
Vector3<T> operator-(const Vector3<T> & rhs);
template<typename T>
bool operator==(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
bool operator!=(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
bool operator<(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
bool operator>(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
bool operator<=(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
bool operator>=(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
Vector3<T> operator!(const Vector3<T> & v);

template<typename T>
T dot(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
Vector3<T> cross(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
T norm(const Vector3<T> & v);
template<typename T>
T norm1(const Vector3<T> & v);
template<typename T>
T norm2(const Vector3<T> & v);
template<typename T>
Vector3<T> dominantComponent(const Vector3<T> & rhs);
template<typename T>
Vector3<T> cyclicPermute(const Vector3<T> & rhs, unsigned int nn);

template <typename T, typename S>
bool vec_eq(const Vector3<T>& lhs, const S & rhs);
template <typename T, typename S>
bool vec_lt(const Vector3<T>& lhs, const S & rhs);
template <typename T, typename S>
bool vec_gt(const Vector3<T>& lhs, const S & rhs);
template <typename T, typename S>
bool vec_le(const Vector3<T>& lhs, const S & rhs);
template <typename T, typename S>
bool vec_ge(const Vector3<T>& lhs, const S & rhs);
template <typename T, typename S>
bool vec_lt(const Vector3<T>& lhs, const Vector3<S>& rhs);
template <typename T, typename S>
bool vec_gt(const Vector3<T>& lhs, const Vector3<S>& rhs);
template <typename T, typename S>
bool vec_le(const Vector3<T>& lhs, const Vector3<S>& rhs);
template <typename T, typename S>
bool vec_ge(const Vector3<T>& lhs, const Vector3<S>& rhs);


template<typename T>
Vector3<T> vec_max(const Vector3<T> & lhs, const Vector3<T> & rhs);
template<typename T>
Vector3<T> vec_min(const Vector3<T> & lhs, const Vector3<T> & rhs); 
template<typename T>
Vector3<T> vec_max(const Vector3<T> & lhs, T rhs);
template<typename T>
Vector3<T> vec_min(const Vector3<T> & lhs, T rhs); 

template<typename T>
Vector3<T> vec_floor(const Vector3<T> & lhs);

template<typename T>
std::ostream & operator<<(std::ostream & str, const Vector3<T> & rhs);

template<typename T>
std::istream & operator>>(std::istream & str, Vector3<T> & rhs);


template<typename T>
class Matrix3
{
public:
    Matrix3();
    Matrix3(const Matrix3<T> & copyMe);
    
    template<typename T2>
    explicit Matrix3(const Matrix3<T2> & copyMe );
    
    template<typename S>
    Matrix3(S m00, S m01, S m02, S m10, S m11, S m12, S m20, S m21, S m22);
    
    static Matrix3<T> eye();
    
    template<typename T2>
    static Matrix3<T> withColumns(const Vector3<T2> & c1,
        const Vector3<T2> & c2, const Vector3<T> & c3);
    
    template<typename T2>
    static Matrix3<T> withRows(const Vector3<T2> & c1, const Vector3<T2> & c2,
        const Vector3<T> & c3);
    
    template<typename T2>
    static Matrix3<T> diagonal(const Vector3<T2> & d);
    
    template<typename T2>
    static Matrix3<T> diagonal(T2 d);
    
    static Matrix3<T> cyclicPermutation();
    
    
    T & operator[](unsigned int n)
        { return m[n]; }
    const T & operator[](unsigned int n) const
        { return m[n]; }
    T & operator()(unsigned int mm, unsigned int nn)
        { return m[3*mm+nn]; }
    const T & operator()(unsigned int mm, unsigned int nn) const
        { return m[3*mm+nn]; }
    /*
    Matrix3<T> transpose() const;
    T determinant() const;
    */
private:
    T m[9];
};

template<typename T>
Matrix3<T> transpose(const Matrix3<T> & lhs);
template<typename T>
T determinant(const Matrix3<T> & lhs);
template<typename T>
Matrix3<T> inverse(const Matrix3<T> & lhs);

template<typename T, typename S>
Matrix3<T> operator+(const Matrix3<T> & lhs, const Matrix3<S> & rhs);
template<typename T, typename S>
Matrix3<T> operator-(const Matrix3<T> & lhs, const Matrix3<S> & rhs);
template<typename T, typename S>
Matrix3<T> operator*(const Matrix3<T> & lhs, const Matrix3<S> & rhs);
template<typename T, typename S>
Matrix3<T> operator%(const Matrix3<T> & lhs, const Matrix3<S> & rhs);

template<typename T, typename S>
Vector3<S> operator*(const Matrix3<T> & lhs, const Vector3<S> & rhs);

// this is like trans(v)*M, but no conjugation or nuthin' happens here...
template<typename T, typename S>
Vector3<S> operator*(const Vector3<S> & lhs, const Matrix3<T> & rhs);

template<typename T, typename S>
Matrix3<T> operator+(const Matrix3<T> & lhs, S & rhs);
template<typename T, typename S>
Matrix3<T> operator-(const Matrix3<T> & lhs, S & rhs);
template<typename T, typename S>
Matrix3<T> operator*(const Matrix3<T> & lhs, S rhs);
template<typename T, typename S>
Matrix3<T> operator/(const Matrix3<T> & lhs, S rhs);

template<typename T, typename S>
Matrix3<S> operator+(S lhs, const Matrix3<T> rhs);
template<typename T, typename S>
Matrix3<S> operator-(S lhs, const Matrix3<T> rhs);
template<typename T, typename S>
Matrix3<S> operator*(S lhs, const Matrix3<T> rhs);
template<typename T, typename S>
Matrix3<S> operator/(S lhs, const Matrix3<T> rhs);

template<typename T>
Matrix3<T> operator+(const Matrix3<T> & rhs);
template<typename T>
Matrix3<T> operator-(const Matrix3<T> & rhs);
template<typename T>
bool operator==(const Matrix3<T> & lhs, const Matrix3<T> & rhs);
template<typename T>
bool operator!=(const Matrix3<T> & lhs, const Matrix3<T> & rhs);
template<typename T>
bool operator<(const Matrix3<T> & lhs, const Matrix3<T> & rhs);
template<typename T>
bool operator>(const Matrix3<T> & lhs, const Matrix3<T> & rhs);
template<typename T>
bool operator<=(const Matrix3<T> & lhs, const Matrix3<T> & rhs);
template<typename T>
bool operator>=(const Matrix3<T> & lhs, const Matrix3<T> & rhs);

template<typename T>
std::ostream & operator<<(std::ostream & str, const Matrix3<T> & rhs);

template<typename T>
std::istream & operator>>(std::istream & str, Matrix3<T> & rhs);



#include "VectorMatrix.hpp"
#endif
