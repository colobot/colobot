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

/**
 * \file graphics/engine/cloud.h
 * \brief Cloud rendering - Gfx::CCloud class
 */

#pragma once

#include "common/event.h"
#include "graphics/core/color.h"
#include "math/point.h"
#include "math/vector.h"

#include <vector>
#include <string>



class CInstanceManager;


namespace Gfx {

class CEngine;
class CTerrain;

/**
 * \struct CloudLine
 * \brief Cloud strip
 */
struct CloudLine
{
    //@{
    //! Beginning (terrain coordinates)
    short       x, y;
    //@}
    //! Length in X direction (terrain coordinates)
    short       len;
    //! X (1, 2) and Z coordinates (world coordinates)
    float       px1, px2, pz;

    CloudLine()
    {
        x = y = 0;
        len = 0;
        px1 = px2 = pz = 0;
    }
};


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
    CCloud(CInstanceManager* iMan, CEngine* engine);
    ~CCloud();

    bool        EventProcess(const Event &event);
    //! Removes all the clouds
    void        Flush();
    //! Creates all areas of cloud
    void        Create(const std::string& fileName, Gfx::Color diffuse, Gfx::Color ambient, float level);
    //! Draw the clouds
    void        Draw();

    //! Modifies the cloud level
    void        SetLevel(float level);
    //! Returns the current level of clouds
    float       GetLevel();

    //! Activate management of clouds
    void        SetEnable(bool enable);
    bool        GetEnable();

protected:
    //! Makes the clouds evolve
    bool        EventFrame(const Event &event);
    //! Adjusts the position to normal, to imitate the clouds at movement
    void        AdjustLevel(Math::Vector &pos, Math::Vector &eye, float deep,
                            Math::Point &uv1, Math::Point &uv2);
    //! Updates the positions, relative to the ground
    void        CreateLine(int x, int y, int len);

protected:
    CInstanceManager* m_iMan;
    Gfx::CEngine*     m_engine;
    Gfx::CTerrain*    m_terrain;

    std::string     m_fileName;
    //! Overall level
    float           m_level;
    //! Feedrate (wind)
    Math::Point     m_speed;
    //! Diffuse color
    Gfx::Color      m_diffuse;
    //! Ambient color
    Gfx::Color      m_ambient;
    float           m_time;
    float           m_lastTest;
    int             m_subdiv;

    //! Wind speed
    Math::Vector    m_wind;
    //! Brick mosaic
    int             m_brick;
    //! Size of a brick element
    float           m_size;

    std::vector<Gfx::CloudLine> m_lines;

    bool            m_enable;
};


}; // namespace Gfx
