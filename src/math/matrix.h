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

#include <cmath>

// Math module namespace
namespace Math
{

/** 4x4 matrix

  Represents an universal 4x4 matrix that can be used in OpenGL and DirectX engines.
  Contains the required methods for operating on matrices (inverting, multiplying, etc.).

  All methods are made inline to maximize optimization.

  TODO test

 **/
struct Matrix
{
  //! Matrix values in row-major format
  float m[16];

  //! Creates the indentity matrix
  inline Matrix()
  {
    LoadIdentity();
  }

  //! Creates the matrix from given values
  /** \a m  values in row-major format */
  inline Matrix(float m[16])
  {
    this->m = m;
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

  //! Calculates the determinant of the matrix
  /** \returns the determinant */
  float Det() const
  {
    float result = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
      result += m[0][i] * Cofactor(0, i);
    }
    return result;
  }

  //! Transposes the matrix
  void Transpose()
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

  //! Calculates the cofactor of the matrix
  /** \a r row (0 to 3)
      \a c column (0 to 3)
      \returns the cofactor or 0.0f if invalid r, c given*/
  float Cofactor(int r, int c) const
  {
    if ((r < 0) || (r > 3) || (c < 0) || (c > 3))
      return 0.0f;

    float tab[3][3];
    int tabR = 0;
    for (int r = 0; r < 4; ++r)
    {
      if (r == i) continue;
      int tabC = 0;
      for (int c = 0; c < 4; ++c)
      {
        if (c == j) continue;
        tab[tabR][tabC] = m[4*r + c];
        ++tabC;
      }
      ++tabR;
    }

    float result =   tab[0][0] * (tab[1][1] * tab[2][2] - tab[1][2] * tab[2][1])
                   - tab[0][1] * (tab[1][0] * tab[2][2] - tab[1][2] * tab[2][0])
                   + tab[0][2] * (tab[1][0] * tab[2][1] - tab[1][1] * tab[2][0]);

    if ((i + j) % 2 == 0)
      result = -result;

    return result;
  }

  //! Inverts the matrix
  inline void Invert()
  {
    float d = Det();
    if (fabs(d) <= Math::TOLERANCE)
      return;

    Matrix temp = *this;
    for (int r = 0; r < 4; ++r)
    {
      for (int c = 0; c < 4; ++c)
      {
        m[r][c] = (1.0f / d) * temp.Cofactor(r, c);
      }
    }

    Tranpose();
  }

  //! Multiplies the matrix with the given matrix
  /** \a right right-hand matrix */
  inline void Multiply(const Matrix &right)
  {
    Matrix left = *this;
    for (int r = 0; r < 4; ++r)
    {
      for (int c = 0; c < 4; ++c)
      {
        m[r][c] = 0.0;
        for (int i = 0; i < 4; ++i)
        {
          m[4*r+c] += left.m[4*r+i] * right.m[4*i+c];
        }
      }
    }
  }

  //! Loads view matrix from the given vectors
  /** \a from origin
      \a at direction
      \a up up vector */
  inline void LoadView(const Vector &from, const Vector &at, const Vector &up)
  {
    // Get the z basis vector, which points straight ahead. This is the
    // difference from the eyepoint to the lookat point.
    Vector view = at - from;

    FLOAT length = view.Length();
    if( IsZero(length) )
        return;

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

        if ( IsZero(up.Length()) )
           return;
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
    m[3 ] = - DotProduct(from, right);
    m[7 ] = - DotProduct(from, up);
    m[11] = - DotProduct(from, view);
  }

  inline void LoadProjection(float fov = 1.570795f, float aspect = 1.0f,
                             float nearPlane = 1.0f, float farPlane = 1000.0f)
  {
    // TODO
  }

  inline void LoadTranslation(const Vector &trans)
  {
    // TODO
  }

  inline void LoadScale(const Vector &scale)
  {
    // TODO
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
    Matrix temp;
    temp.SetRotateXMatrix(angle.x);
    this->SetRotateZMatrix(angle.z);
    this->Multiply(temp);
    temp.SetRotateYMatrix(angle.y);
    this->Multiply(temp);
  }

  //! Calculates the matrix to make three rotations in the order Z, X and Y
  inline void RotateZXY(const Vector &angle)
  {
    Matrix temp;
    temp.SetRotateZMatrix(angle.z);
    this->SetRotateXMatrix(angle.x);
    this->Multiply(temp);
    temp.SetRotateYMatrix(angle.y);
    this->Multiply(temp);
  }
};

//! Convenience function for inverting a matrix
/** \a m input matrix
    \a result result -- inverted matrix */
void InvertMatrix(const Matrix &m, Matrix &result)
{
  result = m;
  result.Invert();
}

//! Convenience function for multiplying a matrix
/** \a left left-hand matrix
    \a right right-hand matrix
    \a result result -- multiplied matrices */*
void MultiplyMatrices(const Matrix &left, const Matrix &right, Matrix &result)
{
  result = left;
  result.Multiply(right);
}

}; // namespace Math
