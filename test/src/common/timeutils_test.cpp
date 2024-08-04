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

#include "common/timeutils.h"

#include <gtest/gtest.h>

namespace TimeUtils
{

TEST(TimeUtilsExactDiffTest, ExactDiff)
{
    auto epoch = TimeStamp{};
    EXPECT_EQ(ExactDiff(epoch, epoch), 0);

    auto duration = std::chrono::microseconds{123456789L};
    auto before = std::chrono::steady_clock::now();
    auto after = before + duration;
    EXPECT_EQ(ExactDiff(before, after), std::chrono::nanoseconds{duration}.count());
    EXPECT_EQ(ExactDiff(after, before), -std::chrono::nanoseconds{duration}.count());
}

constexpr auto TIMESTAMP_START = TimeStamp{std::chrono::nanoseconds{300}};
constexpr auto TIMESTAMP_MID = TimeStamp{std::chrono::nanoseconds{600}};
constexpr auto TIMESTAMP_END = TimeStamp{std::chrono::nanoseconds{900}};

constexpr auto LERP_PARAM_ZERO = 0.0f;
constexpr auto LERP_PARAM_HALF = 0.5f;
constexpr auto LERP_PARAM_ONE = 1.0f;

TEST(TimeUtilsLerpTest, LerpReturnsStartWhenLerpParameterIsZero)
{
    EXPECT_EQ(TIMESTAMP_START, Lerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_ZERO));
}

TEST(TimeUtilsLerpTest, LerpReturnsEndWhenLerpParameterIsOne)
{
    EXPECT_EQ(TIMESTAMP_END, Lerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_ONE));
}

TEST(TimeUtilsLerpTest, LerpReturnsValueBetweenStartAndEndWhenLerpParameterIsBetweenZeroAndOne)
{
    EXPECT_EQ(TIMESTAMP_MID, Lerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_HALF));
}

TEST(TimeUtilsLerpTest, LerpIsMonotonic)
{
    constexpr auto deltaLerpParam = 0.1f;
    auto earlierTimeStamp = Lerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_HALF - deltaLerpParam);
    auto laterTimeStamp = Lerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_HALF + deltaLerpParam);
    EXPECT_TRUE(earlierTimeStamp < laterTimeStamp);
}

TEST(TimeUtilsLerpTest, LerpIsConsistent)
{
    auto timeStamp = TIMESTAMP_START;
    EXPECT_EQ(timeStamp, Lerp(timeStamp, timeStamp, LERP_PARAM_ZERO));
    EXPECT_EQ(timeStamp, Lerp(timeStamp, timeStamp, LERP_PARAM_HALF));
    EXPECT_EQ(timeStamp, Lerp(timeStamp, timeStamp, LERP_PARAM_ONE));
}

} // namespace TimeUtils
