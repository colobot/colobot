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
 * \file graphics/engine/planet.h
 * \brief Planet rendering - CPlanet class
 */

#pragma once


#include "common/event.h"

#include "math/point.h"

#include <vector>



// Graphics module namespace
namespace Gfx {

class CEngine;

/**
 * \struct Planet
 * \brief Planet texture definition
 */
struct Planet
{
    //! Initial position in degrees
    Math::Point     start;
    //! Current position in degrees
    Math::Point     angle;
    //! Dimensions (0..1)
    float           dim;
    //! Speed
    float           speed;
    //! Direction in the sky
    float           dir;
    //! Name of the texture
    std::string     name;
    //! Texture mapping
    Math::Point     uv1, uv2;

    // TODO: make all textures transparent?
    //! Transparent texture
    bool            transparent;

    Planet()
    {
        dim = speed = dir = 0.0f;
        transparent = false;
    }
};


/**
 * \class CPlanet
 * \brief Planet manager
 *
 * Draws the planets orbiting in the sky.
 *
 * Planets are drawn in 2D mode, at coordinates calculated from camera position.
 */
class CPlanet
{
public:
    CPlanet(CEngine* engine);
    ~CPlanet();

    //! Removes all the planets
    void        Flush();
    //! Management of an event
    bool        EventProcess(const Event &event);
    //! Creates a new planet
    void        Create(int mode, Math::Point start, float dim, float speed, float dir,
                       const std::string& name, Math::Point uv1, Math::Point uv2,
                       bool transparent);
    //! Indicates if there is at least one planet
    bool        PlanetExist();
    //! Load all the textures for the planets
    void        LoadTexture();
    //! Draws all the planets
    void        Draw();
    //@{
    //! Choice of mode
    void        SetMode(int mode);
    int         GetMode();
    //@}

protected:
    //! Makes the planets evolve
    bool        EventFrame(const Event &event);

protected:
    CEngine*     m_engine;

    float                    m_time;
    int                      m_mode;
    std::vector<Planet> m_planet[2];
    bool                     m_planetExist;
};


} // namespace Gfx
