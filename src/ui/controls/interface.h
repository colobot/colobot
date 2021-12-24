/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <string>

namespace Gfx
{
class CEngine;
} // namespace Gfx

namespace Ui
{

class CButton;
class CCheck;
class CColor;
class CControl;
class CEdit;
class CEditValue;
class CEnumSlider;
class CGroup;
class CImage;
class CKey;
class CLabel;
class CList;
class CMap;
class CScroll;
class CShortcut;
class CSlider;
class CTarget;
class CWindow;

const int MAXCONTROL = 100;

class CInterface
{
public:
    CInterface();
    ~CInterface();

    bool        EventProcess(const Event &event);
    bool        GetTooltip(const glm::vec2& pos, std::string &name);

    void        Flush();
    CButton*    CreateButton(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CColor*     CreateColor(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CCheck*     CreateCheck(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CKey*       CreateKey(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CGroup*     CreateGroup(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CImage*     CreateImage(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CEdit*      CreateEdit(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CEditValue* CreateEditValue(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CScroll*    CreateScroll(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CSlider*    CreateSlider(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CEnumSlider* CreateEnumSlider(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CShortcut*  CreateShortcut(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CTarget*    CreateTarget(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CMap*       CreateMap(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);

    CWindow*    CreateWindows(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CList*      CreateList(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, float expand=1.2f);
    CLabel*     CreateLabel(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, std::string name);

    bool        DeleteControl(EventType eventMsg);
    CControl*   SearchControl(EventType eventMsg);

    void        Draw();

    void        SetFocus(CControl* focusControl);

protected:
    int GetNextFreeControl();

    template <typename ControlClass>
    ControlClass* CreateControl(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);

    CEventQueue* m_event;
    Gfx::CEngine* m_engine;
    std::array<std::unique_ptr<CControl>, MAXCONTROL> m_controls;
};


} // namespace Ui
