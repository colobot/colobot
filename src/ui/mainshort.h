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

#include "common/event.h"

#include "math/point.h"

#include "object/object_type.h"

#include <vector>

class CRobotMain;
class CObject;

namespace Gfx
{
class CEngine;
} // namespace Gfx

namespace Ui
{

class CInterface;

class CMainShort
{
public:
    CMainShort();
    ~CMainShort();

    void        SetMode(bool bBuilding);
    bool        CreateShortcuts();
    bool        UpdateShortcuts();
    void        SelectShortcut(EventType event);
    void        SelectNext();
    CObject*    DetectShort(Math::Point pos);
    void        SetHighlight(CObject* pObj);

protected:
    int         GetShortcutIcon(ObjectType type);

protected:
    CEventQueue*      m_event;
    Gfx::CEngine*     m_engine;
    CInterface*     m_interface;
    CRobotMain*     m_main;

    std::vector<CObject*> m_shortcuts;
    bool            m_bBuilding;
};

} // namespace Ui
