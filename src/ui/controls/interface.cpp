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


#include "ui/controls/interface.h"

#include "app/app.h"

#include "level/robotmain.h"

#include "ui/controls/button.h"
#include "ui/controls/check.h"
#include "ui/controls/color.h"
#include "ui/controls/control.h"
#include "ui/controls/edit.h"
#include "ui/controls/editvalue.h"
#include "ui/controls/enumslider.h"
#include "ui/controls/group.h"
#include "ui/controls/image.h"
#include "ui/controls/key.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/map.h"
#include "ui/controls/scroll.h"
#include "ui/controls/shortcut.h"
#include "ui/controls/slider.h"
#include "ui/controls/target.h"
#include "ui/controls/window.h"


namespace Ui
{


CInterface::CInterface()
{
    m_event  = CApplication::GetInstancePointer()->GetEventQueue();
    m_engine = Gfx::CEngine::GetInstancePointer();
}

// Object's destructor.

CInterface::~CInterface()
{
}


// Purge all controls.

void CInterface::Flush()
{
    for (auto& control : m_controls)
    {
        control.reset();
    }
}

int CInterface::GetNextFreeControl()
{
    for (int i = 10; i < static_cast<int>(m_controls.size()) - 1; i++)
    {
        if (m_controls[i] == nullptr)
            return i;
    }
    return -1;
}

template <typename ControlClass>
ControlClass* CInterface::CreateControl(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    int index = GetNextFreeControl();
    if (index < 0)
        return nullptr;

    auto control = MakeUnique<ControlClass>();
    control->Create(pos, dim, icon, eventMsg);
    auto* controlPtr = control.get();
    m_controls[index] = std::move(control);
    return controlPtr;
}


// Creates a new button.

CWindow* CInterface::CreateWindows(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    int index = -1;
    switch (eventMsg)
    {
        case EVENT_WINDOW0: index = 0; break;
        case EVENT_WINDOW1: index = 1; break;
        case EVENT_WINDOW2: index = 2; break;
        case EVENT_WINDOW3: index = 3; break;
        case EVENT_WINDOW4: index = 4; break;
        case EVENT_WINDOW5: index = 5; break;
        case EVENT_WINDOW6: index = 6; break;
        case EVENT_WINDOW7: index = 7; break;
        case EVENT_WINDOW8: index = 8; break;
        case EVENT_WINDOW9: index = 9; break;
        case EVENT_TOOLTIP: index = m_controls.size() - 1; break;
        default: index = GetNextFreeControl(); break;
    }

    if (index < 0)
        return nullptr;

    auto window = MakeUnique<CWindow>();
    window->Create(pos, dim, icon, eventMsg);
    auto* windowPtr = window.get();
    m_controls[index] = std::move(window);
    return windowPtr;
}

// Creates a new button.

CButton* CInterface::CreateButton(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CButton>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CColor* CInterface::CreateColor(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CColor>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CCheck* CInterface::CreateCheck(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CCheck>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CKey* CInterface::CreateKey(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CKey>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CGroup* CInterface::CreateGroup(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CGroup>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CImage* CInterface::CreateImage(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CImage>(pos, dim, icon, eventMsg);
}

// Creates a new label.

CLabel* CInterface::CreateLabel(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, std::string name)
{
    CLabel* pc = CreateControl<CLabel>(pos, dim, icon, eventMsg);
    if (pc != nullptr)
        pc->SetName(name);
    return pc;
}

// Creates a new pave editable.

CEdit* CInterface::CreateEdit(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CEdit>(pos, dim, icon, eventMsg);
}

// Creates a new editable area.
CEditValue* CInterface::CreateEditValue(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    CEditValue* ev = CreateControl<CEditValue>(pos, dim, icon, eventMsg);
    ev->SetInterface(this);
    return ev;
}

// Creates a new lift.

CScroll* CInterface::CreateScroll(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CScroll>(pos, dim, icon, eventMsg);
}

// Creates a new cursor.

CSlider* CInterface::CreateSlider(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CSlider>(pos, dim, icon, eventMsg);
}

CEnumSlider* CInterface::CreateEnumSlider(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CEnumSlider>(pos, dim, icon, eventMsg);
}

// Creates a new list.
// if expand is less then zero, then the list would try to use expand's absolute value,
// and try to scale items to some size, so that dim of the list would not change after
// adjusting

CList* CInterface::CreateList(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, float expand)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    int index = GetNextFreeControl();
    if (index < 0)
        return nullptr;

    auto list = MakeUnique<CList>();
    list->Create(pos, dim, icon, eventMsg, expand);
    auto* listPtr = list.get();
    m_controls[index] = std::move(list);
    return listPtr;
}

// Creates a new shortcut.

CShortcut* CInterface::CreateShortcut(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CShortcut>(pos, dim, icon, eventMsg);
}

// Creates a new target.

CTarget* CInterface::CreateTarget(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CTarget>(pos, dim, icon, eventMsg);
}

// Creates a new map.

CMap* CInterface::CreateMap(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CMap>(pos, dim, icon, eventMsg);
}

// Removes a control.

bool CInterface::DeleteControl(EventType eventMsg)
{
    for (auto& control : m_controls)
    {
        if (control != nullptr &&
            eventMsg == control->GetEventType())
        {
            control.reset();
            return true;
        }
    }
    return false;
}

// Gives a control.

CControl* CInterface::SearchControl(EventType eventMsg)
{
    for (auto& control : m_controls)
    {
        if (control != nullptr &&
            eventMsg == control->GetEventType())
        {
            return control.get();
        }
    }
    return nullptr;
}

// Management of an event.

bool CInterface::EventProcess(const Event &event)
{
    if (event.type == EVENT_MOUSE_MOVE || event.type == EVENT_MOUSE_BUTTON_DOWN || event.type == EVENT_MOUSE_BUTTON_UP)
    {
        m_engine->SetMouseType(Gfx::ENG_MOUSE_NORM);
    }

    for (auto it = m_controls.rbegin(); it != m_controls.rend(); ++it)
    {
        auto& control = *it;
        if (control != nullptr && control->TestState(STATE_ENABLE))
        {
            if (! control->EventProcess(event))
                return false;
        }
    }

    return true;
}


// Gives the tooltip binding to the window.

bool CInterface::GetTooltip(Math::Point pos, std::string &name)
{
    for (auto it = m_controls.rbegin(); it != m_controls.rend(); ++it)
    {
        auto& control = *it;
        if (control != nullptr)
        {
            if (control->GetTooltip(pos, name))
                return true;
        }
    }
    return false;
}


// Draws all buttons.

void CInterface::Draw()
{
    for (auto& control : m_controls)
    {
        if (control != nullptr)
            control->Draw();
    }
}

void CInterface::SetFocus(CControl* focusControl)
{
    for (auto& control : m_controls)
    {
        if (control != nullptr)
        {
            control->SetFocus(focusControl);
        }
    }
}


} // namespace Ui
