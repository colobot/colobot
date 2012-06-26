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

#ifndef _MAP_H_
#define _MAP_H_


#include "control.h"
#include "struct.h"
#include "object.h"


class CD3DEngine;
class CTerrain;
class CWater;
class CRobotMain;



#define MAPMAXOBJECT    100

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

typedef struct
{
    char        bUsed;
    CObject*    object;
    MapColor    color;
    ObjectType  type;
    FPOINT      pos;
    float       dir;
}
MapObject;



class CMap : public CControl
{
public:
    CMap(CInstanceManager* iMan);
    ~CMap();

    BOOL        Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg);
    BOOL        EventProcess(const Event &event);
    void        Draw();

    void        UpdateTerrain();
    void        UpdateTerrain(int bx, int by, int ex, int ey);

    void        SetFixImage(char *filename);
    BOOL        RetFixImage();

    void        SetOffset(float ox, float oy);
    void        SetAngle(float angle);
    void        SetMode(int mode);
    void        SetToy(BOOL bToy);
    void        SetDebug(BOOL bDebug);

    void        SetZoom(float value);
    float       RetZoom();

    void        SetEnable(BOOL bEnable);
    BOOL        RetEnable();

    void        SetFloorColor(D3DCOLORVALUE color);
    void        SetWaterColor(D3DCOLORVALUE color);

    void        FlushObject();
    void        UpdateObject(CObject* pObj);

    CObject*    DetectObject(FPOINT pos, BOOL &bInMap);
    void        SetHilite(CObject* pObj);

protected:
    FPOINT      AdjustOffset(FPOINT offset);
    void        SelectObject(FPOINT pos);
    FPOINT      MapInter(FPOINT pos, float dir);
    void        DrawFocus(FPOINT pos, float dir, ObjectType type, MapColor color);
    void        DrawObject(FPOINT pos, float dir, ObjectType type, MapColor color, BOOL bSelect, BOOL bHilite);
    void        DrawObjectIcon(FPOINT pos, FPOINT dim, MapColor color, ObjectType type, BOOL bHilite);
    void        DrawHilite(FPOINT pos);
    void        DrawTriangle(FPOINT p1, FPOINT p2, FPOINT p3, FPOINT uv1, FPOINT uv2);
    void        DrawPenta(FPOINT p1, FPOINT p2, FPOINT p3, FPOINT p4, FPOINT p5, FPOINT uv1, FPOINT uv2);
    void        DrawVertex(FPOINT uv1, FPOINT uv2, float zoom);

protected:
    CTerrain*       m_terrain;
    CWater*         m_water;
    CRobotMain*     m_main;

    BOOL            m_bEnable;
    float           m_time;
    float           m_half;
    float           m_zoom;
    FPOINT          m_offset;
    float           m_angle;
    D3DCOLORVALUE   m_floorColor;
    D3DCOLORVALUE   m_waterColor;
    MapObject       m_map[MAPMAXOBJECT];
    int             m_totalFix;
    int             m_totalMove;
    int             m_hiliteRank;
    FPOINT          m_mapPos;
    FPOINT          m_mapDim;
    BOOL            m_bRadar;
    char            m_fixImage[100];
    int             m_mode;
    BOOL            m_bToy;
    BOOL            m_bDebug;
};


#endif //_MAP_H_
