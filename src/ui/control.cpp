// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
//
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


#include "ui/control.h"

#include "app/app.h"


namespace Ui {
// Object's constructor.
CControl::CControl()
{
    m_event       = CApplication::GetInstancePointer()->GetEventQueue();
    m_sound       = CApplication::GetInstancePointer()->GetSound();
    m_engine      = Gfx::CEngine::GetInstancePointer();
    m_main        = CRobotMain::GetInstancePointer();
    m_particle    = m_engine->GetParticle();
    m_eventType   = EVENT_NULL;
    m_state       = STATE_ENABLE|STATE_VISIBLE|STATE_GLINT;
    m_fontSize    = Gfx::FONT_SIZE_SMALL;
    m_fontType    = Gfx::FONT_COLOBOT;
    m_textAlign   = Gfx::TEXT_ALIGN_CENTER; //instead m_justify
    m_bFocus      = false;
    m_bCapture    = false;

    m_bGlint        = false;
    m_glintCorner1  = Math::Point(0.0f, 0.0f);
    m_glintCorner2  = Math::Point(0.0f, 0.0f);
    m_glintProgress = 999.0f;
    m_glintMouse    = Math::Point(0.0f, 0.0f);
}

// Object's destructor.

CControl::~CControl()
{
}


// Creates a new button.
//  pos: [0..1]

bool CControl::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    char text[100];
    std::string str_text;

    if ( eventType == EVENT_NULL )
        eventType = GetUniqueEventType();

    m_pos = pos;
    m_dim = dim;
    m_icon = icon;
    m_eventType = eventType;

    pos.x = m_pos.x;
    pos.y = m_pos.y + m_dim.y;
    GlintCreate(pos);

    GetResource(RES_EVENT, m_eventType, text);
    str_text = std::string(text);
    auto p = str_text.find("\\");
    if ( p == std::string::npos )
    {
        if ( icon != -1 )
            m_tooltip = str_text;
    }
    else
    {
        m_tooltip = str_text.substr(p + 1);
    }

    return true;
}


void CControl::SetPos(Math::Point pos)
{
    m_pos = pos;

    pos.x = m_pos.x;
    pos.y = m_pos.y + m_dim.y;
    GlintCreate(pos);
}

Math::Point CControl::GetPos()
{
    return m_pos;
}

void CControl::SetDim(Math::Point dim)
{
    Math::Point pos;

    m_dim = dim;

    pos.x = m_pos.x;
    pos.y = m_pos.y + m_dim.y;
    GlintCreate(pos);
}

Math::Point CControl::GetDim()
{
    return m_dim;
}


// Modify an attribute of state.

bool CControl::SetState(int state, bool bState)
{
    if ( bState )  m_state |= state;
    else           m_state &= ~state;
    return true;
}

// Sets an attribute of state.

bool CControl::SetState(int state)
{
    m_state |= state;
    return true;
}

// Removes an attribute of state.

bool CControl::ClearState(int state)
{
    m_state &= ~state;
    return true;
}

// Tests an attribute of state.

bool CControl::TestState(int state)
{
    return (m_state & state) ? true:false;
}

// Returns all attributes of state.

int CControl::GetState()
{
    return m_state;
}


// Management icon.

void CControl::SetIcon(int icon)
{
    m_icon = icon;
}

int CControl::GetIcon()
{
    return m_icon;
}


// Management of the button name.

void CControl::SetName(std::string name, bool bTooltip)
{
    if ( bTooltip )
    {
        auto p = name.find("\\");
        if ( p == std::string::npos )
            m_name = name;
        else
        {
            m_tooltip = name.substr(p + 1);
            m_name = name.substr(0, p);
        }
    }
    else
        m_name = name;
}

std::string CControl::GetName()
{
    return m_name;
}


// Management of the mode of justification (-1,0,1).

void CControl::SetTextAlign(Gfx::TextAlign mode)
{
    m_textAlign = mode;
//    m_justif = mode;
}

int CControl::GetTextAlign()
{
    return m_textAlign;
//    return m_justif;
}


// Management of the size of the font.

void CControl::SetFontSize(float size)
{
    m_fontSize = size;
}

float CControl::GetFontSize()
{
    return m_fontSize;
}


// Management of the stretch of font.

void CControl::SetFontStretch(float stretch)
{
    m_fontStretch = stretch;
}

float CControl::GetFontStretch()
{
    return m_fontStretch;
}


// Choice of the font.

void CControl::SetFontType(Gfx::FontType font)
{
    m_fontType = font;
}

Gfx::FontType CControl::GetFontType()
{
    return m_fontType;
}


// Specifies the tooltip.

bool CControl::SetTooltip(std::string name)
{
    m_tooltip = name;
    return true;
}

bool CControl::GetTooltip(Math::Point pos, std::string &name)
{
    if ( m_tooltip.length() == 0 ) return false;
    if ( (m_state & STATE_VISIBLE) == 0 ) return false;
    if ( (m_state & STATE_ENABLE) == 0 )  return false;
    if ( m_state & STATE_DEAD )  return false;
    if ( !Detect(pos) )  return false;

    name = m_tooltip;
    return true;
}


// Management of the focus.

void CControl::SetFocus(bool bFocus)
{
    m_bFocus = bFocus;
}

bool CControl::GetFocus()
{
    return m_bFocus;
}


// Returns the event associated with the control.

EventType CControl::GetEventType()
{
    return m_eventType;
}


// Management of an event.

bool CControl::EventProcess(const Event &event)
{
    if ( m_state & STATE_DEAD )  return true;

    if ( event.type == EVENT_FRAME && m_bGlint )
    {
        GlintFrame(event);
    }

    if ( event.type == EVENT_MOUSE_MOVE )
    {
        m_glintMouse = event.mousePos;

        if ( Detect(event.mousePos) )
        {
            if ( (m_state & STATE_VISIBLE) &&
                 (m_state & STATE_ENABLE ) )
            {
                m_engine->SetMouseType(Gfx::ENG_MOUSE_HAND);
            }
            SetState(STATE_HILIGHT);
        }
        else
        {
            ClearState(STATE_HILIGHT);
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN && event.mouseButton.button == MOUSE_BUTTON_LEFT)
    {
        if ( Detect(event.mousePos) )
        {
            m_bCapture = true;
            SetState(STATE_PRESS);
        }
    }

    if ( event.type == EVENT_MOUSE_MOVE && m_bCapture )
    {
        if ( Detect(event.mousePos) )
        {
            SetState(STATE_PRESS);
        }
        else
        {
            ClearState(STATE_PRESS);
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_UP && m_bCapture && event.mouseButton.button == MOUSE_BUTTON_LEFT)
    {
        m_bCapture = false;
        ClearState(STATE_PRESS);
    }

    return true;
}


// Removes the reflection.

void CControl::GlintDelete()
{
    m_bGlint = false;
}

// Creates a reflection for that button.

void CControl::GlintCreate(Math::Point ref, bool bLeft, bool bUp)
{
    float   offset;

    offset = 8.0f / 640.0f;
    if ( offset > m_dim.x / 4.0f)  offset = m_dim.x / 4.0f;

    if ( bLeft )
    {
        m_glintCorner1.x = ref.x;
        m_glintCorner2.x = ref.x+offset;
    }
    else
    {
        m_glintCorner1.x = ref.x-offset;
        m_glintCorner2.x = ref.x;
    }

    offset = 8.0f/480.0f;
    if ( offset > m_dim.y / 4.0f)  offset = m_dim.y / 4.0f;

    if ( bUp )
    {
        m_glintCorner1.y = ref.y - offset;
        m_glintCorner2.y = ref.y;
    }
    else
    {
        m_glintCorner1.y = ref.y;
        m_glintCorner2.y = ref.y + offset;
    }

    m_bGlint = true;
}

// Management of reflection.

void CControl::GlintFrame(const Event &event)
{
    Math::Vector    pos, speed;
    Math::Point     dim;

    if ( (m_state & STATE_GLINT  ) == 0 ||
         (m_state & STATE_ENABLE ) == 0 ||
         (m_state & STATE_VISIBLE) == 0 )  return;

    if ( !m_main->GetGlint() )  return;

    m_glintProgress += event.rTime;

    if ( m_glintProgress >= 2.0f && Detect(m_glintMouse) )
    {
        pos.x = m_glintCorner1.x + (m_glintCorner2.x - m_glintCorner1.x) * Math::Rand();
        pos.y = m_glintCorner1.y + (m_glintCorner2.y - m_glintCorner1.y) * Math::Rand();
        pos.z = 0.0f;
        speed = Math::Vector(0.0f, 0.0f, 0.0f);
        dim.x = ((15.0f + Math::Rand() * 15.0f) / 640.0f);
        dim.y = dim.x / 0.75f;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICONTROL,
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE );

        m_glintProgress = 0.0f;
    }
}


// Draw button.

void CControl::Draw()
{
    Math::Point     pos;
    float       zoomExt, zoomInt;
    int         icon;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    m_engine->SetTexture("button1.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

    zoomExt = 1.00f;
    zoomInt = 0.95f;

    if ( m_icon >= 128 )
    {
        zoomInt = 0.80f;
    }

    icon = 2;
    if ( m_state & STATE_CARD )
    {
        icon = 26;
    }
    if ( m_state & STATE_DEFAULT )
    {
        DrawPart(23, 1.3f, 0.0f);

        zoomExt *= 1.15f;
        zoomInt *= 1.15f;
    }
    if ( m_state & STATE_HILIGHT )
    {
        icon = 1;
    }
    if ( m_state & STATE_CHECK )
    {
        if ( m_state & STATE_CARD )
        {
            icon = 27;
        }
        else
        {
            icon = 0;
        }
    }
    if ( m_state & STATE_PRESS )
    {
        icon = 3;
        zoomInt *= 0.9f;
    }
    if ( (m_state & STATE_ENABLE) == 0 )
    {
        icon = 7;
    }
    if ( m_state & STATE_DEAD )
    {
        icon = 17;
    }

    if ( m_state & STATE_OKAY )
    {
        m_engine->SetTexture("button3.png");
        icon = 3;  // yellow with green point pressed
    }

    if ( m_name[0] == 0 )  // button without name?
    {
//?     DrawPart(icon, zoomExt, 0.0f);
        DrawPart(icon, zoomExt, 8.0f / 256.0f);

        if ( m_state & STATE_DEAD )  return;

        icon = m_icon;
        if ( icon >= 192 )
        {
            icon -= 192;
#if _POLISH
            m_engine->SetTexture("textp.png");
#else
            m_engine->SetTexture("text.png");
#endif
            m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        }
        else if ( icon >= 128 )
        {
            icon -= 128;
            m_engine->SetTexture("button3.png");
            m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        }
        else if ( icon >= 64 )
        {
            icon -= 64;
            m_engine->SetTexture("button2.png");
            m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        }
        else
        {
            m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        }
        if ( icon != -1 )
        {
            DrawPart(icon, zoomInt, 0.0f);
        }
    }
    else    // button with the name?
    {
        DrawPart(icon, 1.0f, 8.0f/256.0f);

        if ( m_state & STATE_DEAD )  return;

//        if ( m_justif < 0 )
        if ( m_textAlign == Gfx::TEXT_ALIGN_LEFT )
        {
            pos.x = m_pos.x + m_dim.x - m_dim.y * 0.5f;
            pos.y = m_pos.y + m_dim.y * 0.5f;
            pos.y -= m_engine->GetText()->GetHeight(m_fontType, m_fontSize)/2.0f;
            m_engine->GetText()->DrawText(m_name, m_fontType, m_fontSize, pos, m_dim.x, m_textAlign, 0);
        }
        else if ( m_textAlign == Gfx::TEXT_ALIGN_RIGHT )
//        else if ( m_justif > 0 )
        {
            pos.x = m_pos.x + m_dim.y * 0.5f;
            pos.y = m_pos.y + m_dim.y * 0.5f;
            pos.y -= m_engine->GetText()->GetHeight(m_fontType, m_fontSize)/2.0f;
            m_engine->GetText()->DrawText(m_name, m_fontType, m_fontSize, pos, m_dim.x, m_textAlign, 0);
        }
        else
        {
            pos.x = m_pos.x + m_dim.x * 0.5f;
            pos.y = m_pos.y + m_dim.y * 0.5f;
            pos.y -= m_engine->GetText()->GetHeight( m_fontType, m_fontSize)/2.0f;
            m_engine->GetText()->DrawText(m_name, m_fontType, m_fontSize, pos, m_dim.x, m_textAlign, 0);
        }
    }
}

// Draw the vertex array.

void CControl::DrawPart(int icon, float zoom, float ex)
{
    Math::Point     p1, p2, c, uv1, uv2;
    float       dp;

    p1.x = m_pos.x;
    p1.y = m_pos.y;
    p2.x = m_pos.x + m_dim.x;
    p2.y = m_pos.y + m_dim.y;

    if ( (m_state & STATE_CARD ) &&
         (m_state & STATE_CHECK) )
    {
        p2.y += (2.0f / 480.0f);  // a bit above
    }

    c.x = (p1.x + p2.x)/2.0f;
    c.y = (p1.y + p2.y)/2.0f;  // center

    p1.x = (p1.x - c.x) * zoom + c.x;
    p1.y = (p1.y - c.y) * zoom + c.y;
    p2.x = (p2.x - c.x) * zoom + c.x;
    p2.y = (p2.y - c.y) * zoom + c.y;

    p2.x -= p1.x;
    p2.y -= p1.y;

    uv1.x = (32.0f / 256.0f) * (icon%8);
    uv1.y = (32.0f / 256.0f) * (icon/8);  // uv texture
    uv2.x = (32.0f / 256.0f) + uv1.x;
    uv2.y = (32.0f / 256.0f) + uv1.y;

    dp = 0.5f / 256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    DrawIcon(p1, p2, uv1, uv2, ex);
}

// Draws an icon made up of a rectangular (if x = 0)
// or 3 pieces.

void CControl::DrawIcon(Math::Point pos, Math::Point dim, Math::Point uv1, Math::Point uv2,
                        float ex)
{
    Gfx::CDevice*   device;
    Gfx::Vertex     vertex[8];  // 6 triangles
    Math::Point     p1, p2, p3, p4;
    Math::Vector    n;

    device = m_engine->GetDevice();

    p1.x = pos.x;
    p1.y = pos.y;
    p2.x = pos.x + dim.x;
    p2.y = pos.y + dim.y;

    n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    if ( ex == 0.0f )  // one piece?
    {
        vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x,uv2.y));
        vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(uv1.x,uv1.y));
        vertex[2] = Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(uv2.x,uv2.y));
        vertex[3] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv2.x,uv1.y));

        device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
        m_engine->AddStatisticTriangle(2);
    }
    else    // 3 pieces?
    {
        if ( dim.x >= dim.y )
        {
            p3.x = p1.x + ex*dim.y / (uv2.y - uv1.y);
            p4.x = p2.x - ex*dim.y / (uv2.y - uv1.y);

            vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x,   uv2.y));
            vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(uv1.x,   uv1.y));
            vertex[2] = Gfx::Vertex(Math::Vector(p3.x, p1.y, 0.0f), n, Math::Point(uv1.x+ex,uv2.y));
            vertex[3] = Gfx::Vertex(Math::Vector(p3.x, p2.y, 0.0f), n, Math::Point(uv1.x+ex,uv1.y));
            vertex[4] = Gfx::Vertex(Math::Vector(p4.x, p1.y, 0.0f), n, Math::Point(uv2.x-ex,uv2.y));
            vertex[5] = Gfx::Vertex(Math::Vector(p4.x, p2.y, 0.0f), n, Math::Point(uv2.x-ex,uv1.y));
            vertex[6] = Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(uv2.x,   uv2.y));
            vertex[7] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv2.x,   uv1.y));

            device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 8);
            m_engine->AddStatisticTriangle(6);
        }
        else
        {
            p3.y = p1.y + ex*dim.x / (uv2.x - uv1.x);
            p4.y = p2.y - ex*dim.x / (uv2.x - uv1.x);

            vertex[0] = Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(uv2.x, uv2.y     ));
            vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x, uv2.y     ));
            vertex[2] = Gfx::Vertex(Math::Vector(p2.x, p3.y, 0.0f), n, Math::Point(uv2.x, uv2.y - ex));
            vertex[3] = Gfx::Vertex(Math::Vector(p1.x, p3.y, 0.0f), n, Math::Point(uv1.x, uv2.y - ex));
            vertex[4] = Gfx::Vertex(Math::Vector(p2.x, p4.y, 0.0f), n, Math::Point(uv2.x, uv1.y + ex));
            vertex[5] = Gfx::Vertex(Math::Vector(p1.x, p4.y, 0.0f), n, Math::Point(uv1.x, uv1.y + ex));
            vertex[6] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv2.x, uv1.y     ));
            vertex[7] = Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(uv1.x, uv1.y     ));

            device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 8);
            m_engine->AddStatisticTriangle(6);
        }
    }
}

// Draws a rectangular icon made up of 9 pieces.

void CControl::DrawIcon(Math::Point pos, Math::Point dim, Math::Point uv1, Math::Point uv2,
                        Math::Point corner, float ex)
{
    Gfx::CDevice* device;
    Gfx::Vertex    vertex[8];  // 6 triangles
    Math::Point     p1, p2, p3, p4;
    Math::Vector    n;

    device = m_engine->GetDevice();

    p1.x = pos.x;
    p1.y = pos.y;
    p2.x = pos.x + dim.x;
    p2.y = pos.y + dim.y;

    n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    if ( corner.x > dim.x / 2.0f )  corner.x = dim.x / 2.0f;
    if ( corner.y > dim.y / 2.0f )  corner.y = dim.y / 2.0f;

    p1.x = pos.x;
    p1.y = pos.y;
    p2.x = pos.x + dim.x;
    p2.y = pos.y + dim.y;
    p3.x = p1.x + corner.x;
    p3.y = p1.y + corner.y;
    p4.x = p2.x - corner.x;
    p4.y = p2.y - corner.y;

    // Bottom horizontal band.
    vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x,      uv2.y     ));
    vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p3.y, 0.0f), n, Math::Point(uv1.x,      uv2.y - ex));
    vertex[2] = Gfx::Vertex(Math::Vector(p3.x, p1.y, 0.0f), n, Math::Point(uv1.x + ex, uv2.y     ));
    vertex[3] = Gfx::Vertex(Math::Vector(p3.x, p3.y, 0.0f), n, Math::Point(uv1.x + ex, uv2.y - ex));
    vertex[4] = Gfx::Vertex(Math::Vector(p4.x, p1.y, 0.0f), n, Math::Point(uv2.x - ex, uv2.y     ));
    vertex[5] = Gfx::Vertex(Math::Vector(p4.x, p3.y, 0.0f), n, Math::Point(uv2.x - ex, uv2.y - ex));
    vertex[6] = Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(uv2.x,      uv2.y     ));
    vertex[7] = Gfx::Vertex(Math::Vector(p2.x, p3.y, 0.0f), n, Math::Point(uv2.x,      uv2.y - ex));
    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 8);
    m_engine->AddStatisticTriangle(6);

    // Central horizontal band.
    vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p3.y, 0.0f), n, Math::Point(uv1.x,      uv2.y - ex));
    vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p4.y, 0.0f), n, Math::Point(uv1.x,      uv1.y + ex));
    vertex[2] = Gfx::Vertex(Math::Vector(p3.x, p3.y, 0.0f), n, Math::Point(uv1.x + ex, uv2.y - ex));
    vertex[3] = Gfx::Vertex(Math::Vector(p3.x, p4.y, 0.0f), n, Math::Point(uv1.x + ex, uv1.y + ex));
    vertex[4] = Gfx::Vertex(Math::Vector(p4.x, p3.y, 0.0f), n, Math::Point(uv2.x - ex, uv2.y - ex));
    vertex[5] = Gfx::Vertex(Math::Vector(p4.x, p4.y, 0.0f), n, Math::Point(uv2.x - ex, uv1.y + ex));
    vertex[6] = Gfx::Vertex(Math::Vector(p2.x, p3.y, 0.0f), n, Math::Point(uv2.x,      uv2.y - ex));
    vertex[7] = Gfx::Vertex(Math::Vector(p2.x, p4.y, 0.0f), n, Math::Point(uv2.x,      uv1.y + ex));
    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 8);
    m_engine->AddStatisticTriangle(6);

    // Top horizontal band.
    vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p4.y, 0.0f), n, Math::Point(uv1.x,      uv1.y + ex));
    vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(uv1.x,      uv1.y   ));
    vertex[2] = Gfx::Vertex(Math::Vector(p3.x, p4.y, 0.0f), n, Math::Point(uv1.x + ex, uv1.y + ex));
    vertex[3] = Gfx::Vertex(Math::Vector(p3.x, p2.y, 0.0f), n, Math::Point(uv1.x + ex, uv1.y   ));
    vertex[4] = Gfx::Vertex(Math::Vector(p4.x, p4.y, 0.0f), n, Math::Point(uv2.x - ex, uv1.y + ex));
    vertex[5] = Gfx::Vertex(Math::Vector(p4.x, p2.y, 0.0f), n, Math::Point(uv2.x - ex, uv1.y   ));
    vertex[6] = Gfx::Vertex(Math::Vector(p2.x, p4.y, 0.0f), n, Math::Point(uv2.x,      uv1.y + ex));
    vertex[7] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv2.x,      uv1.y   ));
    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 8);
    m_engine->AddStatisticTriangle(6);
}

// Draw round the hatch of a button.

void CControl::DrawWarning(Math::Point pos, Math::Point dim)
{
    Math::Point     uv1, uv2;
    float       dp;

    dp = 0.5f / 256.0f;

    m_engine->SetTexture("button2.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

    uv1.x =  64.0f / 256.0f;
    uv1.y = 208.0f / 256.0f;
    uv2.x = 160.0f / 256.0f;
    uv2.y = 224.0f / 256.0f;

    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    if ( dim.x < dim.y*4.0f )
    {
        dim.y /= 2.0f;
        DrawIcon(pos, dim, uv1, uv2);
        pos.y += dim.y;
        DrawIcon(pos, dim, uv1, uv2);
    }
    else
    {
        dim.x /= 2.0f;
        dim.y /= 2.0f;
        DrawIcon(pos, dim, uv1, uv2);
        pos.x += dim.x;
        DrawIcon(pos, dim, uv1, uv2);
        pos.x -= dim.x;
        pos.y += dim.y;
        DrawIcon(pos, dim, uv1, uv2);
        pos.x += dim.x;
        DrawIcon(pos, dim, uv1, uv2);
    }
}

// Draw the shade under a button.

void CControl::DrawShadow(Math::Point pos, Math::Point dim, float deep)
{
    Math::Point     uv1, uv2, corner;
    float       dp;

    dp = 0.5f/256.0f;

    m_engine->SetTexture("button2.png");
    m_engine->SetState( Gfx::ENG_RSTATE_TTEXTURE_WHITE);

    pos.x += deep * 0.010f * 0.75f;
    pos.y -= deep * 0.015f;
    dim.x += deep * 0.005f * 0.75f;
    dim.y += deep * 0.005f;

    uv1.x = 192.0f / 256.0f;
    uv1.y =  32.0f / 256.0f;
    uv2.x = 224.0f / 256.0f;
    uv2.y =  64.0f / 256.0f;

    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    corner.x = 10.0f / 640.0f;
    corner.y = 10.0f / 480.0f;

    DrawIcon(pos, dim, uv1, uv2, corner, 6.0f / 256.0f);
}


// Detects whether a position is in the button.

bool CControl::Detect(Math::Point pos)
{
    return ( pos.x >= m_pos.x           &&
              pos.x <= m_pos.x + m_dim.x &&
              pos.y >= m_pos.y           &&
              pos.y <= m_pos.y + m_dim.y );
}

}

