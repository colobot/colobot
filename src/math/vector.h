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

  //! Loads the zero vector (0, 0, 0, 0)
  inline void LoadZero()
  {
    x = y = z = w = 0.0f;
  }

  //! Returns the vector length
  inline float Length() const
  {
    return sqrt(x*x + y*y + z*z + w*w);
  }

  //! Normalizes the vector
  inline void Normalize()
  {
    float l = Length();
    x /= l;
    y /= l;
    z /= l;
    w /= l;
  }

  //! Calculates the cross product with another vector
  /** \a right right-hand side vector */
  inline void Cross(const Vector &right)
  {
    Vector left = *this;
    x = left.y * right.z - left.z * right.y;
    y = left.z * right.x - left.x * right.z;
    z = left.x * right.y - left.y * right.x;
  }

  //! Calculates the dot product with another vector
  inline float Dot(const Vector &right) const
  {
    return x * right.x + y * right.y + z * right.z;
  }

  //! Returns the cosine of angle between this and another vector
  inline float CosAngle(const Vector &right) const
  {
    return Dot(right) / (Length() * right.Length());
  }

  //! Returns angle (in radians) between this and another vector
  inline float Angle(const Vector &right) const
  {
    return acos(CosAngle(right));
  }

  //! Calculates the result of multiplying m * this vector
  inline MatrixMultiply(const Matrix &m)
  {
    float tx = x * m.m[0 ] + y * m.m[4 ] + z * m.m[8 ] + m.m[12];
    float ty = x * m.m[1 ] + y * m.m[5 ] + z * m.m[9 ] + m.m[13];
    float tz = x * m.m[2 ] + y * m.m[6 ] + z * m.m[10] + m.m[14];
    float tw = x * m.m[4 ] + y * m.m[7 ] + z * m.m[11] + m.m[15];

    if (IsZero(tw))
      return;

    x = tx / tw;
    y = ty / tw;
    z = tz / tw;
  }
};

}; // namespace Math
