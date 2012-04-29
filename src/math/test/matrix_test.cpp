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

// math/test/matrix_test.cpp

/* Unit tests for Matrix struct

   Test data was randomly generated and the expected
   results calculated using GNU Octave.

 */

#include "../func.h"
#include "../matrix.h"

#include <cstdio>

using namespace std;

const float TEST_TOLERANCE = 1e-5;

int TestCofactor()
{
  const float TEST_MATRIX[16] =
  {
    -0.306479,
    -0.520207,
     0.127906,
     0.632922,

    -0.782876,
     0.015264,
     0.337479,
     1.466013,

     0.072725,
    -0.315123,
     1.613198,
    -0.577377,

     0.962397,
    -1.320724,
     1.467588,
     0.579020
  };

  const float EXPECTED_RESULTS[4][4] =
  {
    {  2.791599, -0.249952,  1.065075, -1.356570 },
    {  3.715943, -1.537511,  0.094812, -0.074520 },
    {  1.034500, -0.731752, -0.920756, -0.196235 },
    {  1.213928, -1.236857,  0.779741, -0.678482 }
  };

  Math::Matrix mat(TEST_MATRIX);

  for (int r = 0; r < 4; ++r)
  {
    for (int c = 0; c < 4; ++c)
    {
      float ret = mat.Cofactor(r, c);
      float exp = EXPECTED_RESULTS[r][c];
      if (! Math::IsEqual(ret, exp, TEST_TOLERANCE))
      {
        fprintf(stderr, "Cofactor r=%d, c=%d, %f (returned) != %f (expected)\n", r, c, ret, exp);
        return 4*c+r;
      }
    }
  }

  return 0;
}

int TestDet()
{
  const float TEST_MATRIX[16] =
  {
     0.85554,
     0.11624,
     1.30411,
     0.81467,

     0.49692,
    -1.92483,
    -1.33543,
     0.85042,

    -0.16775,
     0.35344,
     1.40673,
     0.13961,

     1.40709,
     0.11731,
     0.69042,
     0.91216
  };

  const float EXPECTED_RESULT = 0.084360;

  float ret = Math::Matrix(TEST_MATRIX).Det();
  if (! Math::IsEqual(ret, EXPECTED_RESULT, TEST_TOLERANCE))
  {
    fprintf(stderr, "Det %f (returned) != %f (expected)\n", ret, EXPECTED_RESULT);
    return 1;
  }

  return 0;
}

int TestInvert()
{
  const float TEST_MATRIX[16] =
  {
     1.4675123,
    -0.2857923,
    -0.0496217,
    -1.2825408,

    -0.2804135,
    -0.0826255,
    -0.6825495,
     1.1661259,

     0.0032798,
     0.5999200,
    -1.8359883,
    -1.1894424,

    -1.1501538,
    -2.8792485,
     0.0299345,
     0.3730919
  };

  const float EXPECTED_RESULT[16] =
  {
     0.685863,
     0.562274,
    -0.229722,
    -0.132079,

    -0.266333,
    -0.139862,
     0.054211,
    -0.305568,

    -0.130817,
    -0.494076,
    -0.358226,
    -0.047477,

     0.069486,
     0.693649,
    -0.261074,
    -0.081200
  };

  Math::Matrix mat(TEST_MATRIX);
  mat.Invert();

  if (! Math::MatricesEqual(mat, EXPECTED_RESULT, TEST_TOLERANCE))
  {
    fprintf(stderr, "Invert mismatch\n");
    return 1;
  }

  return 0;
}

int TestMultiply()
{
  const float TEST_MATRIX_A[16] =
  {
    -1.931420,
     0.843410,
     0.476929,
    -0.493435,
     1.425659,
    -0.176331,
     0.129096,
     0.551081,
    -0.543530,
    -0.190783,
    -0.084744,
     1.379547,
    -0.473377,
     1.643398,
     0.400539,
     0.702937
  };

  const float TEST_MATRIX_B[16] =
  {
     0.3517561,
     1.3903778,
    -0.8048254,
    -0.4090024,

    -1.5542159,
    -0.6798636,
     1.6003393,
    -0.1467117,

     0.5043620,
    -0.0068779,
     2.0697285,
    -0.0463650,

     0.9605451,
    -0.4620149,
     1.2525952,
    -1.3409909
  };

  const float EXPECTED_RESULT[16] =
  {
     1.933875,
    -0.467099,
     0.251638,
    -0.805156,

     1.232207,
    -1.737383,
    -1.023401,
     2.496859,

    -2.086953,
    -0.044468,
     0.045688,
     2.570036,

    -2.559921,
    -1.551155,
    -0.244802,
     0.056808
  };

  Math::Matrix matA(TEST_MATRIX_A);
  Math::Matrix matB(TEST_MATRIX_B);

  Math::Matrix mat;
  Math::MultiplyMatrices(matA, matB, mat);
  if (! Math::MatricesEqual(mat, Math::Matrix(EXPECTED_RESULT), TEST_TOLERANCE ) )
  {
    fprintf(stderr, "Multiply mismath!\n");
    return 1;
  }

  return 0;
}

int main()
{
  int result = 0;

  result = TestCofactor();
  if (result != 0)
    return result;

  result = TestDet();
  if (result != 0)
    return result;

  result = TestInvert();
  if (result != 0)
    return result;

  result = TestMultiply();
  if (result != 0)
    return result;

  return result;
}
