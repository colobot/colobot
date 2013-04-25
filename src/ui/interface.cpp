// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.


#include "ui/interface.h"

#include "app/app.h"

namespace Ui {


CInterface::CInterface()
{
    m_event  = CApplication::GetInstancePointer()->GetEventQueue();
    m_engine = Gfx::CEngine::GetInstancePointer();
    m_camera = nullptr;

    for (int i = 0; i < MAXCONTROL; i++ )
    {
        m_table[i] = nullptr;
    }
}

// Object's destructor.

CInterface::~CInterface()
{
    Flush();
}


// Purge all controls.

void CInterface::Flush()
{
    for (int i = 0; i < MAXCONTROL; i++ ) {
        if ( m_table[i] != nullptr ) {
            delete m_table[i];
            m_table[i] = nullptr;
        }
    }
}


int CInterface::GetNextFreeControl()
{
    for (int i = 10; i < MAXCONTROL-1; i++) {
        if (m_table[i] == nullptr)
            return i;
    }
    return -1;
}


template <typename T> inline T* CInterface::CreateControl(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    T* pc;
    int index;
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    if ((index = GetNextFreeControl()) < 0)
        return nullptr;

    m_table[index] = new T();
    pc = static_cast<T *>(m_table[index]);
    pc->Create(pos, dim, icon, eventMsg);
    return pc;
}


// Creates a new button.

CWindow* CInterface::CreateWindows(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    CWindow* pc;
    int index;
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    switch (eventMsg) {
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
        case EVENT_TOOLTIP: index = MAXCONTROL-1; break;
        default: index = GetNextFreeControl(); break;
    }

    if (index < 0)
        return nullptr;

    m_table[index] = new CWindow();
    pc = static_cast<CWindow *>(m_table[index]);
    pc->Create(pos, dim, icon, eventMsg);
    return pc;
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

// Creates a new pave editable.

CEditValue* CInterface::CreateEditValue(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CEditValue>(pos, dim, icon, eventMsg);
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

// Creates a new list.
// if expand is less then zero, then the list would try to use expand's absolute value,
// and try to scale items to some size, so that dim of the list would not change after
// adjusting

CList* CInterface::CreateList(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, float expand)
{
    CList* pc;
    int index;
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    if ((index = GetNextFreeControl()) < 0)
        return nullptr;

    m_table[index] = new CList();
    pc = static_cast<CList *>(m_table[index]);
    pc->Create(pos, dim, icon, eventMsg, expand);
    return pc;
}

// Creates a new shortcut.

CShortcut* CInterface::CreateShortcut(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CShortcut>(pos, dim, icon, eventMsg);
}

// Creates a new compass.

CCompass* CInterface::CreateCompass(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    return CreateControl<CCompass>(pos, dim, icon, eventMsg);
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
    for (int i = 0; i < MAXCONTROL; i++) {
        if ( m_table[i] != nullptr ) {
            if (eventMsg == m_table[i]->GetEventType()) {
                delete m_table[i];
                m_table[i] = nullptr;
                return true;
            }
        }
    }
    return false;
}

// Gives a control.

CControl* CInterface::SearchControl(EventType eventMsg)
{
    for (int i = 0; i < MAXCONTROL; i++) {
        if (m_table[i] != nullptr) {
            if (eventMsg == m_table[i]->GetEventType())
                return m_table[i];
        }
    }
    return nullptr;
}

// Management of an event.

bool CInterface::EventProcess(const Event &event)
{
    if (event.type == EVENT_MOUSE_MOVE)
    {
        if (m_camera == nullptr)
            m_camera = CRobotMain::GetInstancePointer()->GetCamera();

        m_engine->SetMouseType(m_camera->GetMouseDef(event.mousePos));
    }

    for (int i = MAXCONTROL-1; i >= 0; i--)
    {
        if (m_table[i] != nullptr &&  m_table[i]->TestState(STATE_ENABLE))
        {
            if ( !m_table[i]->EventProcess(event) )
                return false;
        }
    }

    return true;
}


// Gives the tooltip binding to the window.

bool CInterface::GetTooltip(Math::Point pos, std::string &name)
{
    for (int i = MAXCONTROL-1; i >= 0; i--)
    {
        if (m_table[i] != nullptr)
        {
            if (m_table[i]->GetTooltip(pos, name))
                return true;
        }
    }
    return false;
}


// Draws all buttons.

void CInterface::Draw()
{
    for (int i = 0; i < MAXCONTROL; i++)
    {
        if ( m_table[i] != nullptr )
            m_table[i]->Draw();
    }
}

}
