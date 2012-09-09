// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

#include "object/auto/autosafe.h"

#include "common/iman.h"
#include "math/geometry.h"
#include "object/robotmain.h"
#include "script/cmdtoken.h"
#include "ui/interface.h"
#include "ui/window.h"


const float OPEN_DELAY  = 8.0f; // duration of opening


// Object's constructor.

CAutoSafe::CAutoSafe(CInstanceManager* iMan, CObject* object)
                          : CAuto(iMan, object)
{
    int     i;

    for ( i=0 ; i<4 ; i++ )
    {
        m_bKey[i] = false;
        m_keyParti[i] = -1;
    }

    m_bLock = false;
    m_lastParticle = 0.0f;
    m_channelSound = -1;
    Init();
}

// Object's destructor.

CAutoSafe::~CAutoSafe()
{
}


// Destroys the object.

void CAutoSafe::DeleteObject(bool bAll)
{
    CObject*    pObj;

    pObj = SearchVehicle();
    if ( pObj != 0 )
    {
        pObj->DeleteObject();
        delete pObj;
    }

    if ( m_channelSound != -1 )
    {
        m_sound->FlushEnvelope(m_channelSound);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_channelSound = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoSafe::Init()
{
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;

    m_countKeys   = 0;
    m_actualAngle = 0.0f;
    m_finalAngle  = 0.0f;

    m_phase    = ASAP_WAIT;  // waiting ...
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    CAuto::Init();
}


// Management of an event.

bool CAutoSafe::EventProcess(const Event &event)
{
    CObject*    pObj;
    Math::Vector    pos, speed;
    Math::Point     dim;
    int         i, count;

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

    if ( !m_bLock )
    {
        pObj = SearchVehicle();
        if ( pObj != 0 )
        {
            pObj->SetLock(true);  // object not yet usable
            m_main->CreateShortcuts();
            m_bLock = true;
        }
    }

    if ( m_phase == ASAP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            count = CountKeys();  // count these key
            if ( count != m_countKeys )
            {
                m_countKeys = count;

                if ( count == 0 )  m_finalAngle =   0.0f*Math::PI/180.0f;
                if ( count == 1 )  m_finalAngle =   5.0f*Math::PI/180.0f;
                if ( count == 2 )  m_finalAngle =  10.0f*Math::PI/180.0f;
                if ( count == 3 )  m_finalAngle =  15.0f*Math::PI/180.0f;
                if ( count == 4 )  m_finalAngle = 120.0f*Math::PI/180.0f;

                if ( count == 4 )  // all the keys?
                {
                    LockKeys();

                    m_channelSound = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 1.0f, 0.25f, true);
                    m_sound->AddEnvelope(m_channelSound, 1.0f, 2.00f, OPEN_DELAY, SOPER_STOP);

                    m_phase    = ASAP_OPEN;
                    m_progress = 0.0f;
                    m_speed    = 1.0f/OPEN_DELAY;
                    return true;
                }
                else
                {
                    m_channelSound = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 1.0f, 0.25f, true);
                    m_sound->AddEnvelope(m_channelSound, 1.0f, 0.35f, 0.5f, SOPER_STOP);
                }
            }

            m_phase    = ASAP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ASAP_OPEN )
    {
        if ( m_progress < 1.0f )
        {
            DownKeys(m_progress);

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                for ( i=0 ; i<10 ; i++ )
                {
                    pos = m_object->GetPosition(0);
                    pos.x += (Math::Rand()-0.5f)*10.0f;
                    pos.z += (Math::Rand()-0.5f)*10.0f;
                    speed.x = (Math::Rand()-0.5f)*4.0f;
                    speed.z = (Math::Rand()-0.5f)*4.0f;
                    speed.y = Math::Rand()*15.0f;
                    dim.x = Math::Rand()*6.0f+4.0f;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLUE, 1.0f, 0.0f, 0.0f);
                }

                pos = m_object->GetPosition(0);
                pos.x += (Math::Rand()-0.5f)*10.0f;
                pos.z += (Math::Rand()-0.5f)*10.0f;
                speed.x = (Math::Rand()-0.5f)*4.0f;
                speed.z = (Math::Rand()-0.5f)*4.0f;
                speed.y = Math::Rand()*10.0f;
                dim.x = Math::Rand()*3.0f+2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, 1.0f, 0.0f, 0.0f);

                for ( i=0 ; i<4 ; i++ )
                {
                    pos = m_keyPos[i];
                    speed.x = (Math::Rand()-0.5f)*2.0f;
                    speed.z = (Math::Rand()-0.5f)*2.0f;
                    speed.y = 1.0f+Math::Rand()*1.0f;
                    dim.x = Math::Rand()*1.5f+1.5f;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f, 0.0f, 0.0f);
                }
            }
        }
        else
        {
            DeleteKeys();

            pObj = SearchVehicle();
            if ( pObj != 0 )
            {
                pObj->SetLock(false);  // object usable
                m_main->CreateShortcuts();
            }

            m_object->FlushCrashShere();
            m_object->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 0.0f);

            m_sound->Play(SOUND_FINDING, m_object->GetPosition(0));

            m_phase    = ASAP_FINISH;
            m_progress = 0.0f;
            m_speed    = 1.0f/100.0f;
        }
    }

    if ( m_phase == ASAP_FINISH )
    {
        if ( m_progress >= 1.0f )
        {
            m_phase    = ASAP_FINISH;
            m_progress = 0.0f;
            m_speed    = 1.0f/100.0f;
        }
    }

    // Opens or closes the doors.
    if ( m_actualAngle != m_finalAngle )
    {
        if ( m_actualAngle < m_finalAngle )
        {
            m_actualAngle += (105.0f*Math::PI/180.0f)*event.rTime/OPEN_DELAY;
            if ( m_actualAngle > m_finalAngle )  m_actualAngle = m_finalAngle;
        }
        else
        {
            m_actualAngle -= (105.0f*Math::PI/180.0f)*event.rTime/OPEN_DELAY;
            if ( m_actualAngle < m_finalAngle )  m_actualAngle = m_finalAngle;
        }
        m_object->SetAngleZ(1,  m_actualAngle);
        m_object->SetAngleZ(2, -m_actualAngle);
    }

    // Blinks the keys.
    speed = Math::Vector(0.0f, 0.0f, 0.0f);
    dim.x = 2.0f;
    dim.y = dim.x;
    for ( i=0 ; i<4 ; i++ )
    {
        if ( m_phase != ASAP_WAIT || !m_bKey[i] || Math::Mod(m_time, 1.0f) < 0.4f )
        {
            if ( m_keyParti[i] != -1 )
            {
                m_particle->DeleteParticle(m_keyParti[i]);
                m_keyParti[i] = -1;
            }
        }
        else
        {
            if ( m_keyParti[i] == -1 )
            {
                pos = m_keyPos[i];
                pos.y += 2.2f;
                m_keyParti[i] = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISELY, 1.0f, 0.0f, 0.0f);
            }
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoSafe::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return false;

    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 114, EVENT_OBJECT_TYPE);

    return true;
}


// Geturns an error due the state of the automation.

Error CAutoSafe::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }
    return ERR_OK;
}


// Saves all parameters of the controller.

bool CAutoSafe::Write(char *line)
{
    char    name[100];

    if ( m_phase == ASAP_WAIT )  return false;

    sprintf(name, " aExist=%d", 1);
    strcat(line, name);

    CAuto::Write(line);

    sprintf(name, " aPhase=%d", m_phase);
    strcat(line, name);

    sprintf(name, " aProgress=%.2f", m_progress);
    strcat(line, name);

    sprintf(name, " aSpeed=%.2f", m_speed);
    strcat(line, name);

    return true;
}

// Restores all parameters of the controller.

bool CAutoSafe::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoSafePhase >(OpInt(line, "aPhase", ASAP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticle = 0.0f;

    return true;
}


// Counts the number of keys

int CAutoSafe::CountKeys()
{
    CObject*    pObj;
    Math::Vector    cPos, oPos;
    Math::Point     rot;
    ObjectType  oType;
    float       dist, angle, limit, cAngle, oAngle;
    int         i, index;

    cPos   = m_object->GetPosition(0);
    cAngle = m_object->GetAngleY(0);

    for ( index=0 ; index<4 ; index++ )
    {
        m_bKey[index] = false;
        m_keyPos[index] = cPos;
    }

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        oType = pObj->GetType();
        if ( pObj->GetTruck() != 0 )  continue;

        if ( oType != OBJECT_KEYa &&
             oType != OBJECT_KEYb &&
             oType != OBJECT_KEYc &&
             oType != OBJECT_KEYd )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::DistanceProjected(oPos, cPos);
        if ( dist > 20.0f )  continue;

        if ( oType == OBJECT_KEYa )
        {
            limit  = Math::PI*1.0f;
            oAngle = Math::PI*0.0f;
            index  = 0;
        }
        if ( oType == OBJECT_KEYb )
        {
            limit  = Math::PI*0.0f;
            oAngle = Math::PI*1.0f;
            index  = 1;
        }
        if ( oType == OBJECT_KEYc )
        {
            limit  = Math::PI*1.5f;
            oAngle = Math::PI*0.5f;
            index  = 2;
        }
        if ( oType == OBJECT_KEYd )
        {
            limit  = Math::PI*0.5f;
            oAngle = Math::PI*0.0f;
            index  = 3;
        }

        angle = Math::RotateAngle(oPos.x-cPos.x, oPos.z-cPos.z)+cAngle;
        if ( !Math::TestAngle(angle, limit-8.0f*Math::PI/180.0f, limit+8.0f*Math::PI/180.0f) )  continue;

        // Key changes the shape of the base.
        rot = Math::RotatePoint(Math::Point(cPos.x, cPos.z), limit-cAngle, Math::Point(cPos.x+16.0f, cPos.z));
        oPos.x = rot.x;
        oPos.z = rot.y;
        oPos.y = cPos.y+1.0f;
        pObj->SetPosition(0, oPos);
        pObj->SetAngleY(0, oAngle+cAngle);
        m_keyPos[index] = oPos;

        m_bKey[index] = true;
    }

    i = 0;
    for ( index=0 ; index<4 ; index++ )
    {
        if ( m_bKey[index] )  i++;
    }
    return i;
}

// Blocks all keys.

void CAutoSafe::LockKeys()
{
    CObject*    pObj;
    Math::Vector    cPos, oPos;
    ObjectType  oType;
    float       dist;
    int         i;

    cPos = m_object->GetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        oType = pObj->GetType();
        if ( pObj->GetTruck() != 0 )  continue;

        if ( oType != OBJECT_KEYa &&
             oType != OBJECT_KEYb &&
             oType != OBJECT_KEYc &&
             oType != OBJECT_KEYd )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::DistanceProjected(oPos, cPos);
        if ( dist > 20.0f )  continue;

        pObj->SetLock(true);
    }
}

// Sent down all the keys.

void CAutoSafe::DownKeys(float progress)
{
    CObject*    pObj;
    Math::Vector    cPos, oPos;
    ObjectType  oType;
    float       dist;
    int         i;

    cPos = m_object->GetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        oType = pObj->GetType();
        if ( pObj->GetTruck() != 0 )  continue;

        if ( oType != OBJECT_KEYa &&
             oType != OBJECT_KEYb &&
             oType != OBJECT_KEYc &&
             oType != OBJECT_KEYd )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::DistanceProjected(oPos, cPos);
        if ( dist > 20.0f )  continue;

        oPos.y = cPos.y+1.0f-progress*2.2f;
        pObj->SetPosition(0, oPos);
    }
}

// Delete all the keys.

void CAutoSafe::DeleteKeys()
{
    CObject*    pObj;
    Math::Vector    cPos, oPos;
    ObjectType  oType;
    float       dist;
    int         i;
    bool        bDelete;

    cPos = m_object->GetPosition(0);

    do
    {
        bDelete = false;
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
            if ( pObj == 0 )  break;

            oType = pObj->GetType();
            if ( pObj->GetTruck() != 0 )  continue;

            if ( oType != OBJECT_KEYa &&
                 oType != OBJECT_KEYb &&
                 oType != OBJECT_KEYc &&
                 oType != OBJECT_KEYd )  continue;

            oPos = pObj->GetPosition(0);
            dist = Math::DistanceProjected(oPos, cPos);
            if ( dist > 20.0f )  continue;

            pObj->DeleteObject();
            delete pObj;
            bDelete = true;
        }
    }
    while ( bDelete );
}

// Seeking a vehicle in the safe.

CObject* CAutoSafe::SearchVehicle()
{
    CObject*    pObj;
    Math::Vector    cPos, oPos;
    ObjectType  oType;
    float       dist;
    int         i;

    cPos = m_object->GetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        oType = pObj->GetType();
        if ( pObj == m_object )  continue;
        if ( pObj->GetTruck() != 0 )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::DistanceProjected(oPos, cPos);
        if ( dist <= 4.0f )  return pObj;
    }
    return 0;
}



