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

/**
 * \file graphics/engine/water.h
 * \brief Water rendering - CWater class
 */

#pragma once

#include "graphics/engine/particle.h"
#include <filesystem>


class CSoundInterface;
struct Event;


// Graphics module namespace
namespace Gfx
{

class CEngine;
class CTerrain;

/**
 * \enum WaterType
 * \brief Mode of water display
 */
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

/**
 * \class CWater
 * \brief Water manager/renderer
 *
 * Water is drawn where the terrain is below specified level. The mapping
 * is based on terrain coordinates - for each "brick" coordinate, the level
 * of terrain is tested. For every Y coordinate, many lines in X direction
 * are created (WaterLines).
 *
 * There are two parts of drawing process: drawing the background image
 * blocking the normal sky layer and drawing the surface of water.
 * The surface is drawn with texture, so with proper texture it can be lava.
 */
class CWater
{
public:
    CWater(CEngine* engine);
    virtual ~CWater();

    bool        EventProcess(const Event &event);
    //! Removes all the water
    void        Flush();
    //! Creates all expanses of water
    void        Create(WaterType type1, WaterType type2, const std::filesystem::path& fileName,
                       Color diffuse, Color ambient, float level, float glint, glm::vec3 eddy);
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
    void        AdjustEye(glm::vec3 &eye);

protected:
    //! Makes water evolve
    bool        EventFrame(const Event &event);
    //! Makes evolve the steam jets on the lava
    void        LavaFrame(float rTime);
    //! Adjusts the position to normal, to imitate reflections on an expanse of water at rest
    void        AdjustLevel(glm::vec3 &pos, glm::vec3 &norm, glm::vec2& uv1, glm::vec2& uv2);
    //! Indicates if there is water in a given position
    bool        GetWater(int x, int y);
    //! Updates the positions, relative to the ground
    void        CreateLine(int x, int y, int len);

    //! Removes all the steam jets
    void        VaporFlush();
    //! Creates a new steam
    bool        VaporCreate(ParticleType type, glm::vec3 pos, float delay);
    //! Makes evolve a steam jet
    void        VaporFrame(int i, float rTime);

protected:
    CEngine*          m_engine = nullptr;
    CDevice*          m_device = nullptr;
    CTerrain*         m_terrain = nullptr;
    CParticle*        m_particle = nullptr;
    CSoundInterface*  m_sound = nullptr;

    WaterType       m_type[2] = {};
    std::filesystem::path m_fileName;
    //! Overall level
    float           m_level = 0.0f;
    //! Amplitude of reflections
    float           m_glint = 0.0f;
    //! Amplitude of swirls
    glm::vec3       m_eddy = { 0, 0, 0 };
    //! Diffuse color
    Color           m_diffuse;
    //! Ambient color
    Color           m_ambient;
    float           m_time = 0.0f;
    float           m_lastLava = 0.0f;
    int             m_subdiv = 4;

    //! Number of brick*mosaics
    int             m_brickCount = 0;
    //! Size of a item in an brick
    float           m_brickSize = 0;

    /**
     * \struct WaterLine
     * \brief Water strip
     */
    struct WaterLine
    {
        //@{
        //! Beginning of line (terrain coordinates)
        short       x = 0, y = 0;
        //@}
        //! Length in X direction (terrain coordinates)
        short       len = 0;
        //! X (1, 2) and Z coordinates (world coordinates)
        float       px1 = 0, px2 = 0, pz = 0;
    };
    std::vector<WaterLine>  m_lines;

    /**
     * \struct WaterVapor
     * \brief Water particle effect
     */
    struct WaterVapor
    {
        bool              used = false;
        ParticleType type = PARTIWATER;
        glm::vec3         pos = { 0, 0, 0 };
        float             delay = 0.0f;
        float             time = 0.0f;
        float             last = 0.0f;
    };
    std::vector<WaterVapor> m_vapors;

    bool            m_draw = true;
    bool            m_lava = false;
    Color           m_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
};


} // namespace Gfx

