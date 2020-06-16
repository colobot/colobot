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

#include "math/point.h"

#include "object/object_type.h"

class CRobotMain;
class CObjectManager;
class CSoundInterface;
struct Event;

namespace Gfx
{
class CEngine;
}

namespace Ui
{
class CInterface;

/**
 * \class CDebugMenu
 * \brief Handles debug menu (F10)
 *
 * There should always be only one instance of this class for each associated CRobotMain class.
 */
class CDebugMenu
{
public:
    //! Creates the CDebugMenu instance
    CDebugMenu(CRobotMain* main, Gfx::CEngine* engine, CObjectManager* objMan, CSoundInterface* sound);
    //! Destroys the CDebugMenu instance
    //! \note Does not clean up the interface, should be called only when CRobotMain is destroyed
    virtual ~CDebugMenu();

    //! Toggle the debug interface
    void ToggleInterface();
    //! Check if the debug interface is open
    bool IsActive();

    //! Event processing
    bool EventProcess(const Event& event);

protected:
    //! Create the main page of debug interface
    void CreateInterface();
    //! Create the spawn object interface
    void CreateSpawnInterface();
    //! Update controls in the debug interface
    void UpdateInterface();
    //! Destroy the debug interface window
    void DestroyInterface();

    //! Handle frame update
    //! This is used to update the cursor coordinates overlay
    void HandleFrameUpdate(const Event &event);

    //! Handle spawning a new object at mouse position
    //! \return true on success, false on error
    bool HandleSpawnObject(ObjectType type, Math::Point mousePos);
    //! Handle lightning at position
    //! \return true on success, false on error
    bool HandleLightning(Math::Point mousePos);
    //! Handle teleport to position
    //! \return true on success, false on error
    bool HandleTeleport(Math::Point mousePos);
    //! Handle ctrl+c (copy coordinates under cursor to clipboard)
    //! \return true on success, false on error
    bool HandleCopy(Math::Point mousePos);

protected:
    CRobotMain* m_main;
    CInterface* m_interface;
    Gfx::CEngine* m_engine;
    CObjectManager* m_objMan;
    CSoundInterface* m_sound;

    ObjectType m_spawningType = OBJECT_NULL;
    bool m_lightningActive = false;
    bool m_teleportActive = false;
};

}
