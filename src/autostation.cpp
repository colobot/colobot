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
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "light.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "gauge.h"
#include "window.h"
#include "sound.h"
#include "auto.h"
#include "autostation.h"




// Object's constructor.

CAutoStation::CAutoStation(CInstanceManager* iMan, CObject* object)
                           : CAuto(iMan, object)
{
    Init();
}

// Object's destructor.

CAutoStation::~CAutoStation()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoStation::DeleteObject(BOOL bAll)
{
    if ( m_soundChannel != -1 )
    {
        m_sound->Stop(m_soundChannel);
        m_soundChannel = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoStation::Init()
{
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_lastParticule = 0.0f;
    m_soundChannel = -1;
    m_bLastVirus = FALSE;

    CAuto::Init();
}


// Management of an event.

BOOL CAutoStation::EventProcess(const Event &event)
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, ppos, speed;
    FPOINT      dim;
    CObject*    vehicule;
    CObject*    power;
    TerrainRes  res;
    float       big, energy, used, add, freq;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;

    m_timeVirus -= event.rTime;

    if ( m_object->RetVirusMode() )  // contaminated by a virus?
    {
        if ( !m_bLastVirus )
        {
            m_bLastVirus = TRUE;
            m_energyVirus = m_object->RetEnergy();
        }

        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Rand()*0.3f;

            m_object->SetEnergy(Rand());
        }
        return TRUE;
    }
    else
    {
        if ( m_bLastVirus )
        {
            m_bLastVirus = FALSE;
            m_object->SetEnergy(m_energyVirus);
        }
    }

    UpdateInterface(event.rTime);

    big = m_object->RetEnergy();

    res = m_terrain->RetResource(m_object->RetPosition(0));
    if ( res == TR_POWER )
    {
        big += event.rTime*0.01f;  // recharges the large battery
    }

    used = big;
    freq = 1.0f;
    if ( big > 0.0f )
    {
        vehicule = SearchVehicle();
        if ( vehicule != 0 )
        {
            power = vehicule->RetPower();
            if ( power != 0 && power->RetCapacity() == 1.0f )
            {
                energy = power->RetEnergy();
                add = event.rTime*0.2f;
                if ( add > big*4.0f )  add = big*4.0f;
                if ( add > 1.0f-energy )  add = 1.0f-energy;
                energy += add;  // Charging the battery
                power->SetEnergy(energy);
                if ( energy < freq )  freq = energy;
                big -= add/4.0f;  // discharge the large battery
            }

            power = vehicule->RetFret();
            if ( power != 0 && power->RetType() == OBJECT_POWER )
            {
                energy = power->RetEnergy();
                add = event.rTime*0.2f;
                if ( add > big*4.0f )  add = big*4.0f;
                if ( add > 1.0f-energy )  add = 1.0f-energy;
                energy += add;  // Charging the battery
                power->SetEnergy(energy);
                if ( energy < freq )  freq = energy;
                big -= add/4.0f;  // discharge the large battery
            }
        }
    }
    used -= big;  // energy used

    if ( freq < 1.0f )  // charging in progress?
    {
        freq = 1.0f+3.0f*freq;
        if ( m_soundChannel == -1 )
        {
            m_soundChannel = m_sound->Play(SOUND_STATION, m_object->RetPosition(0),
                                           0.3f, freq, TRUE);
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
         m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
    {
        m_lastParticule = m_time;

        mat = m_object->RetWorldMatrix(0);
        pos = D3DVECTOR(-15.0f, 7.0f, 0.0f);  // battery position
        pos = Transform(*mat, pos);
        speed.x = (Rand()-0.5f)*20.0f;
        speed.y = (Rand()-0.5f)*20.0f;
        speed.z = (Rand()-0.5f)*20.0f;
        ppos.x = pos.x;
        ppos.y = pos.y+(Rand()-0.5f)*4.0f;
        ppos.z = pos.z;
        dim.x = 1.5f;
        dim.y = 1.5f;
        m_particule->CreateParticule(ppos, speed, dim, PARTIBLITZ, 1.0f, 0.0f, 0.0f);

#if 0
        ppos = pos;
        ppos.y += 1.0f;
        ppos.x += (Rand()-0.5f)*3.0f;
        ppos.z += (Rand()-0.5f)*3.0f;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = 2.5f+Rand()*6.0f;
        dim.x = Rand()*1.5f+1.0f;
        dim.y = dim.x;
        m_particule->CreateParticule(ppos, speed, dim, PARTISMOKE3, 4.0f);
#else
        ppos = pos;
        ppos.y += 1.0f;
        ppos.x += (Rand()-0.5f)*3.0f;
        ppos.z += (Rand()-0.5f)*3.0f;
        speed.x = 0.0f;
        speed.z = 0.0f;
        speed.y = 2.5f+Rand()*5.0f;
        dim.x = Rand()*1.0f+0.6f;
        dim.y = dim.x;
        m_particule->CreateParticule(ppos, speed, dim, PARTIVAPOR, 3.0f);
#endif
    }

    if ( big < 0.0f )  big = 0.0f;
    if ( big > 1.0f )  big = 1.0f;
    m_object->SetEnergy(big);  // Shift the large battery

    return TRUE;
}


// Seeking the vehicle on the station.

CObject* CAutoStation::SearchVehicle()
{
    CObject*    pObj;
    D3DVECTOR   sPos, oPos;
    ObjectType  type;
    float       dist;
    int         i;

    sPos = m_object->RetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
        if ( type != OBJECT_HUMAN    &&
             type != OBJECT_MOBILEfa &&
             type != OBJECT_MOBILEta &&
             type != OBJECT_MOBILEwa &&
             type != OBJECT_MOBILEia &&
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
             type != OBJECT_MOBILEdr )  continue;

        oPos = pObj->RetPosition(0);
        dist = Length(oPos, sPos);
        if ( dist <= 5.0f )  return pObj;
    }

    return 0;
}


// Returns an error due the state of the automation.

Error CAutoStation::RetError()
{
    TerrainRes  res;

    if ( m_object->RetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    res = m_terrain->RetResource(m_object->RetPosition(0));
    if ( res != TR_POWER )  return ERR_STATION_NULL;

    return ERR_OK;
}


// Crée toute l'interface lorsque l'objet est sélectionné .

BOOL CAutoStation::CreateInterface(BOOL bSelect)
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
    pw->CreateGroup(pos, ddim, 104, EVENT_OBJECT_TYPE);

    return TRUE;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoStation::UpdateInterface(float rTime)
{
    CWindow*    pw;
    CGauge*     pg;

    CAuto::UpdateInterface(rTime);

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->RetSelect() )  return;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GENERGY);
    if ( pg != 0 )
    {
        pg->SetLevel(m_object->RetEnergy());
    }
}


