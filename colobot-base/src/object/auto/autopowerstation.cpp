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


#include "object/auto/autopowerstation.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "sound/sound.h"

#include "ui/controls/gauge.h"
#include "ui/controls/interface.h"
#include "ui/controls/window.h"



// Object's constructor.

CAutoPowerStation::CAutoPowerStation(COldObject* object) : CAuto(object)
{
    assert(object->Implements(ObjectInterfaceType::PowerContainer));
    Init();
}

// Object's destructor.

CAutoPowerStation::~CAutoPowerStation()
{
}


// Destroys the object.

void CAutoPowerStation::DeleteObject(bool bAll)
{
    if ( m_soundChannel != -1 )
    {
        m_sound->Stop(m_soundChannel);
        m_soundChannel = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoPowerStation::Init()
{
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_lastParticle = 0.0f;
    m_soundChannel = -1;
    m_bLastVirus = false;

    CAuto::Init();
}


// Management of an event.

bool CAutoPowerStation::EventProcess(const Event &event)
{
    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_timeVirus -= event.rTime;

    if ( m_object->GetVirusMode() )  // contaminated by a virus?
    {
        if ( !m_bLastVirus )
        {
            m_bLastVirus = true;
            m_energyVirus = dynamic_cast<CPowerContainerObject*>(m_object)->GetEnergyLevel();
        }

        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Math::Rand()*0.3f;

            dynamic_cast<CPowerContainerObject*>(m_object)->SetEnergyLevel(Math::Rand());
        }
        return true;
    }
    else
    {
        if ( m_bLastVirus )
        {
            m_bLastVirus = false;
            dynamic_cast<CPowerContainerObject*>(m_object)->SetEnergyLevel(m_energyVirus);
        }
    }

    UpdateInterface(event.rTime);

    float big = dynamic_cast<CPowerContainerObject*>(m_object)->GetEnergy();

    Gfx::TerrainRes res = m_terrain->GetResource(m_object->GetPosition());
    if ( res == Gfx::TR_POWER )
    {
        big += event.rTime*0.01f;  // recharges the large battery
    }

    float used = big;
    float freq = 1.0f;
    if (big > 0.0f)
    {
        CObject* vehicle = SearchVehicle();
        if (vehicle != nullptr && vehicle->Implements(ObjectInterfaceType::Slotted))
        {
            CSlottedObject* slotted = dynamic_cast<CSlottedObject*>(vehicle);
            for (int slot = slotted->GetNumSlots() - 1; slot >= 0; slot--)
            {
                CObject *power = slotted->GetSlotContainedObject(slot);
                if (power != nullptr && power->Implements(ObjectInterfaceType::PowerContainer))
                {
                    CPowerContainerObject* powerContainer = dynamic_cast<CPowerContainerObject*>(power);
                    if (powerContainer->IsRechargeable())
                    {
                        float energy = powerContainer->GetEnergy();
                        float add = event.rTime*0.2f;
                        if ( add > big*4.0f )  add = big*4.0f;
                        if ( add > 1.0f-energy )  add = 1.0f-energy;
                        energy += add;  // Charging the battery
                        powerContainer->SetEnergy(energy);
                        if ( energy < freq )  freq = energy;
                        big -= add/4.0f;  // discharge the large battery
                    }
                }
            }
        }
    }
    used -= big;  // energy used

    if ( freq < 1.0f )  // charging in progress?
    {
        freq = 1.0f+3.0f*freq;
        if ( m_soundChannel == -1 )
        {
            m_soundChannel = m_sound->Play(SOUND_STATION, m_object->GetPosition(),
                                           0.3f, freq, true);
        }
        m_sound->Frequency(m_soundChannel, freq);
    }
    else
    {
        if ( m_soundChannel != -1 )
        {
            m_sound->Stop(m_soundChannel);
            m_soundChannel = -1;
        }
    }

    if ( used != 0.0f &&
         m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
    {
        m_lastParticle = m_time;

        glm::vec3    pos, ppos, speed;
        glm::vec2     dim;

        glm::mat4 mat = m_object->GetWorldMatrix(0);
        pos = glm::vec3(-15.0f, 7.0f, 0.0f);  // battery position
        pos = Math::Transform(mat, pos);
        speed.x = (Math::Rand()-0.5f)*20.0f;
        speed.y = (Math::Rand()-0.5f)*20.0f;
        speed.z = (Math::Rand()-0.5f)*20.0f;
        ppos.x = pos.x;
        ppos.y = pos.y+(Math::Rand()-0.5f)*4.0f;
        ppos.z = pos.z;
        dim.x = 1.5f;
        dim.y = 1.5f;
        m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 0.0f, 0.0f);

        ppos = pos;
        ppos.y += 1.0f;
        ppos.x += (Math::Rand()-0.5f)*3.0f;
        ppos.z += (Math::Rand()-0.5f)*3.0f;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = 2.5f+Math::Rand()*5.0f;
        dim.x = Math::Rand()*1.0f+0.6f;
        dim.y = dim.x;
        m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTIVAPOR, 3.0f);
    }

    if ( big < 0.0f )  big = 0.0f;
    if ( big > 1.0f )  big = 1.0f;
    dynamic_cast<CPowerContainerObject*>(m_object)->SetEnergy(big);  // Shift the large battery

    return true;
}


// Seeking the vehicle on the station.

CObject* CAutoPowerStation::SearchVehicle()
{
    glm::vec3 sPos = m_object->GetPosition();

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
             type != OBJECT_MOBILEft &&
             type != OBJECT_MOBILEtt &&
             type != OBJECT_MOBILEwt &&
             type != OBJECT_MOBILEit &&
             type != OBJECT_MOBILErp &&
             type != OBJECT_MOBILEst &&
             type != OBJECT_MOBILEtg &&
             type != OBJECT_MOBILEdr )  continue;

        glm::vec3 oPos = obj->GetPosition();
        float dist = glm::distance(oPos, sPos);
        if ( dist <= 5.0f )  return obj;
    }

    return nullptr;
}


// Returns an error due the state of the automation.

Error CAutoPowerStation::GetError()
{
    Gfx::TerrainRes  res;

    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    res = m_terrain->GetResource(m_object->GetPosition());
    if ( res != Gfx::TR_POWER )  return ERR_STATION_NULL;

    return ERR_OK;
}

// Create the all interface when the object is selected.
bool CAutoPowerStation::CreateInterface(bool bSelect)
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

    pos.x = ox+sx*14.5f;
    pos.y = oy+sy*0;
    ddim.x = 14.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    Ui::CGauge* pg = pw->CreateGauge(pos, ddim, 0, EVENT_OBJECT_GENERGY);
    pg->SetLevel(m_object->GetEnergy());

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 104, EVENT_OBJECT_TYPE);

    return true;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoPowerStation::UpdateInterface(float rTime)
{
    Ui::CWindow*    pw;
    Ui::CGauge*     pg;

    CAuto::UpdateInterface(rTime);

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->GetSelect() )  return;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    pg = static_cast< Ui::CGauge* >(pw->SearchControl(EVENT_OBJECT_GENERGY));
    if ( pg != nullptr )
    {
        pg->SetLevel(m_object->GetEnergy());
    }
}
