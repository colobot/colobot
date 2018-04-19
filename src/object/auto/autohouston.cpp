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

#include "object/auto/autohouston.h"

#include "math/geometry.h"

#include "object/old_object.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"


// Object's constructor.

CAutoHouston::CAutoHouston(COldObject* object) : CAuto(object)
{
    for (int i = 0; i < HUSTONMAXLENS; i++)
    {
        m_lens[i].parti = -1;
    }

    m_lens[0].type = Gfx::PARTISELR;
    m_lens[1].type = Gfx::PARTISELR;
    m_lens[2].type = Gfx::PARTISELR;
    m_lens[3].type = Gfx::PARTISELR;
    m_lens[0].pos = Math::Vector(0.0f+13.0f, 34.0f, 30.0f      );
    m_lens[1].pos = Math::Vector(0.0f-13.0f, 34.0f, 30.0f      );
    m_lens[2].pos = Math::Vector(0.0f      , 34.0f, 30.0f+13.0f);
    m_lens[3].pos = Math::Vector(0.0f      , 34.0f, 30.0f-13.0f);
    m_lens[0].dim = 4.0f;
    m_lens[1].dim = 4.0f;
    m_lens[2].dim = 4.0f;
    m_lens[3].dim = 4.0f;
    m_lens[0].total = 1.0f;
    m_lens[1].total = 1.0f;
    m_lens[2].total = 1.0f;
    m_lens[3].total = 1.0f;
    m_lens[0].off = 0.4f;
    m_lens[1].off = 0.4f;
    m_lens[2].off = 0.4f;
    m_lens[3].off = 0.4f;

    int i = 4;

    // Part under the radar.
    m_lens[i].type = Gfx::PARTISELR;
    m_lens[i].pos = Math::Vector(-7.0f, 9.9f, 40.1f);
    m_lens[i].dim = 1.8f;
    m_lens[i].total = 0.4f;
    m_lens[i].off = 0.2f;
    i ++;

    m_lens[i].type = Gfx::PARTISELY;
    m_lens[i].pos = Math::Vector(-7.0f, 7.2f, 34.8f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.7f;
    m_lens[i].off = 0.3f;
    i ++;
    m_lens[i].type = Gfx::PARTISELY;
    m_lens[i].pos = Math::Vector(-7.0f, 6.5f, 34.3f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.7f;
    m_lens[i].off = 0.3f;
    i ++;
    m_lens[i].type = Gfx::PARTISELR;
    m_lens[i].pos = Math::Vector(-7.0f, 6.5f, 33.4f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.0f;
    m_lens[i].off = 0.0f;
    i ++;
    m_lens[i].type = Gfx::PARTISELR;
    m_lens[i].pos = Math::Vector(-7.0f, 6.5f, 33.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 1.0f;
    m_lens[i].off = 0.5f;
    i ++;

    m_lens[i].type = Gfx::PARTISELY;
    m_lens[i].pos = Math::Vector(-7.0f, 8.5f, 14.0f);
    m_lens[i].dim = 1.2f;
    m_lens[i].total = 0.8f;
    m_lens[i].off = 0.2f;
    i ++;

    m_lens[i].type = Gfx::PARTISELR;
    m_lens[i].pos = Math::Vector(4.0f, 6.0f, 8.6f);
    m_lens[i].dim = 1.0f;
    m_lens[i].total = 0.9f;
    m_lens[i].off = 0.7f;
    i ++;

    // Part with three windows.
    m_lens[i].type = Gfx::PARTISELR;
    m_lens[i].pos = Math::Vector(-7.0f, 9.9f, -19.9f);
    m_lens[i].dim = 1.0f;
    m_lens[i].total = 0.6f;
    m_lens[i].off = 0.3f;
    i ++;

    m_lens[i].type = Gfx::PARTISELY;
    m_lens[i].pos = Math::Vector(-7.0f, 7.2f, 34.8f-60.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.7f;
    m_lens[i].off = 0.3f;
    i ++;
    m_lens[i].type = Gfx::PARTISELY;
    m_lens[i].pos = Math::Vector(-7.0f, 6.5f, 34.3f-60.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.0f;
    m_lens[i].off = 0.0f;
    i ++;
    m_lens[i].type = Gfx::PARTISELR;
    m_lens[i].pos = Math::Vector(-7.0f, 6.5f, 33.4f-60.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.6f;
    m_lens[i].off = 0.4f;
    i ++;
    m_lens[i].type = Gfx::PARTISELR;
    m_lens[i].pos = Math::Vector(-7.0f, 6.5f, 33.0f-60.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.8f;
    m_lens[i].off = 0.2f;
    i ++;

    m_lens[i].type = Gfx::PARTISELY;
    m_lens[i].pos = Math::Vector(-6.5f, 13.5f, -37.0f);
    m_lens[i].dim = 1.0f;
    m_lens[i].total = 0.0f;
    m_lens[i].off = 0.0f;
    i ++;

    m_lens[i].type = Gfx::PARTISELY;
    m_lens[i].pos = Math::Vector(-7.0f, 12.2f, -39.8f);
    m_lens[i].dim = 1.8f;
    m_lens[i].total = 1.5f;
    m_lens[i].off = 0.5f;
    i ++;

    m_lens[i].type = Gfx::PARTISELY;
    m_lens[i].pos = Math::Vector(-7.0f, 8.5f, -47.0f);
    m_lens[i].dim = 0.6f;
    m_lens[i].total = 0.7f;
    m_lens[i].off = 0.5f;
    i ++;

    m_lensTotal = i;

    Init();
}

// Object's destructor.

CAutoHouston::~CAutoHouston()
{
}


// Destroys the object.

void CAutoHouston::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoHouston::Init()
{
    m_time = 0.0f;

    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;
}


// Start the object.

void CAutoHouston::Start(int param)
{
}


// Management of an event.

bool CAutoHouston::EventProcess(const Event &event)
{
    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

    float angle;
    angle = -m_time*1.0f;
    m_object->SetPartRotationY(1, angle);  // rotates the radar
    angle = sinf(m_time*4.0f)*0.3f;
    m_object->SetPartRotationX(2, angle);

    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;

    // Flashes the keys.
    for (int i = 0; i < m_lensTotal; i++ )
    {
        if ( m_lens[i].total != 0.0f &&
             Math::Mod(m_time, m_lens[i].total) < m_lens[i].off )
        {
            if ( m_lens[i].parti != -1 )
            {
                m_particle->DeleteParticle(m_lens[i].parti);
                m_lens[i].parti = -1;
            }
        }
        else
        {
            if ( m_lens[i].parti == -1 )
            {
                Math::Point dim;
                dim.x = m_lens[i].dim;
                dim.y = dim.x;

                Math::Vector pos = m_lens[i].pos;
                Math::RotatePoint(Math::Vector(0.0f, 0.0f, 0.0f), -m_object->GetRotationY(), 0.0f, pos);

                m_lens[i].parti = m_particle->CreateParticle(m_object->GetPosition()+pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, m_lens[i].type, 1.0f, 0.0f, 0.0f);
            }
        }
    }

    return true;
}

// Stops the controller.

bool CAutoHouston::Abort()
{
    return true;
}


// Creates all the interface when the object is selected.

bool CAutoHouston::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return false;

    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 115, EVENT_OBJECT_TYPE);

    return true;
}


// Returns an error due to state of the automation.

Error CAutoHouston::GetError()
{
    return ERR_OK;
}

