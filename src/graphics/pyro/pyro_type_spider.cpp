/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/engine/particle.h"

#include "graphics/pyro/pyro.h"
#include "graphics/pyro/pyro_manager.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object.h"
#include "object/old_object.h"

#include "sound/sound.h"

using namespace Gfx;

CSpiderPyro::CSpiderPyro(CObject *obj)
    : CPyro(PT_OTHER, obj)
{}

bool CSpiderPyro::EventProcess(const Event& event)
{
    // Destroys the object that exploded.
    //It should not be destroyed at the end of the Create,
    //because it is sometimes the object itself that makes the Create:
    //  pyro->Create(PT_FRAGT, this);
    DeleteObject(true, true);

    return CPyro::EventProcess(event);
}

void CSpiderPyro::AfterCreate()
{
    m_sound->Play(SOUND_EXPLOi, m_pos);

    m_speed = 1.0f/15.0f;

    Math::Vector pos(-3.0f, 2.0f, 0.0f);
    Math::Matrix* mat = m_object->GetWorldMatrix(0);
    m_pos = Math::Transform(*mat, pos);

    m_engine->DeleteShadowSpot(m_object->GetObjectRank(0));

    LightOperAdd(0.00f, 0.0f, -0.5f, -1.0f, -1.0f);  // dark red
    LightOperAdd(0.05f, 1.0f, -0.5f, -1.0f, -1.0f);  // dark red
    LightOperAdd(1.00f, 0.0f, -0.5f, -1.0f, -1.0f);  // dark red
    CreateLight(m_pos, 40.0f);

    for (int part = 0; part < OBJECTMAXPART; part++)
    {
        CreateTriangle(m_object, m_object->GetType(), part, 5, 20, 15, 25);
    }

    for (int i = 0; i < 50; i++)
    {
        pos = m_pos;
        pos.x += (Math::Rand()-0.5f)*3.0f;
        pos.z += (Math::Rand()-0.5f)*3.0f;
        pos.y += (Math::Rand()-0.5f)*2.0f;
        Math::Vector speed;
        speed.x = (Math::Rand()-0.5f)*24.0f;
        speed.z = (Math::Rand()-0.5f)*24.0f;
        speed.y = 10.0f+Math::Rand()*10.0f;
        Math::Point dim;
        dim.x = 1.0f;
        dim.y = dim.x;
        int channel = m_particle->CreateParticle(pos, speed, dim, PARTIGUN3, 2.0f+Math::Rand()*2.0f, 10.0f);
        m_particle->SetObjectFather(channel, m_object);
    }
    int total = static_cast<int>(10.0f*m_engine->GetParticleDensity());
    for (int i = 0; i < total; i++)
    {
        pos = m_pos;
        pos.x += (Math::Rand()-0.5f)*3.0f;
        pos.z += (Math::Rand()-0.5f)*3.0f;
        pos.y += (Math::Rand()-0.5f)*2.0f;
        Math::Vector speed;
        speed.x = (Math::Rand()-0.5f)*24.0f;
        speed.z = (Math::Rand()-0.5f)*24.0f;
        speed.y = 7.0f+Math::Rand()*7.0f;
        Math::Point dim;
        dim.x = 1.0f;
        dim.y = dim.x;
        m_particle->CreateTrack(pos, speed, dim, PARTITRACK3,
                                2.0f+Math::Rand()*2.0f, 10.0f, 2.0f, 0.6f);
    }

    m_camera->StartEffect(CAM_EFFECT_EXPLO, m_pos, 1.0f);
}
