/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
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
    ProgramStorage, //!< objects that store CBOT programs
    Programmable, //!< objects that can be programmed in CBOT
    TaskExecutor, //!< objects that can execute tasks (CTask classes)
    Jostleable, //!< object that can be jostled
    Carrier, //!< object that can carry other objects
    Powered, //!< object powered with power cell
    Movable, //!< objects that can move
    Flying, //!< objects that can fly
    JetFlying, //!< objects that can fly using a jet engine
    Controllable, //!< objects that can be selected and controlled by the player
    PowerContainer, //!< objects that hold power
    Ranged, //!< objects that have a operation range to be displayed after pressing button in the UI
    TraceDrawing, //!< objects that can draw wheel trace
    Damageable, //!< objects that generate particles when hit
    Destroyable, //!< objects that can be destroyed (base for Shielded and Fragile)
    Fragile, //!< objects that are destroyed immediately after hit
    Shielded, //!< objects that can be destroyed after the shield goes down to 0
    ShieldedAutoRegen, //!< shielded objects with auto shield regeneration
    Old, //!< old objects, TODO: remove once no longer necessary
    Max //!< maximum value (for getting number of items in enum)
};

using ObjectInterfaceTypes = std::array<bool, static_cast<std::size_t>(ObjectInterfaceType::Max)>;
