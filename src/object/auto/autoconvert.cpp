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


#include "object/auto/autoconvert.h"

#include "common/make_unique.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/transportable_object.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"


// Object's constructor.

CAutoConvert::CAutoConvert(COldObject* object) : CAuto(object)
{
    Init();
    m_phase = ACP_STOP;
    m_soundChannel = -1;
}

// Object's destructor.

CAutoConvert::~CAutoConvert()
{
}


// Destroys the object.

void CAutoConvert::DeleteObject(bool all)
{
    if ( !all )
    {
        CObject* cargo = SearchStone(OBJECT_STONE);
        if ( cargo != nullptr )
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

void CAutoConvert::Init()
{
    m_phase = ACP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;

    CAuto::Init();
}


// Management of an event.

bool CAutoConvert::EventProcess(const Event &event)
{
    CObject*    cargo;
    Math::Vector    pos, speed;
    Math::Point     dim, c, p;
    float       angle;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;
    m_timeVirus -= event.rTime;

    if ( m_object->GetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Math::Rand()*0.3f;

            angle = (Math::Rand()-0.5f)*0.3f;
            m_object->SetPartRotationY(1, angle);
            m_object->SetPartRotationY(2, angle);
            m_object->SetPartRotationY(3, angle+Math::PI);

            m_object->SetPartRotationX(2, -Math::PI*0.35f*(0.8f+Math::Rand()*0.2f));
            m_object->SetPartRotationX(3, -Math::PI*0.35f*(0.8f+Math::Rand()*0.2f));
        }
        return true;
    }

    EventProgress(event.rTime);

    if ( m_phase == ACP_STOP )  return true;

    if ( m_phase == ACP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            cargo = SearchStone(OBJECT_STONE);  // Has stone transformed?
            if ( cargo == nullptr || SearchVehicle() )
            {
                m_phase    = ACP_WAIT;  // still waiting ...
                m_progress = 0.0f;
                m_speed    = 1.0f/2.0f;
            }
            else
            {
                cargo->SetLock(true);  // stone usable

                SetBusy(true);
                InitProgressTotal(3.0f+10.0f+1.5f);
                UpdateInterface();

                m_sound->Play(SOUND_OPEN, m_object->GetPosition(), 1.0f, 1.0f);
                m_bSoundClose = false;

                m_phase    = ACP_CLOSE;
                m_progress = 0.0f;
                m_speed    = 1.0f/3.0f;
            }
        }
    }

    if ( m_phase == ACP_CLOSE )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_progress >= 0.8f && !m_bSoundClose )
            {
                m_bSoundClose = true;
                m_sound->Play(SOUND_CLOSE, m_object->GetPosition(), 1.0f, 0.8f);
            }
            angle = -Math::PI*0.35f*(1.0f-Math::Bounce(m_progress, 0.85f, 0.05f));
            m_object->SetPartRotationX(2, angle);
            m_object->SetPartRotationX(3, angle);
        }
        else
        {
            m_object->SetPartRotationX(2, 0.0f);
            m_object->SetPartRotationX(3, 0.0f);

            m_soundChannel = m_sound->Play(SOUND_CONVERT, m_object->GetPosition(), 0.0f, 0.25f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.25f, 0.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.00f, 4.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.25f, 4.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.25f, 0.5f, SOPER_STOP);

            m_phase    = ACP_ROTATE;
            m_progress = 0.0f;
            m_speed    = 1.0f/10.0f;
        }
    }

    if ( m_phase == ACP_ROTATE )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_progress < 0.5f )
            {
                angle = powf((m_progress*2.0f)*5.0f, 2.0f);  // accelerates
            }
            else
            {
                angle = -powf((2.0f-m_progress*2.0f)*5.0f, 2.0f);  // slows
            }
            m_object->SetPartRotationY(1, angle);
            m_object->SetPartRotationY(2, angle);
            m_object->SetPartRotationY(3, angle+Math::PI);

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_object->GetPosition();
                c.x = pos.x;
                c.y = pos.z;
                p.x = c.x;
                p.y = c.y+6.0f;
                p = Math::RotatePoint(c, Math::Rand()*Math::PI*2.0f, p);
                pos.x = p.x;
                pos.z = p.y;
                pos.y += 1.0f;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*2.0f+1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGAS, 1.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            m_object->SetPartRotationY(1, 0.0f);
            m_object->SetPartRotationY(2, 0.0f);
            m_object->SetPartRotationY(3, Math::PI);

            cargo = SearchStone(OBJECT_STONE);
            if ( cargo != nullptr )
            {
                CObjectManager::GetInstancePointer()->DeleteObject(cargo);
            }

            CreateMetal();  // Create the metal
            m_sound->Play(SOUND_OPEN, m_object->GetPosition(), 1.0f, 1.5f);

            m_phase    = ACP_OPEN;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ACP_OPEN )
    {
        if ( m_progress < 1.0f )
        {
            angle = -Math::PI*0.35f*Math::Bounce(m_progress, 0.7f, 0.2f);
            m_object->SetPartRotationX(2, angle);
            m_object->SetPartRotationX(3, angle);

            if ( m_progress < 0.9f &&
                 m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_object->GetPosition();
                pos.x += (Math::Rand()-0.5f)*6.0f;
                pos.z += (Math::Rand()-0.5f)*6.0f;
                pos.y += Math::Rand()*4.0f;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*4.0f+3.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLUE, 1.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            m_soundChannel = -1;
            m_object->SetPartRotationX(2, -Math::PI*0.35f);
            m_object->SetPartRotationX(3, -Math::PI*0.35f);

            SetBusy(false);
            UpdateInterface();

            m_phase    = ACP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    return true;
}

// Returns an error due the state of the automation.

Error CAutoConvert::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    if ( m_phase == ACP_WAIT )  return ERR_CONVERT_EMPTY;
    return ERR_OK;
}

// Cancels the current transformation.

bool CAutoConvert::Abort()
{
    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_object->SetPartRotationY(1, 0.0f);
    m_object->SetPartRotationY(2, 0.0f);
    m_object->SetPartRotationY(3, Math::PI);
    m_object->SetPartRotationX(2, -Math::PI*0.35f);
    m_object->SetPartRotationX(3, -Math::PI*0.35f);

    m_phase    = ACP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    SetBusy(false);
    UpdateInterface();

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoConvert::CreateInterface(bool bSelect)
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
    pw->CreateGroup(pos, ddim, 103, EVENT_OBJECT_TYPE);

    return true;
}


// Saves all parameters of the controller.

bool CAutoConvert::Write(CLevelParserLine* line)
{
    if ( m_phase == ACP_STOP ||
         m_phase == ACP_WAIT )  return false;

    line->AddParam("aExist", MakeUnique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", MakeUnique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", MakeUnique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", MakeUnique<CLevelParserParam>(m_speed));

    return true;
}

// Restores all parameters of the controller.

bool CAutoConvert::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoConvertPhase >(line->GetParam("aPhase")->AsInt(ACP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;

    return true;
}


// Searches for the object before or during processing.

CObject* CAutoConvert::SearchStone(ObjectType type)
{
    Math::Vector cPos = m_object->GetPosition();

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType oType = obj->GetType();
        if ( oType != type )  continue;
        if (IsObjectBeingTransported(obj)) continue;

        Math::Vector oPos = obj->GetPosition();
        float dist = Math::Distance(oPos, cPos);

        if ( dist <= 5.0f )  return obj;
    }

    return nullptr;
}

// Search if a vehicle is too close.

bool CAutoConvert::SearchVehicle()
{
    Math::Vector cPos = m_object->GetPosition();

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (obj == m_object) continue;
        ObjectType type = obj->GetType();
        if ( type == OBJECT_STONE ) continue;

        if (obj->GetCrashSphereCount() == 0) continue;

        auto crashSphere = obj->GetFirstCrashSphere();
        if (Math::DistanceToSphere(cPos, crashSphere.sphere) < 8.0f)
            return true;
    }

    return false;
}

// Creates an object metal.

void CAutoConvert::CreateMetal()
{
    Math::Vector pos = m_object->GetPosition();
    float angle = m_object->GetRotationY();

    CObjectManager::GetInstancePointer()->CreateObject(pos, angle, OBJECT_METAL);

    m_main->DisplayError(INFO_CONVERT, m_object);
}
