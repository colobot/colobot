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

// math/matrix.h

/* Matrix struct and functions */

#pragma once

#include "const.h"
#include "func.h"
#include "vector.h"

#include <cmath>
#include <cassert>

// Math module namespace
namespace Math
{

/** 4x4 matrix

  Represents an universal 4x4 matrix that can be used in OpenGL and DirectX engines.
  Contains the required methods for operating on matrices (inverting, multiplying, etc.).

  The internal representation is a 16-value table in column-major order, thus:

   m[0 ] m[4 ] m[8 ] m[12]
   m[1 ] m[5 ] m[9 ] m[13]
   m[2 ] m[6 ] m[10] m[14]
   m[3 ] m[7 ] m[11] m[15]

  This representation is native to OpenGL; DirectX requires transposing the matrix.

  The order of multiplication of matrix and vector is also OpenGL-native
  (see the method Vector::MultiplyMatrix).

  All methods are made inline to maximize optimization.

  TODO test

 **/
struct Matrix
{
  //! Matrix values in column-major format
  float m[16];

  //! Creates the indentity matrix
  inline Matrix()
  {
    LoadIdentity();
  }

  //! Creates the matrix from given values
  /** \a m  values in column-major format */
  inline Matrix(const float (&m)[16])
  {
    for (int i = 0; i < 16; ++i)
      this->m[i] = m[i];
  }

  //! Loads the zero matrix
  inline void LoadZero()
  {
    for (int i = 0; i < 16; ++i)
      m[i] = 0.0f;
  }

  //! Loads the identity matrix
  inline void LoadIdentity()
  {
    LoadZero();
    m[0] = m[5] = m[10] = m[15] = 1.0f;
  }

  //! Transposes the matrix
  inline void Transpose()
  {
    Matrix temp = *this;
    for (int r = 0; r < 4; ++r)
    {
      for (int c = 0; c < 4; ++c)
      {
        m[4*r+c] = temp.m[4*c+r];
      }
    }
  }

  //! Calculates the determinant of the matrix
  /** \returns the determinant */
  inline float Det() const
  {
    float result = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
      result += m[i] * Cofactor(i, 0);
    }
    return result;
  }

  //! Calculates the cofactor of the matrix
  /** \a r row (0 to 3)
      \a c column (0 to 3)
      \returns the cofactor or 0.0f if invalid r, c given*/
  inline float Cofactor(int r, int c) const
  {
    assert(r >= 0 && r <= 3);
    assert(c >= 0 && c <= 3);

    float result = 0.0f;

    /* That looks horrible, I know. But it's fast :) */

    switch (4*r + c)
    {
      // r=0, c=0
      /* 05 09 13
         06 10 14
         07 11 15 */
      case 0:
        result = + m[5 ] * (m[10] * m[15] - m[14] * m[11])
                 - m[9 ] * (m[6 ] * m[15] - m[14] * m[7 ])
                 + m[13] * (m[6 ] * m[11] - m[10] * m[7 ]);
        break;

      // r=0, c=1
      /* 01 09 13
         02 10 14
         03 11 15 */
      case 1:
        result = - m[1 ] * (m[10] * m[15] - m[14] * m[11])
                 + m[9 ] * (m[2 ] * m[15] - m[14] * m[3 ])
                 - m[13] * (m[2 ] * m[11] - m[10] * m[3 ]);
        break;

      // r=0, c=2
      /* 01 05 13
         02 06 14
         03 07 15 */
      case 2:
        result = + m[1 ] * (m[6 ] * m[15] - m[14] * m[7 ])
                 - m[5 ] * (m[2 ] * m[15] - m[14] * m[3 ])
                 + m[13] * (m[2 ] * m[7 ] - m[6 ] * m[3 ]);
        break;

      // r=0, c=3
      /* 01 05 09
         02 06 10
         03 07 11 */
      case 3:
        result = - m[1 ] * (m[6 ] * m[11] - m[10] * m[7 ])
                 + m[5 ] * (m[2 ] * m[11] - m[10] * m[3 ])
                 - m[9 ] * (m[2 ] * m[7 ] - m[6 ] * m[3 ]);
        break;

      // r=1, c=0
      /* 04 08 12
         06 10 14
         07 11 15 */
      case 4:
        result = - m[4 ] * (m[10] * m[15] - m[14] * m[11])
                 + m[8 ] * (m[6 ] * m[15] - m[14] * m[7 ])
                 - m[12] * (m[6 ] * m[11] - m[10] * m[7 ]);
        break;

      // r=1, c=1
      /* 00 08 12
         02 10 14
         03 11 15 */
      case 5:
        result = + m[0 ] * (m[10] * m[15] - m[14] * m[11])
                 - m[8 ] * (m[2 ] * m[15] - m[14] * m[3 ])
                 + m[12] * (m[2 ] * m[11] - m[10] * m[3 ]);
        break;

      // r=1, c=2
      /* 00 04 12
         02 06 14
         03 07 15 */
      case 6:
        result = - m[0 ] * (m[6 ] * m[15] - m[14] * m[7 ])
                 + m[4 ] * (m[2 ] * m[15] - m[14] * m[3 ])
                 - m[12] * (m[2 ] * m[7 ] - m[6 ] * m[3 ]);
        break;

      // r=1, c=3
      /* 00 04 08
         02 06 10
         03 07 11 */
      case 7:
        result = + m[0 ] * (m[6 ] * m[11] - m[10] * m[7 ])
                 - m[4 ] * (m[2 ] * m[11] - m[10] * m[3 ])
                 + m[8 ] * (m[2 ] * m[7 ] - m[6 ] * m[3 ]);
        break;

      // r=2, c=0
      /* 04 08 12
         05 09 13
         07 11 15 */
      case 8:
        result = + m[4 ] * (m[9 ] * m[15] - m[13] * m[11])
                 - m[8 ] * (m[5 ] * m[15] - m[13] * m[7 ])
                 + m[12] * (m[5 ] * m[11] - m[9 ] * m[7 ]);
        break;

      // r=2, c=1
      /* 00 08 12
         01 09 13
         03 11 15 */
      case 9:
        result = - m[0 ] * (m[9 ] * m[15] - m[13] * m[11])
                 + m[8 ] * (m[1 ] * m[15] - m[13] * m[3 ])
                 - m[12] * (m[1 ] * m[11] - m[9 ] * m[3 ]);
        break;

      // r=2, c=2
      /* 00 04 12
         01 05 13
         03 07 15 */
      case 10:
        result = + m[0 ] * (m[5 ] * m[15] - m[13] * m[7 ])
                 - m[4 ] * (m[1 ] * m[15] - m[13] * m[3 ])
                 + m[12] * (m[1 ] * m[7 ] - m[5 ] * m[3 ]);
        break;

      // r=2, c=3
      /* 00 04 08
         01 05 09
         03 07 11 */
      case 11:
        result = - m[0 ] * (m[5 ] * m[11] - m[9 ] * m[7 ])
                 + m[4 ] * (m[1 ] * m[11] - m[9 ] * m[3 ])
                 - m[8 ] * (m[1 ] * m[7 ] - m[5 ] * m[3 ]);
        break;

      // r=3, c=0
      /* 04 08 12
         05 09 13
         06 10 14 */
      case 12:
        result = - m[4 ] * (m[9 ] * m[14] - m[13] * m[10])
                 + m[8 ] * (m[5 ] * m[14] - m[13] * m[6 ])
                 - m[12] * (m[5 ] * m[10] - m[9 ] * m[6 ]);
        break;

      // r=3, c=1
      /* 00 08 12
         01 09 13
         02 10 14 */
      case 13:
        result = + m[0 ] * (m[9 ] * m[14] - m[13] * m[10])
                 - m[8 ] * (m[1 ] * m[14] - m[13] * m[2 ])
                 + m[12] * (m[1 ] * m[10] - m[9 ] * m[2 ]);
        break;

      // r=3, c=2
      /* 00 04 12
         01 05 13
         02 06 14 */
      case 14:
        result = - m[0 ] * (m[5 ] * m[14] - m[13] * m[6 ])
                 + m[4 ] * (m[1 ] * m[14] - m[13] * m[2 ])
                 - m[12] * (m[1 ] * m[6 ] - m[5 ] * m[2 ]);
        break;

      // r=3, c=3
      /* 00 04 08
         01 05 09
         02 06 10 */
      case 15:
        result = + m[0 ] * (m[5 ] * m[10] - m[9 ] * m[6 ])
                 - m[4 ] * (m[1 ] * m[10] - m[9 ] * m[2 ])
                 + m[8 ] * (m[1 ] * m[6 ] - m[5 ] * m[2 ]);
        break;

      default:
        break;
    }

    return result;
  }

  //! Inverts the matrix
  inline void Invert()
  {
    float d = Det();
    assert(! IsZero(d));

    Matrix temp = *this;
    for (int r = 0; r < 4; ++r)
    {
      for (int c = 0; c < 4; ++c)
      {
        m[4*r+c] = (1.0f / d) * temp.Cofactor(r, c);
      }
    }
  }

  //! Multiplies the matrix with the given matrix
  /** \a right right-hand matrix */
  inline void Multiply(const Matrix &right)
  {
    Matrix left = *this;
    for (int c = 0; c < 4; ++c)
    {
      for (int r = 0; r < 4; ++r)
      {
        m[4*c+r] = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
          m[4*c+r] += left.m[4*i+r] * right.m[4*c+i];
        }
      }
    }
  }

  //! Loads view matrix from the given vectors
  /** \a from origin
      \a at view direction
      \a worldUp up vector */
  inline void LoadView(const Vector &from, const Vector &at, const Vector &worldUp)
  {
    // Get the z basis vector, which points straight ahead. This is the
    // difference from the eyepoint to the lookat point.
    Vector view = at - from;

    float length = view.Length();
    assert(! Math::IsZero(length) );

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
    LoadIdentity();
    m[0 ] = right.x;   m[1 ] = up.x;   m[2 ] = view.x;
    m[4 ] = right.y;   m[5 ] = up.y;   m[6 ] = view.y;
    m[8 ] = right.z;   m[9 ] = up.z;   m[10] = view.z;

    // Do the translation values (rotations are still about the eyepoint)
    m[12] = - DotProduct(from, right);
    m[13] = - DotProduct(from, up);
    m[14] = - DotProduct(from, view);
  }

  inline void LoadProjection(float fov = 1.570795f, float aspect = 1.0f,
                             float nearPlane = 1.0f, float farPlane = 1000.0f)
  {
    // TODO
  }

  inline void LoadTranslation(const Vector &trans)
  {
    LoadIdentity();
    m[12] = trans.x;
    m[13] = trans.y;
    m[14] = trans.z;
  }

  inline void LoadScale(const Vector &scale)
  {
    LoadIdentity();
    m[0] = scale.x;
    m[5] = scale.y;
    m[10] = scale.z;
  }

  inline void LoadRotationX(float angle)
  {
    // TODO
  }

  inline void LoadRotationY(float angle)
  {
    // TODO
  }

  inline void LoadRotationZ(float angle)
  {
    // TODO
  }

  inline void LoadRotation(const Vector &dir, float angle)
  {
    // TODO
  }

  //! Calculates the matrix to make three rotations in the order X, Z and Y
  inline void RotateXZY(const Vector &angle)
  {
    this->LoadRotationX(angle.x);

    Matrix temp;
    temp.LoadRotationZ(angle.z);
    this->Multiply(temp);

    temp.LoadRotationY(angle.y);
    this->Multiply(temp);
  }

  //! Calculates the matrix to make three rotations in the order Z, X and Y
  inline void RotateZXY(const Vector &angle)
  {
    this->LoadRotationZ(angle.z);

    Matrix temp;
    temp.LoadRotationX(angle.x);
    this->Multiply(temp);

    temp.LoadRotationY(angle.y);
    this->Multiply(temp);
  }
};

//! Convenience function for inverting a matrix
/** \a m input matrix
    \a result result -- inverted matrix */
inline void InvertMatrix(const Matrix &m, Matrix &result)
{
  result = m;
  result.Invert();
}

//! Convenience function for multiplying a matrix
/** \a left left-hand matrix
    \a right right-hand matrix
    \a result result -- multiplied matrices */
inline void MultiplyMatrices(const Matrix &left, const Matrix &right, Matrix &result)
{
  result = left;
  result.Multiply(right);
}

//! Calculates the result of multiplying m * v
inline Vector MatrixVectorMultiply(const Matrix &m, const Vector &v)
{
  float x = v.x * m.m[0 ] + v.y * m.m[4 ] + v.z * m.m[8 ] + m.m[12];
  float y = v.x * m.m[1 ] + v.y * m.m[5 ] + v.z * m.m[9 ] + m.m[13];
  float z = v.x * m.m[2 ] + v.y * m.m[6 ] + v.z * m.m[10] + m.m[14];
  float w  = v.x * m.m[4 ] + v.y * m.m[7 ] + v.z * m.m[11] + m.m[15];

  if (IsZero(w))
    return Vector(x, y, z);

  x /= w;
  y /= w;
  z /= w;

  return Vector(x, y, z);
}

//! Checks if two matrices are equal within given \a tolerance
inline bool MatricesEqual(const Math::Matrix &m1, const Math::Matrix &m2,
                          float tolerance = Math::TOLERANCE)
{
  for (int i = 0; i < 16; ++i)
  {
    if (! IsEqual(m1.m[i], m2.m[i], tolerance))
      return false;
  }

  return true;
}

}; // namespace Math
