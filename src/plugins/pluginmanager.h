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

/**
 * \file plugins/pluginmanager.h
 * \brief Plugin manager class.
 */

#pragma once


#include "common/logger.h"
#include "common/profile.h"

#include "common/singleton.h"

#include "plugins/pluginloader.h"

#include <string>
#include <set>
#include <vector>


/**
 * \class CPluginManager
 *
 * \brief Plugin manager class. Plugin manager can load plugins from colobot.ini or manually specified files.
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
         * \param filename plugin filename
         * \return returns true on success
         */
        bool LoadPlugin(std::string filename);

        /** Function unloads specified plugin
         * \param filename plugin filename
         * \return returns true on success
         */
        bool UnloadPlugin(std::string filename);

        /** Function adds path to be checked when searching for plugin file. If path was already added it will be ignored
         * \param dir plugin search path
         * \return returns true on success
         */
        bool AddSearchDirectory(std::string dir);

        /** Function removes path from list
         * \param dir plugin search path
         * \return returns true on success
         */
        bool RemoveSearchDirectory(std::string dir);

        /** Function tries to unload all plugins
         * \return returns true on success
         */
        bool UnloadAllPlugins();

    private:
        bool NameEndsWith(std::string, std::string);

        std::set< std::string > m_folders;
        std::vector<CPluginLoader *> m_plugins;
};

