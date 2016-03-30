/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <math/point.h>
#include <sound/sound.h>
#include "object/object_type.h"

class CRobotMain;
class CObjectManager;
struct Event;

namespace Gfx
{
class CEngine;
}

namespace Ui
{
class CInterface;

class CDebugMenu
{
public:
    CDebugMenu(CRobotMain* main, Gfx::CEngine* engine, CObjectManager* objMan, CSoundInterface* sound);
    virtual ~CDebugMenu();

    void ToggleInterface();
    bool EventProcess(const Event& event);

protected:
    void CreateInterface();
    void CreateSpawnInterface();
    void UpdateInterface();
    void DestroyInterface();

    bool HandleSpawnObject(ObjectType type, Math::Point mousePos);
    bool HandleLightning(Math::Point mousePos);
    bool HandleTeleport(Math::Point mousePos);

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
