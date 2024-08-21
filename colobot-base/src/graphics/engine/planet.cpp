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


#include "graphics/engine/planet.h"

#include "common/event.h"
#include "common/stringutils.h"

#include "graphics/core/device.h"
#include "graphics/core/renderers.h"
#include "graphics/core/transparency.h"

#include "graphics/engine/engine.h"

#include "math/func.h"


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
        m_engine->LoadTexture(TempToPath(planet.name));
    }
}

void CPlanet::Draw()
{
    float eyeDirH = m_engine->GetEyeDirH();
    float eyeDirV = m_engine->GetEyeDirV();

    auto renderer = m_engine->GetDevice()->GetObjectRenderer();

    float dp = 0.5f/256.0f;

    auto windowSize = m_engine->GetWindowSize();
    float inverseAspectRatio = static_cast<float>(windowSize.y) / static_cast<float>(windowSize.x);

    for (const auto& planet : m_planets)
    {
        if (planet.type != m_visibleType)
            continue;

        auto texture = m_engine->LoadTexture(TempToPath(planet.name));

        renderer->SetAlbedoTexture(texture);

        if (planet.transparent)
            renderer->SetTransparency(TransparencyMode::ALPHA);
        else
            renderer->SetTransparency(TransparencyMode::BLACK);

        glm::vec2 p1{}, p2{};

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

        // planet.dim is what percentage of the screen height the planet takes up (e.g. 0.333 = 1/3 of screen height)
        // and then the width is calculated to make it square. (0.333 = 1/4 of screen width assuming 4:3)
        // This matches the behaviour of the 3D scene - the vertical FOV is fixed, and the horizontal FOV changes
        // to match the aspect ratio.

        p1.x -= planet.dim/2.0f*inverseAspectRatio;
        p1.y -= planet.dim/2.0f;
        p2.x = p1.x+planet.dim*inverseAspectRatio;
        p2.y = p1.y+planet.dim;

        float u1 = planet.uv1.x + dp;
        float v1 = planet.uv1.y + dp;
        float u2 = planet.uv2.x - dp;
        float v2 = planet.uv2.y - dp;

        glm::u8vec4 white(255);

        Vertex3D quad[4] =
        {
            { glm::vec3(p1.x, p1.y, 0.0f), white, { u1, v2 } },
            { glm::vec3(p1.x, p2.y, 0.0f), white, { u1, v1 } },
            { glm::vec3(p2.x, p1.y, 0.0f), white, { u2, v2 } },
            { glm::vec3(p2.x, p2.y, 0.0f), white, { u2, v1 } }
        };

        renderer->SetAlbedoColor(Color{ 1.0f, 1.0f, 1.0f, 1.0f });

        renderer->DrawPrimitive(PrimitiveType::TRIANGLE_STRIP, 4, quad);
        m_engine->AddStatisticTriangle(2);
    }
}

void CPlanet::Create(PlanetType type, const glm::vec2& start, float dim, float speed,
                     float dir, const std::string& name, const glm::vec2& uv1, const glm::vec2& uv2,
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
