/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file object/object_interface_type.h
 * \brief ObjectInterfaceType enum
 */

#pragma once

#include <cstddef>
#include <array>

/**
 * \enum ObjectInterfaceType
 * \brief Type of interface that an object implements
 */
enum class ObjectInterfaceType
{
    Interactive, //!< interactive objects can process events from event loop
    Transportable, //!< objects that can be carried by robots or astronaut
    Programmable, //!< objects that can be programmed in CBOT
    Jostleable, //!< object that can be jostled
    Carrier, //!< object that can carry other objects
    Powered, //!< object powered with power cell
    Old, //!< old objects, TODO: remove once no longer necessary
    Max //!< maximum value (for getting number of items in enum)
};

using ObjectInterfaceTypes = std::array<bool, static_cast<std::size_t>(ObjectInterfaceType::Max)>;
