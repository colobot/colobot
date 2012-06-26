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

// slider.cpp

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
#include "text.h"
#include "button.h"
#include "slider.h"



#define CURSOR_WIDTH    (10.0f/640.0f)
#define HOLE_WIDTH      (5.0f/480.0f)




// Object's constructor.

CSlider::CSlider(CInstanceManager* iMan) : CControl(iMan)
{
    m_buttonLeft  = 0;
    m_buttonRight = 0;

    m_min          = 0.0f;
    m_max          = 1.0f;
    m_visibleValue = 0.0f;
    m_step         = 0.0f;

    m_marginButton = 0.0f;
    m_bHoriz       = FALSE;

    m_eventUp   = EVENT_NULL;
    m_eventDown = EVENT_NULL;

    m_bCapture = FALSE;
}

// Object's destructor.

CSlider::~CSlider()
{
    delete m_buttonLeft;
    delete m_buttonRight;
}


// Creates a new button.

BOOL CSlider::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();
    CControl::Create(pos, dim, icon, eventMsg);

    MoveAdjust();
    return TRUE;
}


void CSlider::SetPos(FPOINT pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CSlider::SetDim(FPOINT dim)
{
    CControl::SetDim(dim);
    MoveAdjust();
}

void CSlider::MoveAdjust()
{
    FPOINT      pos, dim;

    m_bHoriz = ( m_dim.x > m_dim.y );

    if ( ( m_bHoriz && m_dim.x < m_dim.y*4.0f) ||
         (!m_bHoriz && m_dim.y < m_dim.x*4.0f) )  // very short slider?
    {
        delete m_buttonLeft;
        m_buttonLeft = 0;

        delete m_buttonRight;
        m_buttonRight = 0;

        m_marginButton = 0.0f;
    }
    else
    {
#if 1
        if ( m_buttonLeft == 0 )
        {
            m_buttonLeft = new CButton(m_iMan);
            m_buttonLeft->Create(FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f), m_bHoriz?55:49, EVENT_NULL);  // </^
            m_buttonLeft->SetRepeat(TRUE);
            if ( m_state & STATE_SHADOW )  m_buttonLeft->SetState(STATE_SHADOW);
            m_eventUp = m_buttonLeft->RetEventMsg();
        }

        if ( m_buttonRight == 0 )
        {
            m_buttonRight = new CButton(m_iMan);
            m_buttonRight->Create(FPOINT(0.0f, 0.0f), FPOINT(0.0f, 0.0f), m_bHoriz?48:50, EVENT_NULL);  // >/v
            m_buttonRight->SetRepeat(TRUE);
            if ( m_state & STATE_SHADOW )  m_buttonRight->SetState(STATE_SHADOW);
            m_eventDown = m_buttonRight->RetEventMsg();
        }

        m_marginButton = m_bHoriz?(m_dim.y*0.75f):(m_dim.x/0.75f);
#endif
    }

    if ( m_buttonLeft != 0 )
    {
        if ( m_bHoriz )
        {
            pos.x = m_pos.x;
            pos.y = m_pos.y;
            dim.x = m_dim.y*0.75f;
            dim.y = m_dim.y;
        }
        else
        {
            pos.x = m_pos.x;
            pos.y = m_pos.y+m_dim.y-m_dim.x/0.75f;
            dim.x = m_dim.x;
            dim.y = m_dim.x/0.75f;
        }
        m_buttonLeft->SetPos(pos);
        m_buttonLeft->SetDim(dim);
    }

    if ( m_buttonRight != 0 )
    {
        if ( m_bHoriz )
        {
            pos.x = m_pos.x+m_dim.x-m_dim.y*0.75f;
            pos.y = m_pos.y;
            dim.x = m_dim.y*0.75f;
            dim.y = m_dim.y;
        }
        else
        {
            pos.x = m_pos.x;
            pos.y = m_pos.y;
            dim.x = m_dim.x;
            dim.y = m_dim.x/0.75f;
        }
        m_buttonRight->SetPos(pos);
        m_buttonRight->SetDim(dim);
    }

    AdjustGlint();
}

// Adjusts the position of reflection.

void CSlider::AdjustGlint()
{
    FPOINT  ref;
    float   w;

    if ( m_bHoriz )
    {
        w = m_dim.x-m_marginButton*0.75f;
        ref.x = m_pos.x+m_marginButton;
        ref.x += (w-CURSOR_WIDTH)*m_visibleValue;
        ref.y = m_pos.y+m_dim.y;
    }
    else
    {
        w = m_dim.y-m_marginButton*2.0f;
        ref.y = m_pos.y+m_marginButton+CURSOR_WIDTH;
        ref.y += (w-CURSOR_WIDTH)*m_visibleValue;
        ref.x = m_pos.x;
    }

    GlintCreate(ref);
}


BOOL CSlider::SetState(int state, BOOL bState)
{
    if ( (state & STATE_ENABLE) ||
         (state & STATE_SHADOW) )
    {
        if ( m_buttonLeft  != 0 )  m_buttonLeft->SetState(state, bState);
        if ( m_buttonRight != 0 )  m_buttonRight->SetState(state, bState);
    }

    return CControl::SetState(state, bState);
}

BOOL CSlider::SetState(int state)
{
    if ( (state & STATE_ENABLE) ||
         (state & STATE_SHADOW) )
    {
        if ( m_buttonLeft  != 0 )  m_buttonLeft->SetState(state);
        if ( m_buttonRight != 0 )  m_buttonRight->SetState(state);
    }

    return CControl::SetState(state);
}

BOOL CSlider::ClearState(int state)
{
    if ( (state & STATE_ENABLE) ||
         (state & STATE_SHADOW) )
    {
        if ( m_buttonLeft  != 0 )  m_buttonLeft->ClearState(state);
        if ( m_buttonRight != 0 )  m_buttonRight->ClearState(state);
    }

    return CControl::ClearState(state);
}


// Management of an event.

BOOL CSlider::EventProcess(const Event &event)
{
    FPOINT  pos, dim;
    float   value;

    if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;

    CControl::EventProcess(event);

    if ( m_buttonLeft != 0 && !m_bCapture )
    {
        if ( !m_buttonLeft->EventProcess(event) )  return FALSE;
    }
    if ( m_buttonRight != 0 && !m_bCapture )
    {
        if ( !m_buttonRight->EventProcess(event) )  return FALSE;
    }

    if ( event.event == m_eventUp && m_step > 0.0f )
    {
        m_visibleValue -= m_bHoriz?m_step:-m_step;
        if ( m_visibleValue < 0.0f )  m_visibleValue = 0.0f;
        if ( m_visibleValue > 1.0f )  m_visibleValue = 1.0f;
        AdjustGlint();

        Event newEvent = event;
        newEvent.event = m_eventMsg;
        m_event->AddEvent(newEvent);
    }

    if ( event.event == m_eventDown && m_step > 0.0f )
    {
        m_visibleValue += m_bHoriz?m_step:-m_step;
        if ( m_visibleValue < 0.0f )  m_visibleValue = 0.0f;
        if ( m_visibleValue > 1.0f )  m_visibleValue = 1.0f;
        AdjustGlint();

        Event newEvent = event;
        newEvent.event = m_eventMsg;
        m_event->AddEvent(newEvent);
    }

    if ( event.event == EVENT_LBUTTONDOWN &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.pos) )
        {
            if ( m_bHoriz )
            {
                pos.x = m_pos.x+m_marginButton;
                dim.x = m_dim.x-m_marginButton*2.0f;
                value = (event.pos.x-pos.x-CURSOR_WIDTH/2.0f);
                value /= (dim.x-CURSOR_WIDTH);
            }
            else
            {
                pos.y = m_pos.y+m_marginButton;
                dim.y = m_dim.y-m_marginButton*2.0f;
                value = (event.pos.y-pos.y-CURSOR_WIDTH/2.0f);
                value /= (dim.y-CURSOR_WIDTH);
            }
            if ( value < 0.0f )  value = 0.0f;
            if ( value > 1.0f )  value = 1.0f;
            m_visibleValue = value;
            AdjustGlint();

            Event newEvent = event;
            newEvent.event = m_eventMsg;
            m_event->AddEvent(newEvent);

            m_bCapture = TRUE;
            m_pressPos = event.pos;
            m_pressValue = m_visibleValue;
        }
    }

    if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
    {
        if ( m_bHoriz )
        {
            pos.x = m_pos.x+m_marginButton;
            dim.x = m_dim.x-m_marginButton*2.0f;
            value = (event.pos.x-pos.x-CURSOR_WIDTH/2.0f);
            value /= (dim.x-CURSOR_WIDTH);
        }
        else
        {
            pos.y = m_pos.y+m_marginButton;
            dim.y = m_dim.y-m_marginButton*2.0f;
            value = (event.pos.y-pos.y-CURSOR_WIDTH/2.0f);
            value /= (dim.y-CURSOR_WIDTH);
        }
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

    if ( event.event == EVENT_LBUTTONUP && m_bCapture )
    {
        m_bCapture = FALSE;
    }

    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELUP    &&
         Detect(event.pos)            &&
         m_buttonLeft != 0            )
    {
        Event newEvent = event;
        newEvent.event = m_buttonLeft->RetEventMsg();
        m_event->AddEvent(newEvent);
    }
    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELDOWN  &&
         Detect(event.pos)            &&
         m_buttonRight != 0           )
    {
        Event newEvent = event;
        newEvent.event = m_buttonRight->RetEventMsg();
        m_event->AddEvent(newEvent);
    }

    return TRUE;
}


// Draws button.

void CSlider::Draw()
{
    FPOINT  pos, dim, ppos, ddim, spos;
    int     icon;
    float   h;
    char    text[100];

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_buttonLeft != 0 )
    {
        m_buttonLeft->Draw();
    }

    if ( m_bHoriz )
    {
        pos.x = m_pos.x+m_marginButton;
        pos.y = m_pos.y;
        dim.x = m_dim.x-m_marginButton*2.0f;
        dim.y = m_dim.y;
    }
    else
    {
        pos.x = m_pos.x;
        pos.y = m_pos.y+m_marginButton;
        dim.x = m_dim.x;
        dim.y = m_dim.y-m_marginButton*2.0f;
    }

    // Draws the bottom.
    if ( m_bHoriz )
    {
        ppos.x = pos.x + CURSOR_WIDTH/2.0f;
        ppos.y = pos.y + (dim.y-HOLE_WIDTH)/2.0f;
        ddim.x = dim.x - CURSOR_WIDTH;
        ddim.y = HOLE_WIDTH;
    }
    else
    {
        ppos.x = pos.x + (dim.x-HOLE_WIDTH*0.75f)/2.0f;
        ppos.y = pos.y + CURSOR_WIDTH/2.0f;
        ddim.x = HOLE_WIDTH*0.75f;
        ddim.y = dim.y - CURSOR_WIDTH;
    }

    if ( m_state & STATE_SHADOW )
    {
        spos = ppos;
        spos.x -= 0.005f*0.75f;
        spos.y += 0.005f;
        DrawShadow(spos, ddim);
    }

    if ( m_state & STATE_ENABLE )  icon = 0;
    else                           icon = 1;
    DrawVertex(ppos, ddim, icon);

    // Draws the cabin.
    if ( m_state & STATE_ENABLE )
    {
        if ( m_bHoriz )
        {
            ppos.x = pos.x + (dim.x-CURSOR_WIDTH)*m_visibleValue;
            ppos.y = pos.y;
            ddim.x = CURSOR_WIDTH;
            ddim.y = dim.y;
        }
        else
        {
            ppos.x = pos.x;
            ppos.y = pos.y + (dim.y-CURSOR_WIDTH)*m_visibleValue;
            ddim.x = dim.x;
            ddim.y = CURSOR_WIDTH;
        }
        DrawShadow(ppos, ddim, 0.7f);
        DrawVertex(ppos, ddim, 2);
    }

    if ( m_buttonRight != 0 )
    {
        m_buttonRight->Draw();
    }

    if ( m_bHoriz )
    {
        sprintf(text, "%d", (int)(m_min+m_visibleValue*(m_max-m_min)));
        h = m_engine->RetText()->RetHeight(m_fontSize, m_fontType);
        pos.x = m_pos.x+m_dim.x+(10.0f/640.0f);
        pos.y = m_pos.y+(m_dim.y-h)/2.0f;
        m_engine->RetText()->DrawText(text, pos, m_dim.x, 1, m_fontSize, m_fontStretch, m_fontType, 0);
    }
    else
    {
        if ( m_state & STATE_VALUE )
        {
            pos.x = m_pos.x+m_dim.x+4.0f/640.0f;
            h = m_dim.y-m_marginButton*2.0f;
            pos.y = m_pos.y+m_marginButton-4.0f/480.0f;
            pos.y += (h-CURSOR_WIDTH)*m_visibleValue;
            dim.x = 50.0f/640.0f;
            dim.y = 16.0f/480.0f;
            sprintf(text, "%d", (int)(m_min+(m_visibleValue*(m_max-m_min))));
            m_engine->RetText()->DrawText(text, pos, dim.x, 1, m_fontSize, m_fontStretch, m_fontType, 0);
        }
    }
}

// Draws a rectangle.

void CSlider::DrawVertex(FPOINT pos, FPOINT dim, int icon)
{
    FPOINT      uv1, uv2, corner;
    float       ex, dp;

    if ( icon == 0 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x =   0.0f/256.0f;  // yellow rectangle
        uv1.y =  32.0f/256.0f;
        uv2.x =  32.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        corner.x = 2.0f/640.0f;
        corner.y = 2.0f/480.0f;
        ex = 4.0f/256.0f;
    }
    else if ( icon == 1 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 128.0f/256.0f;  // gray rectangle
        uv1.y =  32.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        corner.x = 2.0f/640.0f;
        corner.y = 2.0f/480.0f;
        ex = 4.0f/256.0f;
    }
    else
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 224.0f/256.0f;  // cursor
        uv1.y =  32.0f/256.0f;
        uv2.x = 256.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        if ( !m_bHoriz )
        {
            uv1.y += 64.0f/256.0f;
            uv2.y += 64.0f/256.0f;
        }
        corner.x = 2.0f/640.0f;
        corner.y = 2.0f/480.0f;
        ex = 4.0f/256.0f;
    }

    dp = 0.5f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    DrawIcon(pos, dim, uv1, uv2, corner, ex);
}


void CSlider::SetLimit(float min, float max)
{
    m_min = min;
    m_max = max;
}

void CSlider::SetVisibleValue(float value)
{
    value = (value-m_min)/(m_max-m_min);
    if ( value < 0.0 )  value = 0.0f;
    if ( value > 1.0 )  value = 1.0f;
    m_visibleValue = value;
    AdjustGlint();
}

float CSlider::RetVisibleValue()
{
    return m_min+m_visibleValue*(m_max-m_min);
}


void CSlider::SetArrowStep(float step)
{
    m_step = step/(m_max-m_min);
}

float CSlider::RetArrowStep()
{
    return m_step*(m_max-m_min);
}


