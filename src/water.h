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

// water.h

#ifndef _WATER_H_
#define _WATER_H_


#include "d3dengine.h"
#include "particule.h"


class CInstanceManager;
class CTerrain;
class CSound;



#define MAXWATERLINE    500

typedef struct
{
    short       x, y;       // beginning
    short       len;        // length by x
    float       px1, px2, pz;
}
WaterLine;


#define MAXWATVAPOR     10

typedef struct
{
    BOOL            bUsed;
    ParticuleType   type;
    D3DVECTOR       pos;
    float           delay;
    float           time;
    float           last;
}
WaterVapor;


enum WaterType
{
    WATER_NULL      = 0,    // no water
    WATER_TT        = 1,    // transparent texture
    WATER_TO        = 2,    // opaque texture
    WATER_CT        = 3,    // transparent color
    WATER_CO        = 4,    // opaque color
};


class CWater
{
public:
    CWater(CInstanceManager* iMan, CD3DEngine* engine);
    ~CWater();

    void        SetD3DDevice(LPDIRECT3DDEVICE7 device);
    BOOL        EventProcess(const Event &event);
    void        Flush();
    BOOL        Create(WaterType type1, WaterType type2, const char *filename, D3DCOLORVALUE diffuse, D3DCOLORVALUE ambient, float level, float glint, D3DVECTOR eddy);
    void        DrawBack();
    void        DrawSurf();

    BOOL        SetLevel(float level);
    float       RetLevel();
    float       RetLevel(CObject* object);

    void        SetLava(BOOL bLava);
    BOOL        RetLava();

    void        AdjustEye(D3DVECTOR &eye);

protected:
    BOOL        EventFrame(const Event &event);
    void        LavaFrame(float rTime);
    void        AdjustLevel(D3DVECTOR &pos, D3DVECTOR &norm, FPOINT &uv1, FPOINT &uv2);
    BOOL        RetWater(int x, int y);
    BOOL        CreateLine(int x, int y, int len);

    void        VaporFlush();
    BOOL        VaporCreate(ParticuleType type, D3DVECTOR pos, float delay);
    void        VaporFrame(int i, float rTime);

protected:
    CInstanceManager*   m_iMan;
    CD3DEngine*         m_engine;
    LPDIRECT3DDEVICE7   m_pD3DDevice;
    CTerrain*           m_terrain;
    CParticule*         m_particule;
    CSound*             m_sound;

    WaterType       m_type[2];
    char            m_filename[100];
    float           m_level;        // overall level
    float           m_glint;        // amplitude of reflections
    D3DVECTOR       m_eddy;         // amplitude of swirls
    D3DCOLORVALUE       m_diffuse;      // diffuse color
    D3DCOLORVALUE       m_ambient;      // ambient color
    float           m_time;
    float           m_lastLava;
    int         m_subdiv;

    int         m_brick;        // number of brick*mosaics
    float           m_size;         // size of a item in an brick

    int         m_lineUsed;
    WaterLine       m_line[MAXWATERLINE];

    WaterVapor      m_vapor[MAXWATVAPOR];

    BOOL            m_bDraw;
    BOOL            m_bLava;
    D3DCOLOR        m_color;
};


#endif //_WATER_H_
