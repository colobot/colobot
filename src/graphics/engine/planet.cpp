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


#include "graphics/engine/planet.h"

#include "common/event.h"

#include "graphics/core/device.h"

#include "graphics/engine/engine.h"


// Graphics module namespace
namespace Gfx
{

namespace
{
const int PLANET_PREALLOCATE_COUNT = 20;
} // anonymous namespace


CPlanet::CPlanet(CEngine* engine)
    : m_engine(engine)
{
    m_planets.reserve(PLANET_PREALLOCATE_COUNT);
}

CPlanet::~CPlanet()
{
}

void CPlanet::Flush()
{
    m_planets.clear();
    m_visibleType = PlanetType::Sky;
    m_time = 0.0f;
}

bool CPlanet::EventProcess(const Event &event)
{
    if (event.type == EVENT_FRAME)
        return EventFrame(event);

    return true;
}

bool CPlanet::EventFrame(const Event &event)
{
    if (m_engine->GetPause()) return true;

    m_time += event.rTime;

    for (auto& planet : m_planets)
    {
        if (planet.type != m_visibleType)
            continue;

        float a = m_time * planet.speed;
        if (a < 0.0f)
            a += Math::PI*1000.0f;

        planet.angle.x = a + planet.start.x;
        planet.angle.y = sinf(a) * sinf(planet.dir) + planet.start.y;
    }

    return true;
}

void CPlanet::LoadTexture()
{
    for (const auto& planet : m_planets)
    {
        m_engine->LoadTexture(planet.name);
    }
}

void CPlanet::Draw()
{
    CDevice* device = m_engine->GetDevice();
    float eyeDirH = m_engine->GetEyeDirH();
    float eyeDirV = m_engine->GetEyeDirV();

    Math::Vector n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal
    float dp = 0.5f/256.0f;

    for (const auto& planet : m_planets)
    {
        if (planet.type != m_visibleType)
            continue;

        m_engine->SetTexture(planet.name);

        if (planet.transparent)
            m_engine->SetState(ENG_RSTATE_WRAP | ENG_RSTATE_ALPHA);
        else
            m_engine->SetState(ENG_RSTATE_WRAP | ENG_RSTATE_TTEXTURE_BLACK);

        Math::Point p1, p2;

        // Determine the 2D coordinates of the centre of the planet.

        // Not sure why this is + when you'd expect -. Perhaps one of the angles is inverted.
        // Compute the camera-relative angles. (0, 0) is straight ahead (the dead centre of the screen).

        // Why -1.0f? Simply because the old formula included that, and we need it to
        // be consistent for the outer space cutscenes to work.
        float a = planet.angle.x + eyeDirH - 1.0f;
        a = Math::Mod(a+Math::PI, Math::PI*2.0f)-Math::PI; // normalize to -pi <= a < pi
        p1.x = a/m_engine->GetHFovAngle() + 0.5f;

        a = eyeDirV + planet.angle.y;
        p1.y = 0.4f+(Math::Mod(a+Math::PI, Math::PI*2.0f)-Math::PI)*(2.0f/Math::PI);

        p1.x -= planet.dim/2.0f*0.75f;
        p1.y -= planet.dim/2.0f;
        p2.x = p1.x+planet.dim*0.75f;
        p2.y = p1.y+planet.dim;

        float u1 = planet.uv1.x + dp;
        float v1 = planet.uv1.y + dp;
        float u2 = planet.uv2.x - dp;
        float v2 = planet.uv2.y - dp;

        Vertex quad[4] =
        {
            Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(u1, v2)),
            Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(u1, v1)),
            Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(u2, v2)),
            Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(u2, v1))
        };

        device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, quad, 4);
        m_engine->AddStatisticTriangle(2);
    }
}

void CPlanet::Create(PlanetType type, Math::Point start, float dim, float speed,
                     float dir, const std::string& name, Math::Point uv1, Math::Point uv2,
                     bool transparent)
{
    Planet planet;

    planet.type = type;
    planet.start = start;
    planet.angle = start;
    planet.dim   = dim;
    planet.speed = speed;
    planet.dir   = dir;

    planet.name = name;
    planet.uv1   = uv1;
    planet.uv2   = uv2;

    planet.transparent = transparent;

    m_planets.push_back(planet);
}

bool CPlanet::PlanetExist()
{
    return !m_planets.empty();
}

void CPlanet::SetVisiblePlanetType(PlanetType type)
{
    m_visibleType = type;
}

} // namespace Gfx
