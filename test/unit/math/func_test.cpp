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

/*
  Unit tests for math functions.
 */

#include "math/func.h"

#include "gtest/gtest.h"


TEST(IsPowerOfTwo, TestDifferentValues)
{
    EXPECT_FALSE(Math::IsPowerOfTwo(0));
    EXPECT_TRUE(Math::IsPowerOfTwo(1));
    EXPECT_TRUE(Math::IsPowerOfTwo(2));
    EXPECT_FALSE(Math::IsPowerOfTwo(3));
    EXPECT_TRUE(Math::IsPowerOfTwo(4));

    EXPECT_FALSE(Math::IsPowerOfTwo(31));
    EXPECT_TRUE(Math::IsPowerOfTwo(32));
    EXPECT_FALSE(Math::IsPowerOfTwo(33));

    EXPECT_FALSE(Math::IsPowerOfTwo(1234));
}

TEST(NextPowerOfTwo, TestDifferentValues)
{
    EXPECT_EQ(2, Math::NextPowerOfTwo(2));
    EXPECT_EQ(4, Math::NextPowerOfTwo(3));
    EXPECT_EQ(4, Math::NextPowerOfTwo(4));
    EXPECT_EQ(8, Math::NextPowerOfTwo(5));

    EXPECT_EQ(8, Math::NextPowerOfTwo(7));
    EXPECT_EQ(8, Math::NextPowerOfTwo(8));
    EXPECT_EQ(16, Math::NextPowerOfTwo(9));

    EXPECT_EQ(32, Math::NextPowerOfTwo(31));
    EXPECT_EQ(32, Math::NextPowerOfTwo(32));
    EXPECT_EQ(64, Math::NextPowerOfTwo(33));
}
