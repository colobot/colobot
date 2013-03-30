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


#include "ui/scroll.h"

#include "common/event.h"
#include "common/misc.h"

#include "graphics/engine/engine.h"

#include "ui/button.h"



namespace Ui {

// Object's constructor.

CScroll::CScroll() : CControl()
{
    m_buttonUp   = 0;
    m_buttonDown = 0;

    m_visibleValue = 0.0f;
    m_visibleRatio = 1.0f;
    m_step         = 0.0f;

    m_eventUp   = EVENT_NULL;
    m_eventDown = EVENT_NULL;

    m_bCapture = false;
}

// Object's destructor.

CScroll::~CScroll()
{
    delete m_buttonUp;
    delete m_buttonDown;
}


// Creates a new button.

bool CScroll::Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventType();
    CControl::Create(pos, dim, icon, eventMsg);

    MoveAdjust();
    return true;
}


void CScroll::SetPos(Math::Point pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CScroll::SetDim(Math::Point dim)
{
    CControl::SetDim(dim);
    MoveAdjust();
}

// Adjust both buttons.

void CScroll::MoveAdjust()
{
    CButton*    pc;
    Math::Point     pos, dim;

    if ( m_dim.y < m_dim.x*2.0f )  // very short lift?
    {
        delete m_buttonUp;
        m_buttonUp = 0;

        delete m_buttonDown;
        m_buttonDown = 0;
    }
    else
    {
        if ( m_buttonUp == 0 )
        {
            m_buttonUp = new CButton();
            pc = m_buttonUp;
            pc->Create(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), 49, EVENT_NULL);
            pc->SetRepeat(true);
            m_eventUp = pc->GetEventType();
        }

        if ( m_buttonDown == 0 )
        {
            m_buttonDown = new CButton();
            pc = m_buttonDown;
            pc->Create(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), 50, EVENT_NULL);
            pc->SetRepeat(true);
            m_eventDown = pc->GetEventType();
        }
    }

    if ( m_buttonUp != 0 )
    {
        pos.x = m_pos.x;
        pos.y = m_pos.y+m_dim.y-m_dim.x/0.75f;
        dim.x = m_dim.x;
        dim.y = m_dim.x/0.75f;
        m_buttonUp->SetPos(pos);
        m_buttonUp->SetDim(dim);
    }

    if ( m_buttonDown != 0 )
    {
        pos.x = m_pos.x;
        pos.y = m_pos.y;
        dim.x = m_dim.x;
        dim.y = m_dim.x/0.75f;
        m_buttonDown->SetPos(pos);
        m_buttonDown->SetDim(dim);
    }

    AdjustGlint();
}

// Adjusts the position of reflection.

void CScroll::AdjustGlint()
{
    Math::Point ref;
    float   hButton, h;

    hButton = m_buttonUp?m_dim.x/0.75f:0.0f;
    h = m_dim.y-hButton*2.0f;

    ref.x = m_pos.x;
    ref.y = m_pos.y+hButton+h*m_visibleRatio+0.003f;
    ref.y += h*(1.0f-m_visibleRatio)*(1.0f-m_visibleValue);

    GlintCreate(ref);
}



bool CScroll::SetState(int state, bool bState)
{
    if ( state & STATE_ENABLE )
    {
        if ( m_buttonUp   != 0 )  m_buttonUp->SetState(state, bState);
        if ( m_buttonDown != 0 )  m_buttonDown->SetState(state, bState);
    }

    return CControl::SetState(state, bState);
}

bool CScroll::SetState(int state)
{
    if ( state & STATE_ENABLE )
    {
        if ( m_buttonUp   != 0 )  m_buttonUp->SetState(state);
        if ( m_buttonDown != 0 )  m_buttonDown->SetState(state);
    }

    return CControl::SetState(state);
}

bool CScroll::ClearState(int state)
{
    if ( state & STATE_ENABLE )
    {
        if ( m_buttonUp   != 0 )  m_buttonUp->ClearState(state);
        if ( m_buttonDown != 0 )  m_buttonDown->ClearState(state);
    }

    return CControl::ClearState(state);
}


// Management of an event.

bool CScroll::EventProcess(const Event &event)
{
    Math::Point pos, dim;
    float   hButton, h, value;

    CControl::EventProcess(event);

    if ( m_buttonUp != 0 && !m_bCapture )
    {
        if ( !m_buttonUp->EventProcess(event) )  return false;
    }
    if ( m_buttonDown != 0 && !m_bCapture )
    {
        if ( !m_buttonDown->EventProcess(event) )  return false;
    }

    if ( event.type == m_eventUp && m_step > 0.0f )
    {
        m_visibleValue -= m_step;
        if ( m_visibleValue < 0.0f )  m_visibleValue = 0.0f;
        AdjustGlint();

        Event newEvent = event;
        newEvent.type = m_eventType;
        m_event->AddEvent(newEvent);
    }

    if ( event.type == m_eventDown && m_step > 0.0f )
    {
        m_visibleValue += m_step;
        if ( m_visibleValue > 1.0f )  m_visibleValue = 1.0f;
        AdjustGlint();

        Event newEvent = event;
        newEvent.type = m_eventType;
        m_event->AddEvent(newEvent);
    }

    hButton = m_buttonUp?m_dim.x/0.75f:0.0f;

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN &&
            event.mouseButton.button == MOUSE_BUTTON_LEFT &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.mousePos) )
        {
            pos.y = m_pos.y+hButton;
            dim.y = m_dim.y-hButton*2.0f;
            pos.y += dim.y*(1.0f-m_visibleRatio)*(1.0f-m_visibleValue);
            dim.y *= m_visibleRatio;
            if ( event.mousePos.y < pos.y       ||
                 event.mousePos.y > pos.y+dim.y )  // click outside cabin?
            {
                h = (m_dim.y-hButton*2.0f)*(1.0f-m_visibleRatio);
                value = 1.0f-(event.mousePos.y-(m_pos.y+hButton+dim.y*0.5f))/h;
                if ( value < 0.0f )  value = 0.0f;
                if ( value > 1.0f )  value = 1.0f;
                m_visibleValue = value;
                AdjustGlint();

                Event newEvent = event;
                newEvent.type = m_eventType;
                m_event->AddEvent(newEvent);
            }
            m_bCapture = true;
            m_pressPos = event.mousePos;
            m_pressValue = m_visibleValue;
        }
    }

    if ( event.type == EVENT_MOUSE_MOVE && m_bCapture )
    {
        h = (m_dim.y-hButton*2.0f)*(1.0f-m_visibleRatio);
        if ( h != 0 )
        {
            value = m_pressValue - (event.mousePos.y-m_pressPos.y)/h;
            if ( value < 0.0f )  value = 0.0f;
            if ( value > 1.0f )  value = 1.0f;

            if ( value != m_visibleValue )
            {
                m_visibleValue = value;
                AdjustGlint();

                Event newEvent = event;
                newEvent.type = m_eventType;
                m_event->AddEvent(newEvent);
            }
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_UP &&
            event.mouseButton.button == MOUSE_BUTTON_LEFT &&
            m_bCapture )
    {
        m_bCapture = false;
    }

    if (event.type == EVENT_MOUSE_WHEEL &&
        event.mouseWheel.dir == WHEEL_UP &&
        Detect(event.mousePos) &&
        m_buttonUp != 0)
    {
        Event newEvent = event;
        newEvent.type = m_buttonUp->GetEventType();
        m_event->AddEvent(newEvent);
    }
    if (event.type == EVENT_MOUSE_WHEEL &&
        event.mouseWheel.dir == WHEEL_DOWN &&
        Detect(event.mousePos) &&
        m_buttonDown != 0)
    {
        Event newEvent = event;
        newEvent.type = m_buttonDown->GetEventType();
        m_event->AddEvent(newEvent);
    }

    return true;
}


// Draws the button.

void CScroll::Draw()
{
    Math::Point pos, dim, ppos, ddim;
    float   hButton;
    int     icon, n, i;

    hButton = m_buttonUp?m_dim.x/0.75f:0.0f;

    // Draws the bottom.
    pos.x = m_pos.x;
    pos.y = m_pos.y+hButton;
    dim.x = m_dim.x;
    dim.y = m_dim.y-hButton*2.0f;
    if ( m_state & STATE_ENABLE )  icon = 0;
    else                           icon = 1;
    DrawVertex(pos, dim, icon);

    // Draws the cabin.
    if ( m_visibleRatio < 1.0f && (m_state & STATE_ENABLE) )
    {
        pos.x += 0.003f;  // ch'tite(?) margin
        pos.y += 0.003f;
        dim.x -= 0.006f;
        dim.y -= 0.006f;
        pos.y += dim.y*(1.0f-m_visibleRatio)*(1.0f-m_visibleValue);
        dim.y *= m_visibleRatio;
        DrawVertex(pos, dim, 2);

        n = static_cast<int>(dim.y*0.8f/0.012f);
        if ( n < 1 )  n = 1;
        if ( n > 5 )  n = 5;

        ppos.x = pos.x+0.003f;
        ppos.y = pos.y+(dim.y-(n-1)*0.012f-0.008f)/2.0f;
        ddim.x = dim.x-0.006f;
        ddim.y = 0.008f;
        for ( i=0 ; i<n ; i++ )
        {
            DrawVertex(ppos, ddim, 3);  // horizontal bar
            ppos.y += 0.012f;
        }
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

// Draws a rectangle.

void CScroll::DrawVertex(Math::Point pos, Math::Point dim, int icon)
{
    Math::Point     uv1, uv2;
    float       ex, dp;

    if ( icon == 0 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =   0.0f/256.0f;  // yellow rectangle
        uv1.y =  32.0f/256.0f;
        uv2.x =  32.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        ex = 8.0f/256.0f;
    }
    else if ( icon == 1 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x = 128.0f/256.0f;  // gray rectangle
        uv1.y =  32.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        ex = 8.0f/256.0f;
    }
    else if ( icon == 2 )
    {
        m_engine->SetTexture("button1.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =  64.0f/256.0f;  // blue rectangle
        uv1.y =   0.0f/256.0f;
        uv2.x =  96.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        ex = 8.0f/256.0f;
    }
    else
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x = 104.0f/256.0f;  // blue line -
        uv1.y =  32.0f/256.0f;
        uv2.x = 128.0f/256.0f;
        uv2.y =  40.0f/256.0f;
        ex = 0.0f;
    }

    dp = 0.5f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    DrawIcon(pos, dim, uv1, uv2, ex);
}


void CScroll::SetVisibleValue(float value)
{
    if ( value < 0.0 )  value = 0.0f;
    if ( value > 1.0 )  value = 1.0f;
    m_visibleValue = value;
    AdjustGlint();
}

float CScroll::GetVisibleValue()
{
    return m_visibleValue;
}


void CScroll::SetVisibleRatio(float value)
{
    if ( value < 0.1 )  value = 0.1f;
    if ( value > 1.0 )  value = 1.0f;
    m_visibleRatio = value;
    AdjustGlint();
}

float CScroll::GetVisibleRatio()
{
    return m_visibleRatio;
}


void CScroll::SetArrowStep(float step)
{
    m_step = step;
}

float CScroll::GetArrowStep()
{
    return m_step;
}

}
