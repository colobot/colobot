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


#include "object/auto/autopowercaptor.h"

#include "common/make_unique.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/carrier_object.h"
#include "object/interface/powered_object.h"
#include "object/interface/transportable_object.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"



// Object's constructor.

CAutoPowerCaptor::CAutoPowerCaptor(COldObject* object) : CAuto(object)
{
    m_channelSound = -1;
    Init();
}

// Object's destructor.

CAutoPowerCaptor::~CAutoPowerCaptor()
{
}


// Destroys the object.

void CAutoPowerCaptor::DeleteObject(bool bAll)
{
    if ( m_channelSound != -1 )
    {
        m_sound->FlushEnvelope(m_channelSound);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_channelSound = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoPowerCaptor::Init()
{
    Math::Matrix*   mat;

    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;

    mat = m_object->GetWorldMatrix(0);
    m_pos = Math::Transform(*mat, Math::Vector(22.0f, 4.0f, 0.0f));

    m_phase    = APAP_WAIT;  // waiting ...
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    CAuto::Init();
}


// Reception of lightning.

void CAutoPowerCaptor::StartLightning()
{
    m_phase    = APAP_LIGHTNING;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;
}


// Management of an event.

bool CAutoPowerCaptor::EventProcess(const Event &event)
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    int         i;

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

    if ( m_phase == APAP_LIGHTNING )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                for ( i=0 ; i<10 ; i++ )
                {
                    pos = m_object->GetPosition();
                    pos.x += (Math::Rand()-0.5f)*m_progress*40.0f;
                    pos.z += (Math::Rand()-0.5f)*m_progress*40.0f;
                    pos.y += 50.0f-m_progress*50.0f;
                    speed.x = (Math::Rand()-0.5f)*20.0f;
                    speed.z = (Math::Rand()-0.5f)*20.0f;
                    speed.y = 5.0f+Math::Rand()*5.0f;
                    dim.x = 2.0f;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 20.0f, 0.5f);
                }
            }
        }
        else
        {
            m_phase    = APAP_CHARGE;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == APAP_CHARGE )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                for ( i=0 ; i<2 ; i++ )
                {
                    pos = m_object->GetPosition();
                    pos.y += 16.0f;
                    speed.x = (Math::Rand()-0.5f)*10.0f;
                    speed.z = (Math::Rand()-0.5f)*10.0f;
                    speed.y = -Math::Rand()*30.0f;
                    dim.x = 1.0f;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 0.0f, 0.0f);
                }
            }

            ChargeObject(event.rTime);
        }
        else
        {
            m_phase    = APAP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoPowerCaptor::CreateInterface(bool bSelect)
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
    pw->CreateGroup(pos, ddim, 113, EVENT_OBJECT_TYPE);

    pos.x = ox+sx*10.2f;
    pos.y = oy+sy*0.5f;
    ddim.x = 33.0f/640.0f;
    ddim.y = 33.0f/480.0f;
    pw->CreateButton(pos, ddim, 41, EVENT_OBJECT_LIMIT);

    return true;
}


// Returns an error due the state of the automation.

Error CAutoPowerCaptor::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }
    return ERR_OK;
}


// Load all objects under the lightning rod.

void CAutoPowerCaptor::ChargeObject(float rTime)
{
    Math::Vector sPos = m_object->GetPosition();

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        Math::Vector oPos = obj->GetPosition();
        float dist = Math::Distance(oPos, sPos);
        if ( dist > 20.0f )  continue;

        if (! IsObjectBeingTransported(obj) && obj->Implements(ObjectInterfaceType::PowerContainer) )
        {
            CPowerContainerObject* powerContainer = dynamic_cast<CPowerContainerObject*>(obj);
            if (powerContainer->IsRechargeable())
            {
                float energy = powerContainer->GetEnergy();
                energy += rTime/2.0f;
                if ( energy > 1.0f )  energy = 1.0f;
                powerContainer->SetEnergy(energy);
            }
        }

        if (obj->Implements(ObjectInterfaceType::Powered))
        {
            CObject* power = dynamic_cast<CPoweredObject*>(obj)->GetPower();
            if ( power != nullptr && power->Implements(ObjectInterfaceType::PowerContainer) )
            {
                CPowerContainerObject* powerContainer = dynamic_cast<CPowerContainerObject*>(power);
                if (powerContainer->IsRechargeable())
                {
                    float energy = powerContainer->GetEnergy();
                    energy += rTime/2.0f;
                    if ( energy > 1.0f )  energy = 1.0f;
                    powerContainer->SetEnergy(energy);
                }
            }
        }

        if (obj->Implements(ObjectInterfaceType::Carrier))
        {
            CObject* power = dynamic_cast<CCarrierObject*>(obj)->GetCargo();
            if ( power != nullptr && power->Implements(ObjectInterfaceType::PowerContainer) )
            {
                CPowerContainerObject* powerContainer = dynamic_cast<CPowerContainerObject*>(power);
                if (powerContainer->IsRechargeable())
                {
                    float energy = powerContainer->GetEnergy();
                    energy += rTime/2.0f;
                    if ( energy > 1.0f )  energy = 1.0f;
                    powerContainer->SetEnergy(energy);
                }
            }
        }
    }
}


// Saves all parameters of the controller.

bool CAutoPowerCaptor::Write(CLevelParserLine* line)
{
    if ( m_phase == APAP_WAIT )  return false;

    line->AddParam("aExist", MakeUnique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", MakeUnique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", MakeUnique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", MakeUnique<CLevelParserParam>(m_speed));

    return true;
}

// Restores all parameters of the controller.

bool CAutoPowerCaptor::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoPowerCaptorPhase >(line->GetParam("aPhase")->AsInt(APAP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;

    return true;
}
