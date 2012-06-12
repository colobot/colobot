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

// math/test/geometry_test.cpp

/* Unit tests for functions in geometry.h */

#include "../func.h"
#include "../geometry.h"

#include <cstdio>

using namespace std;

const float TEST_TOLERANCE = 1e-5;

// Test for rewritten function RotateAngle()
int TestRotateAngle()
{
  if (! Math::IsEqual(Math::RotateAngle(0.0f, 0.0f), 0.0f, TEST_TOLERANCE))
    return __LINE__;

  if (! Math::IsEqual(Math::RotateAngle(1.0f, 0.0f), 0.0f, TEST_TOLERANCE))
    return __LINE__;

  if (! Math::IsEqual(Math::RotateAngle(1.0f, 1.0f), 0.25f * Math::PI, TEST_TOLERANCE))
    return __LINE__;

  if (! Math::IsEqual(Math::RotateAngle(0.0f, 2.0f), 0.5f * Math::PI, TEST_TOLERANCE))
    return __LINE__;

  if (! Math::IsEqual(Math::RotateAngle(-0.5f, 0.5f), 0.75f * Math::PI, TEST_TOLERANCE))
    return __LINE__;

  if (! Math::IsEqual(Math::RotateAngle(-1.0f, 0.0f), Math::PI, TEST_TOLERANCE))
    return __LINE__;

  if (! Math::IsEqual(Math::RotateAngle(-1.0f, -1.0f), 1.25f * Math::PI, TEST_TOLERANCE))
    return __LINE__;

  if (! Math::IsEqual(Math::RotateAngle(0.0f, -2.0f), 1.5f * Math::PI, TEST_TOLERANCE))
    return __LINE__;

  if (! Math::IsEqual(Math::RotateAngle(1.0f, -1.0f), 1.75f * Math::PI, TEST_TOLERANCE))
    return __LINE__;

  return 0;
}

int main()
{
  // Functions to test
  int (*TESTS[])() =
  {
    TestRotateAngle
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
