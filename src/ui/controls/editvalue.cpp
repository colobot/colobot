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


#include "ui/controls/editvalue.h"

#include "common/event.h"
#include "common/make_unique.h"

#include "level/robotmain.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/interface.h"

#include <cstdio>



namespace Ui
{
// Object's constructor.

CEditValue::CEditValue() : CControl ()
{
    m_type = EVT_100;  // %
    m_stepValue = 0.1f;  // 10%
    m_minValue = 0.0f;  // 0%
    m_maxValue = 1.0f;  // 100%

    m_interface = CRobotMain::GetInstancePointer()->GetInterface();
}

// Object's destructor.

CEditValue::~CEditValue()
{
}


// Creates a new button.

bool CEditValue::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();
    CControl::Create(pos, dim, icon, eventType);

    GlintDelete();

    m_edit = MakeUnique<Ui::CEdit>();
    m_edit->Create(pos, dim, 0, EVENT_NULL);
    m_edit->SetMaxChar(4);

    m_buttonUp = MakeUnique<Ui::CButton>();
    m_buttonUp->Create(pos, dim, 49, EVENT_NULL);  // ^
    m_buttonUp->SetRepeat(true);

    m_buttonDown = MakeUnique<Ui::CButton>();
    m_buttonDown->Create(pos, dim, 50, EVENT_NULL);  // v
    m_buttonDown->SetRepeat(true);

    MoveAdjust();
    return true;
}


void CEditValue::SetPos(Math::Point pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CEditValue::SetDim(Math::Point dim)
{
    CControl::SetDim(dim);
    MoveAdjust();
}

void CEditValue::MoveAdjust()
{
    Math::Point     pos, dim;

    if (m_edit != nullptr)
    {
        pos.x = m_pos.x;
        pos.y = m_pos.y;
        dim.x = m_dim.x-m_dim.y*0.6f;
        dim.y = m_dim.y;
        m_edit->SetPos(pos);
        m_edit->SetDim(dim);
    }

    if (m_buttonUp != nullptr)
    {
        pos.x = m_pos.x+m_dim.x-m_dim.y*0.6f;
        pos.y = m_pos.y+m_dim.y*0.5f;
        dim.x = m_dim.y*0.6f;
        dim.y = m_dim.y*0.5f;
        m_buttonUp->SetPos(pos);
        m_buttonUp->SetDim(dim);
    }

    if (m_buttonDown != nullptr)
    {
        pos.x = m_pos.x+m_dim.x-m_dim.y*0.6f;
        pos.y = m_pos.y;
        dim.x = m_dim.y*0.6f;
        dim.y = m_dim.y*0.5f;
        m_buttonDown->SetPos(pos);
        m_buttonDown->SetDim(dim);
    }
}


// Management of an event.

bool CEditValue::EventProcess(const Event &event)
{
    CControl::EventProcess(event);

    if ( (m_state & STATE_VISIBLE) == 0 )  return true;
    if ( (m_state & STATE_ENABLE) == 0 )  return true;
    if ( m_state & STATE_DEAD )  return true;

    if (m_edit != nullptr)
    {
        if ( m_edit->GetFocus()           &&
             event.type == EVENT_KEY_DOWN &&
             event.GetData<KeyEventData>()->key == KEY(RETURN)     )
        {
            float value = GetValue();
            if ( value > m_maxValue )  value = m_maxValue;
            if ( value < m_minValue )  value = m_minValue;
            SetValue(value, true);
            HiliteValue(event);
        }
        if ( !m_edit->EventProcess(event) )  return false;

        if ( event.type == m_edit->GetEventType() )
        {
            m_event->AddEvent(Event(m_eventType));
        }
    }

    if (m_buttonUp != nullptr)
    {
        if ( event.type == m_buttonUp->GetEventType() )
        {
            float value = GetValue()+m_stepValue;
            if ( value > m_maxValue )  value = m_maxValue;
            SetValue(value, true);
            HiliteValue(event);
        }
        if ( !m_buttonUp->EventProcess(event) )  return false;
    }

    if (m_buttonDown != nullptr)
    {
        if ( event.type == m_buttonDown->GetEventType() )
        {
            float value = GetValue()-m_stepValue;
            if ( value < m_minValue )  value = m_minValue;
            SetValue(value, true);
            HiliteValue(event);
        }
        if ( !m_buttonDown->EventProcess(event) )  return false;
    }

    if (event.type == EVENT_MOUSE_WHEEL &&
        Detect(event.mousePos))
    {
        float value = GetValue() + (m_stepValue * event.GetData<MouseWheelEventData>()->y);
        if ( value < m_minValue )  value = m_minValue;
        if ( value > m_maxValue )  value = m_maxValue;
        SetValue(value, true);
        HiliteValue(event);
        return false;
    }

    return true;
}


// Puts in evidence the edited value.

void CEditValue::HiliteValue(const Event &event)
{
    if (m_edit == nullptr)  return;

    std::size_t pos = m_edit->GetTextLength();
    if ( m_type == EVT_100 && pos > 0 )
        pos --;  // unselect the "%" symbol

    m_edit->SetCursor(pos, 0);
    m_interface->SetFocus(m_edit.get());

    Event newEvent = event.Clone();
    newEvent.type = EVENT_FOCUS;
    newEvent.customParam = m_edit->GetEventType();
    m_event->AddEvent(std::move(newEvent));  // defocus the other objects
}


// Draw button.

void CEditValue::Draw()
{
    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    if (m_edit != nullptr)
    {
        m_edit->SetState(STATE_ENABLE, TestState(STATE_ENABLE));
        m_edit->Draw();
    }
    if (m_buttonUp != nullptr)
    {
        m_buttonUp->SetState(STATE_DEAD, TestState(STATE_DEAD));
        m_buttonUp->Draw();
    }
    if (m_buttonDown != nullptr)
    {
        m_buttonDown->SetState(STATE_DEAD, TestState(STATE_DEAD));
        m_buttonDown->Draw();
    }
}


// Choosing the type of value.

void CEditValue::SetType(EditValueType type)
{
    m_type = type;
}

EditValueType CEditValue::GetType()
{
    return m_type;
}


// Changes the value.

void CEditValue::SetValue(float value, bool bSendMessage)
{
    char    text[100];

    if ( m_edit == nullptr )  return;

    text[0] = 0;

    if ( m_type == EVT_INT )
    {
        sprintf(text, "%d", static_cast<int>(value));
    }

    if ( m_type == EVT_FLOAT )
    {
        sprintf(text, "%.2f", value);
    }

    if ( m_type == EVT_100 )
    {
        sprintf(text, "%d%%", static_cast<int>(value*100.0f));
    }

    m_edit->SetText(text);

    if ( bSendMessage )
    {
        m_event->AddEvent(Event(m_eventType));
    }
}

// Return the edited value.

float CEditValue::GetValue()
{
    std::string text;
    float   value = 0.0f;

    if ( m_edit != nullptr )
    {
        text = m_edit->GetText(100);
        sscanf(text.c_str(), "%f", &value);

        if ( m_type == EVT_100 )
        {
            value = (value+0.5f)/100.0f;
            if ( value < 0.01f )  value = 0.0f;  // less than 1%?
        }
    }
    return value;
}


// Management not for buttons.

void CEditValue::SetStepValue(float value)
{
    m_stepValue = value;
}

float CEditValue::GetStepValue()
{
    return m_stepValue;
}


// Management of the minimum value.

void CEditValue::SetMinValue(float value)
{
    m_minValue = value;
}

float CEditValue::GetMinValue()
{
    return m_minValue;
}


// Management of the maximum value.

void CEditValue::SetMaxValue(float value)
{
    m_maxValue = value;
}

float CEditValue::GetMaxValue()
{
    return m_maxValue;
}

void CEditValue::SetInterface(CInterface* interface)
{
    m_interface = interface;
}

}
