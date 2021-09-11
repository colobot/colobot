/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "common/system/system_other.h"

#include <gtest/gtest.h>

struct SystemTest : ::testing::Test
{
    CSystemUtilsOther system;
};

TEST_F(SystemTest, TimeStampExactDiff)
{
    auto epoch = SystemTimeStamp{};
    EXPECT_EQ(system.TimeStampExactDiff(epoch, epoch), 0);

    auto duration = std::chrono::microseconds{123456789L};
    auto before = std::chrono::high_resolution_clock::now();
    auto after = before + duration;
    EXPECT_EQ(system.TimeStampExactDiff(before, after), std::chrono::nanoseconds{duration}.count());
    EXPECT_EQ(system.TimeStampExactDiff(after, before), -std::chrono::nanoseconds{duration}.count());
}

constexpr auto TIMESTAMP_START = SystemTimeStamp{std::chrono::nanoseconds{300}};
constexpr auto TIMESTAMP_MID = SystemTimeStamp{std::chrono::nanoseconds{600}};
constexpr auto TIMESTAMP_END = SystemTimeStamp{std::chrono::nanoseconds{900}};

constexpr auto LERP_PARAM_ZERO = 0.0f;
constexpr auto LERP_PARAM_HALF = 0.5f;
constexpr auto LERP_PARAM_ONE = 1.0f;

TEST_F(SystemTest, TimeStampLerpReturnsStartWhenLerpParameterIsZero)
{
    EXPECT_EQ(TIMESTAMP_START, system.TimeStampLerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_ZERO));
}

TEST_F(SystemTest, TimeStampLerpReturnsEndWhenLerpParameterIsOne)
{
    EXPECT_EQ(TIMESTAMP_END, system.TimeStampLerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_ONE));
}

TEST_F(SystemTest, TimeStampLerpReturnsValueBetweenStartAndEndWhenLerpParameterIsBetweenZeroAndOne)
{
    EXPECT_EQ(TIMESTAMP_MID, system.TimeStampLerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_HALF));
}

TEST_F(SystemTest, TimeStampLerpIsMonotonic)
{
    constexpr auto deltaLerpParam = 0.1f;
    auto earlierTimeStamp = system.TimeStampLerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_HALF - deltaLerpParam);
    auto laterTimeStamp = system.TimeStampLerp(TIMESTAMP_START, TIMESTAMP_END, LERP_PARAM_HALF + deltaLerpParam);
    EXPECT_TRUE(earlierTimeStamp < laterTimeStamp);
}

TEST_F(SystemTest, TimeStampLerpIsConsistent)
{
    auto timeStamp = TIMESTAMP_START;
    EXPECT_EQ(timeStamp, system.TimeStampLerp(timeStamp, timeStamp, LERP_PARAM_ZERO));
    EXPECT_EQ(timeStamp, system.TimeStampLerp(timeStamp, timeStamp, LERP_PARAM_HALF));
    EXPECT_EQ(timeStamp, system.TimeStampLerp(timeStamp, timeStamp, LERP_PARAM_ONE));
}

