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
 * \file math/matrix.h
 * \brief Matrix struct and related functions
 */

#pragma once


#include "math/const.h"
#include "math/func.h"
#include "math/vector.h"


#include <cmath>
#include <cassert>


// Math module namespace
namespace Math {

/**
 * \struct Matrix math/matrix.h
 * \brief 4x4 matrix
 *
 * Represents an universal 4x4 matrix that can be used in OpenGL and DirectX engines.
 * Contains the required methods for operating on matrices (inverting, multiplying, etc.).
 *
 * The internal representation is a 16-value table in column-major order, thus:

\verbatim
m[0 ] m[4 ] m[8 ] m[12]
m[1 ] m[5 ] m[9 ] m[13]
m[2 ] m[6 ] m[10] m[14]
m[3 ] m[7 ] m[11] m[15]
\endverbatim

 * This representation is native to OpenGL; DirectX requires transposing the matrix.
 *
 * The order of multiplication of matrix and vector is also OpenGL-native
 * (see the function MatrixVectorMultiply).
 *
 * All methods are made inline to maximize optimization.
 *
 * Unit tests for the structure and related functions are in module: math/test/matrix_test.cpp.
 *
 */
struct Matrix
{
    //! Matrix values in column-major order
    float m[16];

    //! Creates the indentity matrix
    inline Matrix()
    {
        LoadIdentity();
    }

    //! Creates the matrix from 1D array
    /** \a m matrix values in column-major order */
    inline explicit Matrix(const float (&_m)[16])
    {
        for (int i = 0; i < 16; ++i)
            m[i] = _m[i];
    }

    //! Creates the matrix from 2D array
    /**
     * The array's first index is row, second is column.
     * \param m array with values
     */
    inline explicit Matrix(const float (&_m)[4][4])
    {
        for (int c = 0; c < 4; ++c)
        {
            for (int r = 0; r < 4; ++r)
            {
                m[4*c+r] = _m[r][c];
            }
        }
    }

    //! Sets value in given row and col
    /**
     * \param row row (1 to 4)
     * \param col column (1 to 4)
     * \param value value
     */
    inline void Set(int row, int col, float value)
    {
        m[(col-1)*4+(row-1)] = value;
    }

    //! Returns the value in given row and col
    /**
     * \param row row (1 to 4)
     * \param col column (1 to 4)
     * \returns value
     */
    inline float Get(int row, int col)
    {
        return m[(col-1)*4+(row-1)];
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
        /* (1,1) */ m[0 ] = 1.0f;
        /* (2,2) */ m[5 ] = 1.0f;
        /* (3,3) */ m[10] = 1.0f;
        /* (4,4) */ m[15] = 1.0f;
    }

    //! Returns the struct cast to \c float* array; use with care!
    inline float* Array()
    {
        return reinterpret_cast<float*>(this);
    }

    //! Transposes the matrix
    inline void Transpose()
    {
        /* (2,1) <-> (1,2) */ Swap(m[1 ], m[4 ]);
        /* (3,1) <-> (1,3) */ Swap(m[2 ], m[8 ]);
        /* (4,1) <-> (1,4) */ Swap(m[3 ], m[12]);
        /* (3,2) <-> (2,3) */ Swap(m[6 ], m[9 ]);
        /* (4,2) <-> (2,4) */ Swap(m[7 ], m[13]);
        /* (4,3) <-> (3,4) */ Swap(m[11], m[14]);
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
    /**
     * \param r  row (0 to 3)
     * \param c  column (0 to 3)
     * \returns  the cofactor
     */
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

    //! Calculates the inverse matrix
    /**
     * The determinant of the matrix must not be zero.
     * \returns the inverted matrix
     */
    inline Matrix Inverse() const
    {
        float d = Det();
        assert(! IsZero(d));

        float result[16] = { 0.0f };

        for (int r = 0; r < 4; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                // Already transposed!
                result[4*r+c] = (1.0f / d) * Cofactor(r, c);
            }
        }

        return Matrix(result);
    }

    //! Calculates the multiplication of this matrix * given matrix
    /**
     * \param right right-hand matrix
     * \returns multiplication result
     */
    inline Matrix Multiply(const Matrix &right) const
    {
        float result[16] = { 0.0f };

        for (int c = 0; c < 4; ++c)
        {
            for (int r = 0; r < 4; ++r)
            {
                result[4*c+r] = 0.0f;
                for (int i = 0; i < 4; ++i)
                {
                    result[4*c+r] += m[4*i+r] * right.m[4*c+i];
                }
            }
        }

        return Matrix(result);
    }
}; // struct Matrix

//! Checks if two matrices are equal within given \a tolerance
inline bool MatricesEqual(const Matrix &m1, const Matrix &m2,
                          float tolerance = TOLERANCE)
{
    for (int i = 0; i < 16; ++i)
    {
        if (! IsEqual(m1.m[i], m2.m[i], tolerance))
            return false;
    }

    return true;
}

//! Convenience function for getting transposed matrix
inline Math::Matrix Transpose(const Math::Matrix &m)
{
    Math::Matrix result = m;
    result.Transpose();
    return result;
}

//! Convenience function for multiplying a matrix
/** \a left left-hand matrix
    \a right right-hand matrix
    \returns multiplied matrices */
inline Math::Matrix MultiplyMatrices(const Math::Matrix &left, const Math::Matrix &right)
{
    return left.Multiply(right);
}

//! Calculates the result of multiplying m * v
/**
    The multiplication is performed thus:

\verbatim
[  m.m[0 ] m.m[4 ] m.m[8 ] m.m[12]  ]   [ v.x ]
[  m.m[1 ] m.m[5 ] m.m[9 ] m.m[13]  ]   [ v.y ]
[  m.m[2 ] m.m[6 ] m.m[10] m.m[14]  ] * [ v.z ]
[  m.m[3 ] m.m[7 ] m.m[11] m.m[15]  ]   [  1  ]
\endverbatim

   The result, a 4x1 vector is then converted to 3x1 by dividing
   x,y,z coords by the fourth coord (w). */
inline Math::Vector MatrixVectorMultiply(const Math::Matrix &m, const Math::Vector &v, bool wDivide = false)
{
    float x = v.x * m.m[0 ] + v.y * m.m[4 ] + v.z * m.m[8 ] + m.m[12];
    float y = v.x * m.m[1 ] + v.y * m.m[5 ] + v.z * m.m[9 ] + m.m[13];
    float z = v.x * m.m[2 ] + v.y * m.m[6 ] + v.z * m.m[10] + m.m[14];

    if (!wDivide)
        return Math::Vector(x, y, z);

    float w = v.x * m.m[3 ] + v.y * m.m[7 ] + v.z * m.m[11] + m.m[15];

    if (IsZero(w))
        return Math::Vector(x, y, z);

    x /= w;
    y /= w;
    z /= w;

    return Math::Vector(x, y, z);
}


} // namespace Math

