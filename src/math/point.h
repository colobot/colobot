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

/** @defgroup MathPointModule math/point.h
   Contains the Point struct and related functions.
 */

#pragma once

#include "const.h"
#include "func.h"

#include <cmath>


// Math module namespace
namespace Math
{

/* @{ */ // start of group

/** \struct Point math/point.h
    \brief 2D point

  Represents a 2D point (x, y).
  Contains the required methods for operating on points.

  All methods are made inline to maximize optimization.

 */
struct Point
{
  //! X coord
  float x;
  //! Y coord
  float y;

  //! Constructs a zero point: (0,0)
  inline Point()
  {
    LoadZero();
  }

  //! Constructs a point from given coords: (x,y)
  inline explicit Point(float x, float y)
  {
    this->x = x;
    this->y = y;
  }

  //! Sets the zero point: (0,0)
  inline void LoadZero()
  {
    x = y = 0.0f;
  }

  //! Returns the distance from (0,0) to the point (x,y)
  inline float Length()
  {
    return sqrt(x*x + y*y);
  }

    //! Returns the inverted point
  inline Point operator-() const
  {
    return Point(-x, -y);
  }

  //! Adds the given point
  inline const Point& operator+=(const Point &right)
  {
    x += right.x;
    y += right.y;
    return *this;
  }

  //! Adds two points
  inline friend const Point operator+(const Point &left, const Point &right)
  {
    return Point(left.x + right.x, left.y + right.y);
  }

  //! Subtracts the given vector
  inline const Point& operator-=(const Point &right)
  {
    x -= right.x;
    y -= right.y;
    return *this;
  }

  //! Subtracts two points
  inline friend const Point operator-(const Point &left, const Point &right)
  {
    return Point(left.x - right.x, left.y - right.y);
  }

  //! Multiplies by given scalar
  inline const Point& operator*=(const float &right)
  {
    x *= right;
    y *= right;
    return *this;
  }

  //! Multiplies point by scalar
  inline friend const Point operator*(const float &left, const Point &right)
  {
    return Point(left * right.x, left * right.y);
  }

  //! Multiplies point by scalar
  inline friend const Point operator*(const Point &left, const float &right)
  {
    return Point(left.x * right, left.y * right);
  }

  //! Divides by given scalar
  inline const Point& operator/=(const float &right)
  {
    x /= right;
    y /= right;
    return *this;
  }

  //! Divides point by scalar
  inline friend const Point operator/(const Point &left, const float &right)
  {
    return Point(left.x / right, left.y / right);
  }

}; // struct Point


//! Checks if two vectors are equal within given \a tolerance
inline bool PointsEqual(const Point &a, const Point &b, float tolerance = TOLERANCE)
{
  return IsEqual(a.x, b.x, tolerance) && IsEqual(a.y, b.y, tolerance);
}

//! Permutes two points
inline void Swap(Point &a, Point &b)
{
  Point  c;

  c = a;
  a = b;
  b = c;
}

//! Returns the distance between two points
inline float Distance(const Point &a, const Point &b)
{
  return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

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

//! Checks if the point is inside triangle defined by vertices \a a, \a b, \a c
inline bool IsInsideTriangle(Point a, Point b, Point c, const Point &p)
{
  if ( p.x < a.x && p.x < b.x && p.x < c.x )  return false;
  if ( p.x > a.x && p.x > b.x && p.x > c.x )  return false;
  if ( p.y < a.y && p.y < b.y && p.y < c.y )  return false;
  if ( p.y > a.y && p.y > b.y && p.y > c.y )  return false;

  if ( a.x > b.x ) Swap(a,b);
  if ( a.x > c.x ) Swap(a,c);
  if ( c.x < a.x ) Swap(c,a);
  if ( c.x < b.x ) Swap(c,b);

  float n, m;

  n = MidPoint(a, b, p.x);
  m = MidPoint(a, c, p.x);
  if ( (n>p.y || p.y>m) && (n<p.y || p.y<m) )
    return false;

  n = MidPoint(c, b, p.x);
  m = MidPoint(c, a, p.x);
  if ( (n>p.y || p.y>m) && (n<p.y || p.y<m) )
    return false;

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
    a += PI_MUL_2;

  return a;
}

/* @} */ // end of group

}; // namespace Math
