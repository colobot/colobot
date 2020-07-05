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

#pragma once

#include "object/object_interface_type.h"

/**
 * \class CPowerContainer
 * \brief Interface for objects hold power (PowerCells and some buildings)
 */
class CPowerContainerObject
{
public:
    explicit CPowerContainerObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::PowerContainer)] = true;
    }
    virtual ~CPowerContainerObject()
    {}

    //! Set energy level (percentage), value in range [0..1]
    virtual void SetEnergyLevel(float level) = 0;
    //! Get energy level (percentage), value in range [0..1]
    virtual float GetEnergyLevel() = 0;

    //! Set amount of energy in the container, value in range [0..GetCapacity()]
    virtual void SetEnergy(float energy)
    {
        SetEnergyLevel(energy / GetCapacity());
    }
    //! Get amount of energy in the container, value in range [0..GetCapacity()]
    virtual float GetEnergy()
    {
        return GetEnergyLevel() * GetCapacity();
    }

    //! Return capacity of this power container
    virtual float GetCapacity() = 0;

    //! Returns true if this power container can be recharged
    virtual bool IsRechargeable() = 0;
};
