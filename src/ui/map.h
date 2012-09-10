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

// map.h

#pragma once


#include <ui/control.h>

#include <object/object.h>

#include <common/event.h>

class CTerrain;
class CWater;

const int MAPMAXOBJECT = 100;

enum MapColor
{
    MAPCOLOR_NULL,
    MAPCOLOR_BASE,
    MAPCOLOR_FIX,
    MAPCOLOR_MOVE,
    MAPCOLOR_ALIEN,
    MAPCOLOR_WAYPOINTb,
    MAPCOLOR_WAYPOINTr,
    MAPCOLOR_WAYPOINTg,
    MAPCOLOR_WAYPOINTy,
    MAPCOLOR_WAYPOINTv,
    MAPCOLOR_BBOX,
};

struct MapObject
{
    char        bUsed;
    CObject*    object;
    MapColor    color;
    ObjectType  type;
    Math::Point pos;
    float       dir;
};



class CMap : public CControl
{
public:
    CMap();
    ~CMap();

    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    bool        EventProcess(const Event &event);
    void        Draw();

    void        UpdateTerrain();
    void        UpdateTerrain(int bx, int by, int ex, int ey);

    void        SetFixImage(char *filename);
    bool        RetFixImage();

    void        SetOffset(float ox, float oy);
    void        SetAngle(float angle);
    void        SetMode(int mode);
    void        SetToy(bool bToy);
    void        SetDebug(bool bDebug);

    void        SetZoom(float value);
    float       RetZoom();

    void        SetEnable(bool bEnable);
    bool        RetEnable();

    void        SetFloorColor(Gfx::Color color);
    void        SetWaterColor(Gfx::Color color);

    void        FlushObject();
    void        UpdateObject(CObject* pObj);

    CObject*    DetectObject(Math::Point pos, bool &bInMap);
    void        SetHilite(CObject* pObj);

protected:
    Math::Point AdjustOffset(Math::Point offset);
    void        SelectObject(Math::Point pos);
    Math::Point MapInter(Math::Point pos, float dir);
    void        DrawFocus(Math::Point pos, float dir, ObjectType type, MapColor color);
    void        DrawObject(Math::Point pos, float dir, ObjectType type, MapColor color, bool bSelect, bool bHilite);
    void        DrawObjectIcon(Math::Point pos, Math::Point dim, MapColor color, ObjectType type, bool bHilite);
    void        DrawHilite(Math::Point pos);
    void        DrawTriangle(Math::Point p1, Math::Point p2, Math::Point p3, Math::Point uv1, Math::Point uv2);
    void        DrawPenta(Math::Point p1, Math::Point p2, Math::Point p3, Math::Point p4, Math::Point p5, Math::Point uv1, Math::Point uv2);
    void        DrawVertex(Math::Point uv1, Math::Point uv2, float zoom);

protected:
    CTerrain*       m_terrain;
    CWater*         m_water;
    CRobotMain*     m_main;

    bool            m_bEnable;
    float           m_time;
    float           m_half;
    float           m_zoom;
    Math::Point     m_offset;
    float           m_angle;
    Gfx::Color      m_floorColor;
    Gfx::Color      m_waterColor;
    MapObject       m_map[MAPMAXOBJECT];
    int             m_totalFix;
    int             m_totalMove;
    int             m_hiliteRank;
    Math::Point     m_mapPos;
    Math::Point     m_mapDim;
    bool            m_bRadar;
    char            m_fixImage[100];
    int             m_mode;
    bool            m_bToy;
    bool            m_bDebug;
};


