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
#include "app/system_windows.h"

#include <gtest/gtest.h>

class CSystemUtilsWindowsWrapper : public CSystemUtilsWindows
{
public:
    CSystemUtilsWindowsWrapper() {}

    void SetFrequency(long long counterFrequency)
    {
        m_counterFrequency = counterFrequency;
    }
};

class SystemUtilsWindowsUT : public testing::Test
{
protected:
    static const long long SEC = 1000000000;

    CSystemUtilsWindowsWrapper systemUtils;
};


TEST_F(SystemUtilsWindowsUT, TimerResolution)
{
    systemUtils.SetFrequency(SEC);
    EXPECT_EQ(1u, systemUtils.GetTimeStampExactResolution());

    systemUtils.SetFrequency(SEC/3);
    EXPECT_EQ(3u, systemUtils.GetTimeStampExactResolution());
}

TEST_F(SystemUtilsWindowsUT, TimeStampDiff)
{
    systemUtils.SetFrequency(SEC);

    SystemTimeStamp before, after;

    before.counterValue = 100;
    after.counterValue = 200;

    long long tDiff = systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( 100, tDiff);

    tDiff = systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-100, tDiff);

    // -------

    systemUtils.SetFrequency(SEC/3);

    before.counterValue = 200;
    after.counterValue = 400;

    tDiff = systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( 200*3, tDiff);

    tDiff = systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-200*3, tDiff);
}
