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

// plugininterface.h


#pragma once


#define PLUGIN_INTERFACE(class_type) \
    static class_type* Plugin##class_type; \
    extern "C" void InstallPluginEntry() { Plugin##class_type = new class_type(); Plugin##class_type->InstallPlugin(); } \
    extern "C" void UninstallPluginEntry() { Plugin##class_type->UninstallPlugin(); delete Plugin##class_type; } \
    extern "C" CPluginInterface* GetPluginInterfaceEntry() { return static_cast<CPluginInterface*>(Plugin##class_type); }


class CPluginInterface {
    public:
        virtual char* PluginName() = 0;
        virtual int PluginVersion() = 0;
        virtual void InstallPlugin() = 0;
        virtual void UninstallPlugin() = 0;
};

