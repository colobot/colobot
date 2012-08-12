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


#pragma once

#include <string>
#include <set>
#include <vector>

#include <common/logger.h>
#include <common/profile.h>

#include <common/singleton.h>

#include "pluginloader.h"


class CPluginManager : public CSingleton<CPluginManager> {
    public:
        CPluginManager();
        ~CPluginManager();

        void LoadFromProfile();

        bool LoadPlugin(std::string);
        bool UnloadPlugin(std::string);

        bool AddSearchDirectory(std::string);
        bool RemoveSearchDirectory(std::string);

        bool UnloadAllPlugins();

    private:
        bool NameEndsWith(std::string, std::string);

        std::set< std::string > m_folders;
        std::vector<CPluginLoader *> m_plugins;
};

