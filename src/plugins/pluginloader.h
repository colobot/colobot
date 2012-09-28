// * This file is part of the COLOBOT source code
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

// pluginloader.h

/**
 *  @file plugin/pluginloader.h
 *  @brief Plugin loader interface
 */

#pragma once

#include <ltdl.h>
#include <string>

#include "common/logger.h"

#include "plugininterface.h"


/**
* @class CPluginLoader
*
* @brief Plugin loader interface. Plugin manager uses this class to load plugins.
*
*/
class CPluginLoader {
    public:
        /** Class contructor
         *  @param std::string plugin filename
         */
        CPluginLoader(std::string);

        /** Function to get plugin name or description
         *  @return returns plugin name
         */
        std::string GetName();

        /** Function to get plugin version
         *  @return returns plugin version
         */
        int GetVersion();

        /** Function to unload plugin
         *  @return returns true on success
         */
        bool UnloadPlugin();

        /** Function to load plugin
         *  @return returns true on success
         */
        bool LoadPlugin();

        /** Function to check if plugin is loaded
         *  @return returns true if plugin is loaded
         */
        bool IsLoaded();

        /** Function to set plugin filename
         *  @return returns true on success. Action can fail if plugin was loaded and cannot be unloaded
         */
        bool SetFilename(std::string);

        /** Function to get plugin filename
         *  @return returns plugin filename
         */
        std::string GetFilename();


    private:
        CPluginInterface* mInterface;
        std::string mFilename;
        lt_dlhandle mHandle;
        bool mLoaded;
};
