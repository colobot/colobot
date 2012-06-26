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

// scroll.cpp

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
#include "button.h"
#include "scroll.h"




// Object's constructor.

CScroll::CScroll(CInstanceManager* iMan) : CControl(iMan)
{
    m_buttonUp   = 0;
    m_buttonDown = 0;

    m_visibleValue = 0.0f;
    m_visibleRatio = 1.0f;
    m_step         = 0.0f;

    m_eventUp   = EVENT_NULL;
    m_eventDown = EVENT_NULL;

    m_bCapture = FALSE;
}

// Object's destructor.

CScroll::~CScroll()
{
    delete m_buttonUp;
    delete m_buttonDown;
}


// Creates a new button.

BOOL CScroll::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();
    CControl::Create(pos, dim, icon, eventMsg);

    MoveAdjust();
    return TRUE;
}


void CScroll::SetPos(FPOINT pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CScroll::SetDim(FPOINT dim)
{
    CControl::SetDim(dim);
    MoveAdjust();
}

// Adjust both buttons.

void CScroll::MoveAdjust()
{
    CButton*    pc;
    FPOINT      pos, dim;

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
            m_buttonUp = new CButton(m_iMan);
            pc = (CButton*)m_buttonUp;
            pc->Create(FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f), 49, EVENT_NULL);
            pc->SetRepeat(TRUE);
            m_eventUp = pc->RetEventMsg();
        }

        if ( m_buttonDown == 0 )
        {
            m_buttonDown = new CButton(m_iMan);
            pc = (CButton*)m_buttonDown;
            pc->Create(FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f), 50, EVENT_NULL);
            pc->SetRepeat(TRUE);
            m_eventDown = pc->RetEventMsg();
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
    FPOINT  ref;
    float   hButton, h;

    hButton = m_buttonUp?m_dim.x/0.75f:0.0f;
    h = m_dim.y-hButton*2.0f;

    ref.x = m_pos.x;
    ref.y = m_pos.y+hButton+h*m_visibleRatio+0.003f;
    ref.y += h*(1.0f-m_visibleRatio)*(1.0f-m_visibleValue);

    GlintCreate(ref);
}



BOOL CScroll::SetState(int state, BOOL bState)
{
    if ( state & STATE_ENABLE )
    {
        if ( m_buttonUp   != 0 )  m_buttonUp->SetState(state, bState);
        if ( m_buttonDown != 0 )  m_buttonDown->SetState(state, bState);
    }

    return CControl::SetState(state, bState);
}

BOOL CScroll::SetState(int state)
{
    if ( state & STATE_ENABLE )
    {
        if ( m_buttonUp   != 0 )  m_buttonUp->SetState(state);
        if ( m_buttonDown != 0 )  m_buttonDown->SetState(state);
    }

    return CControl::SetState(state);
}

BOOL CScroll::ClearState(int state)
{
    if ( state & STATE_ENABLE )
    {
        if ( m_buttonUp   != 0 )  m_buttonUp->ClearState(state);
        if ( m_buttonDown != 0 )  m_buttonDown->ClearState(state);
    }

    return CControl::ClearState(state);
}


// Management of an event.

BOOL CScroll::EventProcess(const Event &event)
{
    FPOINT  pos, dim;
    float   hButton, h, value;

    CControl::EventProcess(event);

    if ( m_buttonUp != 0 && !m_bCapture )
    {
        if ( !m_buttonUp->EventProcess(event) )  return FALSE;
    }
    if ( m_buttonDown != 0 && !m_bCapture )
    {
        if ( !m_buttonDown->EventProcess(event) )  return FALSE;
    }

    if ( event.event == m_eventUp && m_step > 0.0f )
    {
        m_visibleValue -= m_step;
        if ( m_visibleValue < 0.0f )  m_visibleValue = 0.0f;
        AdjustGlint();

        Event newEvent = event;
        newEvent.event = m_eventMsg;
        m_event->AddEvent(newEvent);
    }

    if ( event.event == m_eventDown && m_step > 0.0f )
    {
        m_visibleValue += m_step;
        if ( m_visibleValue > 1.0f )  m_visibleValue = 1.0f;
        AdjustGlint();

        Event newEvent = event;
        newEvent.event = m_eventMsg;
        m_event->AddEvent(newEvent);
    }

    hButton = m_buttonUp?m_dim.x/0.75f:0.0f;

    if ( event.event == EVENT_LBUTTONDOWN &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.pos) )
        {
            pos.y = m_pos.y+hButton;
            dim.y = m_dim.y-hButton*2.0f;
            pos.y += dim.y*(1.0f-m_visibleRatio)*(1.0f-m_visibleValue);
            dim.y *= m_visibleRatio;
            if ( event.pos.y < pos.y       ||
                 event.pos.y > pos.y+dim.y )  // click outside cabin?
            {
                h = (m_dim.y-hButton*2.0f)*(1.0f-m_visibleRatio);
                value = 1.0f-(event.pos.y-(m_pos.y+hButton+dim.y*0.5f))/h;
                if ( value < 0.0f )  value = 0.0f;
                if ( value > 1.0f )  value = 1.0f;
                m_visibleValue = value;
                AdjustGlint();

                Event newEvent = event;
                newEvent.event = m_eventMsg;
                m_event->AddEvent(newEvent);
            }
            m_bCapture = TRUE;
            m_pressPos = event.pos;
            m_pressValue = m_visibleValue;
        }
    }

    if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
    {
        h = (m_dim.y-hButton*2.0f)*(1.0f-m_visibleRatio);
        if ( h != 0 )
        {
            value = m_pressValue - (event.pos.y-m_pressPos.y)/h;
            if ( value < 0.0f )  value = 0.0f;
            if ( value > 1.0f )  value = 1.0f;

            if ( value != m_visibleValue )
            {
                m_visibleValue = value;
                AdjustGlint();

                Event newEvent = event;
                newEvent.event = m_eventMsg;
                m_event->AddEvent(newEvent);
            }
        }
    }

    if ( event.event == EVENT_LBUTTONUP && m_bCapture )
    {
        m_bCapture = FALSE;
    }

    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELUP    &&
         Detect(event.pos)            &&
         m_buttonUp != 0              )
    {
        Event newEvent = event;
        newEvent.event = m_buttonUp->RetEventMsg();
        m_event->AddEvent(newEvent);
    }
    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELDOWN  &&
         Detect(event.pos)            &&
         m_buttonDown != 0            )
    {
        Event newEvent = event;
        newEvent.event = m_buttonDown->RetEventMsg();
        m_event->AddEvent(newEvent);
    }

    return TRUE;
}


// Draws the button.

void CScroll::Draw()
{
    FPOINT  pos, dim, ppos, ddim;
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

        n = (int)(dim.y*0.8f/0.012f);
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

void CScroll::DrawVertex(FPOINT pos, FPOINT dim, int icon)
{
    FPOINT      uv1, uv2;
    float       ex, dp;

    if ( icon == 0 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x =   0.0f/256.0f;  // yellow rectangle
        uv1.y =  32.0f/256.0f;
        uv2.x =  32.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        ex = 8.0f/256.0f;
    }
    else if ( icon == 1 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 128.0f/256.0f;  // gray rectangle
        uv1.y =  32.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        ex = 8.0f/256.0f;
    }
    else if ( icon == 2 )
    {
        m_engine->SetTexture("button1.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x =  64.0f/256.0f;  // blue rectangle
        uv1.y =   0.0f/256.0f;
        uv2.x =  96.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        ex = 8.0f/256.0f;
    }
    else
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);
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

float CScroll::RetVisibleValue()
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

float CScroll::RetVisibleRatio()
{
    return m_visibleRatio;
}


void CScroll::SetArrowStep(float step)
{
    m_step = step;
}

float CScroll::RetArrowStep()
{
    return m_step;
}

