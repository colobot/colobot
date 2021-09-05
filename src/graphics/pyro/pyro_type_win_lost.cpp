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

#include "sound/sound.h"

using namespace Gfx;

CWinPyro::CWinPyro(CObject *obj)
    : CPyro(obj)
{}

CLostPyro::CLostPyro(CObject *obj)
    : CPyro(obj)
{}

void CWinPyro::UpdateEffect()
{
    if ( m_object == nullptr )
        return;

    if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        Math::Vector pos = m_object->GetPosition();
        pos.y += 1.5f;
        Math::Vector speed;
        speed.x = (Math::Rand()-0.5f)*10.0f;
        speed.z = (Math::Rand()-0.5f)*10.0f;
        speed.y = 8.0f+Math::Rand()*8.0f;
        Math::Point dim;
        dim.x = Math::Rand()*0.2f+0.2f;
        dim.y = dim.x;
        m_particle->CreateTrack(pos, speed, dim,
                                 static_cast<ParticleType>(PARTITRACK7+rand()%4),
                                 3.0f, 20.0f, 1.0f, 0.4f);
    }
}

void CLostPyro::UpdateEffect()
{
    if ( m_object == nullptr )
        return;

    if ( m_lastParticle+m_engine->ParticleAdapt(0.10f) <= m_time )
    {
        m_lastParticle = m_time;

        Math::Vector pos = m_object->GetPosition();
        pos.y -= 2.0f;
        pos.x += (Math::Rand()-0.5f)*10.0f;
        pos.z += (Math::Rand()-0.5f)*10.0f;
        Math::Vector speed;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = 1.0f+Math::Rand()*1.0f;
        Math::Point dim;
        dim.x = Math::Rand()*1.0f+1.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, PARTISMOKE1, 8.0f, 0.0f, 0.0f);
    }
}

Error CWinPyro::IsEnded()
{
    return ERR_CONTINUE;
}

Error CLostPyro::IsEnded()
{
    return ERR_CONTINUE;
}

