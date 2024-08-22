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
 * \file graphics/engine/cloud.h
 * \brief Cloud rendering - CCloud class
 */

#pragma once

#include "graphics/core/color.h"

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <filesystem>


struct Event;

// Graphics module namespace
namespace Gfx
{

class CEngine;
class CTerrain;

/**
 * \class CCloud
 * \brief Cloud layer renderer
 *
 * Renders the cloud layer as fog. Cloud layer is similar to water layer
 * - it occurs only at specified level of terrain. Cloud map is created
 * the same way water is created. CloudLine structs are used to specify
 * lines in X direction in XY terrain coordinates.
 */
class CCloud
{
public:
    CCloud(CEngine* engine);
    ~CCloud();

    bool        EventProcess(const Event& event);
    //! Removes all the clouds
    void        Flush();

    //! Creates all areas of cloud
    void        Create(const std::filesystem::path& fileName, const Color& diffuse, const Color& ambient, float level);
    //! Draw the clouds
    void        Draw();

    //! Management of cloud level
    //@{
    void        SetLevel(float level);
    float       GetLevel();
    //@}

    //! Management of clouds
    //@{
    void        SetEnabled(bool enabled);
    bool        GetEnabled();
    //@}

protected:
    //! Makes the clouds evolve
    bool        EventFrame(const Event &event);
    //! Adjusts the position to normal, to imitate the clouds at movement
    void        AdjustLevel(glm::vec3& pos, glm::vec3& eye, float deep,
                            glm::vec2& uv1, glm::vec2& uv2);
    //! Updates the positions, relative to the ground
    void        CreateLine(int x, int y, int len);

protected:
    CEngine*        m_engine = nullptr;
    CTerrain*       m_terrain = nullptr;

    bool            m_enabled = true;
    //! Overall level
    float           m_level = 0.0f;
    //! Texture
    std::filesystem::path m_fileName;
    //! Feedrate (wind)
    glm::vec2       m_speed;
    //! Diffuse color
    Color           m_diffuse;
    //! Ambient color
    Color           m_ambient;
    float           m_time = 0.0f;
    float           m_lastTest = 0.0f;
    int             m_subdiv = 8;

    //! Wind speed
    glm::vec3    m_wind{ 0, 0, 0 };
    //! Brick mosaic
    int             m_brickCount = 0;
    //! Size of a brick element
    float           m_brickSize = 0;

    /**
     * \struct CloudLine
     * \brief Cloud strip
     */
    struct CloudLine
    {
        //@{
        //! Beginning (terrain coordinates)
        short       x = 0, y = 0;
        //@}
        //! Length in X direction (terrain coordinates)
        short       len = 0;
        //! X (1, 2) and Z coordinates (world coordinates)
        float       px1 = 0, px2 = 0, pz = 0;
    };
    std::vector<CloudLine> m_lines;
};


} // namespace Gfx

