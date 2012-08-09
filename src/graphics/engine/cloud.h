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

// cloud.h

#pragma once

#include "common/event.h"
#include "graphics/core/color.h"
#include "math/point.h"
#include "math/vector.h"



class CInstanceManager;


namespace Gfx {

class CEngine;
class CTerrain;

const short MAXCLOUDLINE = 100;

struct CloudLine
{
    short       x, y;       // beginning
    short       len;        // in length x
    float       px1, px2, pz;
};


class CCloud
{
public:
    CCloud(CInstanceManager* iMan, CEngine* engine);
    ~CCloud();

    bool        EventProcess(const Event &event);
    void        Flush();
    bool        Create(const char *filename, Gfx::Color diffuse, Gfx::Color ambient, float level);
    void        Draw();

    bool        SetLevel(float level);
    float       RetLevel();

    void        SetEnable(bool bEnable);
    bool        RetEnable();

protected:
    bool        EventFrame(const Event &event);
    void        AdjustLevel(Math::Vector &pos, Math::Vector &eye, float deep, Math::Point &uv1, Math::Point &uv2);
    bool        CreateLine(int x, int y, int len);

protected:
    CInstanceManager*   m_iMan;
    CEngine*        m_engine;
    CTerrain*       m_terrain;

    char            m_filename[100];
    float           m_level;        // overall level
    Math::Point         m_speed;        // feedrate (wind)
    Gfx::Color      m_diffuse;      // diffuse color
    Gfx::Color      m_ambient;      // ambient color
    float           m_time;
    float           m_lastTest;
    int             m_subdiv;

    Math::Vector        m_wind;         // wind speed
    int         m_brick;        // brick mosaic
    float           m_size;         // size of a brick element

    int         m_lineUsed;
    CloudLine       m_line[MAXCLOUDLINE];

    bool            m_bEnable;
};


}; // namespace Gfx
