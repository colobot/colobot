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


#include "ui/slider.h"

#include "common/event.h"
#include "common/misc.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/text.h"

#include "ui/button.h"

#include <stdio.h>


namespace Ui {

const float CURSOR_WIDTH    = (10.0f/640.0f);
const float HOLE_WIDTH      = (5.0f/480.0f);




// Object's constructor.

CSlider::CSlider() : CControl()
{
    m_buttonLeft  = 0;
    m_buttonRight = 0;

    m_min          = 0.0f;
    m_max          = 1.0f;
    m_visibleValue = 0.0f;
    m_step         = 0.0f;

    m_marginButton = 0.0f;
    m_bHoriz       = false;

    m_eventUp   = EVENT_NULL;
    m_eventDown = EVENT_NULL;

    m_bCapture = false;
}

// Object's destructor.

CSlider::~CSlider()
{
    delete m_buttonLeft;
    delete m_buttonRight;
}


// Creates a new button.

bool CSlider::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();
    CControl::Create(pos, dim, icon, eventType);

    MoveAdjust();
    return true;
}


void CSlider::SetPos(Math::Point pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CSlider::SetDim(Math::Point dim)
{
    CControl::SetDim(dim);
    MoveAdjust();
}

void CSlider::MoveAdjust()
{
    Math::Point     pos, dim;

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
            m_buttonLeft = new CButton();
            m_buttonLeft->Create(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), m_bHoriz?55:49, EVENT_NULL);  // </^
            m_buttonLeft->SetRepeat(true);
            if ( m_state & STATE_SHADOW )  m_buttonLeft->SetState(STATE_SHADOW);
            m_eventUp = m_buttonLeft->GetEventType();
        }

        if ( m_buttonRight == 0 )
        {
            m_buttonRight = new CButton();
            m_buttonRight->Create(Math::Point(0.0f, 0.0f), Math::Point(0.0f, 0.0f), m_bHoriz?48:50, EVENT_NULL);  // >/v
            m_buttonRight->SetRepeat(true);
            if ( m_state & STATE_SHADOW )  m_buttonRight->SetState(STATE_SHADOW);
            m_eventDown = m_buttonRight->GetEventType();
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
    Math::Point ref;
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


bool CSlider::SetState(int state, bool bState)
{
    if ( (state & STATE_ENABLE) ||
         (state & STATE_SHADOW) )
    {
        if ( m_buttonLeft  != 0 )  m_buttonLeft->SetState(state, bState);
        if ( m_buttonRight != 0 )  m_buttonRight->SetState(state, bState);
    }

    return CControl::SetState(state, bState);
}

bool CSlider::SetState(int state)
{
    if ( (state & STATE_ENABLE) ||
         (state & STATE_SHADOW) )
    {
        if ( m_buttonLeft  != 0 )  m_buttonLeft->SetState(state);
        if ( m_buttonRight != 0 )  m_buttonRight->SetState(state);
    }

    return CControl::SetState(state);
}

bool CSlider::ClearState(int state)
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

bool CSlider::EventProcess(const Event &event)
{
    Math::Point pos, dim;
    float   value;

    if ( (m_state & STATE_VISIBLE) == 0 )  return true;

    CControl::EventProcess(event);

    if ( m_buttonLeft != 0 && !m_bCapture )
    {
        if ( !m_buttonLeft->EventProcess(event) )  return false;
    }
    if ( m_buttonRight != 0 && !m_bCapture )
    {
        if ( !m_buttonRight->EventProcess(event) )  return false;
    }

    if ( event.type == m_eventUp && m_step > 0.0f )
    {
        m_visibleValue -= m_bHoriz?m_step:-m_step;
        if ( m_visibleValue < 0.0f )  m_visibleValue = 0.0f;
        if ( m_visibleValue > 1.0f )  m_visibleValue = 1.0f;
        AdjustGlint();

        Event newEvent = event;
        newEvent.type = m_eventType;
        m_event->AddEvent(newEvent);
    }

    if ( event.type == m_eventDown && m_step > 0.0f )
    {
        m_visibleValue += m_bHoriz?m_step:-m_step;
        if ( m_visibleValue < 0.0f )  m_visibleValue = 0.0f;
        if ( m_visibleValue > 1.0f )  m_visibleValue = 1.0f;
        AdjustGlint();

        Event newEvent = event;
        newEvent.type = m_eventType;
        m_event->AddEvent(newEvent);
    }

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN &&
         (event.mouseButton.button == MOUSE_BUTTON_LEFT ) &&
         (m_state & STATE_VISIBLE)             &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.mousePos) )
        {
            if ( m_bHoriz )
            {
                pos.x = m_pos.x+m_marginButton;
                dim.x = m_dim.x-m_marginButton*2.0f;
                value = (event.mousePos.x-pos.x-CURSOR_WIDTH/2.0f);
                value /= (dim.x-CURSOR_WIDTH);
            }
            else
            {
                pos.y = m_pos.y+m_marginButton;
                dim.y = m_dim.y-m_marginButton*2.0f;
                value = (event.mousePos.y-pos.y-CURSOR_WIDTH/2.0f);
                value /= (dim.y-CURSOR_WIDTH);
            }
            if ( value < 0.0f )  value = 0.0f;
            if ( value > 1.0f )  value = 1.0f;
            m_visibleValue = value;
            AdjustGlint();

            Event newEvent = event;
            newEvent.type = m_eventType;
            m_event->AddEvent(newEvent);

            m_bCapture = true;
            m_pressPos = event.mousePos;
            m_pressValue = m_visibleValue;
        }
    }

    if ( event.type == EVENT_MOUSE_MOVE && m_bCapture )
    {
        if ( m_bHoriz )
        {
            pos.x = m_pos.x+m_marginButton;
            dim.x = m_dim.x-m_marginButton*2.0f;
            value = (event.mousePos.x-pos.x-CURSOR_WIDTH/2.0f);
            value /= (dim.x-CURSOR_WIDTH);
        }
        else
        {
            pos.y = m_pos.y+m_marginButton;
            dim.y = m_dim.y-m_marginButton*2.0f;
            value = (event.mousePos.y-pos.y-CURSOR_WIDTH/2.0f);
            value /= (dim.y-CURSOR_WIDTH);
        }
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

    if ( ( event.type == EVENT_MOUSE_BUTTON_UP ) &&
           ( event.mouseButton.button == MOUSE_BUTTON_LEFT )     &&
            m_bCapture )
    {
        m_bCapture = false;
    }

    if (event.type == EVENT_MOUSE_WHEEL &&
        event.mouseWheel.dir == WHEEL_UP &&
        Detect(event.mousePos) &&
        m_buttonLeft != 0)
    {
        Event newEvent = event;
        newEvent.type = m_buttonLeft->GetEventType();
        m_event->AddEvent(newEvent);
    }

    if (event.type == EVENT_MOUSE_WHEEL &&
        event.mouseWheel.dir == WHEEL_DOWN &&
        Detect(event.mousePos) &&
        m_buttonRight != 0)
    {
        Event newEvent = event;
        newEvent.type = m_buttonRight->GetEventType();
        m_event->AddEvent(newEvent);
    }

    return true;
}


// Draws button.

void CSlider::Draw()
{
    Math::Point pos, dim, ppos, ddim, spos;
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
        sprintf(text, "%d", static_cast<int>(m_min+m_visibleValue*(m_max-m_min)));
        h = m_engine->GetText()->GetHeight(m_fontType, m_fontSize);
        pos.x = m_pos.x+m_dim.x+(10.0f/640.0f);
        pos.y = m_pos.y+(m_dim.y-h)/2.0f;
        m_engine->GetText()->DrawText(text, m_fontType, m_fontSize, pos, m_dim.x, Gfx::TEXT_ALIGN_LEFT, 0);
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
            sprintf(text, "%d", static_cast<int>(m_min+(m_visibleValue*(m_max-m_min))));
            m_engine->GetText()->DrawText(text, m_fontType, m_fontSize, pos, dim.x, Gfx::TEXT_ALIGN_RIGHT, 0);
        }
    }
}

// Draws a rectangle.

void CSlider::DrawVertex(Math::Point pos, Math::Point dim, int icon)
{
    Math::Point     uv1, uv2, corner;
    float       ex, dp;

    if ( icon == 0 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
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
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
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
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
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

float CSlider::GetVisibleValue()
{
    return m_min+m_visibleValue*(m_max-m_min);
}


void CSlider::SetArrowStep(float step)
{
    m_step = step/(m_max-m_min);
}

float CSlider::GetArrowStep()
{
    return m_step*(m_max-m_min);
}


}
