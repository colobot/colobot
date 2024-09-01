/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/controls/window.h"

#include "ui/controls/button.h"
#include "ui/controls/check.h"
#include "ui/controls/color.h"
#include "ui/controls/control.h"
#include "ui/controls/edit.h"
#include "ui/controls/editvalue.h"
#include "ui/controls/enumslider.h"
#include "ui/controls/gauge.h"
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

#include "graphics/core/renderers.h"
#include "graphics/core/transparency.h"

#include "graphics/engine/engine.h"

#include <algorithm>


namespace Ui
{
// Object's constructor.

CWindow::CWindow() : CControl()
{
    m_bTrashEvent = true;
    m_bMaximized  = false;
    m_bMinimized  = false;
    m_bFixed      = false;

    m_minDim = { 0.0f, 0.0f };
    m_maxDim = { 1.0f, 1.0f };

    m_bMovable  = false;
    m_bRedim    = false;
    m_bClosable = false;
    m_bCapture  = false;
    m_pressFlags = 0;
    m_pressMouse = Gfx::ENG_MOUSE_NORM;

//    m_fontStretch = NORMSTRETCH*1.2f;
}

// Object's destructor.

CWindow::~CWindow()
{
}


// Purge all the controls.

void CWindow::Flush()
{
    m_controls.clear();

    m_buttonReduce.reset();
    m_buttonFull.reset();
    m_buttonClose.reset();
}


// Creates a new window.

bool CWindow::Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventMsg);
    return true;
}

template<typename ControlClass>
ControlClass* CWindow::CreateControl(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    auto control = std::make_unique<ControlClass>();
    control->Create(pos, dim, icon, eventMsg);
    auto* controlPtr = control.get();
    m_controls.push_back(std::move(control));
    return controlPtr;
}


// Creates a new button.

CButton* CWindow::CreateButton(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    return CreateControl<CButton>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CColor* CWindow::CreateColor(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    return CreateControl<CColor>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CCheck* CWindow::CreateCheck(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    return CreateControl<CCheck>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CKey* CWindow::CreateKey(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    return CreateControl<CKey>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CGroup* CWindow::CreateGroup(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    return CreateControl<CGroup>(pos, dim, icon, eventMsg);
}

// Creates a new button.

CImage* CWindow::CreateImage(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    return CreateControl<CImage>(pos, dim, icon, eventMsg);
}

// Creates a new label.

CLabel* CWindow::CreateLabel(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, std::string name)
{
    CLabel* label = CreateControl<CLabel>(pos, dim, icon, eventMsg);

    auto p = name.find("\\");
    if (p == std::string::npos)
        label->SetName(name, false);
    else
        label->SetName(name.substr(0, p), false);

    return label;
}

CLabel* CWindow::CreateLabelRaw(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, std::string name)
{
    CLabel* label = CreateControl<CLabel>(pos, dim, icon, eventMsg);

    label->SetName(name, false);

    return label;
}

// Creates a new editable pave.

CEdit* CWindow::CreateEdit(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    return CreateControl<CEdit>(pos, dim, icon, eventMsg);
}

// Creates a new editable pave.

CEditValue* CWindow::CreateEditValue(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    return CreateControl<CEditValue>(pos, dim, icon, eventMsg);
}

// Creates a new elevator.

CScroll* CWindow::CreateScroll(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    return CreateControl<CScroll>(pos, dim, icon, eventMsg);
}

// Creates a new cursor.

CSlider* CWindow::CreateSlider(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    return CreateControl<CSlider>(pos, dim, icon, eventMsg);
}

CEnumSlider* CWindow::CreateEnumSlider(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    return CreateControl<CEnumSlider>(pos, dim, icon, eventMsg);
}

// Creates a new list.
// if expand is less then zero, then the list would try to use expand's absolute value,
// and try to scale items to some size, so that dim of the list would not change after
// adjusting

CList* CWindow::CreateList(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, float expand)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    auto list = std::make_unique<CList>();
    list->Create(pos, dim, icon, eventMsg, expand);
    auto* listPtr = list.get();
    m_controls.push_back(std::move(list));
    return listPtr;
}

// Creates a new shortcut.

CShortcut* CWindow::CreateShortcut(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    return CreateControl<CShortcut>(pos, dim, icon, eventMsg);
}

// Creates a new card.

CMap* CWindow::CreateMap(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    return CreateControl<CMap>(pos, dim, icon, eventMsg);
}

// Creates a new gauge.

CGauge* CWindow::CreateGauge(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    return CreateControl<CGauge>(pos, dim, icon, eventMsg);
}

// Creates a new target.

CTarget* CWindow::CreateTarget(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    return CreateControl<CTarget>(pos, dim, icon, eventMsg);
}

// Removes a control.

bool CWindow::DeleteControl(EventType eventMsg)
{
    auto controlIt = std::find_if(m_controls.begin(), m_controls.end(),
                                  [eventMsg](const std::unique_ptr<CControl>& control)
                                  {
                                      return control->GetEventType() == eventMsg;
                                  });

    if (controlIt == m_controls.end())
        return false;

    m_controls.erase(controlIt);
    return true;
}

// Gives a control.

CControl* CWindow::SearchControl(EventType eventMsg)
{
    auto controlIt = std::find_if(m_controls.begin(), m_controls.end(),
                                  [eventMsg](const std::unique_ptr<CControl>& control)
                                  {
                                      return control->GetEventType() == eventMsg;
                                  });

    if (controlIt == m_controls.end())
        return nullptr;

    return controlIt->get();
}


// Makes the tooltip binds to the window.

bool CWindow::GetTooltip(const glm::vec2& pos, std::string &name)
{
    for (auto& control : m_controls)
    {
        if (control->GetTooltip(pos, name))
            return true;
    }

    if (m_buttonClose != nullptr &&
        m_buttonClose->GetTooltip(pos, name))
    {
        return true;
    }
    if (m_buttonFull != nullptr &&
        m_buttonFull->GetTooltip(pos, name))
    {
        return true;
    }
    if (m_buttonReduce != nullptr &&
        m_buttonReduce->GetTooltip(pos, name))
    {
        return true;
    }

    if ( Detect(pos) )  // in the window?
    {
        name = m_tooltip;
        return true;
    }

    return false;
}


// Specifies the name for the title bar.

void CWindow::SetName(std::string name, bool tooltip)
{
    CControl::SetName(name, tooltip);

    m_buttonReduce.reset();
    m_buttonFull.reset();
    m_buttonClose.reset();

    bool bAdjust = false;

    if ( m_name.length() > 0 && m_bRedim )  // title bar exists?
    {
        m_buttonReduce = std::make_unique<CButton>();
        m_buttonReduce->Create(m_pos, m_dim, 0, EVENT_NULL);

        m_buttonFull = std::make_unique<CButton>();
        m_buttonFull->Create(m_pos, m_dim, 0, EVENT_NULL);

        bAdjust = true;
    }

    if ( m_name.length() > 0 && m_bClosable )  // title bar exists?
    {
        m_buttonClose = std::make_unique<CButton>();
        m_buttonClose->Create(m_pos, m_dim, 0, EVENT_NULL);

        bAdjust = true;
    }

    if ( bAdjust )
    {
        AdjustButtons();
    }

    MoveAdjust();
}


void CWindow::SetPos(const glm::vec2& pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CWindow::SetDim(const glm::vec2& dimension)
{
    glm::vec2 dim = dimension;

    if ( dim.x < m_minDim.x )  dim.x = m_minDim.x;
    if ( dim.x > m_maxDim.x )  dim.x = m_maxDim.x;
    if ( dim.y < m_minDim.y )  dim.y = m_minDim.y;
    if ( dim.y > m_maxDim.y )  dim.y = m_maxDim.y;

    CControl::SetDim(dim);
    MoveAdjust();
}

void CWindow::MoveAdjust()
{
    float h = m_engine->GetText()->GetHeight(m_fontType, m_fontSize);
    glm::vec2 dim;
    dim.y = h*1.2f;
    dim.x = dim.y*0.75f;

    float offset = 0.0f;
    if (m_buttonClose != nullptr)
    {
        glm::vec2 pos;
        pos.x = m_pos.x+m_dim.x-0.01f-dim.x;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
        m_buttonClose->SetPos(pos);
        m_buttonClose->SetDim(dim);
        offset = dim.x*1.0f;
    }
    else
    {
        offset = 0.0f;
    }

    if (m_buttonFull != nullptr)
    {
        glm::vec2 pos;
        pos.x = m_pos.x+m_dim.x-0.01f-dim.x-offset;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
        m_buttonFull->SetPos(pos);
        m_buttonFull->SetDim(dim);
    }

    if (m_buttonReduce != nullptr)
    {
        glm::vec2 pos;
        pos.x = m_pos.x+m_dim.x-0.01f-dim.x*2.0f-offset;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
        m_buttonReduce->SetPos(pos);
        m_buttonReduce->SetDim(dim);
    }
}


void CWindow::SetMinDim(const glm::vec2& dim)
{
    m_minDim = dim;
}

void CWindow::SetMaxDim(const glm::vec2& dim)
{
    m_maxDim = dim;
}

glm::vec2 CWindow::GetMinDim()
{
    return m_minDim;
}

glm::vec2 CWindow::GetMaxDim()
{
    return m_maxDim;
}


// Indicates whether the window is moved.

void CWindow::SetMovable(bool bMode)
{
    m_bMovable = bMode;
}

bool CWindow::GetMovable()
{
    return m_bMovable;
}


// Management of the presence of minimize/maximize buttons.

void CWindow::SetRedim(bool bMode)
{
    m_bRedim = bMode;
}

bool CWindow::GetRedim()
{
    return m_bRedim;
}


// Management of the presence of the close button.

void CWindow::SetClosable(bool bMode)
{
    m_bClosable = bMode;
}

bool CWindow::GetClosable()
{
    return m_bClosable;
}


void CWindow::SetMaximized(bool bMaxi)
{
    m_bMaximized = bMaxi;
    AdjustButtons();
}

bool CWindow::GetMaximized()
{
    return m_bMaximized;
}

void CWindow::SetMinimized(bool bMini)
{
    m_bMinimized = bMini;
    AdjustButtons();
}

bool CWindow::GetMinimized()
{
    return m_bMinimized;
}

void CWindow::SetFixed(bool bFix)
{
    m_bFixed = bFix;
}

bool CWindow::GetFixed()
{
    return m_bFixed;
}


// Adjusts the buttons in the title bar.

void CWindow::AdjustButtons()
{
    std::string res;

    if (m_buttonFull != nullptr)
    {
        if ( m_bMaximized )
        {
            m_buttonFull->SetIcon(54);
            GetResource(RES_TEXT, RT_WINDOW_STANDARD, res);
            m_buttonFull->SetTooltip(res);
        }
        else
        {
            m_buttonFull->SetIcon(52);
            GetResource(RES_TEXT, RT_WINDOW_MAXIMIZED, res);
            m_buttonFull->SetTooltip(res);
        }
    }

    if (m_buttonReduce != nullptr)
    {
        if ( m_bMinimized )
        {
            m_buttonReduce->SetIcon(54);
            GetResource(RES_TEXT, RT_WINDOW_STANDARD, res);
            m_buttonReduce->SetTooltip(res);
        }
        else
        {
            m_buttonReduce->SetIcon(51);
            GetResource(RES_TEXT, RT_WINDOW_MINIMIZED, res);
            m_buttonReduce->SetTooltip(res);
        }
    }

    if (m_buttonClose != nullptr)
    {
        m_buttonClose->SetIcon(11);  // x
        GetResource(RES_TEXT, RT_WINDOW_CLOSE, res);
        m_buttonClose->SetTooltip(res);
    }
}


void CWindow::SetTrashEvent(bool bTrash)
{
    m_bTrashEvent = bTrash;
}

bool CWindow::GetTrashEvent()
{
    return m_bTrashEvent;
}


// Returns the message from the button "reduce".

EventType CWindow::GetEventTypeReduce()
{
    if (m_buttonReduce == nullptr)  return EVENT_NULL;
    return m_buttonReduce->GetEventType();
}

// Returns the message from the button "full".

EventType CWindow::GetEventTypeFull()
{
    if (m_buttonFull == nullptr)  return EVENT_NULL;
    return m_buttonFull->GetEventType();
}

// Returns the message from the button "close".

EventType CWindow::GetEventTypeClose()
{
    if (m_buttonClose == nullptr)  return EVENT_NULL;
    return m_buttonClose->GetEventType();
}


// Detects whether the mouse is in an edge of the window, to resize it.
// Bit returns: 0 = left, 1 = down, 2 = right, 3 = up, 1 = all.

int CWindow::BorderDetect(const glm::vec2& pos)
{
    glm::vec2 dim;
    float   h;
    int     flags;

    if ( m_bMaximized || m_bMinimized || m_bFixed )  return 0;

    flags = 0;
    if ( pos.x < m_pos.x+0.030f )
    {
        flags |= (1<<0);
    }
    if ( pos.y < m_pos.y+0.020f )
    {
        flags |= (1<<1);
    }
    if ( pos.x > m_pos.x+m_dim.x-0.030f )
    {
        flags |= (1<<2);
    }
    if ( pos.y > m_pos.y+m_dim.y-0.020f )
    {
        flags |= (1<<3);
    }

    if ( pos.x > m_pos.x+        0.015f &&
         pos.x < m_pos.x+m_dim.x-0.015f &&
         pos.y > m_pos.y+        0.010f &&
         pos.y < m_pos.y+m_dim.y-0.010f )
    {
        flags = 0;
    }

    if ( flags == 0 )
    {
        h = m_engine->GetText()->GetHeight(m_fontType, m_fontSize);
        dim.y = h*1.2f;
        dim.x = dim.y*0.75f;
        if ( pos.x <  m_pos.x+m_dim.x-0.01f-dim.x*3.0f &&
             pos.y >= m_pos.y+m_dim.y-0.01f-h*1.2f     )
        {
            flags = -1;
        }
    }

    return flags;
}

// Management of an event.

bool CWindow::EventProcess(const Event &event)
{
    if ( event.type == EVENT_MOUSE_MOVE || event.type == EVENT_MOUSE_BUTTON_DOWN || event.type == EVENT_MOUSE_BUTTON_UP )
    {
        if ( m_bCapture )
        {
            m_engine->SetMouseType(m_pressMouse);
        }
        else
        {
            m_pressMouse = Gfx::ENG_MOUSE_NORM;

            if ( m_name.length() > 0 && m_bMovable &&  // title bar?
                 Detect(event.mousePos) )
            {
                int flags = BorderDetect(event.mousePos);
                if ( flags == -1 )
                {
                    m_pressMouse = Gfx::ENG_MOUSE_MOVE;  // +
                }
                else if ( ((flags & (1<<0)) && (flags & (1<<3))) ||
                          ((flags & (1<<1)) && (flags & (1<<2))) )
                {
                    m_pressMouse = Gfx::ENG_MOUSE_MOVEI;  // \ //
                }
                else if ( ((flags & (1<<0)) && (flags & (1<<1))) ||
                          ((flags & (1<<2)) && (flags & (1<<3))) )
                {
                    m_pressMouse = Gfx::ENG_MOUSE_MOVED;  // /
                }
                else if ( (flags & (1<<0)) || (flags & (1<<2)) )
                {
                    m_pressMouse = Gfx::ENG_MOUSE_MOVEH;  // -
                }
                else if ( (flags & (1<<1)) || (flags & (1<<3)) )
                {
                    m_pressMouse = Gfx::ENG_MOUSE_MOVEV;  // |
                }
            }

            if ( m_pressMouse != Gfx::ENG_MOUSE_NORM )
            {
                m_engine->SetMouseType(m_pressMouse);
            }
        }
    }

    if ( !m_bCapture )
    {
        for (auto& control : m_controls)
        {
            if (! control->EventProcess(event))
                return false;
        }

        if (m_buttonReduce != nullptr)
        {
            m_buttonReduce->EventProcess(event);
        }
        if (m_buttonFull != nullptr)
        {
            m_buttonFull->EventProcess(event);
        }
        if (m_buttonClose != nullptr)
        {
            m_buttonClose->EventProcess(event);
        }
    }

    if (m_bTrashEvent &&
        event.type == EVENT_MOUSE_BUTTON_DOWN &&
        event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_LEFT)
    {
        if ( Detect(event.mousePos) )
        {
            if ( m_name.length() > 0 && m_bMovable )  // title bar?
            {
                m_pressFlags = BorderDetect(event.mousePos);
                if ( m_pressFlags != 0 )
                {
                    m_bCapture = true;
                    m_pressPos = event.mousePos;
                }
            }
            return false;
        }
    }

    if ( event.type == EVENT_MOUSE_MOVE && m_bCapture )
    {
        glm::vec2 pos = event.mousePos;
        if ( m_pressFlags == -1 )  // all moves?
        {
            m_pos.x += pos.x-m_pressPos.x;
            m_pos.y += pos.y-m_pressPos.y;
        }
        else
        {
            if ( m_pressFlags & (1<<0) )  // left edge?
            {
                if ( pos.x > m_pressPos.x+m_dim.x-m_minDim.x )
                {
                    pos.x = m_pressPos.x+m_dim.x-m_minDim.x;
                }
                m_pos.x += pos.x-m_pressPos.x;
                m_dim.x -= pos.x-m_pressPos.x;
            }
            if ( m_pressFlags & (1<<1) )  // bottom edge?
            {
                if ( pos.y > m_pressPos.y+m_dim.y-m_minDim.y )
                {
                    pos.y = m_pressPos.y+m_dim.y-m_minDim.y;
                }
                m_pos.y += pos.y-m_pressPos.y;
                m_dim.y -= pos.y-m_pressPos.y;
            }
            if ( m_pressFlags & (1<<2) )  // right edge?
            {
                if ( pos.x < m_pressPos.x-m_dim.x+m_minDim.x )
                {
                    pos.x = m_pressPos.x-m_dim.x+m_minDim.x;
                }
                m_dim.x += pos.x-m_pressPos.x;
            }
            if ( m_pressFlags & (1<<3) )  // top edge?
            {
                if ( pos.y < m_pressPos.y-m_dim.y+m_minDim.y )
                {
                    pos.y = m_pressPos.y-m_dim.y+m_minDim.y;
                }
                m_dim.y += pos.y-m_pressPos.y;
            }
        }
        m_pressPos = pos;
        AdjustButtons();

        m_event->AddEvent(Event(m_eventType));
    }

    if (event.type == EVENT_MOUSE_BUTTON_UP &&
        event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_LEFT &&
        m_bCapture)
    {
        m_bCapture = false;
    }

    return true;
}


// Draws the window.

void CWindow::Draw()
{
    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    DrawVertex(m_pos, m_dim, m_icon);  // draws the background

    if ( m_name.length() > 0 )  // title bar?
    {
        float h = m_engine->GetText()->GetHeight(m_fontType, m_fontSize);

        glm::vec2 pos, dim;
        // Draws the shadow under the title bar.
        {
            pos.x = m_pos.x+0.01f;
            dim.x = m_dim.x-0.02f;
            pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
            dim.y = h*1.2f;
            DrawShadow(pos, dim);
        }

        float width = m_dim.x;
        if ( m_bRedim    )  width -= h*1.2f*0.75f*2.0f;
        if ( m_bClosable )  width -= h*1.2f*0.75f;

        pos.x = m_pos.x+0.01f;
        dim.x = width-0.02f;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
        dim.y = h*1.2f;
        DrawVertex(pos, dim, (m_state&STATE_ENABLE)?2:9);

        float sw = m_engine->GetText()->GetStringWidth(m_name, m_fontType, m_fontSize);

        if ( m_state&STATE_ENABLE )
        {
            pos.x = m_pos.x+0.015f;
            dim.x = (width-sw-0.06f)/2.0f;
            pos.y = m_pos.y+m_dim.y-0.01f-h*1.0f;
            dim.y = h*0.8f;
            DrawHach(pos, dim);  // left hatch
            pos.x = m_pos.x+width-dim.x-0.015f;
            DrawHach(pos, dim);  // right hatch
        }

        pos.x = m_pos.x+width/2.0f;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.10f;
        m_engine->GetText()->DrawText(m_name, m_fontType, m_fontSize, pos, width, Gfx::TEXT_ALIGN_CENTER, 0);

        if (m_buttonReduce != nullptr)
        {
            m_buttonReduce->Draw();
        }

        if (m_buttonFull != nullptr)
        {
            m_buttonFull->Draw();
        }

        if (m_buttonClose != nullptr)
        {
            m_buttonClose->Draw();
        }
    }

    for (auto& control : m_controls)
    {
        control->Draw();
    }
}

// Draws a rectangle.

void CWindow::DrawVertex(const glm::vec2& position, const glm::vec2& dimension, int icon)
{
    glm::vec2   uv1, uv2, corner;
    float       dp;
    int         i;

    glm::vec2 pos = position;
    glm::vec2 dim = dimension;

    auto renderer = m_engine->GetUIRenderer();

    dp = 0.5f/256.0f;

    if ( icon == 0 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::WHITE);
        uv1.x =  64.0f/256.0f;  // dark blue transparent
        uv1.y =  64.0f/256.0f;
        uv2.x = 128.0f/256.0f;
        uv2.y = 128.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    else if ( icon == 1 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button1.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x = 128.0f/256.0f;  // white tooltip
        uv1.y =   0.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y =  16.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 2 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button1.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x = 128.0f/256.0f;  // yellow
        uv1.y =  16.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 3 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::BLACK);
        uv1.x =   0.0f/256.0f;  // transparent blue bar with yellow upper
        uv1.y =  64.0f/256.0f;
        uv2.x =  64.0f/256.0f;
        uv2.y = 128.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 4 )  // SatCom ?
    {
        pos.x -=  50.0f/640.0f;
        pos.y -=  30.0f/480.0f;
        dim.x += 100.0f/640.0f;
        dim.y +=  60.0f/480.0f;

        auto texture = m_engine->LoadTexture("textures/objects/human.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x = 140.0f/256.0f;
        uv1.y =  32.0f/256.0f;
        uv2.x = 182.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);  // clothing

        pos.x += 20.0f/640.0f;
        pos.y -= 10.0f/480.0f;
        dim.x -= 20.0f/640.0f;
        dim.y +=  0.0f/480.0f;

        texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::WHITE);
        uv1.x = 192.0f/256.0f;
        uv1.y =  32.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 30.0f/640.0f;
        corner.y = 30.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 5.0f/256.0f);  // shadow

        pos.x +=  0.0f/640.0f;
        pos.y += 20.0f/480.0f;
        dim.x -= 20.0f/640.0f;
        dim.y -= 20.0f/480.0f;

        texture = m_engine->LoadTexture("textures/interface/button1.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x =  64.0f/256.0f;
        uv1.y =   0.0f/256.0f;
        uv2.x =  96.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);  // outside blue

        pos.x += 20.0f/640.0f;
        pos.y += 10.0f/480.0f;
        dim.x -= 40.0f/640.0f;
        dim.y -= 20.0f/480.0f;

        uv1.x =  96.0f/256.0f;
        uv1.y =   0.0f/256.0f;
        uv2.x = 128.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);  // inside blue

        pos.x += 10.0f/640.0f;
        pos.y += 10.0f/480.0f;
        dim.x -= 20.0f/640.0f;
        dim.y -= 20.0f/480.0f;

        texture = m_engine->LoadTexture("textures/interface/button3.png");
        renderer->SetTexture(texture);
        uv1.x =   0.0f/256.0f;
        uv1.y = 224.0f/256.0f;
        uv2.x =  32.0f/256.0f;
        uv2.y = 256.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);  // dark blue background

        texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        uv1.x = 224.0f/256.0f;
        uv1.y = 224.0f/256.0f;
        uv2.x = 249.0f/256.0f;
        uv2.y = 235.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        pos.x = 20.0f/640.0f;
        pos.y = 70.0f/480.0f;
        dim.x = 25.0f/640.0f;
        dim.y = 11.0f/480.0f;
        for ( i=0 ; i<5 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // = bottom/left
            pos.y += 15.0f/480.0f;
        }
        pos.y = (480.0f-70.0f-11.0f)/480.0f;
        for ( i=0 ; i<5 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // = top/left
            pos.y -= 15.0f/480.0f;
        }
        pos.x = (640.0f-25.0f-20.0f)/640.0f;
        pos.y = 70.0f/480.0f;
        for ( i=0 ; i<5 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // = bottom/right
            pos.y += 15.0f/480.0f;
        }
        pos.y = (480.0f-70.0f-11.0f)/480.0f;
        for ( i=0 ; i<5 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // = top/right
            pos.y -= 15.0f/480.0f;
        }

        uv1.x = 208.0f/256.0f;
        uv1.y = 224.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y = 240.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        dim.x = 10.0f/640.0f;
        dim.y = 10.0f/480.0f;
        pos.x = 534.0f/640.0f;
        pos.y = 430.0f/480.0f;
        for ( i=0 ; i<3 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // micro
            pos.x += 12.0f/640.0f;
        }
        pos.x = 528.0f/640.0f;
        pos.y -= 12.0f/480.0f;
        for ( i=0 ; i<4 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // micro
            pos.x += 12.0f/640.0f;
        }
        pos.x = 534.0f/640.0f;
        pos.y -= 12.0f/480.0f;
        for ( i=0 ; i<3 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // micro
            pos.x += 12.0f/640.0f;
        }
    }
    else if ( icon == 5 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::BLACK);
        uv1.x =  64.0f/256.0f;  // transparent green
        uv1.y = 160.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 176.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 6 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::BLACK);
        uv1.x =  64.0f/256.0f;  // transparent red
        uv1.y = 176.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 192.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 7 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::BLACK);
        uv1.x =  64.0f/256.0f;  // transparent blue
        uv1.y = 192.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 208.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 8 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button1.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x =   0.0f/256.0f;  // opaque orange
        uv1.y =   0.0f/256.0f;
        uv2.x =  32.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    else if ( icon == 9 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x =  32.0f/256.0f;  // opaque gray
        uv1.y =  32.0f/256.0f;
        uv2.x =  64.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    else if ( icon == 10 )
    {
        // nothing (in the background image)!
    }
    else if ( icon == 11 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button2.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::BLACK);
        uv1.x =  64.0f/256.0f;  // transparent yellow
        uv1.y = 224.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 240.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 12 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button1.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x = 128.0f/256.0f;  // dirty opaque gray
        uv1.y = 128.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 160.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 6.0f/640.0f;
        corner.y = 6.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 5.0f/256.0f);
    }
    else if ( icon == 13 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button1.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x = 192.0f/256.0f;  //  dirty opaque blue
        uv1.y = 128.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y = 160.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 6.0f/640.0f;
        corner.y = 6.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 5.0f/256.0f);
    }
    else if ( icon == 14 )
    {
        auto texture = m_engine->LoadTexture("textures/interface/button1.png");
        renderer->SetTexture(texture);
        renderer->SetTransparency(Gfx::TransparencyMode::NONE);
        uv1.x = 160.0f/256.0f;  // dirty opaque red
        uv1.y = 128.0f/256.0f;
        uv2.x = 192.0f/256.0f;
        uv2.y = 160.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 6.0f/640.0f;
        corner.y = 6.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 5.0f/256.0f);
    }
}

// Draws hatching.

void CWindow::DrawHach(const glm::vec2& pos, const glm::vec2& dim)
{
    glm::vec2   ppos, ddim, uv1, uv2;
    float       dp, max, ndim;
    bool        bStop;

    auto renderer = m_engine->GetUIRenderer();

    dp = 0.5f/256.0f;
    auto texture = m_engine->LoadTexture("textures/interface/button2.png");
    renderer->SetTexture(texture);
    renderer->SetTransparency(Gfx::TransparencyMode::NONE);
    uv1.x =  64.0f/256.0f;  // hatching
    uv1.y = 208.0f/256.0f;
    uv2.x = 145.0f/256.0f;
    uv2.y = 224.0f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    max = dim.y*(uv2.x-uv1.x)/(uv2.y-uv1.y);

    ppos = pos;
    ddim = dim;
    bStop = false;
    do
    {
        ddim.x = max;
        if ( ppos.x+ddim.x > pos.x+dim.x )
        {
            ndim = pos.x+dim.x-ppos.x;
            uv2.x = uv1.x+(uv2.x-uv1.x)*(ndim/ddim.x);
            ddim.x = ndim;
            bStop = true;
        }
        DrawIcon(ppos, ddim, uv1, uv2);

        ppos.x += ddim.x;
    }
    while ( !bStop );
}

void CWindow::SetFocus(CControl* focusControl)
{
    for (auto& control : m_controls)
    {
        if (control != nullptr)
        {
            control->SetFocus(focusControl);
        }
    }
}

}
