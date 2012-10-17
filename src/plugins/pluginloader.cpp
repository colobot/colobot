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


#include "plugins/pluginloader.h"


CPluginLoader::CPluginLoader(std::string filename)
{
    mInterface = nullptr;
    mFilename = filename;
    mLoaded = false;
}


std::string CPluginLoader::GetName()
{
    if (mLoaded)
        return mInterface->PluginName();
    return "(not loaded)";
}


int CPluginLoader::GetVersion()
{
    if (mLoaded)
        return mInterface->PluginVersion();
    return 0;
}


bool CPluginLoader::IsLoaded()
{
    return mLoaded;
}


bool CPluginLoader::UnloadPlugin()
{
    if (!mLoaded) {
        GetLogger()->Warn("Plugin %s is not loaded.\n");
        return true;
    }

    bool (*uninstall)(std::string &) = reinterpret_cast<bool (*)(std::string &)>( lt_dlsym(mHandle, "UninstallPluginEntry") );
    if (!uninstall) {
        GetLogger()->Error("Error getting UninstallPluginEntry for plugin %s: %s\n", mFilename.c_str(), lt_dlerror());
        return false;
    }

    std::string reason;
    if (!uninstall(reason)) {
        GetLogger()->Error("Could not unload plugin %s: %s\n", mFilename.c_str(), reason.c_str());
        return false;
    }

    lt_dlclose(mHandle);
    mLoaded = false;
    return true;
}


bool CPluginLoader::LoadPlugin()
{
    if (mFilename.length() == 0) {
        GetLogger()->Warn("No plugin filename specified.\n");
        return false;
    }

    mHandle = lt_dlopenext(mFilename.c_str());
    if (!mHandle) {
        GetLogger()->Error("Error loading plugin %s: %s\n", mFilename.c_str(), lt_dlerror());
        return false;
    }

    void (*install)() = reinterpret_cast<void (*)()>( lt_dlsym(mHandle, "InstallPluginEntry") );
    if (!install) {
        GetLogger()->Error("Error getting InstallPluginEntry for plugin %s: %s\n", mFilename.c_str(), lt_dlerror());
        return false;
    }

    CPluginInterface* (*getInterface)()  = reinterpret_cast<CPluginInterface* (*)()>( lt_dlsym(mHandle, "GetPluginInterfaceEntry") );

    if (!getInterface) {
        GetLogger()->Error("Error getting GetPluginInterfaceEntry for plugin %s: %s\n", mFilename.c_str(), lt_dlerror());
        return false;
    }

    install();
    mInterface = getInterface();
    mLoaded = true;
    return true;
}


bool CPluginLoader::SetFilename(std::string filename)
{
    bool ok = true;
    if (mLoaded)
        ok = UnloadPlugin();

    if (ok)
        mFilename = filename;
    return ok;
}


std::string CPluginLoader::GetFilename()
{
    return mFilename;
}
