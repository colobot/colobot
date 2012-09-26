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

// pluginmanager.h

/**
 *  @file plugin/pluginmanager.h
 *  @brief Plugin manager class.
 */

#pragma once

#include <string>
#include <set>
#include <vector>

#include "common/logger.h"
#include "common/profile.h"

#include "common/singleton.h"

#include "pluginloader.h"


/**
* @class CPluginManager
*
* @brief Plugin manager class. Plugin manager can load plugins from colobot.ini or manually specified files.
*
*/
class CPluginManager : public CSingleton<CPluginManager> {
    public:
        CPluginManager();
        ~CPluginManager();

        /** Function loads plugin list and path list from profile file
         */
        void LoadFromProfile();

        /** Function loads specified plugin
         * @param std::string plugin filename
         * @return returns true on success
         */
        bool LoadPlugin(std::string);

        /** Function unloads specified plugin
         * @param std::string plugin filename
         * @return returns true on success
         */
        bool UnloadPlugin(std::string);

        /** Function adds path to be checked when searching for plugin file. If path was already added it will be ignored
         * @param std::string plugin search path
         * @return returns true on success
         */
        bool AddSearchDirectory(std::string);

        /** Function removes path from list
         * @param std::string plugin search path
         * @return returns true on success
         */
        bool RemoveSearchDirectory(std::string);

        /** Function tries to unload all plugins
         * @return returns true on success
         */
        bool UnloadAllPlugins();

    private:
        bool NameEndsWith(std::string, std::string);

        std::set< std::string > m_folders;
        std::vector<CPluginLoader *> m_plugins;
};

