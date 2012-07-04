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

// plugin.h


#pragma once


#define PLUGIN_INTERFACE(class_type, interface_type) \
	extern "C" interface_type* installPlugin() { return (interface_type *)new class_type(); } \
	extern "C" void uninstallPlugin(class_type *_class) { delete _class; }


class CPlugin {
	public:
		virtual char* pluginName() = 0;
		virtual int pluginVersion() = 0;
};

