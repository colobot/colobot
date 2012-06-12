// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

/** @defgroup MathGeometryModule math/geometry.h
   Contains math functions related to 3D geometry calculations,
   transformations, etc.
 */

#pragma once

#include "const.h"
#include "func.h"
#include "point.h"
#include "vector.h"
#include "matrix.h"

#include <cmath>
#include <cstdlib>


// Math module namespace
namespace Math
{

/* @{ */ // start of group


//! Returns py up on the line \a a - \a b
inline float MidPoint(const Point &a, const Point &b, float px)
{
  if (IsEqual(a.x, b.x))
  {
    if (a.y < b.y)
      return HUGE;
    else
      return -HUGE;
  }
  return (b.y-a.y) * (px-a.x) / (b.x-a.x)  + a.y;
}

//! Calculates the parameters a and b of the linear function passing through \a p1 and \a p2
/** Returns \c false if the line is vertical.
  \param p1,p2 points
  \param a,b linear function parameters */
inline bool LinearFunction(const Point &p1, const Point &p2, float &a, float &b)
{
  if ( IsZero(p1.x-p2.x) )
  {
    a = HUGE;
    b = p2.x;
    return false;
  }

  a = (p2.y-p1.y) / (p2.x-p1.x);
  b = p2.y - p2.x*a;

  return true;
}

//! Tests whether the point \a p is inside the triangle (\a a,\a b,\a c)
inline bool IsInsideTriangle(Point a, Point b, Point c, Point p)
{
  float n, m;

  if ( p.x < a.x && p.x < b.x && p.x < c.x )  return false;
  if ( p.x > a.x && p.x > b.x && p.x > c.x )  return false;
  if ( p.y < a.y && p.y < b.y && p.y < c.y )  return false;
  if ( p.y > a.y && p.y > b.y && p.y > c.y )  return false;

  if ( a.x > b.x )  Swap(a,b);
  if ( a.x > c.x )  Swap(a,c);
  if ( c.x < a.x )  Swap(c,a);
  if ( c.x < b.x )  Swap(c,b);

  n = MidPoint(a, b, p.x);
  m = MidPoint(a, c, p.x);
  if ( (n>p.y||p.y>m) && (n<p.y||p.y<m) )  return false;

  n = MidPoint(c, b, p.x);
  m = MidPoint(c, a, p.x);
  if ( (n>p.y||p.y>m) && (n<p.y||p.y<m) )  return false;

  return true;
}

//! Rotates a point around a center
/** \a center center of rotation
    \a angle angle is in radians (positive is counterclockwise (CCW) )
    \a p the point */
inline Point RotatePoint(const Point &center, float angle, const Point &p)
{
  Point a;
  a.x = p.x-center.x;
  a.y = p.y-center.y;

  Point b;
  b.x = a.x*cosf(angle) - a.y*sinf(angle);
  b.y = a.x*sinf(angle) + a.y*cosf(angle);

  b.x += center.x;
  b.y += center.y;

  return b;
}

//! Rotates a point around the origin (0,0)
/** \a angle angle in radians (positive is counterclockwise (CCW) )
    \a p the point */
inline Point RotatePoint(float angle, const Point &p)
{
  float x = p.x*cosf(angle) - p.y*sinf(angle);
  float y = p.x*sinf(angle) + p.y*cosf(angle);

  return Point(x, y);
}

//! Rotates a vector (dist, 0).
/** \a angle angle is in radians (positive is counterclockwise (CCW) )
    \a dist distance to origin */
inline Point RotatePoint(float angle, float dist)
{
  float x = dist*cosf(angle);
  float y = dist*sinf(angle);

  return Point(x, y);
}

//! TODO documentation
inline void RotatePoint(float cx, float cy, float angle, float &px, float &py)
{
  float ax, ay;

  px -= cx;
  py -= cy;

  ax = px*cosf(angle) - py*sinf(angle);
  ay = px*sinf(angle) + py*cosf(angle);

  px = cx+ax;
  py = cy+ay;
}

//! Rotates a point around a center in space.
/** \a center center of rotation
    \a angleH,angleV rotation angles in radians (positive is counterclockwise (CCW) ) )
    \a p the point
    \returns the rotated point */
inline Vector RotatePoint(const Vector &center, float angleH, float angleV, Vector p)
{
  p.x -= center.x;
  p.y -= center.y;
  p.z -= center.z;

  Vector b;
  b.x = p.x*cosf(angleH) - p.z*sinf(angleH);
  b.y = p.z*sinf(angleV) + p.y*cosf(angleV);
  b.z = p.x*sinf(angleH) + p.z*cosf(angleH);

  return center + b;
}

//! Rotates a point around a center in space.
/** \a center center of rotation
    \a angleH,angleV rotation angles in radians (positive is counterclockwise (CCW) ) )
    \a p the point
    \returns the rotated point */
inline Vector RotatePoint2(const Vector center, float angleH, float angleV, Vector p)
{
  p.x -= center.x;
  p.y -= center.y;
  p.z -= center.z;

  Vector a;
  a.x = p.x*cosf(angleH) - p.z*sinf(angleH);
  a.y = p.y;
  a.z = p.x*sinf(angleH) + p.z*cosf(angleH);

  Vector b;
  b.x = a.x;
  b.y = a.z*sinf(angleV) + a.y*cosf(angleV);
  b.z = a.z*cosf(angleV) - a.y*sinf(angleV);

  return center + b;
}

//! Returns the angle between point (x,y) and (0,0)
float RotateAngle(float x, float y)
{
  if ( (x == 0.0f) && (y == 0.0f) )
    return 0.0f;

  float atan = atan2(x, y);

  if ((y < 0.0f) && (x >= 0.0f))
    return -atan + 2.5f*PI;
  else
    return -atan + 0.5f*PI;
}

//! Calculates the angle between two points and one center
/** \a center the center point
    \a p1,p2 the two points
    \returns The angle in radians  (positive is counterclockwise (CCW) ) */
inline float RotateAngle(const Point &center, const Point &p1, const Point &p2)
{
  if (PointsEqual(p1, center))
    return 0;

  if (PointsEqual(p2, center))
    return 0;

  float a1 = asinf((p1.y - center.y) / Distance(p1, center));
  float a2 = asinf((p2.y - center.y) / Distance(p2, center));

  if (p1.x < center.x) a1 = PI - a1;
  if (p2.x < center.x) a2 = PI - a2;

  float a = a2 - a1;
  if (a < 0)
    a += 2.0f*PI;

  return a;
}

//! Loads view matrix from the given vectors
/** \a from origin
    \a at view direction
    \a worldUp up vector */
inline void LoadViewMatrix(Matrix &mat, const Vector &from, const Vector &at, const Vector &worldUp)
{
  // Get the z basis vector, which points straight ahead. This is the
  // difference from the eyepoint to the lookat point.
  Vector view = at - from;

  float length = view.Length();
  assert(! IsZero(length) );

  // Normalize the z basis vector
  view /= length;

  // Get the dot product, and calculate the projection of the z basis
  // vector onto the up vector. The projection is the y basis vector.
  float dotProduct = DotProduct(worldUp, view);

  Vector up = worldUp - dotProduct * view;

  // If this vector has near-zero length because the input specified a
  // bogus up vector, let's try a default up vector
  if ( IsZero(length = up.Length()) )
  {
    up = Vector(0.0f, 1.0f, 0.0f) - view.y * view;

    // If we still have near-zero length, resort to a different axis.
    if ( IsZero(length = up.Length()) )
    {
      up = Vector(0.0f, 0.0f, 1.0f) - view.z * view;

      assert(! IsZero(up.Length()) );
    }
  }

  // Normalize the y basis vector
  up /= length;

  // The x basis vector is found simply with the cross product of the y
  // and z basis vectors
  Vector right = CrossProduct(up, view);

  // Start building the matrix. The first three rows contains the basis
  // vectors used to rotate the view to point at the lookat point
  mat.LoadIdentity();

  /* (1,1) */ mat.m[0 ] = right.x;
  /* (2,1) */ mat.m[1 ] = up.x;
  /* (3,1) */ mat.m[2 ] = view.x;
  /* (1,2) */ mat.m[4 ] = right.y;
  /* (2,2) */ mat.m[5 ] = up.y;
  /* (3,2) */ mat.m[6 ] = view.y;
  /* (1,3) */ mat.m[8 ] = right.z;
  /* (2,3) */ mat.m[9 ] = up.z;
  /* (3,3) */ mat.m[10] = view.z;

  // Do the translation values (rotations are still about the eyepoint)
  /* (1,4) */ mat.m[12] = -DotProduct(from, right);
  /* (2,4) */ mat.m[13] = -DotProduct(from, up);
  /* (3,4) */ mat.m[14] = -DotProduct(from, view);
}

//! Loads a perspective projection matrix
/** \a fov field of view in radians
    \a aspect aspect ratio (width / height)
    \a nearPlane distance to near cut plane
    \a farPlane distance to far cut plane */
inline void LoadProjectionMatrix(Matrix &mat, float fov = 1.570795f, float aspect = 1.0f,
                                 float nearPlane = 1.0f, float farPlane = 1000.0f)
{
  assert(fabs(farPlane - nearPlane) >= 0.01f);
  assert(fabs(sin(fov / 2)) >= 0.01f);

  float w = aspect * (cosf(fov / 2) / sinf(fov / 2));
  float h = 1.0f   * (cosf(fov / 2) / sinf(fov / 2));
  float q = farPlane / (farPlane - nearPlane);

  mat.LoadZero();

  /* (1,1) */ mat.m[0 ] = w;
  /* (2,2) */ mat.m[5 ] = h;
  /* (3,3) */ mat.m[10] = q;
  /* (3,4) */ mat.m[14] = 1.0f;
  /* (4,3) */ mat.m[11] = -q * nearPlane;
}

//! Loads a translation matrix from given vector
/** \a trans vector of translation*/
inline void LoadTranslationMatrix(Matrix &mat, const Vector &trans)
{
  mat.LoadIdentity();
  /* (1,4) */ mat.m[12] = trans.x;
  /* (2,4) */ mat.m[13] = trans.y;
  /* (3,4) */ mat.m[14] = trans.z;
}

//! Loads a scaling matrix fom given vector
/** \a scale vector with scaling factors for X, Y, Z */
inline void LoadScaleMatix(Matrix &mat, const Vector &scale)
{
  mat.LoadIdentity();
  /* (1,1) */ mat.m[0 ] = scale.x;
  /* (2,2) */ mat.m[5 ] = scale.y;
  /* (3,3) */ mat.m[10] = scale.z;
}

//! Loads a rotation matrix along the X axis
/** \a angle angle in radians */
inline void LoadRotationXMatrix(Matrix &mat, float angle)
{
  mat.LoadIdentity();
  /* (2,2) */ mat.m[5 ] =  cosf(angle);
  /* (3,2) */ mat.m[6 ] =  sinf(angle);
  /* (2,3) */ mat.m[9 ] = -sinf(angle);
  /* (3,3) */ mat.m[10] =  cosf(angle);
}

//! Loads a rotation matrix along the Y axis
/** \a angle angle in radians */
inline void LoadRotationYMatrix(Matrix &mat, float angle)
{
  mat.LoadIdentity();
  /* (1,1) */ mat.m[0 ] =  cosf(angle);
  /* (3,1) */ mat.m[2 ] = -sinf(angle);
  /* (1,3) */ mat.m[8 ] =  sinf(angle);
  /* (3,3) */ mat.m[10] =  cosf(angle);
}

//! Loads a rotation matrix along the Z axis
/** \a angle angle in radians */
inline void LoadRotationZMatrix(Matrix &mat, float angle)
{
  mat.LoadIdentity();
  /* (1,1) */ mat.m[0 ] =  cosf(angle);
  /* (2,1) */ mat.m[1 ] =  sinf(angle);
  /* (1,2) */ mat.m[4 ] = -sinf(angle);
  /* (2,2) */ mat.m[5 ] =  cosf(angle);
}

//! Loads a rotation matrix along the given axis
/** \a dir axis of rotation
    \a angle angle in radians */
inline void LoadRotationMatrix(Matrix &mat, const Vector &dir, float angle)
{
  float cos = cosf(angle);
  float sin = sinf(angle);
  Vector v = Normalize(dir);

  mat.LoadIdentity();

  /* (1,1) */ mat.m[0 ] = (v.x * v.x) * (1.0f - cos) + cos;
  /* (2,1) */ mat.m[1 ] = (v.x * v.y) * (1.0f - cos) - (v.z * sin);
  /* (3,1) */ mat.m[2 ] = (v.x * v.z) * (1.0f - cos) + (v.y * sin);

  /* (1,2) */ mat.m[4 ] = (v.y * v.x) * (1.0f - cos) + (v.z * sin);
  /* (2,2) */ mat.m[5 ] = (v.y * v.y) * (1.0f - cos) + cos ;
  /* (3,2) */ mat.m[6 ] = (v.y * v.z) * (1.0f - cos) - (v.x * sin);

  /* (1,3) */ mat.m[8 ] = (v.z * v.x) * (1.0f - cos) - (v.y * sin);
  /* (2,3) */ mat.m[9 ] = (v.z * v.y) * (1.0f - cos) + (v.x * sin);
  /* (3,3) */ mat.m[10] = (v.z * v.z) * (1.0f - cos) + cos;
}

//! Calculates the matrix to make three rotations in the order X, Z and Y
inline void LoadRotationXZYMatrix(Matrix &mat, const Vector &angle)
{
  LoadRotationXMatrix(mat, angle.x);

  Matrix temp;
  LoadRotationZMatrix(temp, angle.z);
  mat.Multiply(temp);

  LoadRotationYMatrix(temp, angle.y);
  mat.Multiply(temp);
}

//! Calculates the matrix to make three rotations in the order Z, X and Y
inline void LoadRotationZXYMatrix(Matrix &mat, const Vector &angle)
{
  LoadRotationZMatrix(mat, angle.z);

  Matrix temp;
  LoadRotationXMatrix(temp, angle.x);
  mat.Multiply(temp);

  LoadRotationYMatrix(temp, angle.y);
  mat.Multiply(temp);
}

//! Returns the distance between projections on XZ plane of two vectors
inline float DistanceProjected(const Vector &a, const Vector &b)
{
  return sqrtf( (a.x-b.x)*(a.x-b.x) +
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

//! Returns a point on the line \a p1 - \a p2, in \a dist distance from \a p1
/** \a p1,p2 line start and end
    \a dist scaling factor from \a p1, relative to distance between \a p1 and \a p2 */
inline Vector SegmentPoint(const Vector &p1, const Vector &p2, float dist)
{
  return p1 + (p2 - p1) * dist;
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

//! TODO documentation
inline Vector Transform(const Matrix &m, const Vector &p)
{
  return MatrixVectorMultiply(m, p);
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

//! Calculates point of view to look at a center two angles and a distance
inline Vector RotateView(Vector center, float angleH, float angleV, float dist)
{
  Matrix mat1, mat2;
  LoadRotationZMatrix(mat1, -angleV);
  LoadRotationYMatrix(mat2, -angleH);

  Matrix mat = MultiplyMatrices(mat1, mat2);

  Vector eye;
  eye.x = 0.0f+dist;
  eye.y = 0.0f;
  eye.z = 0.0f;
  eye = Transform(mat, eye);

  return eye+center;
}

/* @} */ // end of group

}; // namespace Math
