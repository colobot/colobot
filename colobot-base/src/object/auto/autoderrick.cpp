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


#include "object/auto/autoderrick.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"



const float DERRICK_DELAY   = 10.0f;    // duration of the extraction
const float DERRICK_DELAYu  = 30.0f;    // same, but for uranium




// Object's constructor.

CAutoDerrick::CAutoDerrick(COldObject* object) : CAuto(object)
{
    Init();
    m_phase = ADP_WAIT;  // paused until the first Init ()
    m_soundChannel = -1;
}

// Object's destructor.

CAutoDerrick::~CAutoDerrick()
{
}


// Destroys the object.

void CAutoDerrick::DeleteObject(bool all)
{
    if ( !all )
    {
        CObject* cargo = SearchCargo();
        if ( cargo != nullptr && cargo->GetLock() )
        {
            CObjectManager::GetInstancePointer()->DeleteObject(cargo);
        }
    }

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    CAuto::DeleteObject(all);
}


// Initialize the object.

void CAutoDerrick::Init()
{
    glm::vec3 pos = m_object->GetPosition();
    Gfx::TerrainRes res = m_terrain->GetResource(pos);

    if ( res == Gfx::TR_STONE   ||
         res == Gfx::TR_URANIUM ||
         res == Gfx::TR_KEY_A    ||
         res == Gfx::TR_KEY_B    ||
         res == Gfx::TR_KEY_C   ||
         res == Gfx::TR_KEY_D   )
    {
        m_type = OBJECT_NULL;
        if ( res == Gfx::TR_STONE   )  m_type = OBJECT_STONE;
        if ( res == Gfx::TR_URANIUM )  m_type = OBJECT_URANIUM;
        if ( res == Gfx::TR_KEY_A    )  m_type = OBJECT_KEYa;
        if ( res == Gfx::TR_KEY_B    )  m_type = OBJECT_KEYb;
        if ( res == Gfx::TR_KEY_C    )  m_type = OBJECT_KEYc;
        if ( res == Gfx::TR_KEY_D    )  m_type = OBJECT_KEYd;

        m_phase    = ADP_EXCAVATE;
        m_progress = 0.0f;
        m_speed    = 1.0f/(m_type==OBJECT_URANIUM?DERRICK_DELAYu:DERRICK_DELAY);
    }
    else
    {
        m_phase    = ADP_WAIT;
        m_progress = 0.0f;
        m_speed    = 1.0f;
    }

    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;
    m_lastTrack = 0.0f;
}

glm::vec3 CAutoDerrick::GetCargoPos()
{
    glm::vec3 pos = glm::vec3(7.0f, 0.0f, 0.0f);
    glm::mat4 mat = m_object->GetWorldMatrix(0);
    pos = Math::Transform(mat, pos);
    m_terrain->AdjustToFloor(pos);
    return pos;
}


// Management of an event.

bool CAutoDerrick::EventProcess(const Event &event)
{
    CObject*    cargo;
    glm::vec3    pos, speed;
    glm::vec2     dim;
    float       angle, duration, factor;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_phase == ADP_WAIT )  return true;

    m_progress += event.rTime*m_speed;
    m_timeVirus -= event.rTime;

    if ( m_object->GetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Math::Rand()*0.3f;

            pos.x = 0.0f;
            pos.z = 0.0f;
            pos.y = -2.0f*Math::Rand();
            m_object->SetPartPosition(1, pos);  // up / down the drill

            m_object->SetPartRotationY(1, Math::Rand()*0.5f);  // rotates the drill
        }
        return true;
    }

    if ( m_phase == ADP_EXCAVATE )
    {
        if ( m_soundChannel == -1 )
        {
            if ( m_type == OBJECT_URANIUM )
            {
                factor = DERRICK_DELAYu/DERRICK_DELAY;
            }
            else
            {
                factor = 1.0f;
            }
            m_soundChannel = m_sound->Play(SOUND_DERRICK, m_object->GetPosition(), 1.0f, 0.5f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.5f, 4.0f*factor, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.3f, 6.0f*factor, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.5f, 1.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.5f, 4.0f, SOPER_STOP);
        }

        if ( m_progress >= 6.0f/16.0f &&  // penetrates into the ground?
             m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_object->GetPosition();
            speed.x = (Math::Rand()-0.5f)*10.0f;
            speed.z = (Math::Rand()-0.5f)*10.0f;
            speed.y = Math::Rand()*5.0f;
            dim.x = Math::Rand()*3.0f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f);
        }

        if ( m_progress >= 6.0f/16.0f &&  // penetrates into the ground?
             m_lastTrack+m_engine->ParticleAdapt(0.5f) <= m_time )
        {
            m_lastTrack = m_time;

            pos = m_object->GetPosition();
            speed.x = (Math::Rand()-0.5f)*12.0f;
            speed.z = (Math::Rand()-0.5f)*12.0f;
            speed.y = Math::Rand()*10.0f+10.0f;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Math::Rand()*2.0f+2.0f;
            m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK5,
                                     duration, Math::Rand()*10.0f+15.0f,
                                     duration*0.2f, 1.0f);
        }

        if ( m_progress < 1.0f )
        {
            pos.x = 0.0f;
            pos.z = 0.0f;
            pos.y = -m_progress*16.0f;
            m_object->SetPartPosition(1, pos);  // down the drill

            angle = m_object->GetPartRotationY(1);
            angle += event.rTime*8.0f;
            m_object->SetPartRotationY(1, angle);  // rotates the drill
        }
        else
        {
            m_phase    = ADP_ASCEND;
            m_progress = 0.0f;
            m_speed    = 1.0f/5.0f;
        }
    }

    if ( m_phase == ADP_ASCEND )
    {
        if ( m_progress <= 7.0f/16.0f &&
             m_lastParticle+m_engine->ParticleAdapt(0.1f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_object->GetPosition();
            speed.x = (Math::Rand()-0.5f)*10.0f;
            speed.z = (Math::Rand()-0.5f)*10.0f;
            speed.y = Math::Rand()*5.0f;
            dim.x = Math::Rand()*3.0f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f);
        }

        if ( m_progress <= 4.0f/16.0f &&
             m_lastTrack+m_engine->ParticleAdapt(1.0f) <= m_time )
        {
            m_lastTrack = m_time;

            pos = m_object->GetPosition();
            speed.x = (Math::Rand()-0.5f)*12.0f;
            speed.z = (Math::Rand()-0.5f)*12.0f;
            speed.y = Math::Rand()*10.0f+10.0f;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Math::Rand()*2.0f+2.0f;
            m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK5,
                                     duration, Math::Rand()*10.0f+15.0f,
                                     duration*0.2f, 1.0f);
        }

        if ( m_progress < 1.0f )
        {
            pos.x = 0.0f;
            pos.z = 0.0f;
            pos.y = -(1.0f-m_progress)*16.0f;
            m_object->SetPartPosition(1, pos);  // back the drill

            angle = m_object->GetPartRotationY(1);
            angle -= event.rTime*2.0f;
            m_object->SetPartRotationY(1, angle);  // rotates the drill
        }
        else
        {
            m_soundChannel = -1;
            m_bSoundFall = false;

            m_phase    = ADP_EXPORT;
            m_progress = 0.0f;
            m_speed    = 1.0f/5.0f;
        }
    }

    if ( m_phase == ADP_ISFREE )
    {
        if ( m_progress >= 1.0f )
        {
            m_bSoundFall = false;

            m_phase    = ADP_EXPORT;
            m_progress = 0.0f;
            m_speed    = 1.0f/5.0f;
        }
    }

    if ( m_phase == ADP_EXPORT )
    {
        if ( m_progress == 0.0f )
        {
            glm::vec3 cargoPos = GetCargoPos();
            if ( ExistKey() )
            {
                m_phase    = ADP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/10.0f;
            }
            else if ( SearchFree(cargoPos) )
            {
                angle = m_object->GetRotationY();
                CreateCargo(cargoPos, angle, m_type, 16.0f);
            }
            else
            {
                m_phase    = ADP_ISFREE;
                m_progress = 0.0f;
                m_speed    = 1.0f/2.0f;
                return true;
            }
        }

        cargo = SearchCargo();

        if ( cargo != nullptr &&
             m_progress <= 0.5f &&
             m_lastParticle+m_engine->ParticleAdapt(0.1f) <= m_time )
        {
            m_lastParticle = m_time;

            if ( m_progress < 0.3f )
            {
                pos = cargo->GetPosition();
                pos.x += (Math::Rand()-0.5f)*5.0f;
                pos.z += (Math::Rand()-0.5f)*5.0f;
                pos.y += (Math::Rand()-0.5f)*5.0f;
                speed = glm::vec3(0.0f, 0.0f, 0.0f);
                dim.x = 3.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFIRE, 1.0f, 0.0f, 0.0f);
            }
            else
            {
                pos = cargo->GetPosition();
                pos.x += (Math::Rand()-0.5f)*5.0f;
                pos.z += (Math::Rand()-0.5f)*5.0f;
                pos.y += Math::Rand()*2.5f;
                speed = glm::vec3(0.0f, 0.0f, 0.0f);
                dim.x = 1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, 2.0f, 0.0f, 0.0f);
            }
        }

        if ( m_progress < 1.0f )
        {
            if ( cargo != nullptr )
            {
                glm::vec3 cargoPos = GetCargoPos();
                pos = cargo->GetPosition();
                pos.y -= event.rTime*20.0f;  // grave
                if ( !m_bSoundFall && pos.y < cargoPos.y )
                {
                    m_sound->Play(SOUND_BOUM, cargoPos);
                    m_bSoundFall = true;
                }
                if ( pos.y < cargoPos.y )
                {
                    pos.y = cargoPos.y;
                    cargo->SetLock(false);  // object usable
                }
                cargo->SetPosition(pos);
            }
        }
        else
        {
            if ( ExistKey() )  // key already exists?
            {
                m_phase    = ADP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/10.0f;
            }
            else
            {
                m_phase    = ADP_EXCAVATE;
                m_progress = 0.0f;
                m_speed    = 1.0f/(m_type==OBJECT_URANIUM?DERRICK_DELAYu:DERRICK_DELAY);
            }
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoDerrick::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    glm::vec2     pos, ddim;
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
    pw->CreateGroup(pos, ddim, 109, EVENT_OBJECT_TYPE);

    return true;
}


// Saves all parameters of the controller.

bool CAutoDerrick::Write(CLevelParserLine* line)
{
    if ( m_phase == ADP_WAIT )  return false;

    line->AddParam("aExist", std::make_unique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", std::make_unique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", std::make_unique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", std::make_unique<CLevelParserParam>(m_speed));

    return true;
}

// Restores all parameters of the controller.

bool CAutoDerrick::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoDerrickPhase >(line->GetParam("aPhase")->AsInt(ADP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;

    return true;
}


// Seeks the subject cargo.

CObject* CAutoDerrick::SearchCargo()
{
    glm::vec3 cargoPos = GetCargoPos();
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType type = obj->GetType();
        if ( type == OBJECT_DERRICK )  continue;

            glm::vec3 oPos = obj->GetPosition();

        if ( oPos.x == cargoPos.x &&
             oPos.z == cargoPos.z )  return obj;
    }

    return nullptr;
}

// Seeks if a site is free.

bool CAutoDerrick::SearchFree(glm::vec3 pos)
{
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType type = obj->GetType();
        if ( type == OBJECT_DERRICK )  continue;

        for (const auto& crashSphere : obj->GetAllCrashSpheres())
        {
            glm::vec3 sPos = crashSphere.sphere.pos;
            float sRadius = crashSphere.sphere.radius;

            float distance = glm::distance(sPos, pos);
            distance -= sRadius;
            if ( distance < 2.0f )  return false;  // location occupied
        }
    }

    return true;  // location free
}

// Create a transportable object.

void CAutoDerrick::CreateCargo(glm::vec3 pos, float angle, ObjectType type,
                              float height)
{
    CObject* cargo = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type);
    cargo->SetLock(true);  // object not yet usable

    pos = cargo->GetPosition();
    pos.y += height;
    cargo->SetPosition(pos);
}

// Look if there is already a key.

bool CAutoDerrick::ExistKey()
{
    if ( m_type != OBJECT_KEYa &&
         m_type != OBJECT_KEYb &&
         m_type != OBJECT_KEYc &&
         m_type != OBJECT_KEYd )  return false;

    // Make sure to detect event carried objects
    for ( CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects() )
    {
        if ( obj->GetType() == m_type ) return true;
    }
    return false;
}


// returns an error due the state of the automaton.

Error CAutoDerrick::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    if ( m_phase == ADP_WAIT )  return ERR_DERRICK_NULL;
    return ERR_OK;
}
