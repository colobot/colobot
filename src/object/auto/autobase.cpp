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


#include <stdio.h>

#include "object/auto/autobase.h"

#include "common/iman.h"

#include "graphics/engine/terrain.h"
#include "graphics/engine/cloud.h"
#include "graphics/engine/planet.h"
#include "graphics/engine/lightning.h"

#include "math/geometry.h"

#include "object/robotmain.h"

#include "physics/physics.h"

#include "ui/interface.h"
#include "ui/window.h"
#include "ui/displaytext.h"



const float BASE_LAND_TIME          = 7.5f;     // hard landing
const float BASE_TAKO_TIME          = 10.0f;    // hard landing
const float BASE_DOOR_TIME          = 6.0f;     // time opening / closing
const float BASE_DOOR_TIME2         = 2.0f;     // time opening / closing suppl.
const float BASE_PORTICO_TIME_MOVE  = 16.0f;    // gate advance time
const float BASE_PORTICO_TIME_DOWN  = 4.0f;     // gate length down
const float BASE_PORTICO_TIME_OPEN  = 4.0f;     // gate opening duration
const float BASE_TRANSIT_TIME       = 15.0f;    // transit duration




// Object's constructor.

CAutoBase::CAutoBase(CObject* object) : CAuto(object)
{
    m_fogStart = m_engine->GetFogStart();
    m_deepView = m_engine->GetDeepView();
    Init();
    m_phase = ABP_WAIT;
    m_soundChannel = -1;
}

// Object's destructor.

CAutoBase::~CAutoBase()
{
}


// Destroys the object.

void CAutoBase::DeleteObject(bool bAll)
{
    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoBase::Init()
{
    m_bOpen    = false;
    m_time     = 0.0f;
    m_lastParticle = 0.0f;
    m_lastMotorParticle = 0.0f;

    m_pos = m_object->GetPosition(0);
    m_lastPos = m_pos;

    m_phase    = ABP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;
}


// Start the object.

void CAutoBase::Start(int param)
{
    m_phase    = ABP_START;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    m_param = param;
}


// Management of an event.

bool CAutoBase::EventProcess(const Event &event)
{
    Math::Matrix*   mat;
    Event       newEvent;
    CObject*    pObj;
    Math::Vector    pos, speed, vibCir, iPos;
    Math::Point     dim, p;
    Error       err;
    float       angle, dist, time, h, len, vSpeed;
    int         i, max;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

begin:
    iPos = m_object->GetPosition(0);

    if ( m_phase == ABP_START )
    {
        if ( m_param != PARAM_STOP     &&  // not placed on the ground?
             m_param != PARAM_FIXSCENE )
        {
            FreezeCargo(true);  // freeze whole cargo
        }

        if ( m_param == PARAM_STOP )  // raises the ground?
        {
            m_phase    = ABP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;

            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetAngleZ(1+i, Math::PI/2.0f-124.0f*Math::PI/180.0f);
                m_object->SetAngleX(10+i, -10.0f*Math::PI/180.0f);
                m_object->SetAngleX(18+i,  10.0f*Math::PI/180.0f);
                m_object->SetPosition(10+i, Math::Vector(23.5f, 0.0f, -11.5f));
                m_object->SetPosition(18+i, Math::Vector(23.5f, 0.0f,  11.5f));
            }

            pObj = m_main->GetSelectObject();
            m_main->SelectObject(pObj);
            m_camera->SetControllingObject(pObj);
            if ( pObj == 0 )
            {
                m_camera->SetType(Gfx::CAM_TYPE_BACK);
            }
            else
            {
                m_camera->SetType(pObj->GetCameraType());
                m_camera->SetDist(pObj->GetCameraDist());
            }

            m_main->StartMusic();
        }

        if ( m_param == PARAM_FIXSCENE )  // raises the ground?
        {
            m_phase    = ABP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;

            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetAngleZ(1+i, Math::PI/2.0f-124.0f*Math::PI/180.0f);
                m_object->SetAngleX(10+i, -10.0f*Math::PI/180.0f);
                m_object->SetAngleX(18+i,  10.0f*Math::PI/180.0f);
                m_object->SetPosition(10+i, Math::Vector(23.5f, 0.0f, -11.5f));
                m_object->SetPosition(18+i, Math::Vector(23.5f, 0.0f,  11.5f));
            }
        }

        if ( m_param == PARAM_LANDING )  // Landing?
        {
            m_phase    = ABP_LAND;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_LAND_TIME;

            m_main->SetMovieLock(true);  // blocks everything until the end of the landing
            m_bMotor = true;  // lights the jet engine

            m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);

            pos = m_pos;
            pos.x -= 150.0f;
            m_terrain->AdjustToFloor(pos);
            pos.y += 10.0f;
            m_camera->SetScriptEye(pos);
            m_posSound = pos;

            pos = m_object->GetPosition(0);
            pos.y += 300.0f+50.0f;
            m_camera->SetScriptLookat(pos);

            m_camera->FixCamera();
            m_engine->SetFocus(2.0f);

            m_engine->SetFogStart(0.9f);

            if ( m_soundChannel == -1 )
            {
                m_soundChannel = m_sound->Play(SOUND_FLY, m_posSound, 0.3f, 2.0f, true);
                m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.5f, BASE_LAND_TIME, SOPER_CONTINUE);
                m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 2.0f, SOPER_STOP);
            }

            m_main->StartMusic();
        }

        if ( m_param == PARAM_PORTICO )  // gate on the porch?
        {
            pos = m_object->GetPosition(0);
            m_finalPos = pos;
            pos.z += BASE_PORTICO_TIME_MOVE*5.0f;  // back
            pos.y += 10.0f;  // rises (the gate)
            m_object->SetPosition(0, pos);
            MoveCargo();  // all cargo moves

            m_phase    = ABP_PORTICO_MOVE;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_PORTICO_TIME_MOVE;

            m_main->StartMusic();
        }

        if ( m_param == PARAM_TRANSIT1 ||
             m_param == PARAM_TRANSIT2 ||
             m_param == PARAM_TRANSIT3 )  // transit in space?
        {
            m_phase    = ABP_TRANSIT_MOVE;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_TRANSIT_TIME;

            m_object->SetAngleZ(0, -Math::PI/2.0f);
            pos = m_object->GetPosition(0);
            pos.y += 10000.0f;  // in space
            m_finalPos = pos;
            m_object->SetPosition(0, pos);

            m_main->SetMovieLock(true);  // blocks everything until the end of the landing
            m_bMotor = true;  // lights the jet engine

            m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);
            pos.x += 1000.0f;
            pos.z -= 60.0f;
            pos.y += 80.0f;
            m_camera->SetScriptEye(pos);
            m_posSound = pos;
            m_camera->FixCamera();
            m_engine->SetFocus(1.0f);

            BeginTransit();

            mat = m_object->GetWorldMatrix(0);
            speed = Math::Vector(0.0f, 0.0f, 0.0f);
            dim.x = 10.0f;
            dim.y = dim.x;
            pos = Math::Vector(42.0f, -2.0f, 17.0f);
            pos = Transform(*mat, pos);
            m_partiChannel[0] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, BASE_TRANSIT_TIME+1.0f, 0.0f, 0.0f);
            pos = Math::Vector(17.0f, -2.0f, 42.0f);
            pos = Transform(*mat, pos);
            m_partiChannel[1] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, BASE_TRANSIT_TIME+1.0f, 0.0f, 0.0f);
            pos = Math::Vector(42.0f, -2.0f, -17.0f);
            pos = Transform(*mat, pos);
            m_partiChannel[2] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, BASE_TRANSIT_TIME+1.0f, 0.0f, 0.0f);
            pos = Math::Vector(17.0f, -2.0f, -42.0f);
            pos = Transform(*mat, pos);
            m_partiChannel[3] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, BASE_TRANSIT_TIME+1.0f, 0.0f, 0.0f);
            pos = Math::Vector(-42.0f, -2.0f, 17.0f);
            pos = Transform(*mat, pos);
            m_partiChannel[4] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, BASE_TRANSIT_TIME+1.0f, 0.0f, 0.0f);
            pos = Math::Vector(-17.0f, -2.0f, 42.0f);
            pos = Transform(*mat, pos);
            m_partiChannel[5] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, BASE_TRANSIT_TIME+1.0f, 0.0f, 0.0f);
            pos = Math::Vector(-42.0f, -2.0f, -17.0f);
            pos = Transform(*mat, pos);
            m_partiChannel[6] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, BASE_TRANSIT_TIME+1.0f, 0.0f, 0.0f);
            pos = Math::Vector(-17.0f, -2.0f, -42.0f);
            pos = Transform(*mat, pos);
            m_partiChannel[7] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, BASE_TRANSIT_TIME+1.0f, 0.0f, 0.0f);

            if ( m_soundChannel == -1 )
            {
                m_soundChannel = m_sound->Play(SOUND_FLY, m_posSound, 0.0f, 1.2f, true);
                m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, BASE_TRANSIT_TIME*0.55f, SOPER_CONTINUE);
                m_sound->AddEnvelope(m_soundChannel, 0.3f, 0.8f, BASE_TRANSIT_TIME*0.45f, SOPER_STOP);
            }
        }
    }

    if ( event.type == EVENT_UPDINTERFACE )
    {
        if ( m_object->GetSelect() )  CreateInterface(true);
    }

    if ( event.type == EVENT_OBJECT_BTAKEOFF )
    {
        err = CheckCloseDoor();
        if ( err != ERR_OK )
        {
            m_displayText->DisplayError(err, m_object);
            return false;
        }

        err = m_main->CheckEndMission(false);
        if ( err != ERR_OK )
        {
            m_displayText->DisplayError(err, m_object);
            return false;
        }

        FreezeCargo(true);  // freeze whole cargo
        m_main->SetMovieLock(true);  // blocks everything until the end
        m_main->DeselectAll();

        newEvent.type = EVENT_UPDINTERFACE;
        m_eventQueue->AddEvent(newEvent);

        m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);

        pos = m_pos;
        pos.x -= 110.0f;
        m_terrain->AdjustToFloor(pos);
        pos.y += 10.0f;
        m_camera->SetScriptEye(pos);
        m_posSound = pos;

        pos = m_object->GetPosition(0);
        pos.y += 50.0f;
        m_camera->SetScriptLookat(pos);

        m_engine->SetFocus(1.0f);

        m_soundChannel = m_sound->Play(SOUND_MANIP, m_posSound, 0.3f, 1.5f, true);
        m_sound->AddEnvelope(m_soundChannel, 0.3f, 1.5f, BASE_DOOR_TIME2, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.5f, 0.5f, SOPER_STOP);

        m_phase    = ABP_CLOSE2;
        m_progress = 0.0f;
        m_speed    = 1.0f/BASE_DOOR_TIME2;
        return true;
    }

    if ( event.type != EVENT_FRAME )  return true;
    if ( m_phase == ABP_WAIT )  return true;

    m_progress += event.rTime*m_speed;

    if ( m_phase == ABP_LAND )
    {
        if ( m_progress < 1.0f )
        {
            pos = m_pos;
            pos.y += powf(1.0f-m_progress, 2.0f)*300.0f;
            m_object->SetPosition(0, pos);
            MoveCargo();  // all cargo moves

            vibCir.z = sinf(m_time*Math::PI* 2.01f)*(Math::PI/150.0f)+
                       sinf(m_time*Math::PI* 2.51f)*(Math::PI/200.0f)+
                       sinf(m_time*Math::PI*19.01f)*(Math::PI/400.0f);
            vibCir.x = sinf(m_time*Math::PI* 2.03f)*(Math::PI/150.0f)+
                       sinf(m_time*Math::PI* 2.52f)*(Math::PI/200.0f)+
                       sinf(m_time*Math::PI*19.53f)*(Math::PI/400.0f);
            vibCir.y = 0.0f;
            vibCir *= Math::Min(1.0f, (1.0f-m_progress)*3.0f);
            m_object->SetCirVibration(vibCir);

            pos = m_pos;
            pos.x -= 150.0f;
            m_terrain->AdjustToFloor(pos);
            pos.y += 10.0f;
            m_camera->SetScriptEye(pos);

            pos = m_object->GetPosition(0);
            pos.y += 50.0f;
            m_camera->SetScriptLookat(pos);

            m_engine->SetFocus(1.0f+(1.0f-m_progress));

            if ( m_lastParticle+m_engine->ParticleAdapt(0.10f) <= m_time )
            {
                m_lastParticle = m_time;

                // Dust thrown to the ground.
                pos = m_pos;
                pos.x += (Math::Rand()-0.5f)*10.0f;
                pos.z += (Math::Rand()-0.5f)*10.0f;
                angle = Math::Rand()*(Math::PI*2.0f);
                dist = m_progress*50.0f;
                p = Math::RotatePoint(angle, dist);
                speed.x = p.x;
                speed.z = p.y;
                speed.y = 0.0f;
                dim.x = (Math::Rand()*15.0f+15.0f)*m_progress;
                dim.y = dim.x;
                if ( dim.x >= 1.0f )
                {
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 2.0f);
                }

                // Particles are ejected from the jet engine.
                pos = m_object->GetPosition(0);
                pos.y += 6.0f;
                h = m_terrain->GetHeightToFloor(pos)/300.0f;
                speed.x = (Math::Rand()-0.5f)*(80.0f-50.0f*h);
                speed.z = (Math::Rand()-0.5f)*(80.0f-50.0f*h);
                speed.y = -(Math::Rand()*(h+1.0f)*40.0f+(h+1.0f)*40.0f);
                dim.x = Math::Rand()*2.0f+2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 2.0f, 10.0f, 2.0f);

                // Black smoke from the jet engine.
                if ( m_progress > 0.8f )
                {
                    pos = m_pos;
                    pos.x += (Math::Rand()-0.5f)*8.0f;
                    pos.z += (Math::Rand()-0.5f)*8.0f;
                    pos.y += 3.0f;
                    speed.x = (Math::Rand()-0.5f)*8.0f;
                    speed.z = (Math::Rand()-0.5f)*8.0f;
                    speed.y = 0.0f;
                    dim.x = Math::Rand()*4.0f+4.0f;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f, 0.0f, 2.0f);
                }
            }
        }
        else
        {
            m_bMotor = false;  // put out the reactor

            m_object->SetPosition(0, m_pos);  // setting down
            m_object->SetCirVibration(Math::Vector(0.0f, 0.0f, 0.0f));
            MoveCargo();  // all cargo moves

            // Impact with the ground.
            max = static_cast<int>(50.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                angle = Math::Rand()*(Math::PI*2.0f);
                p = Math::RotatePoint(angle, 46.0f);
                pos = m_pos;
                pos.x += p.x;
                pos.z += p.y;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*10.0f+10.0f;
                dim.y = dim.x;
                time = Math::Rand()*2.0f+1.5f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, time, 0.0f, 2.0f);
            }

//?         m_camera->StartEffect(CE_CRASH, m_pos, 1.0f);
            m_camera->StartEffect(Gfx::CAM_EFFECT_EXPLO, m_pos, 2.0f);
            m_engine->SetFocus(1.0f);
            m_sound->Play(SOUND_BOUM, m_posSound, 0.6f, 0.5f);

            m_phase    = ABP_OPENWAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == ABP_OPENWAIT )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticle+m_engine->ParticleAdapt(0.10f) <= m_time )
            {
                m_lastParticle = m_time;

                // Black smoke from the reactor.
                pos = m_pos;
                pos.x += (Math::Rand()-0.5f)*8.0f;
                pos.z += (Math::Rand()-0.5f)*8.0f;
                pos.y += 3.0f;
                speed.x = (Math::Rand()-0.5f)*8.0f;
                speed.z = (Math::Rand()-0.5f)*8.0f;
                speed.y = 0.0f;
                dim.x = Math::Rand()*4.0f+4.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f, 0.0f, 2.0f);
            }
        }
        else
        {
            m_soundChannel = m_sound->Play(SOUND_MANIP, m_posSound, 0.0f, 0.3f, true);
            m_sound->AddEnvelope(m_soundChannel, 0.3f, 0.3f, 1.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.3f, 1.0f, BASE_DOOR_TIME-1.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.3f, 1.0f, SOPER_STOP);

            m_phase    = ABP_OPEN;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_DOOR_TIME;
        }
    }

    if ( m_phase == ABP_OPEN )
    {
        if ( m_progress < 1.0f )
        {
            angle = -m_progress*124.0f*Math::PI/180.0f;
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetAngleZ(1+i, Math::PI/2.0f+angle);
            }

            if ( m_param != PARAM_PORTICO )
            {
                angle = m_progress*Math::PI*2.0f;
                p = Math::RotatePoint(angle, -150.0f);
                pos = m_pos;
                pos.x += p.x;
                pos.z += p.y;
                m_terrain->AdjustToFloor(pos);
                pos.y += 10.0f;
                pos.y += m_progress*40.0f;
                m_camera->SetScriptEye(pos);

                m_engine->SetFogStart(0.9f-(0.9f-m_fogStart)*m_progress);
            }
        }
        else
        {
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetAngleZ(1+i, Math::PI/2.0f-124.0f*Math::PI/180.0f);
            }

            // Clash the doors with the ground.
            max = static_cast<int>(20.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                angle = Math::Rand()*(20.0f*Math::PI/180.0f)-(10.0f*Math::PI/180.0f);
                angle += (Math::PI/4.0f)*(rand()%8);
                p = Math::RotatePoint(angle, 74.0f);
                pos = m_pos;
                pos.x += p.x;
                pos.z += p.y;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*8.0f+8.0f;
                dim.y = dim.x;
                time = Math::Rand()*2.0f+1.5f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, time, 0.0f, 2.0f);
            }

            m_soundChannel = m_sound->Play(SOUND_MANIP, m_posSound, 0.3f, 1.5f, true);
            m_sound->AddEnvelope(m_soundChannel, 0.3f, 1.5f, BASE_DOOR_TIME2, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.5f, 0.5f, SOPER_STOP);

            m_phase    = ABP_OPEN2;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_DOOR_TIME2;
        }
    }

    if ( m_phase == ABP_OPEN2 )
    {
        if ( m_progress < 1.0f )
        {
            len = 7.0f-m_progress*(7.0f+11.5f);
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetPosition(10+i, Math::Vector(23.5f, 0.0f,  len));
                m_object->SetPosition(18+i, Math::Vector(23.5f, 0.0f, -len));
                m_object->SetAngleX(10+i, -10.0f*Math::PI/180.0f*m_progress);
                m_object->SetAngleX(18+i,  10.0f*Math::PI/180.0f*m_progress);
            }

            if ( m_param != PARAM_PORTICO )
            {
                angle = m_progress*Math::PI/2.0f;
                p = Math::RotatePoint(angle, -150.0f);
                pos = m_pos;
                pos.x += p.x;
                pos.z += p.y;
                m_terrain->AdjustToFloor(pos);
                pos.y += 10.0f;
                pos.y += m_progress*40.0f;
                m_camera->SetScriptEye(pos);

                m_engine->SetFogStart(0.9f-(0.9f-m_fogStart)*m_progress);
            }
        }
        else
        {
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetPosition(10+i, Math::Vector(23.5f, 0.0f, -11.5f));
                m_object->SetPosition(18+i, Math::Vector(23.5f, 0.0f,  11.5f));
                m_object->SetAngleX(10+i, -10.0f*Math::PI/180.0f);
                m_object->SetAngleX(18+i,  10.0f*Math::PI/180.0f);
            }

            m_phase    = ABP_LDWAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ABP_LDWAIT )
    {
        if ( m_progress >= 1.0f )
        {
            FreezeCargo(false);  // frees all cargo

            if ( m_param != PARAM_PORTICO )
            {
                m_main->SetMovieLock(false);  // you can play!

                pObj = m_main->GetSelectObject();
                m_main->SelectObject(pObj);
                m_camera->SetControllingObject(pObj);
                if ( pObj == 0 )
                {
                    m_camera->SetType(Gfx::CAM_TYPE_BACK);
                }
                else
                {
                    m_camera->SetType(pObj->GetCameraType());
                    m_camera->SetDist(pObj->GetCameraDist());
                }
                m_sound->Play(SOUND_BOUM, m_object->GetPosition(0));
                m_soundChannel = -1;

                m_engine->SetFogStart(m_fogStart);
            }

            m_bOpen    = true;
            m_phase    = ABP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ABP_CLOSE2 )
    {
        if ( m_progress < 1.0f )
        {
            len = 7.0f-(1.0f-m_progress)*(7.0f+11.5f);
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetPosition(10+i, Math::Vector(23.5f, 0.0f,  len));
                m_object->SetPosition(18+i, Math::Vector(23.5f, 0.0f, -len));
                m_object->SetAngleX(10+i, -10.0f*Math::PI/180.0f*(1.0f-m_progress));
                m_object->SetAngleX(18+i,  10.0f*Math::PI/180.0f*(1.0f-m_progress));
            }
        }
        else
        {
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetPosition(10+i, Math::Vector(23.5f, 0.0f,  7.0f));
                m_object->SetPosition(18+i, Math::Vector(23.5f, 0.0f, -7.0f));
                m_object->SetAngleX(10+i, 0.0f);
                m_object->SetAngleX(18+i, 0.0f);
            }

            m_soundChannel = m_sound->Play(SOUND_MANIP, m_posSound, 0.0f, 0.3f, true);
            m_sound->AddEnvelope(m_soundChannel, 0.3f, 0.3f, 1.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.3f, 1.0f, BASE_DOOR_TIME-1.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.3f, 1.0f, SOPER_STOP);

            m_phase    = ABP_CLOSE;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_DOOR_TIME;
        }
    }

    if ( m_phase == ABP_CLOSE )
    {
        if ( m_progress < 1.0f )
        {
            angle = -(1.0f-m_progress)*124.0f*Math::PI/180.0f;
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetAngleZ(1+i, Math::PI/2.0f+angle);
            }
        }
        else
        {
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetAngleZ(1+i, Math::PI/2.0f);
            }
            m_bMotor = true;  // lights the jet engine

            // Shock of the closing doors.
            max = static_cast<int>(20.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                angle = Math::Rand()*Math::PI*2.0f;
                p = Math::RotatePoint(angle, 32.0f);
                pos = m_pos;
                pos.x += p.x;
                pos.z += p.y;
                pos.y += 85.0f;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*3.0f+3.0f;
                dim.y = dim.x;
                time = Math::Rand()*1.0f+1.0f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, time);
            }
            m_sound->Play(SOUND_BOUM, m_object->GetPosition(0));

            m_soundChannel = -1;
            m_bOpen    = false;
            m_phase    = ABP_TOWAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == ABP_TOWAIT )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_soundChannel == -1 )
            {
                m_soundChannel = m_sound->Play(SOUND_FLY, m_posSound, 0.0f, 0.5f, true);
                m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.5f, 2.0f, SOPER_CONTINUE);
                m_sound->AddEnvelope(m_soundChannel, 0.3f, 2.0f, BASE_TAKO_TIME, SOPER_STOP);
            }

            vibCir.z = sinf(m_time*Math::PI*19.01f)*(Math::PI/400.0f);
            vibCir.x = sinf(m_time*Math::PI*19.53f)*(Math::PI/400.0f);
            vibCir.y = 0.0f;
            vibCir *= m_progress*1.0f;
            m_object->SetCirVibration(vibCir);

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                // Particles are ejected from the reactor.
                pos = m_object->GetPosition(0);
                pos.y += 6.0f;
                speed.x = (Math::Rand()-0.5f)*160.0f;
                speed.z = (Math::Rand()-0.5f)*160.0f;
                speed.y = -(Math::Rand()*10.0f+10.0f);
                dim.x = Math::Rand()*2.0f+2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 2.0f, 10.0f, 2.0f);
            }

            m_engine->SetFogStart(m_fogStart+(0.9f-m_fogStart)*m_progress);
        }
        else
        {
            m_engine->SetFogStart(0.9f);

            m_phase    = ABP_TAKEOFF;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_TAKO_TIME;
        }
    }

    if ( m_phase == ABP_TAKEOFF )
    {
        if ( m_progress < 1.0f )
        {
            pos = m_pos;
            pos.y += powf(m_progress, 2.0f)*600.0f;
            m_object->SetPosition(0, pos);
            MoveCargo();  // all cargo moves

            vibCir.z = sinf(m_time*Math::PI*19.01f)*(Math::PI/400.0f);
            vibCir.x = sinf(m_time*Math::PI*19.53f)*(Math::PI/400.0f);
            vibCir.y = 0.0f;
            m_object->SetCirVibration(vibCir);

            pos = m_pos;
            pos.x -= 110.0f+m_progress*250.0f;
            m_terrain->AdjustToFloor(pos);
            pos.y += 10.0f;
            m_camera->SetScriptEye(pos);

            pos = m_object->GetPosition(0);
            pos.y += 50.0f;
            m_camera->SetScriptLookat(pos);

            m_engine->SetFocus(1.0f+m_progress);

            if ( m_lastParticle+m_engine->ParticleAdapt(0.10f) <= m_time )
            {
                m_lastParticle = m_time;

                // Dust thrown to the ground.
                pos = m_pos;
                pos.x += (Math::Rand()-0.5f)*10.0f;
                pos.z += (Math::Rand()-0.5f)*10.0f;
                angle = Math::Rand()*(Math::PI*2.0f);
                dist = (1.0f-m_progress)*50.0f;
                p = Math::RotatePoint(angle, dist);
                speed.x = p.x;
                speed.z = p.y;
                speed.y = 0.0f;
                dim.x = (Math::Rand()*10.0f+10.0f)*(1.0f-m_progress);
                dim.y = dim.x;
                if ( dim.x >= 1.0f )
                {
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 2.0f);
                }

                // Particles are ejected from the reactor.
                pos = m_object->GetPosition(0);
                pos.y += 6.0f;
                speed.x = (Math::Rand()-0.5f)*40.0f;
                speed.z = (Math::Rand()-0.5f)*40.0f;
                time = 5.0f+150.0f*m_progress;
                speed.y = -(Math::Rand()*time+time);
                time = 2.0f+m_progress*12.0f;
                dim.x = Math::Rand()*time+time;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 2.0f, 10.0f, 2.0f);

                // Black smoke from the reactor.
                pos = m_object->GetPosition(0);
                pos.y += 3.0f;
                speed.x = (Math::Rand()-0.5f)*10.0f*(4.0f-m_progress*3.0f);
                speed.z = (Math::Rand()-0.5f)*10.0f*(4.0f-m_progress*3.0f);
                speed.y = 0.0f;
                dim.x = Math::Rand()*20.0f+20.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 10.0f, 0.0f, 2.0f);
            }
        }
        else
        {
            m_soundChannel = -1;
            newEvent.type = EVENT_WIN;
            m_eventQueue->AddEvent(newEvent);

            m_phase    = ABP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == ABP_PORTICO_MOVE )  // advance of the gate?
    {
        if ( m_progress < 1.0f )
        {
            pos = m_object->GetPosition(0);
            pos.z -= event.rTime*5.0f;
            m_object->SetPosition(0, pos);
            MoveCargo();  // all cargo moves
        }
        else
        {
            m_phase    = ABP_PORTICO_WAIT1;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ABP_PORTICO_WAIT1 )  // expectation the gate?
    {
        if ( m_progress >= 1.0f )
        {
            m_phase    = ABP_PORTICO_DOWN;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_PORTICO_TIME_DOWN;
        }
    }

    if ( m_phase == ABP_PORTICO_DOWN )  // down the gate?
    {
        if ( m_progress < 1.0f )
        {
            pos = m_object->GetPosition(0);
            pos.y -= event.rTime*(10.0f/BASE_PORTICO_TIME_DOWN);
            m_object->SetPosition(0, pos);
            MoveCargo();  // all cargo moves
        }
        else
        {
            // Impact with the ground.
            max = static_cast<int>(50.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                angle = Math::Rand()*(Math::PI*2.0f);
                p = Math::RotatePoint(angle, 46.0f);
                pos = m_pos;
                pos.x += p.x;
                pos.z += p.y;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*10.0f+10.0f;
                dim.y = dim.x;
                time = Math::Rand()*2.0f+1.5f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, time, 0.0f, 2.0f);
            }

            m_phase    = ABP_PORTICO_WAIT2;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ABP_PORTICO_WAIT2 )  // expectation the gate?
    {
        if ( m_progress >= 1.0f )
        {
            m_phase    = ABP_PORTICO_OPEN;
            m_progress = 0.0f;
            m_speed    = 1.0f/BASE_PORTICO_TIME_OPEN;
        }
    }

    if ( m_phase == ABP_PORTICO_OPEN )  // opening the gate?
    {
        if ( m_progress < 1.0f )
        {
        }
        else
        {
            m_phase    = ABP_OPEN;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == ABP_TRANSIT_MOVE )  // transit in space?
    {
        if ( m_progress < 1.0f )
        {
            pos = m_object->GetPosition(0);
            pos.x += event.rTime*(2000.0f/BASE_TRANSIT_TIME);
            m_object->SetPosition(0, pos);
            pos.x += 60.0f;
            m_camera->SetScriptLookat(pos);
        }
        else
        {
            m_object->SetAngleZ(0, 0.0f);

            m_param = PARAM_LANDING;
            m_phase    = ABP_START;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;

            EndTransit();

            if ( m_soundChannel != -1 )
            {
                m_sound->FlushEnvelope(m_soundChannel);
                m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.8f, 0.01f, SOPER_STOP);
                m_soundChannel = -1;
            }
            goto begin;
        }
    }

    if ( m_bMotor )
    {
        if ( m_lastMotorParticle+m_engine->ParticleAdapt(0.02f) <= m_time )
        {
            m_lastMotorParticle = m_time;

            mat = m_object->GetWorldMatrix(0);

            if ( event.rTime == 0.0f )
            {
                vSpeed = 0.0f;
            }
            else
            {
                pos = m_object->GetPosition(0);
                if ( m_phase == ABP_TRANSIT_MOVE )
                {
                    vSpeed = (pos.x-iPos.x)/event.rTime;
                }
                else
                {
                    vSpeed = (pos.y-iPos.y)/event.rTime;
                }
                if ( vSpeed < 0.0f )  vSpeed *= 1.5f;
            }

            pos = Math::Vector(0.0f, 6.0f, 0.0f);
            speed.x = (Math::Rand()-0.5f)*4.0f;
            speed.z = (Math::Rand()-0.5f)*4.0f;
            speed.y = vSpeed*0.8f-(8.0f+Math::Rand()*6.0f);
            speed += pos;
            pos = Transform(*mat, pos);
            speed = Transform(*mat, speed);
            speed -= pos;

            dim.x = 4.0f+Math::Rand()*4.0f;
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBASE, 3.0f, 0.0f, 0.0f);

            if ( m_phase == ABP_TRANSIT_MOVE )
            {
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = 12.0f;
                dim.y = dim.x;
                pos = Math::Vector(0.0f, 7.0f, 0.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 1.0f, 0.0f, 0.0f);

                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = 4.0f;
                dim.y = dim.x;
                pos = Math::Vector(42.0f, 0.0f, 17.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 0.5f, 0.0f, 0.0f);
                pos = Math::Vector(17.0f, 0.0f, 42.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 0.5f, 0.0f, 0.0f);
                pos = Math::Vector(42.0f, 0.0f, -17.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 0.5f, 0.0f, 0.0f);
                pos = Math::Vector(17.0f, 0.0f, -42.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 0.5f, 0.0f, 0.0f);
                pos = Math::Vector(-42.0f, 0.0f, 17.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 0.5f, 0.0f, 0.0f);
                pos = Math::Vector(-17.0f, 0.0f, 42.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 0.5f, 0.0f, 0.0f);
                pos = Math::Vector(-42.0f, 0.0f, -17.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 0.5f, 0.0f, 0.0f);
                pos = Math::Vector(-17.0f, 0.0f, -42.0f);
                pos.x += (Math::Rand()-0.5f)*2.0f;  pos.z += (Math::Rand()-0.5f)*2.0f;
                pos = Transform(*mat, pos);
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 0.5f, 0.0f, 0.0f);

                pos = Math::Vector(42.0f, -2.0f, 17.0f);
                pos = Transform(*mat, pos);
                m_particle->SetPosition(m_partiChannel[0], pos);
                pos = Math::Vector(17.0f, -2.0f, 42.0f);
                pos = Transform(*mat, pos);
                m_particle->SetPosition(m_partiChannel[1], pos);
                pos = Math::Vector(42.0f, -2.0f, -17.0f);
                pos = Transform(*mat, pos);
                m_particle->SetPosition(m_partiChannel[2], pos);
                pos = Math::Vector(17.0f, -2.0f, -42.0f);
                pos = Transform(*mat, pos);
                m_particle->SetPosition(m_partiChannel[3], pos);
                pos = Math::Vector(-42.0f, -2.0f, 17.0f);
                pos = Transform(*mat, pos);
                m_particle->SetPosition(m_partiChannel[4], pos);
                pos = Math::Vector(-17.0f, -2.0f, 42.0f);
                pos = Transform(*mat, pos);
                m_particle->SetPosition(m_partiChannel[5], pos);
                pos = Math::Vector(-42.0f, -2.0f, -17.0f);
                pos = Transform(*mat, pos);
                m_particle->SetPosition(m_partiChannel[6], pos);
                pos = Math::Vector(-17.0f, -2.0f, -42.0f);
                pos = Transform(*mat, pos);
                m_particle->SetPosition(m_partiChannel[7], pos);
            }
        }
    }

    if ( m_soundChannel != -1 )
    {
        pos = m_engine->GetEyePt();
        m_sound->Position(m_soundChannel, pos);
    }

    return true;
}

// Stops the controller.

bool CAutoBase::Abort()
{
    Event       newEvent;
    CObject*    pObj;
    int         i;

    if ( m_phase == ABP_TRANSIT_MOVE )  // transit ?
    {
        m_object->SetAngleZ(0, 0.0f);

        m_param = PARAM_LANDING;
        m_phase    = ABP_START;
        m_progress = 0.0f;
        m_speed    = 1.0f/1.0f;

        EndTransit();

        if ( m_soundChannel != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.8f, 0.01f, SOPER_STOP);
            m_soundChannel = -1;
        }
        return true;
    }

    if ( m_param == PARAM_PORTICO )  // gate on the porch?
    {
        m_object->SetPosition(0, m_finalPos);
        MoveCargo();  // all cargo moves

        for ( i=0 ; i<8 ; i++ )
        {
            m_object->SetAngleZ(1+i, Math::PI/2.0f-124.0f*Math::PI/180.0f);
            m_object->SetAngleX(10+i, -10.0f*Math::PI/180.0f);
            m_object->SetAngleX(18+i,  10.0f*Math::PI/180.0f);
            m_object->SetPosition(10+i, Math::Vector(23.5f, 0.0f, -11.5f));
            m_object->SetPosition(18+i, Math::Vector(23.5f, 0.0f,  11.5f));
        }
    }
    else
    {
        if ( m_phase == ABP_LAND     ||
             m_phase == ABP_OPENWAIT ||
             m_phase == ABP_OPEN     ||
             m_phase == ABP_OPEN2    )  // Landing?
        {
            m_bMotor = false;  // put out the jet engine
            m_bOpen = true;

            m_object->SetPosition(0, m_pos);  // setting down
            m_object->SetCirVibration(Math::Vector(0.0f, 0.0f, 0.0f));
            MoveCargo();  // all cargo moves
            for ( i=0 ; i<8 ; i++ )
            {
                m_object->SetAngleZ(1+i, Math::PI/2.0f-124.0f*Math::PI/180.0f);
                m_object->SetAngleX(10+i, -10.0f*Math::PI/180.0f);
                m_object->SetAngleX(18+i,  10.0f*Math::PI/180.0f);
                m_object->SetPosition(10+i, Math::Vector(23.5f, 0.0f, -11.5f));
                m_object->SetPosition(18+i, Math::Vector(23.5f, 0.0f,  11.5f));
            }

            m_main->SetMovieLock(false);  // you can play!

            pObj = m_main->GetSelectObject();
            m_main->SelectObject(pObj);
            m_camera->SetControllingObject(pObj);
            if ( pObj == 0 )
            {
                m_camera->SetType(Gfx::CAM_TYPE_BACK);
            }
            else
            {
                m_camera->SetType(pObj->GetCameraType());
                m_camera->SetDist(pObj->GetCameraDist());
            }

            m_engine->SetFogStart(m_fogStart);
        }

        if ( m_phase == ABP_CLOSE2  ||
             m_phase == ABP_CLOSE   ||
             m_phase == ABP_TOWAIT  ||
             m_phase == ABP_TAKEOFF )  // off?
        {
            newEvent.type = EVENT_WIN;
            m_eventQueue->AddEvent(newEvent);
        }
    }

    m_object->SetAngleZ(0, 0.0f);
    FreezeCargo(false);  // frees all cargo

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_phase    = ABP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    return true;
}


// Returns an error due the state of the automation.

Error CAutoBase::GetError()
{
    return ERR_OK;
}


// Creates all the interface when the object is selected.

bool CAutoBase::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, dim, ddim;
    float       ox, oy, sx, sy;
    float       sleep, delay, magnetic, progress;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return false;

    dim.x = 33.0f/640.0f;
    dim.y = 33.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    ddim.x = dim.x*1.5f;
    ddim.y = dim.y*1.5f;

//? pos.x = ox+sx*7.25f;
//? pos.y = oy+sy*0.25f;
//? pw->CreateButton(pos, ddim, 63, EVENT_OBJECT_BHELP);

    pos.x = ox+sx*8.00f;
    pos.y = oy+sy*0.25f;
    pw->CreateButton(pos, ddim, 28, EVENT_OBJECT_BTAKEOFF);

    if ( m_lightning->GetStatus(sleep, delay, magnetic, progress) )
    {
        pos.x = ox+sx*10.2f;
        pos.y = oy+sy*0.5f;
        ddim.x = dim.x*1.0f;
        ddim.y = dim.y*1.0f;
        pw->CreateButton(pos, ddim, 41, EVENT_OBJECT_LIMIT);
    }

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 100, EVENT_OBJECT_TYPE);

    UpdateInterface();

    return true;
}

// Updates the status of all interface buttons.

void CAutoBase::UpdateInterface()
{
//    Ui::CWindow*    pw;

    if ( !m_object->GetSelect() )  return;

    CAuto::UpdateInterface();

//    pw = static_cast< Ui::CWindow* >( m_interface->SearchControl(EVENT_WINDOW0));
}


// Freeze or frees all cargo.

void CAutoBase::FreezeCargo(bool bFreeze)
{
    CObject*    pObj;
    CPhysics*   physics;
    Math::Vector    oPos;
    float       dist;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        pObj->SetCargo(false);

        if ( pObj == m_object )  continue;  // yourself?
        if ( pObj->GetTruck() != 0 )  continue;  // transport object?

        oPos = pObj->GetPosition(0);
        dist = Math::DistanceProjected(m_pos, oPos);
        if ( dist < 32.0f )
        {
            if ( bFreeze )
            {
                pObj->SetCargo(true);
            }

            physics = pObj->GetPhysics();
            if ( physics != 0 )
            {
                physics->SetFreeze(bFreeze);
            }
        }
    }
}

// All cargo moves vertically with the ship.

void CAutoBase::MoveCargo()
{
    CObject*    pObj;
    Math::Vector    oPos, sPos;
    int         i;

    sPos = m_object->GetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast < CObject* > (m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        if ( !pObj->GetCargo() )  continue;

        oPos = pObj->GetPosition(0);
        oPos.y = sPos.y+30.0f;
        oPos.y += pObj->GetCharacter()->height;
        oPos.x += sPos.x-m_lastPos.x;
        oPos.z += sPos.z-m_lastPos.z;
        pObj->SetPosition(0, oPos);
    }

    m_lastPos = sPos;
}


// Checks whether it is possible to close the doors.

Error CAutoBase::CheckCloseDoor()
{
    CObject*    pObj;
    Math::Vector    oPos;
    ObjectType  type;
    float       oRad, dist;
    int         i, j;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* > (m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        if ( pObj == m_object )  continue;  // yourself?
        if ( !pObj->GetActif() )  continue;  // inactive?

        type = pObj->GetType();
        if ( type == OBJECT_PORTICO )  continue;

        j = 0;
        while ( pObj->GetCrashSphere(j++, oPos, oRad) )
        {
            dist = Math::DistanceProjected(m_pos, oPos);
            if ( dist+oRad > 32.0f &&
                 dist-oRad < 72.0f )
            {
                return ERR_BASE_DLOCK;
            }

            if ( type == OBJECT_HUMAN &&
                 dist+oRad > 32.0f    )
            {
                return ERR_BASE_DHUMAN;
            }
        }
    }
    return ERR_OK;
}


// Start a transit.

void CAutoBase::BeginTransit()
{
    if ( m_param == PARAM_TRANSIT2 )
    {
        m_bgBack = "back01.png";  // clouds orange / blue
    }
    else if ( m_param == PARAM_TRANSIT3 )
    {
        m_bgBack = "back22.png";  // blueberries clouds
    }
    else
    {
#if _DEMO
        m_bgBack = "back46b.png";  // paintings
#else
        m_bgBack = "back46.png";  // paintings
#endif
    }

    m_engine->SetFogStart(0.9f);  // hardly any fog
    m_engine->SetDeepView(2000.0f);  // we see very far
    m_engine->ApplyChange();

    Math::Point scale;
    bool full;
    m_engine->GetBackground(m_bgName, m_bgUp, m_bgDown, m_bgCloudUp, m_bgCloudDown, full, scale);
    m_engine->DeleteTexture(m_bgName);

    m_engine->SetBackground(m_bgBack, Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f));
    m_engine->LoadTexture(m_bgBack);

    m_cloud->SetEnabled(false);  // cache clouds
    m_planet->SetMode(1);
}

// End of a transit.

void CAutoBase::EndTransit()
{
    m_engine->SetFogStart(m_fogStart);  // gives initial fog
    m_engine->SetDeepView(m_deepView);  // gives initial depth
    m_engine->ApplyChange();

    m_engine->DeleteTexture(m_bgBack);

    m_engine->SetBackground(m_bgName, m_bgUp, m_bgDown, m_bgCloudUp, m_bgCloudDown);
    m_engine->LoadTexture(m_bgName);

    m_cloud->SetEnabled(true);  // gives the clouds
    m_planet->SetMode(0);

    m_main->StartMusic();
}

