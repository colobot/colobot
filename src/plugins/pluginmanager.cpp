// * This file is part of the COLOBOT source code
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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


#include "plugins/pluginmanager.h"


template<> CPluginManager* CSingleton<CPluginManager>::mInstance = nullptr;


CPluginManager::CPluginManager()
{
    lt_dlinit();
}


CPluginManager::~CPluginManager()
{
    UnloadAllPlugins();
    lt_dlexit();
}



void CPluginManager::LoadFromProfile()
{
    GetLogger()->Info("Trying to load from profile...\n");
    std::vector< std::string > dirs = GetProfile().GetLocalProfileSection("Plugins", "Path");
    std::vector< std::string > plugins = GetProfile().GetLocalProfileSection("Plugins", "File");

    GetLogger()->Info("Path %d, files %d\n", dirs.size(), plugins.size());
    for (std::string dir : dirs)
        m_folders.insert(dir);

    for (std::string plugin : plugins) {
        GetLogger()->Info("Trying to load plugin %s...\n", plugin.c_str());
        LoadPlugin(plugin);
    }
}


bool CPluginManager::LoadPlugin(std::string filename)
{
    bool result = false;
    CPluginLoader *loader = new CPluginLoader("");
    for (std::string dir : m_folders) {
        loader->SetFilename(dir + "/" + filename);
        result = loader->LoadPlugin();
        if (result) {
            GetLogger()->Info("Plugin %s (%s) version %0.2f loaded!\n", filename.c_str(), loader->GetName().c_str(), loader->GetVersion() / 100.0f);
            m_plugins.push_back(loader);
            break;
        }
    }
    return result;
}


bool CPluginManager::UnloadPlugin(std::string filename)
{
    std::vector<CPluginLoader *>::iterator it;
    GetLogger()->Info("Trying to unload plugin %s...\n", filename.c_str());
    for (it = m_plugins.begin(); it != m_plugins.end(); it++) {
        CPluginLoader *plugin = *it;
        if (NameEndsWith(plugin->GetFilename(), filename)) {
            m_plugins.erase(it);
            plugin->UnloadPlugin();
            delete plugin;
            return true;
        }
    }
    return false;
}


bool CPluginManager::AddSearchDirectory(std::string dir)
{
    m_folders.insert(dir);
    return true;
}


bool CPluginManager::RemoveSearchDirectory(std::string dir)
{
    m_folders.erase(dir);
    return false;
}


bool CPluginManager::UnloadAllPlugins()
{
    bool allOk = true;
    std::vector<CPluginLoader *>::iterator it;
    for (it = m_plugins.begin(); it != m_plugins.end(); it++) {
        CPluginLoader *plugin = *it;
        bool result;

        GetLogger()->Info("Trying to unload plugin %s (%s)...\n", plugin->GetFilename().c_str(), plugin->GetName().c_str());
        result = plugin->UnloadPlugin();
        if (!result) {
            allOk = false;
            continue;
        }
        delete plugin;
        m_plugins.erase(it);
    }

    return allOk;
}


bool CPluginManager::NameEndsWith(std::string filename, std::string ending)
{
    if (filename.length() > ending.length()) {
        std::string fileEnd =  filename.substr(filename.length() - ending.length());
        return (fileEnd == ending);
    }
    return false;
}
