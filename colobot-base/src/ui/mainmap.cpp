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


#include "ui/mainmap.h"

#include "app/app.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "ui/controls/interface.h"
#include "ui/controls/map.h"
#include "ui/controls/scroll.h"
#include "ui/controls/slider.h"
#include "ui/controls/window.h"


namespace Ui
{


const float ZOOM_MIN = 1.0f;
const float ZOOM_MAX = 16.0f;


// Constructor of the application card.

CMainMap::CMainMap()
{
    m_interface = CRobotMain::GetInstancePointer()->GetInterface();
    m_event     = CApplication::GetInstancePointer()->GetEventQueue();
    m_engine    = Gfx::CEngine::GetInstancePointer();

    m_bFixImage = false;
}

// Destructor of the application card.

CMainMap::~CMainMap()
{
}


// Created the mini-map and the corresponding buttons.

void CMainMap::CreateMap()
{
    CWindow*    pw;
    glm::vec2   pos, dim;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
    {
        pos.x = 0.0f;
        pos.y = 0.0f;
        dim.x = 0.0f;
        dim.y = 0.0f;
        pw = m_interface->CreateWindows(pos, dim, 10, EVENT_WINDOW1);
    }

    dim.x = 10.0f / 640.0f;
    dim.y = 10.0f / 480.0f;
    pos.x = 10.0f / 640.0f;
    pos.y = 10.0f / 480.0f;
    pw->CreateMap   (pos, dim, 2, EVENT_OBJECT_MAP);
    pw->CreateSlider(pos, dim, 0, EVENT_OBJECT_MAPZOOM);

    DimMap();
}

// Indicates whether the mini-map should display a still image.

void CMainMap::SetFixImage(const std::filesystem::path& filename)
{
    CWindow*    pw;
    CMap*       pm;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;

    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm == nullptr)
        return;

    pw->DeleteControl(EVENT_OBJECT_MAPZOOM);
    m_bFixImage = true;

    pm->SetFixImage(filename);
}

// Choosing colors of soil and water for the mini-map.

void CMainMap::FloorColorMap(Gfx::Color floor, Gfx::Color water)
{
    CWindow*    pw;
    CMap*       pm;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;

    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm != nullptr)
    {
        pm->SetFloorColor(floor);
        pm->SetWaterColor(water);
    }
}

// Shows or hides the minimap.

void CMainMap::ShowMap(bool bShow)
{
    CWindow*    pw;
    CMap*       pm;
    CSlider*    ps;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;


    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm != nullptr)
        pm->SetState(STATE_VISIBLE, bShow);

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_OBJECT_MAPZOOM));
    if (ps != nullptr)
        ps->SetState(STATE_VISIBLE, bShow);

    if (bShow)
    {
        DimMap();
    }
}

// Dimensions of the mini-map.

void CMainMap::DimMap()
{
    CWindow*    pw;
    CMap*       pm;
    CSlider*    ps;
    glm::vec2   pos, dim;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;
    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm == nullptr)
        return;

    dim.x = 100.0f/640.0f;
    dim.y = 100.0f/480.0f;
    pos.x = 540.0f/640.0f;
    pos.y =   0.0f/480.0f;
    pm->SetPos(pos);
    pm->SetDim(dim);

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_OBJECT_MAPZOOM));
    if (ps != nullptr)
    {
        dim.x = SCROLL_WIDTH;
        dim.y =  66.0f / 480.0f;
        pos.x = 523.0f / 640.0f;
        pos.y =   3.0f / 480.0f;
        ps->SetPos(pos);
        ps->SetDim(dim);

        value = pm->GetZoom();
        value = (value-ZOOM_MIN) / (ZOOM_MAX-ZOOM_MIN);
        value = powf(value, 0.5f);
        ps->SetVisibleValue(value);
        ps->SetArrowStep(0.2f);
    }
}

// Returns the current zoom of the minimap.

float CMainMap::GetZoomMap()
{
    CWindow*    pw;
    CMap*       pm;
    CSlider*    ps;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return ZOOM_MIN;

    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm == nullptr)
        return ZOOM_MIN;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_OBJECT_MAPZOOM));
    if (ps == nullptr)
        return ZOOM_MIN;

    return pm->GetZoom();
}

// Zoom the mini-map of any factor.

void CMainMap::ZoomMap(float zoom)
{
    CWindow*    pw;
    CMap*       pm;
    CSlider*    ps;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;
    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm == nullptr)
        return;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_OBJECT_MAPZOOM));
    if (ps == nullptr)
        return;

    if (zoom < ZOOM_MIN)
        zoom = ZOOM_MIN;
    if (zoom > ZOOM_MAX)
        zoom = ZOOM_MAX;
    pm->SetZoom(zoom);

    DimMap();
}

// The mini-map zoom depending on the slider.

void CMainMap::ZoomMap()
{
    CWindow*    pw;
    CMap*       pm;
    CSlider*    ps;
    float       zoom;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;
    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm == nullptr)
        return;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_OBJECT_MAPZOOM));
    if (ps == nullptr)
        return;


    zoom = ps->GetVisibleValue();
    zoom = powf(zoom, 2.0f);
    zoom = ZOOM_MIN+zoom*(ZOOM_MAX - ZOOM_MIN);
    pm->SetZoom(zoom);

    DimMap();
}

// Enables or disables the card.

void CMainMap::MapEnable(bool bEnable)
{
    CWindow*    pw;
    CMap*       pm;
    CSlider*    ps;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;

    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm != nullptr)
        pm->SetEnable(bEnable);

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_OBJECT_MAPZOOM));
    if (ps != nullptr)
        ps->SetState(STATE_ENABLE, bEnable);
}

// Specifies the type of icon for the selected object.

void CMainMap::SetToy(bool bToy)
{
    CWindow*    pw;
    CMap*       pm;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;

    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm == nullptr)
        return;

    pm->SetToy(bToy);
}

// Specifies the parameters when using a still image.

void CMainMap::SetFixParam(float zoom, float ox, float oy, float angle,
                           int mode, bool bDebug)
{
    CWindow*    pw;
    CMap*       pm;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;

    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm == nullptr)
        return;

    pm->SetZoom(zoom);
    pm->SetOffset(ox, oy);
    pm->SetAngle(angle);
    pm->SetMode(mode);
    pm->SetDebug(bDebug);
}

// Updates the mini-map following to a change of terrain.

void CMainMap::UpdateMap()
{
    CWindow*    pw;
    CMap*       pm;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;

    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm != nullptr)
        pm->UpdateTerrain();
}

// Indicates whether the mini-map displays a still image.

bool CMainMap::GetFixImage()
{
    return m_bFixImage;
}


// The object is detected in the mini-map.

CObject* CMainMap::DetectMap(const glm::vec2& pos, bool &bInMap)
{
    CWindow*    pw;
    CMap*       pm;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return nullptr;

    bInMap = false;
    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm == nullptr)
        return nullptr;
    return pm->DetectObject(pos, bInMap);
}


// Indicates the object with the mouse hovers over.

void CMainMap::SetHighlight(CObject* pObj)
{
    CWindow* pw;
    CMap*   pm;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if (pw == nullptr)
        return;

    pm = static_cast<CMap*>(pw->SearchControl(EVENT_OBJECT_MAP));
    if (pm != nullptr)
        pm->SetHighlight(pObj);
}


}
