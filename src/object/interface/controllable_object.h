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

    //! Disables remote control of an object (only programming allowed)
    virtual void SetTrainer(bool trainer) = 0;
    //! Checks if remote control is disabled
    virtual bool GetTrainer() = 0;

    // TODO: Refactor SetSelect/GetSelect to be stored in CRobotMain? There can be ever only one selected object...
    //! Selects the object
    virtual void SetSelect(bool select, bool bDisplayError = true) = 0;
    //! Checks if the object is selected
    virtual bool GetSelect() = 0;

    //! Returns true if the object is selectable
    virtual bool GetSelectable() = 0;

    //! Highlights the object on mouse over
    virtual void SetHighlight(bool highlight) = 0;

    //! Set camera type for this object
    virtual void SetCameraType(Gfx::CameraType type) = 0;
    //! Return camera type for this object
    virtual Gfx::CameraType GetCameraType() = 0;
    //! Disallow camera changes
    virtual void SetCameraLock(bool lock) = 0;
    //! Check if camera changes are disallowed
    virtual bool GetCameraLock() = 0;

    //! Adjust camera parameters for the given object
    /** TODO: Seems to be for onboard camera only? Check and rename appropriately */
    virtual void AdjustCamera(Math::Vector &eye, float &dirH, float &dirV,
                              Math::Vector &lookat, Math::Vector &upVec,
                              Gfx::CameraType type) = 0;
};
