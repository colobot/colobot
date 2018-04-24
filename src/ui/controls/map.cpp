/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "ui/controls/map.h"

#include "common/image.h"

#include "graphics/core/device.h"

#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/interface/controllable_object.h"
#include "object/interface/transportable_object.h"

#include <cstring>


namespace Ui
{

// Object's constructor.

CMap::CMap() : CControl()
{
    m_main    = CRobotMain::GetInstancePointer();
    m_terrain = m_main->GetTerrain();
    m_water   = Gfx::CEngine::GetInstancePointer()->GetWater();

    m_bEnable  = true;
    m_time     = 0.0f;
    m_zoom     = 2.0f;
    m_offset.x = 0.0f;
    m_offset.y = 0.0f;
    m_angle    = 0.0f;

    m_floorColor.r = 1.00f;
    m_floorColor.g = 0.50f;
    m_floorColor.b = 0.00f;  // orange

    m_waterColor.r = 0.00f;
    m_waterColor.g = 0.80f;
    m_waterColor.b = 1.00f;  // blue

    m_half = m_terrain->GetMosaicCount() * m_terrain->GetBrickCount() * m_terrain->GetBrickSize() / 2.0f;

    m_highlightRank = -1;
    m_totalFix  = 0;
    m_totalMove = 0;
    m_bRadar = false;

    FlushObject();

    m_mode = 0;
    m_bToy = false;
    m_bDebug = false;
}

// Object's destructor.

CMap::~CMap()
{
}


// Creates a new button.

bool CMap::Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventMsg);
    return true;
}


// Choice of the offset, when image is displayed.

void CMap::SetOffset(float ox, float oy)
{
    m_offset.x = ox;
    m_offset.y = oy;
    m_half = m_terrain->GetMosaicCount() * m_terrain->GetBrickCount() * m_terrain->GetBrickSize() / 2.0f;
}

// Choice of the global angle of rotation.

void CMap::SetAngle(float angle)
{
    m_angle = angle;
}

// Specifies the alternate mode.

void CMap::SetMode(int mode)
{
    m_mode = mode;
}

// Specifies the type of icon for the selected object.

void CMap::SetToy(bool bToy)
{
    m_bToy = bToy;
}

void CMap::SetDebug(bool bDebug)
{
    m_bDebug = bDebug;
}


//Choice of magnification of the map.

void CMap::SetZoom(float value)
{
    m_zoom = value;
    m_half = m_terrain->GetMosaicCount() * m_terrain->GetBrickCount() * m_terrain->GetBrickSize() / 2.0f;
}

float CMap::GetZoom()
{
    return m_zoom;
}

// Choosing a fixed offset.

// Enables or disables the card.

void CMap::SetEnable(bool bEnable)
{
    m_bEnable = bEnable;
    SetState(STATE_DEAD, !bEnable);
}

bool CMap::GetEnable()
{
    return m_bEnable;
}


// Choosing the color of the soil.

void CMap::SetFloorColor(Gfx::Color color)
{
    m_floorColor = color;
}

// Choosing the color of the water.

void CMap::SetWaterColor(Gfx::Color color)
{
    m_waterColor = color;
}


// Specifies a fixed image in place of the drawing of the relief.

void CMap::SetFixImage(const std::string& filename)
{
    m_fixImage = filename;
}

// Whether to use a still image.

bool CMap::GetFixImage()
{
    return ! m_fixImage.empty();
}


// Management of an event.

bool CMap::EventProcess(const Event &event)
{
    if ( (m_state & STATE_VISIBLE) == 0 )
        return true;

    CControl::EventProcess(event);

    if ( event.type == EVENT_FRAME )
        m_time += event.rTime;

    if ( event.type == EVENT_MOUSE_MOVE || event.type == EVENT_MOUSE_BUTTON_DOWN || event.type == EVENT_MOUSE_BUTTON_UP )
    {
        if (Detect(event.mousePos))
        {
            m_engine->SetMouseType(Gfx::ENG_MOUSE_NORM);
            bool inMap = false;
            if (DetectObject(event.mousePos, inMap) != nullptr)
                m_engine->SetMouseType(Gfx::ENG_MOUSE_HAND);
        }
    }

    if (event.type == EVENT_MOUSE_BUTTON_DOWN &&
        event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_LEFT)
    {
        if ( CControl::Detect(event.mousePos) )
        {
            SelectObject(event.mousePos);
            return false;
        }
    }

    return true;
}

// Adjusts the offset to not exceed the card.

Math::Point CMap::AdjustOffset(Math::Point offset)
{
    float   limit;

    limit = m_half - m_half / m_zoom;
    if ( offset.x < -limit )  offset.x = -limit;
    if ( offset.x >  limit )  offset.x =  limit;
    if ( offset.y < -limit )  offset.y = -limit;
    if ( offset.y >  limit )  offset.y =  limit;

    return offset;
}

// Indicates the object with the mouse hovers over.

void CMap::SetHighlight(CObject* pObj)
{
    m_highlightRank = -1;
    if ( m_bToy || !m_fixImage.empty())
        return;  // card with still image?
    if ( pObj == nullptr )
        return;

    for (int i = 0; i < MAPMAXOBJECT; i++)
    {
        if ( !m_map[i].bUsed )
            continue;

        if ( m_map[i].object == pObj )
        {
            m_highlightRank = i;
            break;
        }
    }
}

// Detects an object in the map.

CObject* CMap::DetectObject(Math::Point pos, bool &bInMap)
{
    float dist, min;
    int best;

    bInMap = false;
    if ( pos.x < m_pos.x ||
         pos.y < m_pos.y ||
         pos.x > m_pos.x + m_dim.x ||
         pos.y > m_pos.y + m_dim.y )
        return nullptr;

    bInMap = true;

    pos.x = (pos.x - m_pos.x) / m_dim.x * 256.0f;
    pos.y = (pos.y - m_pos.y) / m_dim.y * 256.0f;  // 0..256
    pos.x = (pos.x - 128.0f) * m_half / (m_zoom * 128.0f) + m_offset.x;
    pos.y = (pos.y - 128.0f) * m_half / (m_zoom * 128.0f) + m_offset.y;

    min = 10000.0f;
    best = -1;
    for (int i = MAPMAXOBJECT - 1; i >= 0; i--)
    {
        if ( !m_map[i].bUsed )
            continue;
        if ( m_map[i].color == MAPCOLOR_BBOX  && !m_bRadar )
            continue;
        if ( m_map[i].color == MAPCOLOR_ALIEN && !m_bRadar )
            continue;

        dist = Math::Point(m_map[i].pos.x - pos.x, m_map[i].pos.y - pos.y).Length();
        if ( dist > m_half / m_zoom * 8.0f / 100.0f )
            continue;  // too far?
        if ( dist < min )
        {
            min = dist;
            best = i;
        }
    }
    if ( best == -1 )
        return nullptr;
    return m_map[best].object;
}

// Selects an object.

void CMap::SelectObject(Math::Point pos)
{
    CObject *pObj;
    bool bInMap;

    pObj = DetectObject(pos, bInMap);
    if ( pObj != nullptr )
        m_main->SelectObject(pObj);
}


// Draw the map.

void CMap::Draw()
{
    Math::Point     uv1, uv2;
    int i;

    if ( (m_state & STATE_VISIBLE) == 0 )
        return;

    CControl::Draw();  // draws the bottom (button)

    if ( !m_bEnable )
        return;

    if (m_fixImage.empty() && m_map[MAPMAXOBJECT - 1].bUsed)
        m_offset = AdjustOffset(m_map[MAPMAXOBJECT - 1].pos);

    if (m_fixImage.empty()) // drawing of the relief?
    {
        m_engine->SetTexture("textures/interface/map.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x = 0.5f + (m_offset.x - (m_half / m_zoom)) / (m_half * 2.0f);
        uv1.y = 0.5f - (m_offset.y + (m_half / m_zoom)) / (m_half * 2.0f);
        uv2.x = 0.5f + (m_offset.x + (m_half / m_zoom)) / (m_half * 2.0f);
        uv2.y = 0.5f - (m_offset.y - (m_half / m_zoom)) / (m_half * 2.0f);
        DrawVertex(uv1, uv2, 0.97f);  // drawing the map
    }
    else // still image?
    {
        m_engine->LoadTexture(m_fixImage);
        m_engine->SetTexture(m_fixImage);
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x = 0.0f;
        uv1.y = 0.0f;
        uv2.x = 1.0f;
        uv2.y = 1.0f;
        DrawVertex(uv1, uv2, 0.97f);  // drawing the map
    }

    i = MAPMAXOBJECT-1;
    if ( m_map[i].bUsed )  // selection:
        DrawFocus(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color);

    for ( i=0 ; i<m_totalFix ; i++ ) // fixed objects:
    {
        if ( i == m_highlightRank )
            continue;
        DrawObject(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color, false, false);
    }

    for ( i=MAPMAXOBJECT-2 ; i>m_totalMove ; i-- ) // moving objects:
    {
        if ( i == m_highlightRank )
            continue;
        DrawObject(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color, false, false);
    }

    i = MAPMAXOBJECT-1;
    if ( m_map[i].bUsed && i != m_highlightRank )  // selection:
        DrawObject(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color, true, false);

    if ( m_highlightRank != -1 && m_map[m_highlightRank].bUsed )
    {
        i = m_highlightRank;
        DrawObject(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color, false, true);
        DrawHighlight(m_map[i].pos);
    }
}

// Computing a point for drawFocus.

Math::Point CMap::MapInter(Math::Point pos, float dir)
{
    Math::Point p1;
    float   limit;

    p1.x = pos.x + 1.0f;
    p1.y = pos.y;
    p1 = Math::RotatePoint(pos, dir, p1);

    p1.x -= pos.x;
    p1.y -= pos.y;

    limit = m_mapPos.x + m_mapDim.x - pos.x;
    if ( p1.x > limit ) // exceeds the right?
    {
        p1.y = limit*p1.y/p1.x;
        p1.x = limit;
    }
    limit = m_mapPos.y * 0.75f + m_mapDim.y * 0.75f - pos.y;
    if ( p1.y > limit ) // exceeds the top?
    {
        p1.x = limit * p1.x / p1.y;
        p1.y = limit;
    }
    limit = m_mapPos.x - pos.x;
    if ( p1.x < limit ) // exceeds the left?
    {
        p1.y = limit * p1.y / p1.x;
        p1.x = limit;
    }

    limit = m_mapPos.y * 0.75f - pos.y;
    if ( p1.y < limit ) // exceeds the bottom?
    {
        p1.x = limit * p1.x / p1.y;
        p1.y = limit;
    }

    p1.x += pos.x;
    p1.y += pos.y;
    return p1;
}

// Draw the field of vision of the selected object.

void CMap::DrawFocus(Math::Point pos, float dir, ObjectType type, MapColor color)
{
    Math::Point p0, p1, p2, uv1, uv2, rel;
    float   aMin, aMax, aOct, focus, a;
    float   limit[5];
    bool    bEnding;
    int     quart;

    if (m_bToy || !m_fixImage.empty())  return;  // map with still image?
    if ( color != MAPCOLOR_MOVE )  return;

    pos.x = (pos.x-m_offset.x)*(m_zoom*0.5f)/m_half+0.5f;
    pos.y = (pos.y-m_offset.y)*(m_zoom*0.5f)/m_half+0.5f;

    if ( pos.x < 0.0f || pos.x > 1.0f ||
         pos.y < 0.0f || pos.y > 1.0f )  return;

    rel.x = pos.x*2.0f-1.0f;
    rel.y = pos.y*2.0f-1.0f;  // rel [-1..1]

    pos.x = m_mapPos.x+m_mapDim.x*pos.x;
    pos.y = m_mapPos.y*0.75f+m_mapDim.y*pos.y*0.75f;

    focus = m_engine->GetFocus();
    dir += Math::PI/2.0f;
    aMin = Math::NormAngle(dir-Math::PI/4.0f*focus);
    aMax = Math::NormAngle(dir+Math::PI/4.0f*focus);

    if ( aMin > aMax )
    {
        aMax += Math::PI*2.0f;  // aMax always after aMin
    }

    limit[0] = Math::RotateAngle( 1.0f-rel.x,  1.0f-rel.y);  // upper/right
    limit[1] = Math::RotateAngle(-1.0f-rel.x,  1.0f-rel.y);  // upper/left
    limit[2] = Math::RotateAngle(-1.0f-rel.x, -1.0f-rel.y);  // lower/left
    limit[3] = Math::RotateAngle( 1.0f-rel.x, -1.0f-rel.y);  // lower/right
    limit[4] = limit[0]+Math::PI*2.0f;

    a = Math::NormAngle(aMin);
    for ( quart=0 ; quart<4 ; quart++ )
    {
        if ( a >= limit[quart+0] &&
             a <= limit[quart+1] )  break;
    }
    if ( quart == 4 )  quart = -1;

    uv1.x = 113.0f/256.0f;  // degrade green
    uv1.y = 240.5f/256.0f;
    uv2.x = 126.0f/256.0f;
    uv2.y = 255.0f/256.0f;

    m_engine->SetTexture("textures/interface/button2.png");
    m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);

    bEnding = false;
    do
    {
        quart ++;
        aOct = limit[quart%4];
        if ( quart >= 4 )  aOct += Math::PI*2.0f;
        if ( aOct >= aMax - Math::VERY_SMALL_NUM )
        {
            aOct = aMax;
            bEnding = true;
        }

        p0 = pos;
        p1 = MapInter(pos, aMin);
        p2 = MapInter(pos, aOct);
        p0.y /= 0.75f;
        p1.y /= 0.75f;
        p2.y /= 0.75f;
        DrawTriangle(p0, p2, p1, uv1, uv2);

        aMin = aOct;
    }
    while ( !bEnding );
}

// Draw an object.

void CMap::DrawObject(Math::Point pos, float dir, ObjectType type, MapColor color,
                      bool bSelect, bool bHilite)
{
    Math::Point     p1, p2, p3, p4, p5, dim, uv1, uv2;
    bool        bOut, bUp, bDown, bLeft, bRight;

    pos.x = (pos.x-m_offset.x)*(m_zoom*0.5f)/m_half+0.5f;
    pos.y = (pos.y-m_offset.y)*(m_zoom*0.5f)/m_half+0.5f;

    bOut = bUp = bDown = bLeft = bRight = false;
    if ( pos.x < 0.06f )  { pos.x = 0.02f;  bOut = bLeft  = true; }
    if ( pos.y < 0.06f )  { pos.y = 0.02f;  bOut = bDown  = true; }
    if ( pos.x > 0.94f )  { pos.x = 0.98f;  bOut = bRight = true; }
    if ( pos.y > 0.94f )  { pos.y = 0.98f;  bOut = bUp    = true; }

    pos.x = m_mapPos.x+m_mapDim.x*pos.x;
    pos.y = m_mapPos.y+m_mapDim.y*pos.y;
    dim.x = 2.0f/128.0f*0.75f;
    dim.y = 2.0f/128.0f;

    if ( bOut )  // outside the map?
    {
        if ( color == MAPCOLOR_BBOX  && !m_bRadar )  return;
        if ( color == MAPCOLOR_ALIEN && !m_bRadar )  return;

        if ( Math::Mod(m_time+(pos.x+pos.y)*4.0f, 0.6f) > 0.2f )
        {
            return;  // flashes
        }

        m_engine->SetTexture("textures/interface/button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        if ( bUp )
        {
            uv1.x = 160.5f/256.0f;  // yellow triangle ^
            uv1.y = 240.5f/256.0f;
            uv2.x = 175.0f/256.0f;
            uv2.y = 255.0f/256.0f;
        }
        if ( bDown )
        {
            uv1.x = 160.5f/256.0f;  // yellow triangle v
            uv1.y = 255.0f/256.0f;
            uv2.x = 175.0f/256.0f;
            uv2.y = 240.5f/256.0f;
        }
        if ( bRight )
        {
            uv1.x = 176.5f/256.0f;  // yellow triangle >
            uv1.y = 240.5f/256.0f;
            uv2.x = 191.0f/256.0f;
            uv2.y = 255.0f/256.0f;
        }
        if ( bLeft )
        {
            uv1.x = 191.0f/256.0f;  // yellow triangle <
            uv1.y = 240.5f/256.0f;
            uv2.x = 176.5f/256.0f;
            uv2.y = 255.0f/256.0f;
        }
        pos.x -= dim.x/2.0f;
        pos.y -= dim.y/2.0f;
        DrawIcon(pos, dim, uv1, uv2);
        return;
    }

    if ( bSelect )
    {
        if ( m_bToy )
        {
            dim.x *= 1.2f+sinf(m_time*8.0f)*0.1f;
            dim.y *= 1.2f+sinf(m_time*8.0f)*0.1f;
        }
        else
        {
            dim.x *= 1.2f+sinf(m_time*8.0f)*0.3f;
            dim.y *= 1.2f+sinf(m_time*8.0f)*0.3f;
        }
    }
    if ( color == MAPCOLOR_BASE ||
         color == MAPCOLOR_FIX  ||
         color == MAPCOLOR_MOVE )
    {
        if ( bHilite )
        {
            dim.x *= 2.2f;
            dim.y *= 2.2f;
        }
        else
        {
            dim.x *= 0.6f;
            dim.y *= 0.6f;
        }
    }
    if ( color == MAPCOLOR_ALIEN )
    {
        dim.x *= 1.4f;
        dim.y *= 1.4f;
    }
    if ( type == OBJECT_TEEN28 )  // bottle?
    {
        dim.x *= 3.0f;
        dim.y *= 3.0f;
        bHilite = true;
    }
    if ( type == OBJECT_TEEN34 )  // stone?
    {
        dim.x *= 2.0f;
        dim.y *= 2.0f;
        bHilite = true;
    }

    if ( color == MAPCOLOR_MOVE && bSelect )
    {
        if ( m_bToy )
        {
            p1.x = pos.x;
            p1.y = pos.y+dim.y*1.4f;
            p1 = Math::RotatePoint(pos, dir, p1);
            p1.x = pos.x+(p1.x-pos.x)*0.75f;

            p2.x = pos.x+dim.x*1.2f;
            p2.y = pos.y+dim.y*0.8f;
            p2 = Math::RotatePoint(pos, dir, p2);
            p2.x = pos.x+(p2.x-pos.x)*0.75f;

            p3.x = pos.x+dim.x*1.2f;
            p3.y = pos.y-dim.y*1.0f;
            p3 = Math::RotatePoint(pos, dir, p3);
            p3.x = pos.x+(p3.x-pos.x)*0.75f;

            p4.x = pos.x-dim.x*1.2f;
            p4.y = pos.y-dim.y*1.0f;
            p4 = Math::RotatePoint(pos, dir, p4);
            p4.x = pos.x+(p4.x-pos.x)*0.75f;

            p5.x = pos.x-dim.x*1.2f;
            p5.y = pos.y+dim.y*0.8f;
            p5 = Math::RotatePoint(pos, dir, p5);
            p5.x = pos.x+(p5.x-pos.x)*0.75f;
        }
        else
        {
            p1.x = pos.x;
            p1.y = pos.y+dim.y*2.4f;
            p1 = Math::RotatePoint(pos, dir, p1);
            p1.x = pos.x+(p1.x-pos.x)*0.75f;

            p2.x = pos.x+dim.x*1.0f;
            p2.y = pos.y-dim.y*1.6f;
            p2 = Math::RotatePoint(pos, dir, p2);
            p2.x = pos.x+(p2.x-pos.x)*0.75f;

            p3.x = pos.x-dim.x*1.0f;
            p3.y = pos.y-dim.y*1.6f;
            p3 = Math::RotatePoint(pos, dir, p3);
            p3.x = pos.x+(p3.x-pos.x)*0.75f;
        }
    }

    pos.x -= dim.x/2.0f;
    pos.y -= dim.y/2.0f;

    if ( color == MAPCOLOR_BASE ||
         color == MAPCOLOR_FIX  )
    {
        DrawObjectIcon(pos, dim, color, type, bHilite);
    }

    if ( color == MAPCOLOR_MOVE )
    {
        if ( bSelect )
        {
            m_engine->SetTexture("textures/interface/button2.png");
            m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
            if ( m_bToy )
            {
                uv1.x = 164.5f/256.0f;  // black pentagon
                uv1.y = 228.5f/256.0f;
                uv2.x = 172.0f/256.0f;
                uv2.y = 236.0f/256.0f;
                DrawPenta(p1, p2, p3, p4, p5, uv1, uv2);
            }
            else
            {
                uv1.x = 144.5f/256.0f;  // red triangle
                uv1.y = 240.5f/256.0f;
                uv2.x = 159.0f/256.0f;
                uv2.y = 255.0f/256.0f;
                DrawTriangle(p1, p2, p3, uv1, uv2);
            }
        }
        DrawObjectIcon(pos, dim, color, type, bHilite);
    }

    if ( color == MAPCOLOR_BBOX )
    {
        if ( m_bRadar )
        {
            m_engine->SetTexture("textures/interface/button2.png");
            m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
            uv1.x =  64.5f/256.0f;  // blue triangle
            uv1.y = 240.5f/256.0f;
            uv2.x =  79.0f/256.0f;
            uv2.y = 255.0f/256.0f;
            DrawIcon(pos, dim, uv1, uv2);
        }
    }

    if ( color == MAPCOLOR_ALIEN )
    {
        if ( m_bRadar )
        {
            DrawObjectIcon(pos, dim, color, type, true);
        }
    }

    if ( color == MAPCOLOR_WAYPOINTb )
    {
        m_engine->SetTexture("textures/interface/button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x = 192.5f/256.0f;  // blue cross
        uv1.y = 240.5f/256.0f;
        uv2.x = 207.0f/256.0f;
        uv2.y = 255.0f/256.0f;
        DrawIcon(pos, dim, uv1, uv2);
    }
    if ( color == MAPCOLOR_WAYPOINTr )
    {
        m_engine->SetTexture("textures/interface/button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x = 208.5f/256.0f;  // red cross
        uv1.y = 240.5f/256.0f;
        uv2.x = 223.0f/256.0f;
        uv2.y = 255.0f/256.0f;
        DrawIcon(pos, dim, uv1, uv2);
    }
    if ( color == MAPCOLOR_WAYPOINTg )
    {
        m_engine->SetTexture("textures/interface/button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x = 224.5f/256.0f;  // green cross
        uv1.y = 240.5f/256.0f;
        uv2.x = 239.0f/256.0f;
        uv2.y = 255.0f/256.0f;
        DrawIcon(pos, dim, uv1, uv2);
    }
    if ( color == MAPCOLOR_WAYPOINTy )
    {
        m_engine->SetTexture("textures/interface/button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x = 240.5f/256.0f;  // yellow cross
        uv1.y = 240.5f/256.0f;
        uv2.x = 255.0f/256.0f;
        uv2.y = 255.0f/256.0f;
        DrawIcon(pos, dim, uv1, uv2);
    }
    if ( color == MAPCOLOR_WAYPOINTv )
    {
        m_engine->SetTexture("textures/interface/button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x = 192.5f/256.0f;  // violet cross
        uv1.y = 224.5f/256.0f;
        uv2.x = 207.0f/256.0f;
        uv2.y = 239.0f/256.0f;
        DrawIcon(pos, dim, uv1, uv2);
    }
}

// Draws the icon of an object.

void CMap::DrawObjectIcon(Math::Point pos, Math::Point dim, MapColor color,
                          ObjectType type, bool bHilite)
{
    Math::Point ppos, ddim, uv1, uv2;
    float   dp;
    int     icon;

    dp = 0.5f/256.0f;

    m_engine->SetTexture("textures/interface/button3.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
    if ( color == MAPCOLOR_MOVE )
    {
        uv1.x = 160.0f/256.0f;  // blue
        uv1.y = 224.0f/256.0f;
    }
    else if ( color == MAPCOLOR_ALIEN )
    {
        uv1.x = 224.0f/256.0f;  // green
        uv1.y = 224.0f/256.0f;
    }
    else
    {
        uv1.x = 192.0f/256.0f;  // yellow
        uv1.y = 224.0f/256.0f;
    }
    uv2.x = uv1.x+32.0f/256.0f;
    uv2.y = uv1.y+32.0f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;
    DrawIcon(pos, dim, uv1, uv2);  // background colors

    if ( bHilite )
    {
        icon = -1;
        if ( type == OBJECT_FACTORY  )  icon = 32;
        if ( type == OBJECT_DERRICK  )  icon = 33;
        if ( type == OBJECT_CONVERT  )  icon = 34;
        if ( type == OBJECT_RESEARCH )  icon = 35;
        if ( type == OBJECT_STATION  )  icon = 36;
        if ( type == OBJECT_TOWER    )  icon = 37;
        if ( type == OBJECT_LABO     )  icon = 38;
        if ( type == OBJECT_ENERGY   )  icon = 39;
        if ( type == OBJECT_RADAR    )  icon = 40;
        if ( type == OBJECT_INFO     )  icon = 44;
        if ( type == OBJECT_REPAIR   )  icon = 41;
        if ( type == OBJECT_DESTROYER)  icon = 41;
        if ( type == OBJECT_NUCLEAR  )  icon = 42;
        if ( type == OBJECT_PARA     )  icon = 46;
        if ( type == OBJECT_SAFE     )  icon = 47;
        if ( type == OBJECT_HUSTON   )  icon = 48;
        if ( type == OBJECT_TARGET1  )  icon = 45;
        if ( type == OBJECT_BASE     )  icon = 43;
        if ( type == OBJECT_HUMAN    )  icon =  8;
        if ( type == OBJECT_MOBILEfa )  icon = 11;
        if ( type == OBJECT_MOBILEta )  icon = 10;
        if ( type == OBJECT_MOBILEwa )  icon =  9;
        if ( type == OBJECT_MOBILEia )  icon = 22;
        if ( type == OBJECT_MOBILEfc )  icon = 17;
        if ( type == OBJECT_MOBILEtc )  icon = 16;
        if ( type == OBJECT_MOBILEwc )  icon = 15;
        if ( type == OBJECT_MOBILEic )  icon = 23;
        if ( type == OBJECT_MOBILEfi )  icon = 27;
        if ( type == OBJECT_MOBILEti )  icon = 26;
        if ( type == OBJECT_MOBILEwi )  icon = 25;
        if ( type == OBJECT_MOBILEii )  icon = 28;
        if ( type == OBJECT_MOBILEfs )  icon = 14;
        if ( type == OBJECT_MOBILEts )  icon = 13;
        if ( type == OBJECT_MOBILEws )  icon = 12;
        if ( type == OBJECT_MOBILEis )  icon = 24;
        if ( type == OBJECT_MOBILErt )  icon = 18;
        if ( type == OBJECT_MOBILErc )  icon = 19;
        if ( type == OBJECT_MOBILErr )  icon = 20;
        if ( type == OBJECT_MOBILErs )  icon = 29;
        if ( type == OBJECT_MOBILEsa )  icon = 21;
        if ( type == OBJECT_MOBILEft )  icon = 30;
        if ( type == OBJECT_MOBILEtt )  icon = 30;
        if ( type == OBJECT_MOBILEwt )  icon = 30;
        if ( type == OBJECT_MOBILEit )  icon = 30;
        if ( type == OBJECT_MOBILEtg )  icon = 45;
        if ( type == OBJECT_MOBILEdr )  icon = 48;
        if ( type == OBJECT_APOLLO2  )  icon = 49;
        if ( type == OBJECT_MOTHER   )  icon = 31;
        if ( type == OBJECT_ANT      )  icon = 31;
        if ( type == OBJECT_SPIDER   )  icon = 31;
        if ( type == OBJECT_BEE      )  icon = 31;
        if ( type == OBJECT_WORM     )  icon = 31;
        if ( type == OBJECT_TEEN28    )  icon = 48;  // bottle
        if ( type == OBJECT_TEEN34    )  icon = 48;  // stone
        if ( icon == -1 )  return;

        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x = (32.0f/256.0f)*(icon%8);
        uv1.y = (32.0f/256.0f)*(icon/8);
        uv2.x = uv1.x+32.0f/256.0f;
        uv2.y = uv1.y+32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);  // icon
    }
}

// Draw the object with the mouse hovers over.

void CMap::DrawHighlight(Math::Point pos)
{
    Math::Point dim, uv1, uv2;

    if (m_bToy || !m_fixImage.empty())  return;  // map with still image?

    pos.x = (pos.x-m_offset.x)*(m_zoom*0.5f)/m_half+0.5f;
    pos.y = (pos.y-m_offset.y)*(m_zoom*0.5f)/m_half+0.5f;

    pos.x = m_mapPos.x+m_mapDim.x*pos.x;
    pos.y = m_mapPos.y+m_mapDim.y*pos.y;
    dim.x = 2.0f/128.0f*0.75f;
    dim.y = 2.0f/128.0f;
    dim.x *= 2.0f+cosf(m_time*8.0f)*0.5f;
    dim.y *= 2.0f+cosf(m_time*8.0f)*0.5f;

    m_engine->SetTexture("textures/interface/button2.png");
    m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
    uv1.x = 160.5f/256.0f;  // hilite
    uv1.y = 224.5f/256.0f;
    uv2.x = 175.0f/256.0f;
    uv2.y = 239.0f/256.0f;
    pos.x -= dim.x/2.0f;
    pos.y -= dim.y/2.0f;
    DrawIcon(pos, dim, uv1, uv2);
}

// Draws a triangular icon.

void CMap::DrawTriangle(Math::Point p1, Math::Point p2, Math::Point p3, Math::Point uv1, Math::Point uv2)
{
    Gfx::CDevice* device;
    Gfx::VertexTex2  vertex[3];  // 1 triangle
    Math::Vector    n;

    device = m_engine->GetDevice();

    n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    vertex[0] = Gfx::VertexTex2(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x,uv1.y));
    vertex[1] = Gfx::VertexTex2(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv1.x,uv2.y));
    vertex[2] = Gfx::VertexTex2(Math::Vector(p3.x, p3.y, 0.0f), n, Math::Point(uv2.x,uv2.y));

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLES, vertex, 3);
    m_engine->AddStatisticTriangle(1);
}

// Draw a pentagon icon (a 5 rating, what!).

void CMap::DrawPenta(Math::Point p1, Math::Point p2, Math::Point p3, Math::Point p4, Math::Point p5, Math::Point uv1, Math::Point uv2)
{
    Gfx::CDevice* device;
    Gfx::VertexTex2  vertex[5];  // 1 pentagon
    Math::Vector    n;

    device = m_engine->GetDevice();

    n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    vertex[0] = Gfx::VertexTex2(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x,uv1.y));
    vertex[1] = Gfx::VertexTex2(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv1.x,uv2.y));
    vertex[2] = Gfx::VertexTex2(Math::Vector(p5.x, p5.y, 0.0f), n, Math::Point(uv2.x,uv2.y));
    vertex[3] = Gfx::VertexTex2(Math::Vector(p3.x, p3.y, 0.0f), n, Math::Point(uv2.x,uv2.y));
    vertex[4] = Gfx::VertexTex2(Math::Vector(p4.x, p4.y, 0.0f), n, Math::Point(uv2.x,uv2.y));

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 5);
    m_engine->AddStatisticTriangle(3);
}

// Draw the vertex array.

void CMap::DrawVertex(Math::Point uv1, Math::Point uv2, float zoom)
{
    Gfx::CDevice* device;
    Gfx::VertexTex2  vertex[4];  // 2 triangles
    Math::Point     p1, p2, c;
    Math::Vector    n;

    device = m_engine->GetDevice();

    p1.x = m_pos.x;
    p1.y = m_pos.y;
    p2.x = m_pos.x + m_dim.x;
    p2.y = m_pos.y + m_dim.y;

    c.x = (p1.x+p2.x)/2.0f;
    c.y = (p1.y+p2.y)/2.0f;  // center

    p1.x = (p1.x-c.x)*zoom + c.x;
    p1.y = (p1.y-c.y)*zoom + c.y;

    p2.x = (p2.x-c.x)*zoom + c.x;
    p2.y = (p2.y-c.y)*zoom + c.y;

    m_mapPos   = p1;
    m_mapDim.x = p2.x-p1.x;
    m_mapDim.y = p2.y-p1.y;

    n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    vertex[0] = Gfx::VertexTex2(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x,uv2.y));
    vertex[1] = Gfx::VertexTex2(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(uv1.x,uv1.y));
    vertex[2] = Gfx::VertexTex2(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(uv2.x,uv2.y));
    vertex[3] = Gfx::VertexTex2(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv2.x,uv1.y));

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    m_engine->AddStatisticTriangle(2);
}


// Updates the field in the map.

void CMap::UpdateTerrain()
{
    if (! m_fixImage.empty()) return;  // still image?

    CImage img(Math::IntPoint(256, 256));

    float scale = m_terrain->GetReliefScale();
    float water = m_water->GetLevel();

    Gfx::Color color;
    color.a = 0.0f;

    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            Math::Vector pos;
            pos.x =  (static_cast<float>(x) - 128.0f) * m_half / 128.0f;
            pos.z = -(static_cast<float>(y) - 128.0f) * m_half / 128.0f;
            pos.y = 0.0f;

            float level;

            if ( pos.x >= -m_half && pos.x <= m_half &&
                 pos.z >= -m_half && pos.z <= m_half )
            {
                level = m_terrain->GetFloorLevel(pos, true) / scale;
            }
            else
            {
                level = 1000.0f;
            }

            float intensity = level / 256.0f;
            if (intensity < 0.0f) intensity = 0.0f;
            if (intensity > 1.0f) intensity = 1.0f;

            if (level >= water)  // on water?
            {
                color.r = Math::Norm(m_floorColor.r + (intensity - 0.5f));
                color.g = Math::Norm(m_floorColor.g + (intensity - 0.5f));
                color.b = Math::Norm(m_floorColor.b + (intensity - 0.5f));
            }
            else    // underwater?
            {
                color.r = Math::Norm(m_waterColor.r + (intensity - 0.5f));
                color.g = Math::Norm(m_waterColor.g + (intensity - 0.5f));
                color.b = Math::Norm(m_waterColor.b + (intensity - 0.5f));
            }

            img.SetPixel(Math::IntPoint(x, y), color);
        }
    }

    m_engine->DeleteTexture("interface/map.png");
    m_engine->LoadTexture("textures/interface/map.png", &img);
}

// Updates the field in the map.

void CMap::UpdateTerrain(int bx, int by, int ex, int ey)
{
    Gfx::Color   color;
    Math::Vector        pos;
    float           scale, water, level, intensity;
    int             x, y;

    if (! m_fixImage.empty())  return;  // still image?

    // TODO: map texture manipulation
    return;

    //if ( !m_engine->OpenImage("map.png") )  return;
    //m_engine->LoadImage();

    scale = m_terrain->GetReliefScale();
    water = m_water->GetLevel();
    color.a = 0.0f;

    for ( y=by ; y<ey ; y++ )
    {
        for ( x=bx ; x<ex ; x++ )
        {
            pos.x =  (static_cast<float>(x)-128.0f)*m_half/128.0f;
            pos.z = -(static_cast<float>(y)-128.0f)*m_half/128.0f;
            pos.y = 0.0f;

            if ( pos.x >= -m_half && pos.x <= m_half &&
                 pos.z >= -m_half && pos.z <= m_half )
            {
                level = m_terrain->GetFloorLevel(pos, true)/scale;
            }
            else
            {
                level = 1000.0f;
            }

            intensity = level/256.0f;
            if ( intensity < 0.0f )  intensity = 0.0f;
            if ( intensity > 1.0f )  intensity = 1.0f;

            if ( level > water )  // on water?
            {
                color.r = m_floorColor.r + (intensity-0.5f);
                color.g = m_floorColor.g + (intensity-0.5f);
                color.b = m_floorColor.b + (intensity-0.5f);
            }
            else    // underwater?
            {
                color.r = m_waterColor.r + (intensity-0.5f);
                color.g = m_waterColor.g + (intensity-0.5f);
                color.b = m_waterColor.b + (intensity-0.5f);
            }

            //m_engine->SetDot(x, y, color);
        }
    }

    //m_engine->CopyImage();  // copy the ground drawing
    //m_engine->CloseImage();
}


// Empty all objects.

void CMap::FlushObject()
{
    m_totalFix  = 0;  // object index fixed
    m_totalMove = MAPMAXOBJECT-2;  // moving vehicles index
    m_bRadar = m_main->GetRadar();

    for (int i = 0; i < MAPMAXOBJECT; i++)
    {
        m_map[i].bUsed = false;
    }
}

// Updates an object in the map.

void CMap::UpdateObject(CObject* pObj)
{
    ObjectType      type;
    MapColor        color;
    Math::Vector        pos;
    Math::Point         ppos;
    float           dir;

    if ( !m_bEnable )  return;
    if ( m_totalFix >= m_totalMove )  return;  // full table?

    type = pObj->GetType();
    if ( !pObj->GetDetectable() )  return;
    if ( type != OBJECT_MOTHER   &&
         type != OBJECT_ANT      &&
         type != OBJECT_SPIDER   &&
         type != OBJECT_BEE      &&
         type != OBJECT_WORM     &&
         type != OBJECT_MOBILEtg )
    {
        if (pObj->Implements(ObjectInterfaceType::Controllable) && !dynamic_cast<CControllableObject*>(pObj)->GetSelectable()) return;
    }
    if ( pObj->GetProxyActivate() )  return;
    if (IsObjectBeingTransported(pObj))  return;

    pos  = pObj->GetPosition();
    dir  = -(pObj->GetRotationY()+Math::PI/2.0f);

    if ( m_angle != 0.0f )
    {
        ppos = RotatePoint(m_angle, Math::Point(pos.x, pos.z));
        pos.x = ppos.x;
        pos.z = ppos.y;
        dir += m_angle;
    }

    color = MAPCOLOR_NULL;
    if ( type == OBJECT_BASE )
    {
        color = MAPCOLOR_BASE;
    }
    if ( type == OBJECT_DERRICK  ||
         type == OBJECT_FACTORY  ||
         type == OBJECT_STATION  ||
         type == OBJECT_CONVERT  ||
         type == OBJECT_REPAIR   ||
         type == OBJECT_DESTROYER||
         type == OBJECT_TOWER    ||
         type == OBJECT_RESEARCH ||
         type == OBJECT_RADAR    ||
         type == OBJECT_INFO     ||
         type == OBJECT_ENERGY   ||
         type == OBJECT_LABO     ||
         type == OBJECT_NUCLEAR  ||
         type == OBJECT_PARA     ||
         type == OBJECT_SAFE     ||
         type == OBJECT_HUSTON   ||
         type == OBJECT_TARGET1  ||
         type == OBJECT_START    ||
         type == OBJECT_END      ||  // stationary object?
         type == OBJECT_TEEN28    ||  // bottle?
         type == OBJECT_TEEN34    )   // stone?
    {
        color = MAPCOLOR_FIX;
    }
    if ( type == OBJECT_BBOX ||
         type == OBJECT_KEYa ||
         type == OBJECT_KEYb ||
         type == OBJECT_KEYc ||
         type == OBJECT_KEYd )
    {
        color = MAPCOLOR_BBOX;
    }
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_APOLLO2  )  // moving vehicle?
    {
        color = MAPCOLOR_MOVE;
    }
    if ( type == OBJECT_ANT      ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     ||
         type == OBJECT_SPIDER   )  // mobile enemy?
    {
        color = MAPCOLOR_ALIEN;
    }
    if ( type == OBJECT_WAYPOINT ||
         type == OBJECT_FLAGb    )
    {
        color = MAPCOLOR_WAYPOINTb;
    }
    if ( type == OBJECT_FLAGr )
    {
        color = MAPCOLOR_WAYPOINTr;
    }
    if ( type == OBJECT_FLAGg )
    {
        color = MAPCOLOR_WAYPOINTg;
    }
    if ( type == OBJECT_FLAGy )
    {
        color = MAPCOLOR_WAYPOINTy;
    }
    if ( type == OBJECT_FLAGv )
    {
        color = MAPCOLOR_WAYPOINTv;
    }

    if ( color == MAPCOLOR_NULL )  return;

    /*if (!m_fixImage.empty() && !m_bDebug)  // map with still image?
    {
        if ( (type == OBJECT_TEEN28 ||
              type == OBJECT_TEEN34 ) &&
             m_mode == 0 )  return;

        if ( type != OBJECT_TEEN28 &&
             type != OBJECT_TEEN34 &&
             color != MAPCOLOR_MOVE )  return;
    }*/

    if ( pObj->Implements(ObjectInterfaceType::Controllable) && dynamic_cast<CControllableObject*>(pObj)->GetSelect() )
    {
        m_map[MAPMAXOBJECT-1].type   = type;
        m_map[MAPMAXOBJECT-1].object = pObj;
        m_map[MAPMAXOBJECT-1].color  = color;
        m_map[MAPMAXOBJECT-1].pos.x  = pos.x;
        m_map[MAPMAXOBJECT-1].pos.y  = pos.z;
        m_map[MAPMAXOBJECT-1].dir    = dir;
        m_map[MAPMAXOBJECT-1].bUsed  = true;
    }
    else
    {
        if ( color == MAPCOLOR_BASE ||
             color == MAPCOLOR_FIX  )
        {
            m_map[m_totalFix].type   = type;
            m_map[m_totalFix].object = pObj;
            m_map[m_totalFix].color  = color;
            m_map[m_totalFix].pos.x  = pos.x;
            m_map[m_totalFix].pos.y  = pos.z;
            m_map[m_totalFix].dir    = dir;
            m_map[m_totalFix].bUsed  = true;
            m_totalFix ++;
        }
        else
        {
            m_map[m_totalMove].type   = type;
            m_map[m_totalMove].object = pObj;
            m_map[m_totalMove].color  = color;
            m_map[m_totalMove].pos.x  = pos.x;
            m_map[m_totalMove].pos.y  = pos.z;
            m_map[m_totalMove].dir    = dir;
            m_map[m_totalMove].bUsed  = true;
            m_totalMove --;
        }
    }
}

}
