// math/matrix.h

/* Matrix struct and functions */

#pragma once

#include "const.h"

#include <cmath>

/* TODO

 void      MatRotateXZY(D3DMATRIX &mat, D3DVECTOR angle);
 void      MatRotateZXY(D3DMATRIX &mat, D3DVECTOR angle);

*/

// Math module namespace
namespace Math
{

/** 4x4 matrix

  Represents an universal 4x4 matrix that can be used in OpenGL and DirectX engines.
  Contains the required methods for operating on matrices (inverting, multiplying, etc.).

  All methods are made inline to maximize optimization.

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
    m[0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
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
        m[r][c] = temp.m[c][r];
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
        tab[tabR][tabC] = m[r][c];
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
          m[r][c] += left.m[r][i] * right.m[i][c];
        }
      }
    }
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
