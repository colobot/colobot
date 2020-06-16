/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "common/system/system.h"
#include "common/system/system_windows.h"

#include <gtest/gtest.h>

class CSystemUtilsWindowsWrapper : public CSystemUtilsWindows
{
public:
    void SetFrequency(long long frequency)
    {
        m_counterFrequency = frequency;
    }
};

class CSystemUtilsWindowsUT : public testing::Test
{
protected:
    static const long long SEC = 1000000000;

    CSystemUtilsWindowsWrapper m_systemUtils;
};

TEST_F(CSystemUtilsWindowsUT, TimeStampDiff)
{
    m_systemUtils.SetFrequency(SEC);

    SystemTimeStamp before, after;

    before.counterValue = 100;
    after.counterValue = 200;

    long long tDiff = m_systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( 100, tDiff);

    tDiff = m_systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-100, tDiff);

    // -------

    m_systemUtils.SetFrequency(SEC/3);

    before.counterValue = 200;
    after.counterValue = 400;

    tDiff = m_systemUtils.TimeStampExactDiff(&before, &after);
    EXPECT_EQ( 200*3, tDiff);

    tDiff = m_systemUtils.TimeStampExactDiff(&after, &before);
    EXPECT_EQ(-200*3, tDiff);
}
