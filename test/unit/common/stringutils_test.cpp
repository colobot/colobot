/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#include "common/stringutils.h"

#include <gtest/gtest.h>

namespace StringUtilsTesta
{

TEST(StringUtilTests, ReplaceShortToLong)
{
    std::string text = "Test {123}, {123}, {123}{123} Test";

    auto result = StrUtils::Replace(text, "{123}", "[0987654]");

    std::string expected = "Test [0987654], [0987654], [0987654][0987654] Test";

    EXPECT_EQ(result, expected);
}

TEST(StringUtilTests, ReplaceLongToShort)
{
    std::string text = "Test {1234567}, {1234567}, {1234567}{1234567} Test";

    auto result = StrUtils::Replace(text, "{1234567}", "[64]");

    std::string expected = "Test [64], [64], [64][64] Test";

    EXPECT_EQ(result, expected);
}

TEST(StringUtilTests, ReplaceSameLength)
{
    std::string text = "Test {123}, {123}, {123}{123} Test";

    auto result = StrUtils::Replace(text, "{123}", "[432]");

    std::string expected = "Test [432], [432], [432][432] Test";

    EXPECT_EQ(result, expected);
}

TEST(StringUtilTests, ToLowerTest)
{
    std::string text = u8",./;AaBbĄąĘę";

    auto result = StrUtils::ToLower(text);

    std::string expected = u8",./;aabbąąęę";

    EXPECT_EQ(result, expected);
}

TEST(StringUtilTests, ToUpperTest)
{
    std::string text = u8",./;AaBbĄąĘę";

    auto result = StrUtils::ToUpper(text);

    std::string expected = u8",./;AABBĄĄĘĘ";

    EXPECT_EQ(result, expected);
}

} // namespace StringUtils
