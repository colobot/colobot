// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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


#include "ui/editvalue.h"

#include "common/event.h"
#include "common/misc.h"

#include "ui/edit.h"
#include "ui/button.h"




namespace Ui {
// Object's constructor.

CEditValue::CEditValue() : CControl ()
{
    m_edit       = 0;
    m_buttonUp   = 0;
    m_buttonDown = 0;

    m_type = EVT_100;  // %
    m_stepValue = 0.1f;  // 10%
    m_minValue = 0.0f;  // 0%
    m_maxValue = 1.0f;  // 100%
}

// Object's destructor.

CEditValue::~CEditValue()
{
    delete m_edit;
    delete m_buttonUp;
    delete m_buttonDown;
}


// Creates a new button.

bool CEditValue::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    Ui::CEdit*      pe;
    Ui::CButton*    pc;

    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();
    CControl::Create(pos, dim, icon, eventType);

    GlintDelete();

    m_edit = new Ui::CEdit();
    pe = static_cast<Ui::CEdit*>(m_edit);
    pe->Create(pos, dim, 0, EVENT_NULL);
    pe->SetMaxChar(4);

    m_buttonUp = new Ui::CButton();
    pc = static_cast<Ui::CButton*>(m_buttonUp);
    pc->Create(pos, dim, 49, EVENT_NULL);  // ^
    pc->SetRepeat(true);

    m_buttonDown = new Ui::CButton();
    pc = static_cast<Ui::CButton*>(m_buttonDown);
    pc->Create(pos, dim, 50, EVENT_NULL);  // v
    pc->SetRepeat(true);

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

    if ( m_edit != 0 )
    {
        pos.x = m_pos.x;
        pos.y = m_pos.y;
        dim.x = m_dim.x-m_dim.y*0.6f;
        dim.y = m_dim.y;
        m_edit->SetPos(pos);
        m_edit->SetDim(dim);
    }

    if ( m_buttonUp != 0 )
    {
        pos.x = m_pos.x+m_dim.x-m_dim.y*0.6f;
        pos.y = m_pos.y+m_dim.y*0.5f;
        dim.x = m_dim.y*0.6f;
        dim.y = m_dim.y*0.5f;
        m_buttonUp->SetPos(pos);
        m_buttonUp->SetDim(dim);
    }

    if ( m_buttonDown != 0 )
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
    float   value;

    CControl::EventProcess(event);

    if ( (m_state & STATE_VISIBLE) == 0 )  return true;
    if ( (m_state & STATE_ENABLE) == 0 )  return true;

    if ( m_edit != 0 )
    {
        if ( m_edit->GetFocus()           &&
             event.type == EVENT_KEY_DOWN &&
             event.key.key == KEY(RETURN)     )
        {
            value = GetValue();
            if ( value > m_maxValue )  value = m_maxValue;
            if ( value < m_minValue )  value = m_minValue;
            SetValue(value, true);
            HiliteValue(event);
        }
        if ( !m_edit->EventProcess(event) )  return false;

        if ( event.type == m_edit->GetEventType() )
        {
            Event       newEvent(m_eventType);
            m_event->AddEvent(newEvent);
        }
    }

    if ( m_buttonUp != 0 )
    {
        if ( event.type == m_buttonUp->GetEventType() )
        {
            value = GetValue()+m_stepValue;
            if ( value > m_maxValue )  value = m_maxValue;
            SetValue(value, true);
            HiliteValue(event);
        }
        if ( !m_buttonUp->EventProcess(event) )  return false;
    }

    if ( m_buttonDown != 0 )
    {
        if ( event.type == m_buttonDown->GetEventType() )
        {
            value = GetValue()-m_stepValue;
            if ( value < m_minValue )  value = m_minValue;
            SetValue(value, true);
            HiliteValue(event);
        }
        if ( !m_buttonDown->EventProcess(event) )  return false;
    }

    if (event.type == EVENT_MOUSE_WHEEL &&
        event.mouseWheel.dir == WHEEL_UP &&
        Detect(event.mousePos))
    {
        value = GetValue()+m_stepValue;
        if ( value > m_maxValue )  value = m_maxValue;
        SetValue(value, true);
        HiliteValue(event);
    }
    if ( event.type == EVENT_KEY_DOWN &&
         event.mouseWheel.dir == WHEEL_DOWN &&
         Detect(event.mousePos))
    {
        value = GetValue()-m_stepValue;
        if ( value < m_minValue )  value = m_minValue;
        SetValue(value, true);
        HiliteValue(event);
    }

    return true;
}


// Puts in evidence the edited value.

void CEditValue::HiliteValue(const Event &event)
{
    int     pos;

    if ( m_edit == 0 )  return;

    pos = m_edit->GetTextLength();
    if ( m_type == EVT_100 && pos > 0 )
    {
        pos --;  // not only selects the "%"
    }

    m_edit->SetCursor(pos, 0);
    m_edit->SetFocus(true);

    Event newEvent = event;
    newEvent.type = EVENT_FOCUS;
    newEvent.customParam = m_edit->GetEventType();
    m_event->AddEvent(newEvent);  // defocus the other objects
}


// Draw button.

void CEditValue::Draw()
{
    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    if ( m_edit != 0 )
    {
        m_edit->Draw();
    }
    if ( m_buttonUp != 0 )
    {
        m_buttonUp->Draw();
    }
    if ( m_buttonDown != 0 )
    {
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

    if ( m_edit == 0 )  return;

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
        Event       newEvent(m_eventType);
        m_event->AddEvent(newEvent);
    }
}

// Return the edited value.

float CEditValue::GetValue()
{
    char    text[100];
    float   value = 0.0f;

    if ( m_edit != 0 )
    {
        m_edit->GetText(text, 100);
        sscanf(text, "%f", &value);

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

}

