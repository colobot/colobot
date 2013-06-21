/*
  Unit tests for math functions.
 */

#include "math/func.h"

#include <gtest/gtest.h>


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
