// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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

// mainmap.cpp


#include <ui/mainmap.h>


namespace Ui {


const float ZOOM_MIN = 1.0f;
const float ZOOM_MAX = 16.0f;


// Constructor of the application card.

CMainMap::CMainMap()
{
    m_iMan = CInstanceManager::GetInstancePointer();
    m_iMan->AddInstance(CLASS_MAP, this);

    m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
    m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);

    m_mapMode = 1;
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
    Math::Point     pos, dim;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr) {
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

void CMainMap::SetFixImage(const char *filename)
{
    CWindow*    pw;
    CMap*       pm;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;

    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;

    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm != nullptr) {
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;

    if (bShow) {
        DimMap();
    } else {
        pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
        if (pm != nullptr)
            pm->ClearState(STATE_VISIBLE);

        ps = (CSlider*)pw->SearchControl(EVENT_OBJECT_MAPZOOM);
        if (ps != nullptr)
            ps->ClearState(STATE_VISIBLE);
    }
}

// Dimensions of the mini-map.

void CMainMap::DimMap()
{
    CWindow*    pw;
    CMap*       pm;
    CSlider*    ps;
    Math::Point     pos, dim;
    float       value;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;
    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm == nullptr)
        return;

    pm->SetState(STATE_VISIBLE, (m_mapMode != 0));

    dim.x = 100.0f/640.0f;
    dim.y = 100.0f/480.0f;
    pos.x = 540.0f/640.0f;
    pos.y =   0.0f/480.0f;
    pm->SetPos(pos);
    pm->SetDim(dim);

    ps = (CSlider*)pw->SearchControl(EVENT_OBJECT_MAPZOOM);
    if (ps != nullptr) {
        ps->SetState(STATE_VISIBLE, (m_mapMode != 0));

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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return ZOOM_MIN;

    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm == nullptr)
        return ZOOM_MIN;

    ps = (CSlider*)pw->SearchControl(EVENT_OBJECT_MAPZOOM);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;
    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm == nullptr)
        return;

    ps = (CSlider*)pw->SearchControl(EVENT_OBJECT_MAPZOOM);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;
    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm == nullptr)
        return;

    ps = (CSlider*)pw->SearchControl(EVENT_OBJECT_MAPZOOM);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;

    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm != nullptr)
        pm->SetEnable(bEnable);

    ps = (CSlider*)pw->SearchControl(EVENT_OBJECT_MAPZOOM);
    if (ps != nullptr)
        ps->SetState(STATE_ENABLE, bEnable);
}

// Specifies the type of icon for the selected object.

void CMainMap::SetToy(bool bToy)
{
    CWindow*    pw;
    CMap*       pm;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;

    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;

    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
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

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;

    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm != nullptr)
        pm->UpdateTerrain();
}

// Indicates if the mini-map is visible.

bool CMainMap::GetShowMap()
{
    return ( m_mapMode != 0 );
}

// Indicates whether the mini-map displays a still image.

bool CMainMap::GetFixImage()
{
    return m_bFixImage;
}


// The object is detected in the mini-map.

CObject* CMainMap::DetectMap(Math::Point pos, bool &bInMap)
{
    CWindow*    pw;
    CMap*       pm;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return 0;

    bInMap = false;
    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm == nullptr)
        return 0;
    return pm->DetectObject(pos, bInMap);
}


// Indicates the object with the mouse hovers over.

void CMainMap::SetHighlight(CObject* pObj)
{
    CWindow* pw;
    CMap*   pm;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW1);
    if (pw == nullptr)
        return;

    pm = (CMap*)pw->SearchControl(EVENT_OBJECT_MAP);
    if (pm != nullptr)
        pm->SetHighlight(pObj);
}


}
