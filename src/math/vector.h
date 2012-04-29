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

// math/vector.h

/* Vector struct and functions */

#pragma once

#include "const.h"
#include "func.h"

#include <cmath>

/*
 TODO

float   Length(const D3DVECTOR &a, const D3DVECTOR &b);
float   Length2d(const D3DVECTOR &a, const D3DVECTOR &b);
D3DVECTOR Transform(const D3DMATRIX &m, D3DVECTOR p);
D3DVECTOR Projection(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &p);

D3DVECTOR SegmentDist(const D3DVECTOR &p1, const D3DVECTOR &p2, float dist);

BOOL      Intersect(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR d, D3DVECTOR e, D3DVECTOR &i);
BOOL      IntersectY(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR &p);


D3DVECTOR RotateView(D3DVECTOR center, float angleH, float angleV, float dist);
D3DVECTOR LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length );

void      MappingObject( D3DVERTEX2* pVertices, int nb, float scale );
void      SmoothObject( D3DVERTEX2* pVertices, int nb );

float   DistancePlanPoint(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &c, const D3DVECTOR &p);
BOOL      IsSamePlane(D3DVECTOR *plan1, D3DVECTOR *plan2);
 */

// Math module namespace
namespace Math
{

/** 3x1 Vector

  Represents an universal 3x1 vector that can be used in OpenGL and DirectX engines.
  Contains the required methods for operating on vectors.

  All methods are made inline to maximize optimization.

  TODO test

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
  {
    LoadZero();
  }

  //! Creates a vector from given values
  inline Vector(float x, float y, float z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  //! Loads the zero vector (0, 0, 0)
  inline void LoadZero()
  {
    x = y = z = 0.0f;
  }

  //! Returns the vector length
  inline float Length() const
  {
    return sqrt(x*x + y*y + z*z);
  }

  //! Normalizes the vector
  inline void Normalize()
  {
    float l = Length();
    x /= l;
    y /= l;
    z /= l;
  }

  //! Calculates the cross product with another vector
  /** \a right right-hand side vector */
  inline void CrossMultiply(const Vector &right)
  {
    Vector left = *this;
    x = left.y * right.z - left.z * right.y;
    y = left.z * right.x - left.x * right.z;
    z = left.x * right.y - left.y * right.x;
  }

  //! Calculates the dot product with another vector
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
};

//! Convenience function for calculating dot product
inline float DotProduct(const Vector &left, const Vector &right)
{
  return left.DotMultiply(right);
}

//! Convenience function for calculating cross product
inline Vector CrossProduct(const Vector &left, const Vector &right)
{
  Vector result = left;
  result.CrossMultiply(right);
  return result;
}

}; // namespace Math
