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

#include <cmath>


/* TODO

 FPOINT    RotatePoint(FPOINT center, float angle, FPOINT p);
 FPOINT    RotatePoint(float angle, FPOINT p);
 FPOINT    RotatePoint(float angle, float dist);
 void      RotatePoint(float cx, float cy, float angle, float &px, float &py);
 void      RotatePoint(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p);
 void      RotatePoint2(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p);

 float   RotateAngle(float x, float y);
 float   RotateAngle(FPOINT center, FPOINT p1, FPOINT p2);
 float   MidPoint(FPOINT a, FPOINT b, float px);
 BOOL      IsInsideTriangle(FPOINT a, FPOINT b, FPOINT c, FPOINT p);

 BOOL      LineFunction(FPOINT p1, FPOINT p2, float &a, float &b);

 float IsInsideTriangle(FPOINT a, FPOINT b, FPOINT c);

 */

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
  inline Point(float x, float y)
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
    return std::sqrt(x*x + y*y);
  }
};

//! Returns the distance between two points
inline float Distance(const Point &a, const Point &b)
{
  return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

/* @} */ // end of group

}; // namespace Math
