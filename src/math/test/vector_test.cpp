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

// math/test/vector_test.cpp

/* Unit tests for Vector struct

   Test data was randomly generated and the expected results
   calculated using GNU Octave.

 */

#include "../func.h"
#include "../vector.h"

#include <cstdio>

using namespace std;

const float TEST_TOLERANCE = 1e-6;

int TestLength()
{
  Math::Vector vec(-1.288447945923275, 0.681452565308134, -0.633761098985957);
  const float expectedLength = 1.58938001708428;

  if (! Math::IsEqual(vec.Length(), expectedLength, TEST_TOLERANCE) )
  {
    fprintf(stderr, "Length mismatch!\n");
    return __LINE__;
  }

  return 0;
}

int TestNormalize()
{
  Math::Vector vec(1.848877241804398, -0.157262961268577, -1.963031403332377);
  const Math::Vector expectedNormalized(0.6844609421393856, -0.0582193085618106, -0.7267212194481797);

  vec.Normalize();

  if (! Math::VectorsEqual(vec, expectedNormalized, TEST_TOLERANCE))
  {
    fprintf(stderr, "Normalize mismatch!\n");
    return __LINE__;
  }

  return 0;
}

int TestDot()
{
  Math::Vector vecA(0.8202190530968309, 0.0130926060162780, 0.2411914183883510);
  Math::Vector vecB(-0.0524083951404069, 1.5564932716738220, -0.8971342631500536);

  float expectedDot = -0.238988896477326;

  if (! Math::IsEqual(Math::DotProduct(vecA, vecB), expectedDot, TEST_TOLERANCE) )
  {
    fprintf(stderr, "Dot product mismatch!\n");
    return __LINE__;
  }

  return 0;
}

int TestCross()
{
  Math::Vector vecA(1.37380499798567, 1.18054518384682, 1.95166361293121);
  Math::Vector vecB(0.891657855926886, 0.447591335394532, -0.901604070087823);

  Math::Vector expectedCross(-1.937932065431669, 2.978844370287636, -0.437739173833581);
  Math::Vector expectedReverseCross = -expectedCross;

  if (! Math::VectorsEqual(vecA.CrossMultiply(vecB), expectedCross, TEST_TOLERANCE) )
  {
    fprintf(stderr, "Cross product mismatch!\n");
    return __LINE__;
  }

  if (! Math::VectorsEqual(vecB.CrossMultiply(vecA), expectedReverseCross, TEST_TOLERANCE) )
  {
    fprintf(stderr, "Reverse cross product mismatch!\n");
    return __LINE__;
  }

  return 0;
}

int main()
{
  // Functions to test
  int (*TESTS[])() =
  {
    TestLength,
    TestNormalize,
    TestDot,
    TestCross
  };
  const int TESTS_SIZE = sizeof(TESTS) / sizeof(*TESTS);

  int result = 0;
  for (int i = 0; i < TESTS_SIZE; ++i)
  {
    result = TESTS[i]();
    if (result != 0)
      return result;
  }

  fprintf(stderr, "All tests successful\n");

  return 0;
}
