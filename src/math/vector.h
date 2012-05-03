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

/** @defgroup MathVectorModule math/vector.h
   Contains the Vector struct and related functions.
 */

#pragma once

#include "const.h"
#include "func.h"

#include <cmath>


// Math module namespace
namespace Math
{

/* @{ */ // start of group

/** \struct Vector math/vector.h
    \brief 3D (3x1) vector

  Represents a universal 3x1 vector that can be used in OpenGL and DirectX engines.
  Contains the required methods for operating on vectors.

  All methods are made inline to maximize optimization.

  Unit tests for the structure and related functions are in module: math/test/vector_test.cpp.

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
  inline explicit Vector(float x, float y, float z)
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
    if (IsZero(l))
      return;

    x /= l;
    y /= l;
    z /= l;
  }

  //! Calculates the cross product with another vector
  /** \a right right-hand side vector
      \returns the cross product*/
  inline Vector CrossMultiply(const Vector &right) const
  {
    float px = y * right.z - z * right.y;
    float py = z * right.x - x * right.z;
    float pz = x * right.y - y * right.x;
    return Vector(px, py, pz);
  }

  //! Calculates the dot product with another vector
  /** \a right right-hand side vector
      \returns the dot product */
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

}; // struct Point

//! Checks if two vectors are equal within given \a tolerance
inline bool VectorsEqual(const Vector &a, const Vector &b, float tolerance = TOLERANCE)
{
  return IsEqual(a.x, b.x, tolerance)
      && IsEqual(a.y, b.y, tolerance)
      && IsEqual(a.z, b.z, tolerance);
}

//! Convenience function for getting normalized vector
inline Vector Normalize(const Vector &v)
{
  Vector result = v;
  result.Normalize();
  return result;
}

//! Convenience function for calculating dot product
inline float DotProduct(const Vector &left, const Vector &right)
{
  return left.DotMultiply(right);
}

//! Convenience function for calculating cross product
inline Vector CrossProduct(const Vector &left, const Vector &right)
{
  return left.CrossMultiply(right);
}

//! Convenience function for calculating angle (in radians) between two vectors
inline float Angle(const Vector &a, const Vector &b)
{
  return a.Angle(b);
}

//! Returns the distance between two points
inline float Distance(const Vector &a, const Vector &b)
{
  return sqrt( (a.x-b.x)*(a.x-b.x) +
               (a.y-b.y)*(a.y-b.y) +
               (a.z-b.z)*(a.z-b.z) );
}

//! Returns the distance between projections on XZ plane of two vectors
inline float DistanceProjected(const Vector &a, const Vector &b)
{
  return sqrt( (a.x-b.x)*(a.x-b.x) +
               (a.z-b.z)*(a.z-b.z) );
}

//! Returns the normal vector to a plane
/** \param p1,p2,p3 points defining the plane */
inline Vector NormalToPlane(const Vector &p1, const Vector &p2, const Vector &p3)
{
  Vector u = p3 - p1;
  Vector v = p2 - p1;

  return Normalize(CrossProduct(u, v));
}

//! Returns the distance between given point and a plane
/** \param p the point
    \param a,b,c points defining the plane */
inline float DistanceToPlane(const Vector &a, const Vector &b, const Vector &c, const Vector &p)
{
  Vector n = NormalToPlane(a, b, c);
  float d = -(n.x*a.x + n.y*a.y + n.z*a.z);

  return fabs(n.x*p.x + n.y*p.y + n.z*p.z + d);
}

//! Checks if two planes defined by three points are the same
/** \a plane1 array of three vectors defining the first plane
    \a plane2 array of three vectors defining the second plane */
inline bool IsSamePlane(const Vector (&plane1)[3], const Vector (&plane2)[3])
{
  Vector n1 = NormalToPlane(plane1[0], plane1[1], plane1[2]);
  Vector n2 = NormalToPlane(plane2[0], plane2[1], plane2[2]);

  if ( fabs(n1.x-n2.x) > 0.1f ||
       fabs(n1.y-n2.y) > 0.1f ||
       fabs(n1.z-n2.z) > 0.1f )
    return false;

  float dist = DistanceToPlane(plane1[0], plane1[1], plane1[2], plane2[0]);
  if ( dist > 0.1f )
    return false;

  return true;
}

//! Calculates the projection of the point \a p on a straight line \a a to \a b.
/** \a p point to project
    \a a,b two ends of the line */
inline Vector Projection(const Vector &a, const Vector &b, const Vector &p)
{
  float k = DotProduct(b - a, p - a);
  k /= DotProduct(b - a, b - a);

  return a + k*(b-a);
}

//! Returns a point on the line \a p1 - \a p2, in \a dist distance from \a p1
/** \a p1,p2 line start and end
    \a dist scaling factor from \a p1, relative to distance between \a p1 and \a p2 */
inline Vector SegmentPoint(const Vector &p1, const Vector &p2, float dist)
{
  return p1 + (p2 - p1) * dist;
}

//! Rotates a point around a center in space.
/** \a center center of rotation
    \a angleH,angleV rotation angles in radians (positive is counterclockwise (CCW) ) )
    \a p the point
    \returns the rotated point */
inline Vector RotatePoint(const Vector &center, float angleH, float angleV, Vector p)
{
  Vector a, b;

  p.x -= center.x;
  p.y -= center.y;
  p.z -= center.z;

  b.x = p.x*cosf(angleH) - p.z*sinf(angleH);
  b.y = p.z*sinf(angleV) + p.y*cosf(angleV);
  b.z = p.x*sinf(angleH) + p.z*cosf(angleH);

  float x = center.x+b.x;
  float y = center.y+b.y;
  float z = center.z+b.z;

  return Vector(x, y, z);
}

//! Rotates a point around a center in space.
/** \a center center of rotation
    \a angleH,angleV rotation angles in radians (positive is counterclockwise (CCW) ) )
    \a p the point
    \returns the rotated point */
inline Vector RotatePoint2(const Vector center, float angleH, float angleV, Vector p)
{
  Vector a, b;

  p.x -= center.x;
  p.y -= center.y;
  p.z -= center.z;

  a.x = p.x*cosf(angleH) - p.z*sinf(angleH);
  a.y = p.y;
  a.z = p.x*sinf(angleH) + p.z*cosf(angleH);

  b.x = a.x;
  b.y = a.z*sinf(angleV) + a.y*cosf(angleV);
  b.z = a.z*cosf(angleV) - a.y*sinf(angleV);

  float x = center.x+b.x;
  float y = center.y+b.y;
  float z = center.z+b.z;

  return Vector(x, y, z);
}

//! Calculates the intersection "i" right "of" the plane "abc".
inline bool Intersect(const Vector &a, const Vector &b, const Vector &c, const Vector &d, const Vector &e, Vector &i)
{
  float d1 = (d.x-a.x)*((b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z)) -
     (d.y-a.y)*((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z)) +
     (d.z-a.z)*((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y));

  float d2 = (d.x-e.x)*((b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z)) -
     (d.y-e.y)*((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z)) +
     (d.z-e.z)*((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y));

  if (d2 == 0)
    return false;

  i.x = d.x + d1/d2*(e.x-d.x);
  i.y = d.y + d1/d2*(e.y-d.y);
  i.z = d.z + d1/d2*(e.z-d.z);

  return true;
}

//! Calculates the intersection of the straight line passing through p (x, z)
/** Line is parallel to the y axis, with the plane abc. Returns p.y. */
inline bool IntersectY(const Vector &a, const Vector &b, const Vector &c, Vector &p)
{
  float d  = (b.x-a.x)*(c.z-a.z) - (c.x-a.x)*(b.z-a.z);
  float d1 = (p.x-a.x)*(c.z-a.z) - (c.x-a.x)*(p.z-a.z);
  float d2 = (b.x-a.x)*(p.z-a.z) - (p.x-a.x)*(b.z-a.z);

  if (d == 0.0f)
    return false;

  p.y = a.y + d1/d*(b.y-a.y) + d2/d*(c.y-a.y);

  return true;
}

//! Calculates the end point
inline Vector LookatPoint(const Vector &eye, float angleH, float angleV, float length)
{

  Vector lookat = eye;
  lookat.z += length;

  RotatePoint(eye, angleH, angleV, lookat);

  return lookat;
}

/* @} */ // end of group

}; // namespace Math
