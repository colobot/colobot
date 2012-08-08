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

#include "common/event.h"
#include "graphics/engine/particle.h"


class CInstanceManager;
class CSoundInterface;


namespace Gfx {

class CEngine;
class CTerrain;

struct WaterLine
{
    //! Beginning
    short       x, y;
    //! Length by x
    short       len;
    float       px1, px2, pz;

    WaterLine()
    {
        x = y = 0;
        len = 0;
        px1 = px2 = pz = 0.0f;
    }
};

struct WaterVapor
{
    bool              used;
    Gfx::ParticleType type;
    Math::Vector      pos;
    float             delay;
    float             time;
    float             last;

    WaterVapor()
    {
        used = false;
        type = Gfx::PARTIWATER;
        delay = time = last = 0.0f;
    }
};

enum WaterType
{
    //! No water
    WATER_NULL      = 0,
    //! Transparent texture
    WATER_TT        = 1,
    //! Opaque texture
    WATER_TO        = 2,
    //! Transparent color
    WATER_CT        = 3,
    //! Opaque color
    WATER_CO        = 4,
};


class CWater
{
public:
    CWater(CInstanceManager* iMan, Gfx::CEngine* engine);
    ~CWater();

    void        SetDevice(Gfx::CDevice* device);
    bool        EventProcess(const Event &event);
    //! Removes all the water
    void        Flush();
    //! Creates all expanses of water
    void        Create(WaterType type1, WaterType type2, const std::string& fileName,
                       Gfx::Color diffuse, Gfx::Color ambient, float level, float glint, Math::Vector eddy);
    //! Draw the back surface of the water
    void        DrawBack();
    //! Draws the flat surface of the water
    void        DrawSurf();

    //! Changes the level of the water
    void        SetLevel(float level);
    //! Returns the current level of water
    float       GetLevel();
    //! Returns the current level of water for a given object
    float       GetLevel(CObject* object);

    //@{
    //! Management of the mode of lava/water
    void        SetLava(bool lava);
    bool        GetLava();
    //@}

    //! Adjusts the eye of the camera, not to be in the water
    void        AdjustEye(Math::Vector &eye);

protected:
    //! Makes water evolve
    bool        EventFrame(const Event &event);
    //! Makes evolve the steam jets on the lava
    void        LavaFrame(float rTime);
    //! Adjusts the position to normal, to imitate reflections on an expanse of water at rest
    void        AdjustLevel(Math::Vector &pos, Math::Vector &norm, Math::Point &uv1, Math::Point &uv2);
    //! Indicates if there is water in a given position
    bool        GetWater(int x, int y);
    //! Updates the positions, relative to the ground
    void        CreateLine(int x, int y, int len);

    //! Removes all the steam jets
    void        VaporFlush();
    //! Creates a new steam
    bool        VaporCreate(ParticleType type, Math::Vector pos, float delay);
    //! Makes evolve a steam jet
    void        VaporFrame(int i, float rTime);

protected:
    CInstanceManager* m_iMan;
    Gfx::CEngine*     m_engine;
    Gfx::CDevice*     m_device;
    Gfx::CTerrain*    m_terrain;
    Gfx::CParticle*   m_particule;
    CSoundInterface*  m_sound;

    WaterType       m_type[2];
    std::string     m_fileName;
    //! Overall level
    float           m_level;
    //! Amplitude of reflections
    float           m_glint;
    //! Amplitude of swirls
    Math::Vector    m_eddy;
    //! Diffuse color
    Gfx::Color      m_diffuse;
    //! Ambient color
    Gfx::Color      m_ambient;
    float           m_time;
    float           m_lastLava;
    int             m_subdiv;

    //! Number of brick*mosaics
    int             m_brick;
    //! Size of a item in an brick
    float           m_size;

    std::vector<WaterLine> m_line;
    std::vector<WaterVapor> m_vapor;

    bool            m_draw;
    bool            m_lava;
    long            m_color;
};

}; // namespace Gfx
