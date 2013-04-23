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


#include "object/task/tasksearch.h"

#include "common/iman.h"

#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "physics/physics.h"

#include "ui/displaytext.h"




// Object's constructor.

CTaskSearch::CTaskSearch(CObject* object) : CTask(object)
{
    m_hand = TSH_UP;
}

// Object's destructor.

CTaskSearch::~CTaskSearch()
{
}


// Management of an event.

bool CTaskSearch::EventProcess(const Event &event)
{
    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       angle;
    int         i;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    m_progress += event.rTime*m_speed;  // others advance
    m_time += event.rTime;

    if ( m_phase == TSP_DOWN ||
         m_phase == TSP_UP   )
    {
        for ( i=0 ; i<3 ; i++ )
        {
            angle = (m_finalAngle[i]-m_initialAngle[i])*m_progress;
            angle += m_initialAngle[i];
            m_object->SetAngleZ(i+1, angle);
        }
    }

    if ( m_phase == TSP_SEARCH &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        mat = m_object->GetWorldMatrix(0);
        pos = Math::Vector(6.5f, 0.2f, 0.0f);
        pos = Math::Transform(*mat, pos);  // sensor position

        speed.x = (Math::Rand()-0.5f)*20.0f;
        speed.z = (Math::Rand()-0.5f)*20.0f;
        speed.y = 0.0f;
        dim.x = Math::Rand()*1.0f+1.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS);
    }

    return true;
}


// Initializes the initial and final angles.

void CTaskSearch::InitAngle()
{
    int     i;

    if ( m_hand == TSH_UP )
    {
        m_finalAngle[0] =  110.0f*Math::PI/180.0f;  // arm
        m_finalAngle[1] = -110.0f*Math::PI/180.0f;  // forearm
        m_finalAngle[2] =  -65.0f*Math::PI/180.0f;  // sensor
    }
    if ( m_hand == TSH_DOWN )
    {
        m_finalAngle[0] =   25.0f*Math::PI/180.0f;  // arm
        m_finalAngle[1] =  -70.0f*Math::PI/180.0f;  // forearm
        m_finalAngle[2] =  -45.0f*Math::PI/180.0f;  // sensor
    }

    for ( i=0 ; i<3 ; i++ )
    {
        m_initialAngle[i] = m_object->GetAngleZ(i+1);
    }
}


// Assigns the goal was achieved.

Error CTaskSearch::Start()
{
    ObjectType  type;
    Math::Vector    speed;
    int         i;

    m_bError = true;
    if ( !m_physics->GetLand() )  return ERR_SEARCH_FLY;

    speed = m_physics->GetMotorSpeed();
    if ( speed.x != 0.0f ||
         speed.z != 0.0f )  return ERR_SEARCH_MOTOR;

    type = m_object->GetType();
    if ( type != OBJECT_MOBILEfs &&
         type != OBJECT_MOBILEts &&
         type != OBJECT_MOBILEws &&
         type != OBJECT_MOBILEis )  return ERR_SEARCH_VEH;

    m_hand     = TSH_DOWN;
    m_phase    = TSP_DOWN;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;
    m_time     = 0.0f;
    m_lastParticle = 0.0f;

    InitAngle();
    m_bError = false;  // ok

    m_camera->StartCentering(m_object, Math::PI*0.50f, 99.9f, 0.0f, 1.0f);

    i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 0.0f, 0.3f, true);
    m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.9f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f, 0.1f, SOPER_STOP);

    m_physics->SetFreeze(true);  // it does not move

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskSearch::IsEnded()
{
    int     i;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    if ( m_phase == TSP_DOWN ||
         m_phase == TSP_UP   )
    {
        for ( i=0 ; i<3 ; i++ )
        {
            m_object->SetAngleZ(i+1, m_finalAngle[i]);
        }
    }

    if ( m_phase == TSP_DOWN )
    {
        m_sound->Play(SOUND_REPAIR, m_object->GetPosition(0));

        m_phase = TSP_SEARCH;
        m_speed = 1.0f/4.0f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TSP_SEARCH )
    {
        CreateMark();

        m_hand  = TSH_UP;
        InitAngle();

        i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 0.0f, 0.3f, true);
        m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
        m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.9f, SOPER_CONTINUE);
        m_sound->AddEnvelope(i, 0.0f, 0.3f, 0.1f, SOPER_STOP);

        m_phase = TSP_UP;
        m_speed = 1.0f/1.0f;
        return ERR_CONTINUE;
    }

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskSearch::Abort()
{
    m_hand  = TSH_UP;
    InitAngle();
    for (int i = 0; i < 3; i++) {
        m_object->SetAngleZ(i+1, m_finalAngle[i]);
    }
    
    m_camera->StopCentering(m_object, 2.0f);
    m_physics->SetFreeze(false);  // is moving again
    return true;
}


// Creates a mark if possible.

bool CTaskSearch::CreateMark()
{
    CObject*        fret;
    ObjectType      type;
    Math::Matrix*   mat;
    Math::Vector    pos;
    Gfx::TerrainRes res;
    Error           info;

    mat = m_object->GetWorldMatrix(0);
    pos = Math::Vector(7.5f, 0.0f, 0.0f);
    pos = Math::Transform(*mat, pos);  // sensor position

    res = m_terrain->GetResource(pos);
    if ( res == Gfx::TR_NULL )  return false;

    type = OBJECT_NULL;
    if ( res == Gfx::TR_STONE )
    {
        type = OBJECT_MARKSTONE;
        info = INFO_MARKSTONE;
    }
    if ( res == Gfx::TR_URANIUM )
    {
        type = OBJECT_MARKURANIUM;
        info = INFO_MARKURANIUM;
    }
    if ( res == Gfx::TR_POWER )
    {
        type = OBJECT_MARKPOWER;
        info = INFO_MARKPOWER;
    }
    if ( res == Gfx::TR_KEY_A )
    {
        type = OBJECT_MARKKEYa;
        info = INFO_MARKKEYa;
    }
    if ( res == Gfx::TR_KEY_B )
    {
        type = OBJECT_MARKKEYb;
        info = INFO_MARKKEYb;
    }
    if ( res == Gfx::TR_KEY_C )
    {
        type = OBJECT_MARKKEYc;
        info = INFO_MARKKEYc;
    }
    if ( res == Gfx::TR_KEY_D )
    {
        type = OBJECT_MARKKEYd;
        info = INFO_MARKKEYd;
    }
    if ( type == OBJECT_NULL )  return false;

//? DeleteMark(type);

    fret = new CObject();
    if ( !fret->CreateResource(pos, 0.0f, type) )
    {
        delete fret;
        m_displayText->DisplayError(ERR_TOOMANY, m_object);
        return false;
    }

    m_displayText->DisplayError(info, pos, 5.0f, 50.0f);  // displays the message

    return true;
}

// Destroys the marks of a given type.

void CTaskSearch::DeleteMark(ObjectType type)
{
    CObject*    pObj;
    Math::Vector    oPos;
    int         i;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        if ( type == pObj->GetType() )
        {
            pObj->DeleteObject();  // removes the mark
            delete pObj;
            break;
        }
    }
}


