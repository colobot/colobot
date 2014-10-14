/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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
#include "app/system.h"
#include "app/system_linux.h"

#include <gtest/gtest.h>

class CSystemUtilsLinuxWrapper : public CSystemUtilsLinux
{
public:
    CSystemUtilsLinuxWrapper() {}
};

class SystemUtilsLinuxUT : public testing::Test
{
protected:
    static const long long SEC = 1000000000;

    CSystemUtilsLinuxWrapper systemUtils;
};


TEST_F(SystemUtilsLinuxUT, TimeStampDiff)
{
    SystemTimeStamp before, after;

    before.clockTime.tv_sec = 1;
    before.clockTime.tv_nsec = 100;

    after.clockTime.tv_sec = 1;
    after.clockTime.tv_nsec = 900;

    long long tDiff = systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( 800, tDiff);

    tDiff = systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-800, tDiff);

    // -------

    before.clockTime.tv_sec = 2;
    before.clockTime.tv_nsec = 200;

    after.clockTime.tv_sec = 3;
    after.clockTime.tv_nsec = 500;

    tDiff = systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( SEC + 300, tDiff);

    tDiff = systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-SEC - 300, tDiff);

    // -------

    before.clockTime.tv_sec = 3;
    before.clockTime.tv_nsec = 200;

    after.clockTime.tv_sec = 4;
    after.clockTime.tv_nsec = 100;

    tDiff = systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( SEC - 100, tDiff);

    tDiff = systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-SEC + 100, tDiff);
}
