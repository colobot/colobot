// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// editvalue.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "edit.h"
#include "button.h"
#include "editvalue.h"




// Object's constructor.

CEditValue::CEditValue(CInstanceManager* iMan) : CControl(iMan)
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

BOOL CEditValue::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CEdit*      pe;
    CButton*    pc;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();
    CControl::Create(pos, dim, icon, eventMsg);

    GlintDelete();

    m_edit = new CEdit(m_iMan);
    pe = (CEdit*)m_edit;
    pe->Create(pos, dim, 0, EVENT_NULL);
    pe->SetMaxChar(4);

    m_buttonUp = new CButton(m_iMan);
    pc = (CButton*)m_buttonUp;
    pc->Create(pos, dim, 49, EVENT_NULL);  // ^
    pc->SetRepeat(TRUE);

    m_buttonDown = new CButton(m_iMan);
    pc = (CButton*)m_buttonDown;
    pc->Create(pos, dim, 50, EVENT_NULL);  // v
    pc->SetRepeat(TRUE);

    MoveAdjust();
    return TRUE;
}


void CEditValue::SetPos(FPOINT pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CEditValue::SetDim(FPOINT dim)
{
    CControl::SetDim(dim);
    MoveAdjust();
}

void CEditValue::MoveAdjust()
{
    FPOINT      pos, dim;

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

BOOL CEditValue::EventProcess(const Event &event)
{
    float   value;

    CControl::EventProcess(event);

    if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
    if ( (m_state & STATE_ENABLE) == 0 )  return TRUE;

    if ( m_edit != 0 )
    {
        if ( m_edit->RetFocus()           &&
             event.event == EVENT_KEYDOWN &&
             event.param == VK_RETURN     )
        {
            value = RetValue();
            if ( value > m_maxValue )  value = m_maxValue;
            if ( value < m_minValue )  value = m_minValue;
            SetValue(value, TRUE);
            HiliteValue(event);
        }
        if ( !m_edit->EventProcess(event) )  return FALSE;

        if ( event.event == m_edit->RetEventMsg() )
        {
            Event       newEvent;
            m_event->MakeEvent(newEvent, m_eventMsg);
            m_event->AddEvent(newEvent);
        }
    }

    if ( m_buttonUp != 0 )
    {
        if ( event.event == m_buttonUp->RetEventMsg() )
        {
            value = RetValue()+m_stepValue;
            if ( value > m_maxValue )  value = m_maxValue;
            SetValue(value, TRUE);
            HiliteValue(event);
        }
        if ( !m_buttonUp->EventProcess(event) )  return FALSE;
    }

    if ( m_buttonDown != 0 )
    {
        if ( event.event == m_buttonDown->RetEventMsg() )
        {
            value = RetValue()-m_stepValue;
            if ( value < m_minValue )  value = m_minValue;
            SetValue(value, TRUE);
            HiliteValue(event);
        }
        if ( !m_buttonDown->EventProcess(event) )  return FALSE;
    }

    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELUP    &&
         Detect(event.pos)            )
    {
        value = RetValue()+m_stepValue;
        if ( value > m_maxValue )  value = m_maxValue;
        SetValue(value, TRUE);
        HiliteValue(event);
    }
    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELDOWN  &&
         Detect(event.pos)            )
    {
        value = RetValue()-m_stepValue;
        if ( value < m_minValue )  value = m_minValue;
        SetValue(value, TRUE);
        HiliteValue(event);
    }

    return TRUE;
}


// Puts in evidence the edited value.

void CEditValue::HiliteValue(const Event &event)
{
    int     pos;

    if ( m_edit == 0 )  return;

    pos = m_edit->RetTextLength();
    if ( m_type == EVT_100 && pos > 0 )
    {
        pos --;  // not only selects the "%"
    }

    m_edit->SetCursor(pos, 0);
    m_edit->SetFocus(TRUE);

    Event newEvent = event;
    newEvent.event = EVENT_FOCUS;
    newEvent.param = m_edit->RetEventMsg();
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

EditValueType CEditValue::RetType()
{
    return m_type;
}


// Changes the value.

void CEditValue::SetValue(float value, BOOL bSendMessage)
{
    char    text[100];

    if ( m_edit == 0 )  return;

    text[0] = 0;

    if ( m_type == EVT_INT )
    {
        sprintf(text, "%d", (int)value);
    }

    if ( m_type == EVT_FLOAT )
    {
        sprintf(text, "%.2f", value);
    }

    if ( m_type == EVT_100 )
    {
        sprintf(text, "%d%%", (int)(value*100.0f));
    }

    m_edit->SetText(text);

    if ( bSendMessage )
    {
        Event       newEvent;
        m_event->MakeEvent(newEvent, m_eventMsg);
        m_event->AddEvent(newEvent);
    }
}

// Return the edited value.

float CEditValue::RetValue()
{
    char    text[100];
    float   value;

    if ( m_edit == 0 )  0.0f;

    m_edit->GetText(text, 100);
    sscanf(text, "%f", &value);

    if ( m_type == EVT_100 )
    {
        value = (value+0.5f)/100.0f;
        if ( value < 0.01f )  value = 0.0f;  // less than 1%?
    }

    return value;
}


// Management not for buttons.

void CEditValue::SetStepValue(float value)
{
    m_stepValue = value;
}

float CEditValue::RetStepValue()
{
    return m_stepValue;
}


// Management of the minimum value.

void CEditValue::SetMinValue(float value)
{
    m_minValue = value;
}

float CEditValue::RetMinValue()
{
    return m_minValue;
}


// Management of the maximum value.

void CEditValue::SetMaxValue(float value)
{
    m_maxValue = value;
}

float CEditValue::RetMaxValue()
{
    return m_maxValue;
}

