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

#include "object/object.h"

using namespace Gfx;

void CResetPyro::AfterCreate()
{
    m_speed = 1.0f/2.0f;
    m_resetAngle = m_object->GetRotationY();
    LightOperAdd(0.00f, 1.0f,  4.0f,  4.0f,  2.0f);  // yellow
    LightOperAdd(1.00f, 0.0f,  4.0f,  4.0f,  2.0f);  // yellow
    CreateLight(m_pos, 40.0f);
}

void CResetPyro::UpdateEffect()
{
    if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        Math::Vector pos = m_pos;
        pos.x += (Math::Rand()-0.5f)*5.0f;
        pos.z += (Math::Rand()-0.5f)*5.0f;
        Math::Vector speed;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = 5.0f+Math::Rand()*5.0f;
        Math::Point dim;
        dim.x = Math::Rand()*2.0f+2.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, PARTIGLINTb, 2.0f);

        pos = m_pos;
        speed.x = (Math::Rand()-0.5f)*20.0f;
        speed.z = (Math::Rand()-0.5f)*20.0f;
        speed.y = Math::Rand()*10.0f;
        speed *= 0.5f+m_progress*0.5f;
        dim.x = 0.6f;
        dim.y = dim.x;
        pos.y += dim.y;
        float duration = Math::Rand()*1.5f+1.5f;
        m_particle->CreateTrack(pos, speed, dim, PARTITRACK6,
                                 duration, 0.0f,
                                 duration*0.9f, 0.7f);
    }

    if(m_object != nullptr)
    {
        float angle = m_resetAngle;
        m_object->SetRotationY(angle-powf((1.0f-m_progress)*5.0f, 2.0f));
        m_object->SetScale(m_progress);
    }
}

void CResetPyro::AfterEnd()
{
    m_object->SetRotationY(m_resetAngle);
    m_object->SetScale(1.0f);
}
