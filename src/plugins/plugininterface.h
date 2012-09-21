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

/**
 *  @file plugin/plugininterface.h
 *  @brief Generic plugin interface
 */

#pragma once

#include <string>

#define PLUGIN_INTERFACE(class_type) \
    static class_type* Plugin##class_type; \
    extern "C" void InstallPluginEntry() { Plugin##class_type = new class_type(); Plugin##class_type->InstallPlugin(); } \
    extern "C" bool UninstallPluginEntry(std::string &reason) { bool result = Plugin##class_type->UninstallPlugin(reason); \
                                                                if (!result) \
                                                                    return false; \
                                                                delete Plugin##class_type; \
                                                                return true; } \
    extern "C" CPluginInterface* GetPluginInterfaceEntry() { return static_cast<CPluginInterface*>(Plugin##class_type); }


/**
* @class CPluginInterface
*
* @brief Generic plugin interface. All plugins that will be managed by plugin manager have to derive from this class.
*
*/
class CPluginInterface {
    public:
        /** Function to get plugin name or description
         *  @return returns plugin name
         */
        inline virtual std::string PluginName() { return "abc"; };

        /** Function to get plugin version. 1 means version 0.01, 2 means 0.02 etc.
         *  @return number indicating plugin version
         */
        inline virtual int PluginVersion() { return 0; };

        /** Function to initialize plugin
         */
        inline virtual void InstallPlugin() {};

        /** Function called before removing plugin
         */
        inline virtual bool UninstallPlugin(std::string &) { return true; };
};

