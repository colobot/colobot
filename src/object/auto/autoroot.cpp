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


#include "object/auto/autoroot.h"

#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include <stdio.h>


// Object's constructor.

CAutoRoot::CAutoRoot(CObject* object) : CAuto(object)
{
    Init();
}

// Object's destructor.

CAutoRoot::~CAutoRoot()
{
}


// Destroys the object.

void CAutoRoot::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoRoot::Init()
{
    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    Math::Point     dim;

    m_time = 0.0f;
    m_lastParticle = 0.0f;

    mat = m_object->GetWorldMatrix(0);
    pos = Math::Vector(-5.0f, 28.0f, -4.0f);  // peak position
    pos = Math::Transform(*mat, pos);
    m_center = pos;

    speed = Math::Vector(0.0f, 0.0f, 0.0f);
    dim.x = 100.0f;
    dim.y = dim.x;
    m_particle->CreateParticle(m_center, speed, dim, Gfx::PARTISPHERE5, 0.5f, 0.0f, 0.0f);

    m_terrain->AddFlyingLimit(pos, 100.0f, 80.0f, pos.y-60.0f);
}


// Management of an event.

bool CAutoRoot::EventProcess(const Event &event)
{
    Math::Vector    pos, speed;
    Math::Point     dim;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_object->SetZoomX(1, 1.0f+sinf(m_time*2.0f)*0.2f);
    m_object->SetZoomY(1, 1.0f+sinf(m_time*2.3f)*0.2f);
    m_object->SetZoomZ(1, 1.0f+sinf(m_time*2.7f)*0.2f);

    if ( m_lastParticle+m_engine->ParticleAdapt(0.10f) <= m_time )
    {
        m_lastParticle = m_time;

        pos = m_center;
        pos.x += (Math::Rand()-0.5f)*8.0f;
        pos.z += (Math::Rand()-0.5f)*8.0f;
        pos.y += 0.0f;
        speed.x = (Math::Rand()-0.5f)*12.0f;
        speed.z = (Math::Rand()-0.5f)*12.0f;
        speed.y = Math::Rand()*12.0f;
        dim.x = Math::Rand()*6.0f+4.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIROOT, 1.0f, 0.0f, 0.0f);
    }

    return true;
}


// Returns an error due the state of the automation.

Error CAutoRoot::GetError()
{
    return ERR_OK;
}


