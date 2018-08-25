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

/**
 * \file app/main.cpp
 * \brief Entry point of application - main() function
 */

#include "common/config.h"

#include "app/app.h"
#include "app/signal_handlers.h"

#include "common/logger.h"
#include "common/make_unique.h"
#include "common/profiler.h"
#include "common/restext.h"
#include "common/version.h"

#include "common/resources/resourcemanager.h"

#include "common/system/system.h"
#if PLATFORM_WINDOWS
    #include "common/system/system_windows.h"
#endif

#if PLATFORM_WINDOWS
    #include <windows.h>
#endif

#include <memory>
#include <vector>
#include <boost/filesystem.hpp>

/**
\mainpage

Doxygen documentation of Colobot: Gold Edition project.

<b>Colobot</b> <i>(COLOnize with BOTs)</i> is a game combining elements of real time strategy (RTS)
and educational game, aiming to teach programming through entertainment. You are playing as an astronaut
on a journey with robot helpers to find a planet for colonization. It features a C++ and Java-like,
object-oriented language, CBOT, which can be used to program the robots available in the game.

The original version of the game was developed by [Epsitec](http://www.epsitec.ch/) and released in 2001.
Later, in 2005 another version named Ceebot was released. In March 2012, through attempts
by Polish Colobot fans, Epsitec agreeed to release the source code of the game on GPLv3 license.
The license was given specfifically to our community, <b>TerranovaTeam</b>,
part of <b>International Colobot Community (ICC)</b> (previously known as <i>Polish Portal of Colobot (PPC)</i>;
Polish: <i>Polski Portal Colobota</i>) with our website at http://colobot.info/.

\section Intro Introduction

The source code released by Epitec was sparsely documented. This documentation, written from scratch,
will aim to describe the various components of the code.

Currently, the only documented classes are the ones written from scratch or the old ones rewritten
to match the new code.
In time, the documentation will be extended to cover every major part of the code.

\section Structure Code structure

The source code was split from the original all-in-one directory to subdirectories,
each containing one major part of the project.
The current layout is the following:
 - src/CBot - separate library with CBot language
 - src/app - class CApplication and everything concerned with SDL
 - src/common - shared structs, enums, defines, etc.; should not have any external dependencies
 - src/common/resources - filesystem management using PHYSFS library
 - src/common/system - system-dependent code such as displaying a message box, finding files, etc.
 - src/common/thread - wrapper classes for SDL threads
 - src/graphics/core - abstract interface of graphics device (abstract CDevice class)
 - src/graphics/engine - main graphics engine based on abstract graphics device; is composed
   of CEngine class and associated classes implementing the 3D engine
 - src/graphics/model - code related to loading/saving model files
 - src/graphics/opengl - concrete implementation of CDevice class in OpenGL: CGLDevice
 - src/graphics/d3d - in (far) future - perhaps a newer implementation in DirectX (9? 10?)
 - src/math - mathematical structures and functions
 - src/object - non-grphical game object logic, that is robots, buildings, etc.
 - src/level - main part of non-graphical game engine, that is loading levels etc.
 - src/level/parser - parser for loading/saving level files from format known as <i>scene files</i>
 - src/ui - 2D user interface (menu, buttons, check boxes, etc.)
 - src/sound - sound and music engine written using fmod library
 - src/physics - physics engine
 - src/script - link with the CBot library
*/

//! Entry point to the program
extern "C"
{

int main(int argc, char *argv[])
{
    CLogger logger; // single instance of logger
    logger.AddOutput(stderr);

    auto systemUtils = CSystemUtils::Create(); // platform-specific utils
    systemUtils->Init();

    CProfiler::SetSystemUtils(systemUtils.get());

    // Add file output to the logger
    std::string logFileName;
    #if DEV_BUILD
        logFileName = "log.txt";
    #else
        boost::filesystem::create_directories(systemUtils->GetSaveDir());
        logFileName = systemUtils->GetSaveDir() + "/log.txt";
    #endif
    FILE* logFile = fopen(logFileName.c_str(), "w");
    if (logFile)
        logger.AddOutput(logFile);
    else
        logger.Error("Failed to create log file, writing log to file disabled\n");


    // Workaround for character encoding in argv on Windows
    #if PLATFORM_WINDOWS
    int wargc = 0;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if (wargv == nullptr)
    {
        logger.Error("CommandLineToArgvW failed\n");
        return 1;
    }

    std::vector<std::vector<char>> windowsArgs;
    for (int i = 0; i < wargc; i++)
    {
        std::wstring warg = wargv[i];
        std::string arg = CSystemUtilsWindows::UTF8_Encode(warg);
        std::vector<char> argVec(arg.begin(), arg.end());
        argVec.push_back('\0');
        windowsArgs.push_back(std::move(argVec));
    }

    auto windowsArgvPtrs = MakeUniqueArray<char*>(wargc);
    for (int i = 0; i < wargc; i++)
        windowsArgvPtrs[i] = windowsArgs[i].data();

    argv = windowsArgvPtrs.get();

    LocalFree(wargv);
    #endif

    logger.Info("%s starting\n", COLOBOT_FULLNAME);

    CSignalHandlers::Init(systemUtils.get());

    CResourceManager manager(argv[0]);

    // Initialize static string arrays
    InitializeRestext();
    InitializeEventTypeTexts();

    int code = 0;
    CApplication app(systemUtils.get(), argv[0]); // single instance of the application

    ParseArgsStatus status = app.ParseArguments(argc, argv);
    if (status == PARSE_ARGS_FAIL)
    {
        systemUtils->SystemDialog(SDT_ERROR, "COLOBOT - Fatal Error", "Invalid commandline arguments!\n");
        return app.GetExitCode();
    }
    else if (status == PARSE_ARGS_HELP)
    {
        return app.GetExitCode();
    }

    if (! app.Create())
    {
        code = app.GetExitCode();
        if (code != 0 && !app.GetErrorMessage().empty())
        {
            systemUtils->SystemDialog(SDT_ERROR, "COLOBOT - Fatal Error", app.GetErrorMessage());
        }
        logger.Info("Didn't run main loop. Exiting with code %d\n", code);
        return code;
    }

    code = app.Run();

    logger.Info("Exiting with code %d\n", code);

    return code;
}

} // extern "C"
