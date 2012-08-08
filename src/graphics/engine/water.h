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

// water.h

#pragma once

#include "graphics/engine/engine.h"
#include "graphics/engine/particle.h"
#include "common/event.h"


class CInstanceManager;
class CSound;


namespace Gfx {

class CTerrain;


const short MAXWATERLINE = 500;

struct WaterLine
{
    short       x, y;       // beginning
    short       len;        // length by x
    float       px1, px2, pz;
};


const short MAXWATVAPOR = 10;

struct WaterVapor
{
    bool            bUsed;
    ParticleType   type;
    Math::Vector        pos;
    float           delay;
    float           time;
    float           last;
};


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
    CWater(CInstanceManager* iMan, Gfx::CEngine* engine);
    ~CWater();

    void        SetDevice(Gfx::CDevice* device);
    bool        EventProcess(const Event &event);
    void        Flush();
    bool        Create(WaterType type1, WaterType type2, const char *filename, Gfx::Color diffuse, Gfx::Color ambient, float level, float glint, Math::Vector eddy);
    void        DrawBack();
    void        DrawSurf();

    bool        SetLevel(float level);
    float       GetLevel();
    float       GetLevel(CObject* object);

    void        SetLava(bool bLava);
    bool        GetLava();

    void        AdjustEye(Math::Vector &eye);

protected:
    bool        EventFrame(const Event &event);
    void        LavaFrame(float rTime);
    void        AdjustLevel(Math::Vector &pos, Math::Vector &norm, Math::Point &uv1, Math::Point &uv2);
    bool        GetWater(int x, int y);
    bool        CreateLine(int x, int y, int len);

    void        VaporFlush();
    bool        VaporCreate(ParticleType type, Math::Vector pos, float delay);
    void        VaporFrame(int i, float rTime);

protected:
    CInstanceManager*   m_iMan;
    CEngine*            m_engine;
    CDevice*            m_device;
    CTerrain*           m_terrain;
    CParticle*          m_particule;
    CSound*             m_sound;

    WaterType       m_type[2];
    char            m_filename[100];
    float           m_level;        // overall level
    float           m_glint;        // amplitude of reflections
    Math::Vector        m_eddy;         // amplitude of swirls
    Gfx::Color      m_diffuse;      // diffuse color
    Gfx::Color      m_ambient;      // ambient color
    float           m_time;
    float           m_lastLava;
    int         m_subdiv;

    int         m_brick;        // number of brick*mosaics
    float           m_size;         // size of a item in an brick

    int         m_lineUsed;
    WaterLine       m_line[MAXWATERLINE];

    WaterVapor      m_vapor[MAXWATVAPOR];

    bool            m_bDraw;
    bool            m_bLava;
    long        m_color;
};

}; // namespace Gfx
