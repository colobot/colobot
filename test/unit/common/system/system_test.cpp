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

struct SystemTest : ::testing::Test {
    CSystemUtilsOther system;
};

TEST_F(SystemTest, TimeStampExactDiff) {
    auto epoch = SystemTimeStamp{};
    EXPECT_EQ(system.TimeStampExactDiff(epoch, epoch), 0);

    auto duration = std::chrono::microseconds{123456789L};
    auto before = std::chrono::high_resolution_clock::now();
    auto after = before + duration;
    EXPECT_EQ(system.TimeStampExactDiff(before, after), std::chrono::nanoseconds{duration}.count());
    EXPECT_EQ(system.TimeStampExactDiff(after, before), -std::chrono::nanoseconds{duration}.count());
}
