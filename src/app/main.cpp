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


//! Entry point to the program
int main(int argc, char *argv[])
{
    CLogger logger; // Create the logger

    logger.Info("Colobot starting\n");

    CApplication app; // single instance of the application

    Error err = app.ParseArguments(argc, argv);
    if (err != ERR_OK)
    {
        SystemDialog(SDT_ERROR, "COLOBOT", "Invalid commandline arguments!\n");
    }

    int code = 0;

    if (! app.Create())
    {
        code = app.GetExitCode();
        logger.Info("Didn't run main loop. Exiting with code %d\n", code);
        return code;
    }

    code = app.Run();

    logger.Info("Exiting with code %d\n", code);
    return code;
}

