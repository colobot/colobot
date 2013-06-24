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


#include "object/auto/autotower.h"

#include "common/iman.h"

#include "math/geometry.h"

#include "physics/physics.h"

#include "script/cmdtoken.h"

#include "ui/interface.h"
#include "ui/displaytext.h"
#include "ui/window.h"
#include "ui/gauge.h"

#include <stdio.h>
#include <string.h>


const float TOWER_SCOPE     = 200.0f;   // range of beam
const float ENERGY_FIRE     = 0.125f;   // energy consumed by fire


// Object's constructor.

CAutoTower::CAutoTower(CObject* object) : CAuto(object)
{
    int     i;

    for ( i=0 ; i<4 ; i++ )
    {
        m_partiStop[i] = -1;
    }

    Init();
    m_phase = ATP_WAIT;  // paused until the first Init ()
    m_time = 0.0f;
    m_lastUpdateTime = 0.0f;
}

// Object's destructor.

CAutoTower::~CAutoTower()
{
}


// Destroys the object.

void CAutoTower::DeleteObject(bool bAll)
{
    FireStopUpdate(0.0f, false);
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoTower::Init()
{
    m_phase    = ATP_ZERO;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_lastParticle = 0.0f;
}


// Management of an event.

bool CAutoTower::EventProcess(const Event &event)
{
    CObject*    power;
    CObject*    target;
    Math::Vector    pos;
    float       angle, energy, quick;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_timeVirus -= event.rTime;

    if ( m_object->GetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Math::Rand()*0.3f;

            angle = m_object->GetAngleY(1);
            angle += Math::Rand()*0.5f;
            m_object->SetAngleY(1, angle);

            m_object->SetAngleZ(2, Math::Rand()*0.5f);
        }
        return true;
    }

    UpdateInterface(event.rTime);

    if ( m_phase == ATP_WAIT )  return true;

    m_progress += event.rTime*m_speed;

    if ( m_phase == ATP_ZERO )
    {
        FireStopUpdate(m_progress, true);  // blinks
        if ( m_progress < 1.0f )
        {
            energy = 0.0f;
            power = m_object->GetPower();
            if ( power != 0 )
            {
                energy = power->GetEnergy();
            }
            if ( energy >= ENERGY_FIRE )
            {
                m_phase    = ATP_SEARCH;
                m_progress = 0.0f;
                m_speed    = 1.0f/3.0f;
            }
        }
        else
        {
            m_phase    = ATP_ZERO;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ATP_SEARCH )
    {
        FireStopUpdate(m_progress, false);  // extinguished
        if ( m_progress < 1.0f )
        {
            quick = 1.0f;
//?         if ( g_researchDone & RESEARCH_QUICK )  quick = 3.0f;

            angle = m_object->GetAngleY(1);
            angle -= event.rTime*quick*2.0f;
            m_object->SetAngleY(1, angle);

            angle = m_object->GetAngleZ(2);
            angle += event.rTime*quick*0.5f;
            if ( angle > 0.0f )  angle = 0.0f;
            m_object->SetAngleZ(2, angle);
        }
        else
        {
            energy = 0.0f;
            power = m_object->GetPower();
            if ( power != 0 )
            {
                energy = power->GetEnergy();
            }

            target = SearchTarget(m_targetPos);
            if ( energy < ENERGY_FIRE )
            {
                m_displayText->DisplayError(ERR_TOWER_ENERGY, m_object);
            }
            if ( target == 0 || energy < ENERGY_FIRE )
            {
                m_phase    = ATP_ZERO;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
            }
            else
            {
                pos = m_object->GetPosition(0);
                pos.y += 24.5f;
                m_angleYfinal = Math::RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);  // CW !
                m_angleYfinal += Math::PI*2.0f;
                m_angleYfinal -= m_object->GetAngleY(0);
                m_angleYactual = Math::NormAngle(m_object->GetAngleY(1));

                m_angleZfinal = -Math::PI/2.0f;
                m_angleZfinal -= Math::RotateAngle(Math::DistanceProjected(m_targetPos, pos), pos.y-m_targetPos.y);  // CW !
                m_angleZactual = m_object->GetAngleZ(2);

                m_phase    = ATP_TURN;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
//?             if ( g_researchDone & RESEARCH_QUICK )  m_speed = 1.0f/0.2f;
            }
        }
    }

    if ( m_phase == ATP_TURN )
    {
        if ( m_progress < 1.0f )
        {
            angle = m_angleYactual+(m_angleYfinal-m_angleYactual)*m_progress;
            m_object->SetAngleY(1, angle);

            angle = m_angleZactual+(m_angleZfinal-m_angleZactual)*m_progress;
            m_object->SetAngleZ(2, angle);
        }
        else
        {
            m_object->SetAngleY(1, m_angleYfinal);
            m_object->SetAngleZ(2, m_angleZfinal);

            power = m_object->GetPower();
            if ( power != 0 )
            {
                energy = power->GetEnergy();
                energy -= ENERGY_FIRE/power->GetCapacity();
                power->SetEnergy(energy);
            }

            m_sound->Play(SOUND_GGG, m_object->GetPosition(0));

            m_phase    = ATP_FIRE;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ATP_FIRE )
    {
        if ( m_progress == 0.0f )
        {
            pos = m_object->GetPosition(0);
            pos.y += 24.5f;
            m_particle->CreateRay(pos, m_targetPos, Gfx::PARTIRAY1,
                                   Math::Point(5.0f, 5.0f), 1.5f);
        }
        if ( m_progress >= 1.0f )
        {
            m_phase    = ATP_ZERO;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return true;
}


// Seeks the nearest target object.

CObject* CAutoTower::SearchTarget(Math::Vector &impact)
{
    CObject*    pObj;
    CObject*    pBest = 0;
    CPhysics*   physics;
    Math::Vector    iPos, oPos;
    ObjectType  oType;
    float       distance, min, radius, speed;
    int         i;

    iPos = m_object->GetPosition(0);
    min = 1000000.0f;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        oType = pObj->GetType();
        if ( oType != OBJECT_MOTHER &&
             oType != OBJECT_ANT    &&
             oType != OBJECT_SPIDER &&
             oType != OBJECT_BEE    &&
             oType != OBJECT_WORM   )  continue;

        if ( !pObj->GetActif() )  continue;  // inactive?

//?     if ( g_researchDone & RESEARCH_QUICK )
        if ( false )
        {
            physics = pObj->GetPhysics();
            if ( physics != 0 )
            {
                speed = fabs(physics->GetLinMotionX(MO_REASPEED));
                if ( speed > 20.0f )  continue;  // moving too fast?
            }
        }

        if ( !pObj->GetCrashSphere(0, oPos, radius) )  continue;
        distance = Math::Distance(oPos, iPos);
        if ( distance > TOWER_SCOPE )  continue;  // too far
        if ( distance < min )
        {
            min = distance;
            pBest = pObj;
        }
    }
    if ( pBest == 0 )  return 0;

    impact = pBest->GetPosition(0);
    return pBest;
}


// Returns an error due the state of the automation.

Error CAutoTower::GetError()
{
    CObject*    power;

    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    power = m_object->GetPower();
    if ( power == 0 )
    {
        return ERR_TOWER_POWER;  // no battery
    }
    else
    {
        if ( power->GetEnergy() < ENERGY_FIRE )
        {
            return ERR_TOWER_ENERGY;  // not enough energy
        }
    }
    return ERR_OK;
}


// Updates the stop lights.

void CAutoTower::FireStopUpdate(float progress, bool bLightOn)
{
    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    Math::Point     dim;
    int         i;

    static float listpos[8] =
    {
         4.5f,   0.0f,
         0.0f,   4.5f,
        -4.5f,   0.0f,
         0.0f,  -4.5f,
    };

    if ( !bLightOn )  // extinguished?
    {
        for ( i=0 ; i<4 ; i++ )
        {
            if ( m_partiStop[i] != -1 )
            {
                m_particle->DeleteParticle(m_partiStop[i]);
                m_partiStop[i] = -1;
            }
        }
        return;
    }

    mat = m_object->GetWorldMatrix(0);

    speed = Math::Vector(0.0f, 0.0f, 0.0f);
    dim.x = 2.0f;
    dim.y = dim.x;

    for ( i=0 ; i<4 ; i++ )
    {
        if ( Math::Mod(progress+i*0.125f, 0.5f) < 0.2f )
        {
            if ( m_partiStop[i] != -1 )
            {
                m_particle->DeleteParticle(m_partiStop[i]);
                m_partiStop[i] = -1;
            }
        }
        else
        {
            if ( m_partiStop[i] == -1 )
            {
                pos.x = listpos[i*2+0];
                pos.y = 18.0f;
                pos.z = listpos[i*2+1];
                pos = Transform(*mat, pos);

                m_partiStop[i] = m_particle->CreateParticle(pos, speed,
                                                              dim, Gfx::PARTISELR,
                                                              1.0f, 0.0f, 0.0f);
            }
        }
    }
}


// Creates all the interface when the object is selected.

bool CAutoTower::CreateInterface(bool bSelect)
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

    pos.x = ox+sx*14.5f;
    pos.y = oy+sy*0;
    ddim.x = 14.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGauge(pos, ddim, 0, EVENT_OBJECT_GENERGY);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 107, EVENT_OBJECT_TYPE);

    pos.x = ox+sx*10.2f;
    pos.y = oy+sy*0.5f;
    ddim.x = 33.0f/640.0f;
    ddim.y = 33.0f/480.0f;
    pw->CreateButton(pos, ddim, 41, EVENT_OBJECT_LIMIT);

    return true;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoTower::UpdateInterface(float rTime)
{
    Ui::CWindow*    pw;
    Ui::CGauge*     pg;
    CObject*    power;
    float       energy;

    CAuto::UpdateInterface(rTime);

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->GetSelect() )  return;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return;

    pg = static_cast< Ui::CGauge* >(pw->SearchControl(EVENT_OBJECT_GENERGY));
    if ( pg != 0 )
    {
        energy = 0.0f;
        power = m_object->GetPower();
        if ( power != 0 )
        {
            energy = power->GetEnergy();
        }
        pg->SetLevel(energy);
    }
}


// Saves all parameters of the controller.

bool CAutoTower::Write(char *line)
{
    char    name[100];

    if ( m_phase == ATP_WAIT )  return false;

    sprintf(name, " aExist=%d", 1);
    strcat(line, name);

    CAuto::Write(line);

    sprintf(name, " aPhase=%d", m_phase);
    strcat(line, name);

    sprintf(name, " aProgress=%.2f", m_progress);
    strcat(line, name);

    sprintf(name, " aSpeed=%.2f", m_speed);
    strcat(line, name);

    sprintf(name, " aTargetPos=%.2f;%.2f;%.2f", m_targetPos.x, m_targetPos.y, m_targetPos.z);
    strcat(line, name);

    sprintf(name, " aAngleYactual=%.2f", m_angleYactual);
    strcat(line, name);

    sprintf(name, " aAngleZactual=%.2f", m_angleZactual);
    strcat(line, name);

    sprintf(name, " aAngleYfinal=%.2f", m_angleYfinal);
    strcat(line, name);

    sprintf(name, " aAngleZfinal=%.2f", m_angleZfinal);
    strcat(line, name);

    return true;
}

// Restores all parameters of the controller.

bool CAutoTower::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoTowerPhase >(OpInt(line, "aPhase", ATP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);
    m_targetPos = OpDir(line, "aTargetPos");
    m_angleYactual = OpFloat(line, "aAngleYactual", 0.0f);
    m_angleZactual = OpFloat(line, "aAngleZactual", 0.0f);
    m_angleYfinal = OpFloat(line, "aAngleYfinal", 0.0f);
    m_angleZfinal = OpFloat(line, "aAngleZfinal", 0.0f);

    m_lastUpdateTime = 0.0f;

    return true;
}

