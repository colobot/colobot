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


#include "object/auto/autonuclearplant.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/slotted_object.h"
#include "object/interface/transportable_object.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"


const float NUCLEARPLANT_DELAY = 30.0f;  // duration of the generation




// Object's constructor.

CAutoNuclearPlant::CAutoNuclearPlant(COldObject* object) : CAuto(object)
{
    m_channelSound = -1;
    Init();

    assert(m_object->GetNumSlots() == 1);
}

// Object's destructor.

CAutoNuclearPlant::~CAutoNuclearPlant()
{
}


// Destroys the object.

void CAutoNuclearPlant::DeleteObject(bool all)
{
    if ( !all )
    {
        CObject* cargo = SearchUranium();
        if ( cargo != nullptr )
        {
            CObjectManager::GetInstancePointer()->DeleteObject(cargo);
        }
    }

    if ( m_channelSound != -1 )
    {
        m_sound->FlushEnvelope(m_channelSound);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_channelSound = -1;
    }

    CAuto::DeleteObject(all);
}


// Initialize the object.

void CAutoNuclearPlant::Init()
{
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;

    glm::mat4 mat = m_object->GetWorldMatrix(0);
    m_pos = Math::Transform(mat, glm::vec3(22.0f, 4.0f, 0.0f));

    m_phase    = ANUP_WAIT;  // waiting ...
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    CAuto::Init();
}


// Management of an event.

bool CAutoNuclearPlant::EventProcess(const Event &event)
{
    CObject*    cargo;
    glm::vec3    pos, speed;
    glm::vec2     dim;
    float       angle;
    int         i, max;

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
        }
        return true;
    }

    EventProgress(event.rTime);

    if ( m_phase == ANUP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            cargo = SearchUranium();  // transform uranium?
            if ( cargo == nullptr || SearchVehicle() )
            {
                m_phase    = ANUP_WAIT;  // still waiting ...
                m_progress = 0.0f;
                m_speed    = 1.0f/2.0f;
            }
            else
            {
                cargo->SetLock(true);  // usable uranium

                SetBusy(true);
                InitProgressTotal(1.5f+NUCLEARPLANT_DELAY+1.5f);
                UpdateInterface();

                m_sound->Play(SOUND_OPEN, m_object->GetPosition(), 1.0f, 1.4f);

                m_phase    = ANUP_CLOSE;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.5f;
            }
        }
    }

    if ( m_phase == ANUP_CLOSE )
    {
        if ( m_progress < 1.0f )
        {
            angle = (1.0f-m_progress)*(135.0f*Math::PI/180.0f);
            m_object->SetPartRotationZ(1, angle);
        }
        else
        {
            m_object->SetPartRotationZ(1, 0.0f);

            glm::mat4 mat = m_object->GetWorldMatrix(0);
            max = static_cast< int >(10.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                pos.x = 27.0f;
                pos.y =  0.0f;
                pos.z = (Math::Rand()-0.5f)*8.0f;
                pos = Math::Transform(mat, pos);
                speed.y = 0.0f;
                speed.x = 0.0f;
                speed.z = 0.0f;
                dim.x = Math::Rand()*1.0f+1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH);
            }

            m_sound->Play(SOUND_CLOSE, m_object->GetPosition(), 1.0f, 1.0f);

            m_channelSound = m_sound->Play(SOUND_NUCLEAR, m_object->GetPosition(), 1.0f, 0.1f, true);
            m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, NUCLEARPLANT_DELAY-1.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 2.0f, SOPER_STOP);

            m_phase    = ANUP_GENERATE;
            m_progress = 0.0f;
            m_speed    = 1.0f/NUCLEARPLANT_DELAY;
        }
    }

    if ( m_phase == ANUP_GENERATE )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticle+m_engine->ParticleAdapt(0.10f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_object->GetPosition();
                pos.y += 30.0f;
                pos.x += (Math::Rand()-0.5f)*6.0f;
                pos.z += (Math::Rand()-0.5f)*6.0f;
                speed.y = Math::Rand()*15.0f+15.0f;
                speed.x = 0.0f;
                speed.z = 0.0f;
                dim.x = Math::Rand()*8.0f+8.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH);

                pos = m_pos;
                speed.x = (Math::Rand()-0.5f)*20.0f;
                speed.y = (Math::Rand()-0.5f)*20.0f;
                speed.z = (Math::Rand()-0.5f)*20.0f;
                dim.x = 2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            cargo = SearchUranium();
            if ( cargo != nullptr )
            {
                CObjectManager::GetInstancePointer()->DeleteObject(cargo);
            }

            CreatePower();  // creates the atomic cell

            max = static_cast< int >(20.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                pos = m_pos;
                pos.x += (Math::Rand()-0.5f)*3.0f;
                pos.y += (Math::Rand()-0.5f)*3.0f;
                pos.z += (Math::Rand()-0.5f)*3.0f;
                speed.y = 0.0f;
                speed.x = 0.0f;
                speed.z = 0.0f;
                dim.x = Math::Rand()*2.0f+2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLUE, Math::Rand()*5.0f+5.0f, 0.0f, 0.0f);
            }

            m_sound->Play(SOUND_OPEN, m_object->GetPosition(), 1.0f, 1.4f);

            m_phase    = ANUP_OPEN;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ANUP_OPEN )
    {
        if ( m_progress < 1.0f )
        {
            angle = m_progress*(135.0f*Math::PI/180.0f);
            m_object->SetPartRotationZ(1, angle);
        }
        else
        {
            m_object->SetPartRotationZ(1, 135.0f*Math::PI/180.0f);

            SetBusy(false);
            UpdateInterface();

            m_main->DisplayError(INFO_NUCLEAR, m_object);

            m_phase    = ANUP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoNuclearPlant::CreateInterface(bool bSelect)
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
    pw->CreateGroup(pos, ddim, 110, EVENT_OBJECT_TYPE);

    return true;
}


// Seeking the uranium.

CObject* CAutoNuclearPlant::SearchUranium()
{
    CObject* obj = m_object->GetSlotContainedObject(0);
    if (obj == nullptr) return nullptr;
    if (obj->GetType() == OBJECT_URANIUM) return obj;
    return nullptr;
}

// Seeks if a vehicle is too close.

bool CAutoNuclearPlant::SearchVehicle()
{
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType type = obj->GetType();
        if ( type != OBJECT_HUMAN    &&
             type != OBJECT_MOBILEfa &&
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
             type != OBJECT_MOTHER   &&
             type != OBJECT_ANT      &&
             type != OBJECT_SPIDER   &&
             type != OBJECT_BEE      &&
             type != OBJECT_WORM     )  continue;

        if (obj->GetCrashSphereCount() == 0) continue;

        auto crashSphere = obj->GetFirstCrashSphere();
        if (Math::DistanceToSphere(m_pos, crashSphere.sphere) < 10.0f)
            return true;
    }

    return false;
}

// Creates an object stack.

void CAutoNuclearPlant::CreatePower()
{
    glm::vec3 pos = m_object->GetPosition();
    float angle = m_object->GetRotationY();

    float powerLevel = 1.0f;
    CObject* power = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, OBJECT_ATOMIC, powerLevel);

    dynamic_cast<CTransportableObject&>(*power).SetTransporter(m_object);
    power->SetPosition(glm::vec3(22.0f, 3.0f, 0.0f));
    m_object->SetSlotContainedObject(0, power);
}


// Returns an error due the state of the automation.

Error CAutoNuclearPlant::GetError()
{
//? TerrainRes  res;

    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

//? res = m_terrain->GetResource(m_object->GetPosition());
//? if ( res != TR_POWER )  return ERR_NUCLEAR_NULL;

//? if ( m_object->GetEnergy() < ENERGY_POWER )  return ERR_NUCLEAR_LOW;

    CObject* obj = m_object->GetSlotContainedObject(0);
    if ( obj == nullptr )  return ERR_NUCLEAR_EMPTY;
    if ( obj->GetLock() )  return ERR_OK;
    ObjectType type = obj->GetType();
    if ( type == OBJECT_ATOMIC  )  return ERR_OK;
    if ( type != OBJECT_URANIUM )  return ERR_NUCLEAR_BAD;

    return ERR_OK;
}


// Saves all parameters of the controller.

bool CAutoNuclearPlant::Write(CLevelParserLine* line)
{
    if ( m_phase == ANUP_STOP ||
         m_phase == ANUP_WAIT )  return false;

    line->AddParam("aExist", std::make_unique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", std::make_unique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", std::make_unique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", std::make_unique<CLevelParserParam>(m_speed));

    return true;
}

// Restores all parameters of the controller.

bool CAutoNuclearPlant::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoNuclearPlantPhase >(line->GetParam("aPhase")->AsInt(ANUP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;

    return true;
}
