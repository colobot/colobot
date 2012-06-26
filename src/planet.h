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

// planet.h

#ifndef _PLANET_H_
#define _PLANET_H_


#include "struct.h"


class CInstanceManager;
class CD3DEngine;



#define MAXPLANET   10

typedef struct
{
    char        bUsed;      // TRUE -> planet exists
    FPOINT      start;      // initial position in degrees
    FPOINT      angle;      // current position in degrees
    float       dim;        // dimensions (0..1)
    float       speed;      // speed
    float       dir;        // direction in the sky
    char        name[20];   // name of the texture
    FPOINT      uv1, uv2;   // texture mapping
    char        bTGA;       // texture .TGA
}
Planet;




class CPlanet
{
public:
    CPlanet(CInstanceManager* iMan, CD3DEngine* engine);
    ~CPlanet();

    void        Flush();
    BOOL        EventProcess(const Event &event);
    BOOL        Create(int mode, FPOINT start, float dim, float speed, float dir, char *name, FPOINT uv1, FPOINT uv2);
    BOOL        PlanetExist();
    void        LoadTexture();
    void        Draw();
    void        SetMode(int mode);
    int         RetMode();

protected:
    BOOL        EventFrame(const Event &event);

protected:
    CInstanceManager*   m_iMan;
    CD3DEngine*         m_engine;

    float           m_time;
    int             m_mode;
    Planet          m_planet[2][MAXPLANET];
    BOOL            m_bPlanetExist;
};


#endif //_PLANET_H_
