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

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "global.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "physics.h"
#include "interface.h"
#include "button.h"
#include "gauge.h"
#include "window.h"
#include "sound.h"
#include "displaytext.h"
#include "cmdtoken.h"
#include "auto.h"
#include "autotower.h"



#define TOWER_SCOPE     200.0f      // range of beam
#define ENERGY_FIRE     0.125f      // energy consumed by fire


// Object's constructor.

CAutoTower::CAutoTower(CInstanceManager* iMan, CObject* object)
                       : CAuto(iMan, object)
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
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoTower::DeleteObject(BOOL bAll)
{
    FireStopUpdate(0.0f, FALSE);
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
    m_lastParticule = 0.0f;
}


// Management of an event.

BOOL CAutoTower::EventProcess(const Event &event)
{
    CObject*    power;
    CObject*    target;
    D3DVECTOR   pos;
    float       angle, energy, quick;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;

    m_timeVirus -= event.rTime;

    if ( m_object->RetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Rand()*0.3f;

            angle = m_object->RetAngleY(1);
            angle += Rand()*0.5f;
            m_object->SetAngleY(1, angle);

            m_object->SetAngleZ(2, Rand()*0.5f);
        }
        return TRUE;
    }

    UpdateInterface(event.rTime);

    if ( m_phase == ATP_WAIT )  return TRUE;

    m_progress += event.rTime*m_speed;

    if ( m_phase == ATP_ZERO )
    {
        FireStopUpdate(m_progress, TRUE);  // blinks
        if ( m_progress < 1.0f )
        {
            energy = 0.0f;
            power = m_object->RetPower();
            if ( power != 0 )
            {
                energy = power->RetEnergy();
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
        FireStopUpdate(m_progress, FALSE);  // extinguished
        if ( m_progress < 1.0f )
        {
            quick = 1.0f;
//?         if ( g_researchDone & RESEARCH_QUICK )  quick = 3.0f;

            angle = m_object->RetAngleY(1);
            angle -= event.rTime*quick*2.0f;
            m_object->SetAngleY(1, angle);

            angle = m_object->RetAngleZ(2);
            angle += event.rTime*quick*0.5f;
            if ( angle > 0.0f )  angle = 0.0f;
            m_object->SetAngleZ(2, angle);
        }
        else
        {
            energy = 0.0f;
            power = m_object->RetPower();
            if ( power != 0 )
            {
                energy = power->RetEnergy();
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
                pos = m_object->RetPosition(0);
                pos.y += 24.5f;
                m_angleYfinal = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);  // CW !
                m_angleYfinal += PI*2.0f;
                m_angleYfinal -= m_object->RetAngleY(0);
                m_angleYactual = NormAngle(m_object->RetAngleY(1));

                m_angleZfinal = -PI/2.0f;
                m_angleZfinal -= RotateAngle(Length2d(m_targetPos, pos), pos.y-m_targetPos.y);  // CW !
                m_angleZactual = m_object->RetAngleZ(2);

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

            power = m_object->RetPower();
            if ( power != 0 )
            {
                energy = power->RetEnergy();
                energy -= ENERGY_FIRE/power->RetCapacity();
                power->SetEnergy(energy);
            }

            m_sound->Play(SOUND_GGG, m_object->RetPosition(0));

            m_phase    = ATP_FIRE;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ATP_FIRE )
    {
        if ( m_progress == 0.0f )
        {
            pos = m_object->RetPosition(0);
            pos.y += 24.5f;
            m_particule->CreateRay(pos, m_targetPos, PARTIRAY1,
                                   FPOINT(5.0f, 5.0f), 1.5f);
        }
        if ( m_progress >= 1.0f )
        {
            m_phase    = ATP_ZERO;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return TRUE;
}


// Seeks the nearest target object.

CObject* CAutoTower::SearchTarget(D3DVECTOR &impact)
{
    CObject*    pObj;
    CObject*    pBest = 0;
    CPhysics*   physics;
    D3DVECTOR   iPos, oPos;
    ObjectType  oType;
    float       distance, min, radius, speed;
    int         i;

    iPos = m_object->RetPosition(0);
    min = 1000000.0f;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        oType = pObj->RetType();
        if ( oType != OBJECT_MOTHER &&
             oType != OBJECT_ANT    &&
             oType != OBJECT_SPIDER &&
             oType != OBJECT_BEE    &&
             oType != OBJECT_WORM   )  continue;

        if ( !pObj->RetActif() )  continue;  // inactive?

//?     if ( g_researchDone & RESEARCH_QUICK )
        if ( FALSE )
        {
            physics = pObj->RetPhysics();
            if ( physics != 0 )
            {
                speed = Abs(physics->RetLinMotionX(MO_REASPEED));
                if ( speed > 20.0f )  continue;  // moving too fast?
            }
        }

        if ( !pObj->GetCrashSphere(0, oPos, radius) )  continue;
        distance = Length(oPos, iPos);
        if ( distance > TOWER_SCOPE )  continue;  // too far
        if ( distance < min )
        {
            min = distance;
            pBest = pObj;
        }
    }
    if ( pBest == 0 )  return 0;

    impact = pBest->RetPosition(0);
    return pBest;
}


// Returns an error due the state of the automation.

Error CAutoTower::RetError()
{
    CObject*    power;

    if ( m_object->RetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    power = m_object->RetPower();
    if ( power == 0 )
    {
        return ERR_TOWER_POWER;  // no battery
    }
    else
    {
        if ( power->RetEnergy() < ENERGY_FIRE )
        {
            return ERR_TOWER_ENERGY;  // not enough energy
        }
    }
    return ERR_OK;
}


// Updates the stop lights.

void CAutoTower::FireStopUpdate(float progress, BOOL bLightOn)
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
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
                m_particule->DeleteParticule(m_partiStop[i]);
                m_partiStop[i] = -1;
            }
        }
        return;
    }

    mat = m_object->RetWorldMatrix(0);

    speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
    dim.x = 2.0f;
    dim.y = dim.x;

    for ( i=0 ; i<4 ; i++ )
    {
        if ( Mod(progress+i*0.125f, 0.5f) < 0.2f )
        {
            if ( m_partiStop[i] != -1 )
            {
                m_particule->DeleteParticule(m_partiStop[i]);
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
                m_partiStop[i] = m_particule->CreateParticule(pos, speed,
                                                              dim, PARTISELR,
                                                              1.0f, 0.0f, 0.0f);
            }
        }
    }
}


// Creates all the interface when the object is selected.

BOOL CAutoTower::CreateInterface(BOOL bSelect)
{
    CWindow*    pw;
    FPOINT      pos, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return TRUE;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return FALSE;

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

    return TRUE;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoTower::UpdateInterface(float rTime)
{
    CWindow*    pw;
    CGauge*     pg;
    CObject*    power;
    float       energy;

    CAuto::UpdateInterface(rTime);

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->RetSelect() )  return;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GENERGY);
    if ( pg != 0 )
    {
        energy = 0.0f;
        power = m_object->RetPower();
        if ( power != 0 )
        {
            energy = power->RetEnergy();
        }
        pg->SetLevel(energy);
    }
}


// Saves all parameters of the controller.

BOOL CAutoTower::Write(char *line)
{
    char    name[100];

    if ( m_phase == ATP_WAIT )  return FALSE;

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

    return TRUE;
}

// Restores all parameters of the controller.

BOOL CAutoTower::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

    CAuto::Read(line);

    m_phase = (AutoTowerPhase)OpInt(line, "aPhase", ATP_WAIT);
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);
    m_targetPos = OpDir(line, "aTargetPos");
    m_angleYactual = OpFloat(line, "aAngleYactual", 0.0f);
    m_angleZactual = OpFloat(line, "aAngleZactual", 0.0f);
    m_angleYfinal = OpFloat(line, "aAngleYfinal", 0.0f);
    m_angleZfinal = OpFloat(line, "aAngleZfinal", 0.0f);

    m_lastUpdateTime = 0.0f;

    return TRUE;
}


