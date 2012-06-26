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

// planet.h

#pragma once

#include "common/event.h"
#include "math/point.h"


class CInstanceManager;


namespace Gfx {

class CEngine;


const short MAXPLANET = 10;

struct Planet
{
    char        bUsed;      // TRUE -> planet exists
    Math::Point     start;      // initial position in degrees
    Math::Point     angle;      // current position in degrees
    float       dim;        // dimensions (0..1)
    float       speed;      // speed
    float       dir;        // direction in the sky
    char        name[20];   // name of the texture
    Math::Point     uv1, uv2;   // texture mapping
    char        bTGA;       // texture .TGA
};




class CPlanet {
public:
    CPlanet(CInstanceManager* iMan, CEngine* engine);
    ~CPlanet();

    void        Flush();
    bool        EventProcess(const Event &event);
    bool        Create(int mode, Math::Point start, float dim, float speed, float dir, char *name, Math::Point uv1, Math::Point uv2);
    bool        PlanetExist();
    void        LoadTexture();
    void        Draw();
    void        SetMode(int mode);
    int         RetMode();

protected:
    bool        EventFrame(const Event &event);

protected:
    CInstanceManager*   m_iMan;
    CEngine*        m_engine;

    float           m_time;
    int             m_mode;
    Planet          m_planet[2][MAXPLANET];
    bool            m_bPlanetExist;
};


}; // namespace Gfx
