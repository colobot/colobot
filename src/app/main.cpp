// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file app/main.cpp
 * \brief Entry point of application - main() function
 */

#include "app/app.h"
#include "app/system.h"

#include "common/config.h"
#include "common/logger.h"
#include "common/misc.h"
#include "common/restext.h"


/* Doxygen main page */

/**

\mainpage

Doxygen documentation of Colobot project

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
 - src/app - class CApplication and everything concerned with SDL plus other system-dependent
   code such as displaying a message box, finding files, etc.
 - src/common - shared structs, enums, defines, etc.; should not have any external dependencies
 - src/graphics/core - abstract interface of graphics device (abstract CDevice class)
   (split from old src/graphics/common)
 - src/graphics/engine - main graphics engine based on abstract graphics device; is composed
   of CEngine class and associated classes implementing the 3D engine (split from old src/graphics/common)
 - src/graphics/opengl - concrete implementation of CDevice class in OpenGL: CGLDevice
 - src/graphics/d3d - in (far) future - perhaps a newer implementation in DirectX (9? 10?)
 - src/math - mathematical structures and functions
 - src/object - non-graphical game engine, that is robots, buildings, etc.
 - src/ui - 2D user interface (menu, buttons, check boxes, etc.)
 - src/sound - sound and music engine written using fmod library
 - src/physics - physics engine
 - src/script - link with the CBot library
*/


//! Entry point to the program
extern "C"
{

int SDL_MAIN_FUNC(int argc, char *argv[])
{
    CLogger logger; // single istance of logger

    InitializeRestext(); // init static translation strings

    CSystemUtils* systemUtils = CSystemUtils::Create(); // platform-specific utils
    systemUtils->Init();

    logger.Info("Colobot starting\n");

    CApplication* app = new CApplication(); // single instance of the application

    ParseArgsStatus status = app->ParseArguments(argc, argv);
    if (status == PARSE_ARGS_FAIL)
    {
        systemUtils->SystemDialog(SDT_ERROR, "COLOBOT - Fatal Error", "Invalid commandline arguments!\n");
        return app->GetExitCode();
    }
    else if (status == PARSE_ARGS_HELP)
    {
        return app->GetExitCode();
    }

    int code = 0;

    if (! app->Create())
    {
        app->Destroy(); // ensure a clean exit
        code = app->GetExitCode();
        if ( code != 0 && !app->GetErrorMessage().empty() )
        {
            systemUtils->SystemDialog(SDT_ERROR, "COLOBOT - Fatal Error", app->GetErrorMessage());
        }
        logger.Info("Didn't run main loop. Exiting with code %d\n", code);
        return code;
    }

    code = app->Run();

    delete app;
    delete systemUtils;

    logger.Info("Exiting with code %d\n", code);
    return code;
}

} // extern "C"

