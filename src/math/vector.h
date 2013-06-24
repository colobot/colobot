// * This file is part of the COLOBOT source code
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file math/vector.h
 * \brief Vector struct and related functions
 */

#pragma once


#include "math/const.h"
#include "math/func.h"


#include <cmath>
#include <sstream>


// Math module namespace
namespace Math {


/**
 * \struct Vector
 * \brief 3D (3x1) vector
 *
 * Represents a universal 3x1 vector that can be used in OpenGL and DirectX engines.
 * Contains the required methods for operating on vectors.
 *
 * All methods are made inline to maximize optimization.
 *
 * Unit tests for the structure and related functions are in module: math/test/vector_test.cpp.
 *
 */
struct Vector
{
    //! X - 1st coord
    float x;
    //! Y - 2nd coord
    float y;
    //! Z - 3rd coord
    float z;

    //! Creates a zero vector (0, 0, 0)
    inline Vector()
     : x(0.0f)
     , y(0.0f)
     , z(0.0f)
    {}

    //! Creates a vector from given values
    inline explicit Vector(float _x, float _y, float _z)
     : x(_x)
     , y(_y)
     , z(_z)
    {}

    //! Loads the zero vector (0, 0, 0)
    inline void LoadZero()
    {
        x = y = z = 0.0f;
    }

    //! Returns the struct cast to \c float* array; use with care!
    inline float* Array()
    {
        return reinterpret_cast<float*>(this);
    }

    //! Returns the struct cast to <tt>const float*</tt> array; use with care!
    inline const float* Array() const
    {
        return reinterpret_cast<const float*>(this);
    }

    //! Returns the vector length
    inline float Length() const
    {
        return sqrtf(x*x + y*y + z*z);
    }

    //! Normalizes the vector
    inline void Normalize()
    {
        float l = Length();
        if (IsZero(l))
            return;

        x /= l;
        y /= l;
        z /= l;
    }

    //! Calculates the cross product with another vector
    /**
     * \param right right-hand side vector
     * \returns the cross product
     */
    inline Vector CrossMultiply(const Vector &right) const
    {
        float px = y * right.z - z * right.y;
        float py = z * right.x - x * right.z;
        float pz = x * right.y - y * right.x;
        return Vector(px, py, pz);
    }

    //! Calculates the dot product with another vector
    /**
     * \param right right-hand side vector
     * \returns the dot product
     */
    inline float DotMultiply(const Vector &right) const
    {
        return x * right.x + y * right.y + z * right.z;
    }

    //! Returns the cosine of angle between this and another vector
    inline float CosAngle(const Vector &right) const
    {
        return DotMultiply(right) / (Length() * right.Length());
    }

    //! Returns angle (in radians) between this and another vector
    inline float Angle(const Vector &right) const
    {
        return acos(CosAngle(right));
    }


    /* Operators */

    //! Returns the inverted vector
    inline Vector operator-() const
    {
        return Vector(-x, -y, -z);
    }

    //! Adds the given vector
    inline const Vector& operator+=(const Vector &right)
    {
        x += right.x;
        y += right.y;
        z += right.z;
        return *this;
    }

    //! Adds two vectors
    inline friend const Vector operator+(const Vector &left, const Vector &right)
    {
        return Vector(left.x + right.x, left.y + right.y, left.z + right.z);
    }

    //! Subtracts the given vector
    inline const Vector& operator-=(const Vector &right)
    {
        x -= right.x;
        y -= right.y;
        z -= right.z;
        return *this;
    }

    //! Subtracts two vectors
    inline friend const Vector operator-(const Vector &left, const Vector &right)
    {
        return Vector(left.x - right.x, left.y - right.y, left.z - right.z);
    }

    //! Multiplies by given scalar
    inline const Vector& operator*=(const float &right)
    {
        x *= right;
        y *= right;
        z *= right;
        return *this;
    }

    //! Multiplies vector by scalar
    inline friend const Vector operator*(const float &left, const Vector &right)
    {
        return Vector(left * right.x, left * right.y, left * right.z);
    }

    //! Multiplies vector by scalar
    inline friend const Vector operator*(const Vector &left, const float &right)
    {
        return Vector(left.x * right, left.y * right, left.z * right);
    }

    //! Divides by given scalar
    inline const Vector& operator/=(const float &right)
    {
        x /= right;
        y /= right;
        z /= right;
        return *this;
    }

    //! Divides vector by scalar
    inline friend const Vector operator/(const Vector &left, const float &right)
    {
        return Vector(left.x / right, left.y / right, left.z / right);
    }


    //! Returns a string "[x, y, z]"
    inline std::string ToString() const
    {
        std::stringstream s;
        s.precision(3);
        s << "[" << x << ", " << y << ", " << z << "]";
        return s.str();
    }

}; // struct Vector

//! Checks if two vectors are equal within given \a tolerance
inline bool VectorsEqual(const Math::Vector &a, const Math::Vector &b, float tolerance = TOLERANCE)
{
    return IsEqual(a.x, b.x, tolerance)
            && IsEqual(a.y, b.y, tolerance)
            && IsEqual(a.z, b.z, tolerance);
}

//! Convenience function for getting normalized vector
inline Vector Normalize(const Math::Vector &v)
{
    Vector result = v;
    result.Normalize();
    return result;
}

//! Convenience function for calculating dot product
inline float DotProduct(const Math::Vector &left, const Math::Vector &right)
{
    return left.DotMultiply(right);
}

//! Convenience function for calculating cross product
inline Vector CrossProduct(const Math::Vector &left, const Math::Vector &right)
{
    return left.CrossMultiply(right);
}

//! Convenience function for calculating angle (in radians) between two vectors
inline float Angle(const Math::Vector &a, const Math::Vector &b)
{
    return a.Angle(b);
}

//! Returns the distance between the ends of two vectors
inline float Distance(const Math::Vector &a, const Math::Vector &b)
{
    return sqrtf( (a.x-b.x)*(a.x-b.x) +
                  (a.y-b.y)*(a.y-b.y) +
                  (a.z-b.z)*(a.z-b.z) );
}

//! Clamps the vector \a vec to range between \a min and \a max
inline Vector Clamp(const Vector &vec, const Vector &min, const Vector &max)
{
    Vector clamped;
    clamped.x = Min(Max(min.x, vec.x), max.x);
    clamped.y = Min(Max(min.y, vec.y), max.y);
    clamped.z = Min(Max(min.z, vec.z), max.z);
    return clamped;
}


} // namespace Math

