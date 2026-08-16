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

#include "common/logger.h"

#include <gtest/gtest.h>
#include <cpptrace/from_current.hpp>

#include <clocale>

// For compatibility with gtest < 1.12.0
#ifndef GTEST_FLAG_GET
#define GTEST_FLAG_GET(name) ::testing::GTEST_FLAG(name)
#endif
#ifndef GTEST_FLAG_SET
#define GTEST_FLAG_SET(name, value) (void)(::testing::GTEST_FLAG(name) = value)
#endif

extern bool g_cbotTestSaveState;

int main(int argc, char* argv[])
{
    CLogger logger;

    ::testing::InitGoogleTest(&argc, argv);

    setlocale(LC_ALL, "en_US.UTF-8");

    // parse arguments not removed by InitGoogleTest
    for (int i = 1; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if (arg == "--CBotUT_TestSaveState")
            g_cbotTestSaveState = true;
    }

    if (!GTEST_FLAG_GET(catch_exceptions))
    {
        // Pass the exception through to the debugger
        return RUN_ALL_TESTS();
    }

    GTEST_FLAG_SET(catch_exceptions, false);
    CPPTRACE_TRY
    {
        return RUN_ALL_TESTS();
    }
    CPPTRACE_CATCH(const std::exception& e)
    {
        cpptrace::from_current_exception().print(std::cout);
        std::cout << "Exception: " << e.what() << std::endl;
        throw;
    }
    CPPTRACE_CATCH_ALT(...)
    {
        cpptrace::from_current_exception().print(std::cout);
        std::cout << "Not std::exception" << std::endl;
        throw;
    }
}
