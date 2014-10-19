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
#include "common/profile.h"
#include "common/logger.h"
#include "app/system.h"

#include <iostream>
#include <string>
#include <vector>
#include <gtest/gtest.h>


class CProfileTest : public testing::Test
{
protected:
    CProfile m_profile;
};

TEST_F(CProfileTest, ReadTest)
{
    m_profile.SetUseCurrentDirectory(true);

    ASSERT_TRUE(m_profile.Init()); // load colobot.ini file

    std::string result;
    ASSERT_TRUE(m_profile.GetStringProperty("test_string", "string_value", result));
    ASSERT_STREQ("Hello world", result.c_str());

    int int_value;
    ASSERT_TRUE(m_profile.GetIntProperty("test_int", "int_value", int_value));
    ASSERT_EQ(42, int_value);

    float float_value;
    ASSERT_TRUE(m_profile.GetFloatProperty("test_float", "float_value", float_value));
    ASSERT_FLOAT_EQ(1.5, float_value);

    std::vector<std::string> list;
    list = m_profile.GetSection("test_multi", "entry");
    ASSERT_EQ(5u, list.size());
}
