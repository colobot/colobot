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


#include "object/auto/automush.h"

#include "graphics/engine/engine.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/func.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "sound/sound.h"


// Object's constructor.

CAutoMush::CAutoMush(COldObject* object) : CAuto(object)
{
    Init();
}

// Object's destructor.

CAutoMush::~CAutoMush()
{
}


// Destroys the object.

void CAutoMush::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoMush::Init()
{
    m_phase    = AMP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/4.0f;

    m_time     = 0.0f;
    m_lastParticle = 0.0f;
}


// Management of an event.

bool CAutoMush::EventProcess(const Event &event)
{
    glm::vec3    pos, speed, dir;
    glm::vec2     dim;
    float       factor, zoom, size, angle;
    int         i, channel;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;

    factor = 0.0f;
    size = 1.0f;

    if ( m_phase == AMP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            if ( !SearchTarget() )
            {
                m_phase    = AMP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/(2.0f+Math::Rand()*2.0f);
            }
            else
            {
                m_phase    = AMP_SNIF;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.5f;
            }
        }
    }

    if ( m_phase == AMP_SNIF )
    {
        if ( m_progress < 1.0f )
        {
            factor = m_progress;
        }
        else
        {
            m_phase    = AMP_ZOOM;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == AMP_ZOOM )
    {
        if ( m_progress < 1.0f )
        {
            factor = 1.0f;
            size = 1.0f+m_progress*0.3f;
        }
        else
        {
            m_sound->Play(SOUND_MUSHROOM, m_object->GetPosition());

            m_phase    = AMP_FIRE;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == AMP_FIRE )
    {
        if ( m_progress < 1.0f )
        {
            factor = 1.0f-m_progress;
            size = 1.0f+(1.0f-m_progress)*0.3f;

            if ( m_lastParticle <= m_time )
            {
                m_lastParticle += 0.05;

                for ( i=0 ; i<10 ; i++ )
                {
                    pos = m_object->GetPosition();
                    pos.y += 5.0f;
                    speed.x = (Math::Rand()-0.5f)*200.0f;
                    speed.z = (Math::Rand()-0.5f)*200.0f;
                    speed.y = -(20.0f+Math::Rand()*20.0f);
                    dim.x = 1.0f;
                    dim.y = dim.x;
                    channel = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGUN2, 2.0f, 100.0f, 0.0f);
                    m_particle->SetObjectFather(channel, m_object);
                }
            }
        }
        else
        {
            m_phase    = AMP_SMOKE;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == AMP_SMOKE )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticle+m_engine->ParticleAdapt(0.10f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_object->GetPosition();
                pos.y += 5.0f;
                speed.x = (Math::Rand()-0.5f)*4.0f;
                speed.z = (Math::Rand()-0.5f)*4.0f;
                speed.y = -(0.5f+Math::Rand()*0.5f);
                dim.x = Math::Rand()*2.5f+2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            m_phase    = AMP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/(2.0f+Math::Rand()*2.0f);
        }
    }

    if ( factor != 0.0f || size != 1.0f )
    {
        dir.x = sinf(m_time*Math::PI*4.0f);
        dir.z = cosf(m_time*Math::PI*4.0f);

        angle = sinf(m_time*10.0f)*factor*0.04f;
        m_object->SetRotationX(angle*dir.z);
        m_object->SetRotationZ(angle*dir.x);

        zoom = 1.0f+sinf(m_time*8.0f)*factor*0.06f;
        m_object->SetScaleX(zoom*size);
        zoom = 1.0f+sinf(m_time*5.0f)*factor*0.06f;
        m_object->SetScaleY(zoom*size);
        zoom = 1.0f+sinf(m_time*7.0f)*factor*0.06f;
        m_object->SetScaleZ(zoom*size);
    }
    else
    {
        m_object->SetRotationX(0.0f);
        m_object->SetRotationZ(0.0f);
        m_object->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
    }

    return true;
}


// Seeking a nearby target.

bool CAutoMush::SearchTarget()
{
    glm::vec3 iPos = m_object->GetPosition();

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( obj->GetLock() )  continue;

        ObjectType type = obj->GetType();
        if ( type != OBJECT_MOBILEfa &&
             type != OBJECT_MOBILEta &&
             type != OBJECT_MOBILEwa &&
             type != OBJECT_MOBILEia &&
             type != OBJECT_MOBILEfb &&
             type != OBJECT_MOBILEtb &&
             type != OBJECT_MOBILEwb &&
             type != OBJECT_MOBILEib &&
             type != OBJECT_MOBILEfc &&
             type != OBJECT_MOBILEtc &&
             type != OBJECT_MOBILEwc &&
             type != OBJECT_MOBILEic &&
             type != OBJECT_MOBILEfi &&
             type != OBJECT_MOBILEti &&
             type != OBJECT_MOBILEwi &&
             type != OBJECT_MOBILEii &&
             type != OBJECT_MOBILEfs &&
             type != OBJECT_MOBILEts &&
             type != OBJECT_MOBILEws &&
             type != OBJECT_MOBILEis &&
             type != OBJECT_MOBILErt &&
             type != OBJECT_MOBILErc &&
             type != OBJECT_MOBILErr &&
             type != OBJECT_MOBILErs &&
             type != OBJECT_MOBILEsa &&
             type != OBJECT_MOBILEtg &&
             type != OBJECT_MOBILEft &&
             type != OBJECT_MOBILEtt &&
             type != OBJECT_MOBILEwt &&
             type != OBJECT_MOBILEit &&
             type != OBJECT_MOBILErp &&
             type != OBJECT_MOBILEst &&
             type != OBJECT_MOBILEdr &&
             type != OBJECT_DERRICK  &&
             type != OBJECT_STATION  &&
             type != OBJECT_FACTORY  &&
             type != OBJECT_REPAIR   &&
             type != OBJECT_DESTROYER&&
             type != OBJECT_CONVERT  &&
             type != OBJECT_TOWER    &&
             type != OBJECT_RESEARCH &&
             type != OBJECT_RADAR    &&
             type != OBJECT_INFO     &&
             type != OBJECT_ENERGY   &&
             type != OBJECT_LABO     &&
             type != OBJECT_NUCLEAR  &&
             type != OBJECT_PARA     &&
             type != OBJECT_HUMAN    )  continue;

        glm::vec3 oPos = obj->GetPosition();
        float dist = glm::distance(oPos, iPos);
        if ( dist < 50.0f )  return true;
    }

    return false;
}


// Returns an error due the state of the automation.

Error CAutoMush::GetError()
{
    return ERR_OK;
}


// Saves all parameters of the controller.

bool CAutoMush::Write(CLevelParserLine* line)
{
    if ( m_phase == AMP_WAIT )  return false;

    line->AddParam("aExist", std::make_unique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", std::make_unique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", std::make_unique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", std::make_unique<CLevelParserParam>(m_speed));

    return true;
}

// Restores all parameters of the controller.

bool CAutoMush::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoMushPhase >(line->GetParam("aPhase")->AsInt(AMP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;

    return true;
}
