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

// mainmap.h

#pragma once

#include "common/event.h"

#include "graphics/core/color.h"

#include "math/point.h"

#include "object/object.h"

#include "ui/interface.h"


namespace Ui {

class CMainMap
{
public:
    CMainMap();
    ~CMainMap();

    void        UpdateMap();
    void        CreateMap();
    void        SetFixImage(const char *filename);
    void        FloorColorMap(Gfx::Color floor, Gfx::Color water);
    void        ShowMap(bool bShow);
    void        DimMap();
    float       GetZoomMap();
    void        ZoomMap(float zoom);
    void        ZoomMap();
    void        MapEnable(bool bEnable);
    bool        GetShowMap();
    bool        GetFixImage();
    CObject*    DetectMap(Math::Point pos, bool &bInMap);
    void        SetHighlight(CObject* pObj);
    void        SetToy(bool bToy);
    void        SetFixParam(float zoom, float ox, float oy, float angle, int mode, bool bDebug);

protected:
    void        CenterMap();

protected:
    CEventQueue*    m_event;
    Gfx::CEngine*   m_engine;
    CInterface*     m_interface;

    int             m_mapMode;
    bool            m_bFixImage;
};

}

