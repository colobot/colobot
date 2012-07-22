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

// main.cpp

#include "app/app.h"
#include "app/system.h"
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

Currently, the only documented classes are the ones written from scratch or the old ones rewritten to match the new code.
In time, the documentation will be extended to cover every major part of the code.

\section Structure Code structure

The source code was split from the original all-in-one directory to subdirectories, each containing one major part of the project.
The current layout is this:
 - src/CBot - separate library with CBot language
 - src/app - class CApplication and everything concerned with SDL plus other system-dependent code such as displaying a message box, finding files, etc.
 - src/common - shared structs, enums, defines, etc.; should not have any external dependencies
 - src/graphics/common - interface of graphics engine (CEngine) and device (CDevice), without concrete implementation, shared structs such as Vertex, Material, etc., “effects” classes: CCamera, CLight, CParticle that will use the graphics engine interface
 - src/graphics/opengl - concrete implementation of CEngine and CDevice classes in OpenGL: CGLEngine and CGLDevice
 - src/graphics/d3d - in (far) future - perhaps a newer implementation in DirectX (9? 10?)
 - src/math - mathematical structures and functions
 - src/object - non-graphical game engine, that is robots, buildings, etc.; dependent only on interface of graphics engine, not on concrete implementation
 - src/ui - 2D user interface (menu, buttons, check boxes, etc.); also without dependencies to concrete implementation of graphics engine
 - src/sound - sound and music engine written using fmod library
 - src/physics - physics engine
 - src/script - link with the CBot library
 - src/metafile - separate program for packing data files to .dat format
*/


//! Entry point to the program
int main(int argc, char *argv[])
{
    CLogger logger; // Create the logger

    logger.Info("Colobot starting\n");

    CApplication app; // single instance of the application

    if (! app.ParseArguments(argc, argv))
    {
        SystemDialog(SDT_ERROR, "COLOBOT", "Invalid commandline arguments!\n");
        return app.GetExitCode();
    }

    int code = 0;

    if (! app.Create())
    {
        app.Destroy(); // ensure a clean exit
        code = app.GetExitCode();
        logger.Info("Didn't run main loop. Exiting with code %d\n", code);
        return code;
    }

    code = app.Run();

    logger.Info("Exiting with code %d\n", code);
    return code;
}

