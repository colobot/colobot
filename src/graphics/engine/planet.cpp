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


#include "graphics/engine/planet.h"

#include "graphics/core/device.h"
#include "graphics/engine/engine.h"


// Graphics module namespace
namespace Gfx {


const int PLANET_PREALLOCATE_COUNT = 10;


CPlanet::CPlanet(CEngine* engine)
{
    m_planet[0].reserve(PLANET_PREALLOCATE_COUNT);
    m_planet[1].reserve(PLANET_PREALLOCATE_COUNT);

    m_engine = engine;
    Flush();

}

CPlanet::~CPlanet()
{
}

void CPlanet::Flush()
{
    for (int j = 0; j < 2; j++)
        m_planet[j].clear();

    m_planetExist = false;
    m_mode = 0;
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

    for (int i = 0; i < static_cast<int>( m_planet[m_mode].size() ); i++)
    {
        float a = m_time*m_planet[m_mode][i].speed;
        if (a < 0.0f)
            a += Math::PI*1000.0f;

        m_planet[m_mode][i].angle.x = a+m_planet[m_mode][i].start.x;
        m_planet[m_mode][i].angle.y = sinf(a)*sinf(m_planet[m_mode][i].dir)+m_planet[m_mode][i].start.y;
    }

    return true;
}

void CPlanet::LoadTexture()
{
    for (int j = 0; j < 2; j++)
    {
        for (int i = 0; i < static_cast<int>( m_planet[j].size() ); i++)
        {
            m_engine->LoadTexture(m_planet[j][i].name);
        }
    }
}

void CPlanet::Draw()
{
    CDevice* device = m_engine->GetDevice();
    float eyeDirH = m_engine->GetEyeDirH();
    float eyeDirV = m_engine->GetEyeDirV();

    Math::Vector n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal
    float dp = 0.5f/256.0f;

    for (int i = 0; i < static_cast<int>( m_planet[m_mode].size() ); i++)
    {
        m_engine->SetTexture(m_planet[m_mode][i].name);

        if (m_planet[m_mode][i].transparent)
            m_engine->SetState(ENG_RSTATE_WRAP | ENG_RSTATE_ALPHA);
        else
            m_engine->SetState(ENG_RSTATE_WRAP | ENG_RSTATE_TTEXTURE_BLACK);

        Math::Point p1, p2;

        float a = eyeDirH + m_planet[m_mode][i].angle.x;
        p1.x = Math::Mod(a, Math::PI*2.0f)-0.5f;

        a = eyeDirV + m_planet[m_mode][i].angle.y;
        p1.y = 0.4f+(Math::Mod(a+Math::PI, Math::PI*2.0f)-Math::PI)*(2.0f/Math::PI);

        p1.x -= m_planet[m_mode][i].dim/2.0f*0.75f;
        p1.y -= m_planet[m_mode][i].dim/2.0f;
        p2.x = p1.x+m_planet[m_mode][i].dim*0.75f;
        p2.y = p1.y+m_planet[m_mode][i].dim;

        float u1 = m_planet[m_mode][i].uv1.x + dp;
        float v1 = m_planet[m_mode][i].uv1.y + dp;
        float u2 = m_planet[m_mode][i].uv2.x - dp;
        float v2 = m_planet[m_mode][i].uv2.y - dp;

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

void CPlanet::Create(int mode, Math::Point start, float dim, float speed,
                     float dir, const std::string& name, Math::Point uv1, Math::Point uv2,
                     bool transparent)
{
    if (mode < 0) mode = 0;
    if (mode > 1) mode = 1;

    Planet planet;

    planet.start = start;
    planet.angle = start;
    planet.dim   = dim;
    planet.speed = speed;
    planet.dir   = dir;

    planet.name = name;
    planet.uv1   = uv1;
    planet.uv2   = uv2;

    planet.transparent = transparent;

    m_planet[mode].push_back(planet);

    m_planetExist = true;
}

bool CPlanet::PlanetExist()
{
    return m_planetExist;
}

void CPlanet::SetMode(int mode)
{
    if (mode < 0) mode = 0;
    if (mode > 1) mode = 1;
    m_mode = mode;
}

int CPlanet::GetMode()
{
    return m_mode;
}


} // namespace Gfx

