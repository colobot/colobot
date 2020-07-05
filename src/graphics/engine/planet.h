/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file graphics/engine/planet.h
 * \brief Planet rendering - CPlanet class
 */

#pragma once

#include "graphics/engine/planet_type.h"

#include "math/point.h"

#include <vector>

struct Event;

// Graphics module namespace
namespace Gfx
{

class CEngine;

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
    void        Create(PlanetType type, Math::Point start, float dim, float speed, float dir,
                       const std::string& name, Math::Point uv1, Math::Point uv2,
                       bool transparent);
    //! Indicates if there is at least one planet
    bool        PlanetExist();
    //! Load all the textures for the planets
    void        LoadTexture();
    //! Draws all the planets
    void        Draw();

    //! Set which planet types to display
    void        SetVisiblePlanetType(PlanetType type);

protected:
    //! Makes the planets evolve
    bool        EventFrame(const Event &event);

protected:
    CEngine* m_engine = nullptr;
    float m_time = 0.0f;
    PlanetType m_visibleType = PlanetType::Sky;

    /**
    * \struct Planet
    * \brief Planet texture definition
    */
    struct Planet
    {
        //! Type of planet
        PlanetType      type = PlanetType::Sky;
        //! Initial position in degrees
        Math::Point     start;
        //! Current position in degrees
        Math::Point     angle;
        //! Dimensions (0..1)
        float           dim = 0.0f;
        //! Speed
        float           speed = 0.0f;
        //! Direction in the sky
        float           dir = 0.0f;
        //! Name of the texture
        std::string     name;
        //! Texture mapping
        Math::Point     uv1, uv2;

        // TODO: make all textures transparent?
        //! Transparent texture
        bool            transparent = false;
    };
    std::vector<Planet> m_planets;
};


} // namespace Gfx

