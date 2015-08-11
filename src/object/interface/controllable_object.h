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

#pragma once

#include "graphics/engine/camera.h"

#include "object/object_interface_type.h"

/**
 * \class CControllableObject
 * \brief Interface for objects that can be selected and controlled by the player
 */
class CControllableObject
{
public:
    explicit CControllableObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::Controllable)] = true;
    }
    virtual ~CControllableObject()
    {}

    virtual void SetTrainer(bool trainer) = 0;
    virtual bool GetTrainer() = 0;

    virtual void SetSelect(bool select, bool bDisplayError = true) = 0;
    virtual bool GetSelect() = 0;
    virtual bool GetSelectable() = 0;

    virtual void SetCameraType(Gfx::CameraType type) = 0;
    virtual Gfx::CameraType GetCameraType() = 0;
    virtual void SetCameraDist(float dist) = 0;
    virtual float GetCameraDist() = 0;
    virtual void SetCameraLock(bool lock) = 0;
    virtual bool GetCameraLock() = 0;
};
