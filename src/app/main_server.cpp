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

/**
 * \file app/main_server.cpp
 * \brief Entry point of dedicated server
 */

#include "app/app.h"
#include "app/system.h"

#include "common/config.h"
#include "common/logger.h"
#include "common/misc.h"
#include "common/restext.h"


/* Doxygen main page */

#include "common/resources/resourcemanager.h"

//! Entry point to the program
extern "C"
{
    
    int SDL_MAIN_FUNC(int argc, char *argv[])
    {
        CLogger logger; // single istance of logger
        CResourceManager manager(argv[0]);
        
        // Initialize static string arrays
        InitializeRestext();
        InitializeEventTypeTexts();
        
        logger.Info("%s dedicated server starting\n", COLOBOT_FULLNAME);
        
        int code = 0;
        while(true) {
            CSystemUtils* systemUtils = CSystemUtils::Create(); // platform-specific utils
            systemUtils->Init();
            
            CApplication* app = new CApplication(true); // single instance of the application
            
            ParseArgsStatus status = app->ParseArguments(argc, argv);
            if (status == PARSE_ARGS_FAIL)
            {
                logger.Error("Invalid commandline arguments!\n");
                return app->GetExitCode();
            }
            else if (status == PARSE_ARGS_HELP)
            {
                return app->GetExitCode();
            }
            
            
            if (! app->Create())
            {
                app->Destroy(); // ensure a clean exit
                code = app->GetExitCode();
                if ( code != 0 && !app->GetErrorMessage().empty() )
                {
                    logger.Error("%s\n", app->GetErrorMessage().c_str());
                }
                logger.Info("Didn't run main loop. Exiting with code %d\n", code);
                return code;
            }
            
            code = app->Run();
            bool restarting = app->IsRestarting();
            
            delete app;
            delete systemUtils;
            if(!restarting) break;
        }
        
        logger.Info("Exiting with code %d\n", code);
        return code;
    }
    
} // extern "C"

