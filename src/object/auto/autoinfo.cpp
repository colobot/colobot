// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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


#include "object/auto/autoinfo.h"

#include "script/cmdtoken.h"
#include "ui/interface.h"
#include "ui/list.h"
#include "ui/window.h"

#include <stdio.h>
#include <string.h>



// Object's constructor.

CAutoInfo::CAutoInfo(CObject* object) : CAuto(object)
{
    Init();
}

// Object's destructor.

CAutoInfo::~CAutoInfo()
{
}


// Destroys the object.

void CAutoInfo::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoInfo::Init()
{
    m_phase = AIP_WAIT;
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_bLastVirus = false;

    CAuto::Init();
}


// Start a emission.

void CAutoInfo::Start(int param)
{
    Math::Vector    pos, speed;
    Math::Point     dim;

    if ( param == 0 )  // instruction "receive" ?
    {
        m_phase = AIP_EMETTE;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }
    else if ( param == 2 )  // instruction "send" ?
    {
        m_phase = AIP_RECEIVE;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }
    else
    {
        m_phase = AIP_ERROR;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }

    m_lastParticle = 0;
    m_goal = m_object->GetPosition(0);

    if ( m_phase == AIP_EMETTE )
    {
        pos = m_goal;
        pos.y += 9.5f;
        speed = Math::Vector(0.0f, 0.0f, 0.0f);
        dim.x = 30.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISPHERE4, 1.5f, 0.0f, 0.0f);

        m_sound->Play(SOUND_LABO, pos, 1.0f, 2.0f);
    }
    if ( m_phase == AIP_RECEIVE )
    {
        pos = m_goal;
        pos.y += 9.5f;
        speed = Math::Vector(0.0f, 0.0f, 0.0f);
        dim.x = 50.0f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISPHERE6, 1.5f, 0.0f, 0.0f);

        m_sound->Play(SOUND_LABO, pos, 1.0f, 2.0f);
    }
    if ( m_phase == AIP_ERROR )
    {
        m_sound->Play(SOUND_GGG, pos, 1.0f, 0.5f);
    }
}


// Management of an event.

bool CAutoInfo::EventProcess(const Event &event)
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       duration, angle, rTime;
    int         i;

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
            angle += Math::Rand()*0.3f;
            m_object->SetAngleY(1, angle);

            m_object->SetAngleX(2, (Math::Rand()-0.5f)*0.3f);
            m_object->SetAngleX(4, (Math::Rand()-0.5f)*0.3f);
            m_object->SetAngleX(6, (Math::Rand()-0.5f)*0.3f);

            m_object->SetAngleZ(2, (Math::Rand()-0.5f)*0.3f);
            m_object->SetAngleZ(4, (Math::Rand()-0.5f)*0.3f);
            m_object->SetAngleZ(6, (Math::Rand()-0.5f)*0.3f);

            UpdateListVirus();
        }
        m_bLastVirus = true;
        return true;
    }
    else
    {
        if ( m_bLastVirus )
        {
            m_bLastVirus = false;
            UpdateList();  // normally returns the list
        }
        else
        {
            if ( m_object->GetInfoUpdate() )
            {
                UpdateList();  // updates the list
            }
        }
    }

    UpdateInterface(event.rTime);

    rTime = event.rTime;

    if ( m_phase == AIP_EMETTE )  // instruction "receive" ?
    {
        if ( m_progress < 0.5f &&
             m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            for ( i=0 ; i<4 ; i++ )
            {
                pos = m_goal;
                pos.y += 9.5f;
                speed.x = (Math::Rand()-0.5f)*50.0f;
                speed.z = (Math::Rand()-0.5f)*50.0f;
                speed.y = (Math::Rand()-0.5f)*50.0f;
                speed *= 0.5f+m_progress*0.5f;
                dim.x = 0.6f;
                dim.y = dim.x;
                duration = Math::Rand()*0.5f+0.5f;
                m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                         duration, 0.0f,
                                         duration*0.9f, 0.7f);
            }
        }

        if ( m_progress < 1.0f )
        {
            m_progress += rTime*m_speed;

            m_object->SetAngleZ(2, m_progress*2.0f*Math::PI);
            m_object->SetAngleZ(4, m_progress*2.0f*Math::PI);
            m_object->SetAngleZ(6, m_progress*2.0f*Math::PI);
        }
        else
        {
            m_phase = AIP_WAIT;

            m_object->SetAngleX(2, 0.0f);
            m_object->SetAngleX(4, 0.0f);
            m_object->SetAngleX(6, 0.0f);

            m_object->SetAngleZ(2, 0.0f);
            m_object->SetAngleZ(4, 0.0f);
            m_object->SetAngleZ(6, 0.0f);
        }
    }

    if ( m_phase == AIP_RECEIVE )  // instruction "send" ?
    {
        if ( m_progress < 0.5f &&
             m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            for ( i=0 ; i<4 ; i++ )
            {
                pos = m_goal;
                pos.y += 9.5f;
                speed = pos;
                pos.x += (Math::Rand()-0.5f)*40.0f;
                pos.y += (Math::Rand()-0.5f)*40.0f;
                pos.z += (Math::Rand()-0.5f)*40.0f;
                speed = (speed-pos)*1.0f;
//?             speed *= 0.5f+m_progress*0.5f;
                dim.x = 0.6f;
                dim.y = dim.x;
                duration = Math::Rand()*0.5f+0.5f;
                m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                         duration, 0.0f,
                                         duration*0.9f, 0.7f);
            }
        }

        if ( m_progress < 1.0f )
        {
            m_progress += rTime*m_speed;

            m_object->SetAngleZ(2, m_progress*2.0f*Math::PI);
            m_object->SetAngleZ(4, m_progress*2.0f*Math::PI);
            m_object->SetAngleZ(6, m_progress*2.0f*Math::PI);
        }
        else
        {
            m_phase = AIP_WAIT;

            m_object->SetAngleX(2, 0.0f);
            m_object->SetAngleX(4, 0.0f);
            m_object->SetAngleX(6, 0.0f);

            m_object->SetAngleZ(2, 0.0f);
            m_object->SetAngleZ(4, 0.0f);
            m_object->SetAngleZ(6, 0.0f);
        }
    }

    if ( m_phase == AIP_ERROR )
    {
        if ( m_progress < 0.5f &&
             m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_goal;
            speed.x = (Math::Rand()-0.5f)*5.0f;
            speed.z = (Math::Rand()-0.5f)*5.0f;
            speed.y = 5.0f+Math::Rand()*5.0f;
            dim.x = 5.0f+Math::Rand()*5.0f;
            dim.y = dim.x;
            duration = Math::Rand()*0.5f+0.5f;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, 4.0f);
        }

        if ( m_progress < 1.0f )
        {
            m_progress += rTime*m_speed;
            rTime = 0.0f;  // stops the rotation

            if ( m_progress < 0.5f )
            {
                angle = m_progress/0.5f;
            }
            else
            {
                angle = 1.0f-(m_progress-0.5f)/0.5f;
            }
            m_object->SetAngleX(2, angle*0.5f);
            m_object->SetAngleX(4, angle*0.5f);
            m_object->SetAngleX(6, angle*0.5f);

            m_object->SetAngleZ(2, (Math::Rand()-0.5f)*0.2f);
            m_object->SetAngleZ(4, (Math::Rand()-0.5f)*0.2f);
            m_object->SetAngleZ(6, (Math::Rand()-0.5f)*0.2f);
        }
        else
        {
            m_phase = AIP_WAIT;

            m_object->SetAngleX(2, 0.0f);
            m_object->SetAngleX(4, 0.0f);
            m_object->SetAngleX(6, 0.0f);

            m_object->SetAngleZ(2, 0.0f);
            m_object->SetAngleZ(4, 0.0f);
            m_object->SetAngleZ(6, 0.0f);
        }
    }

    angle = m_object->GetAngleY(1);
    angle += rTime*0.5f;
    m_object->SetAngleY(1, angle);

    m_object->SetAngleX(3, sinf(m_time*6.0f+Math::PI*0.0f/3.0f)*0.3f);
    m_object->SetAngleX(5, sinf(m_time*6.0f+Math::PI*2.0f/3.0f)*0.3f);
    m_object->SetAngleX(7, sinf(m_time*6.0f+Math::PI*4.0f/3.0f)*0.3f);

    return true;
}


// Returns an error due the state of the automation.

Error CAutoInfo::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}


// Creates all the interface when the object is selected.

bool CAutoInfo::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Ui::CList*      pl;
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

    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*0.0f;
    ddim.x = 160.0f/640.0f;
    ddim.y =  66.0f/480.0f;
    pl = pw->CreateList(pos, ddim, 1, EVENT_OBJECT_GINFO, 1.10f);
    pl->SetSelectCap(false);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 112, EVENT_OBJECT_TYPE);

    UpdateList();
    return true;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoInfo::UpdateInterface(float rTime)
{
    CAuto::UpdateInterface(rTime);
}


// Updates the contents of the list.

void CAutoInfo::UpdateList()
{
    Ui::CWindow*    pw;
    Ui::CList*      pl;
    Info        info;
    int         total, i;
    char        text[100];

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    pl = static_cast< Ui::CList* >(pw->SearchControl(EVENT_OBJECT_GINFO));
    if ( pl == nullptr )  return;

    pl->Flush();
    total = m_object->GetInfoTotal();
    if ( total == 0 )
    {
        pl->ClearState(Ui::STATE_ENABLE);
    }
    else
    {
        pl->SetState(Ui::STATE_ENABLE);

        for ( i=0 ; i<total ; i++ )
        {
            info = m_object->GetInfo(i);
            sprintf(text, "%s = %.2f", info.name, info.value);
            pl->SetItemName(i, text);
        }
    }

    m_object->SetInfoUpdate(false);
}

// Updates the content of contaminating the list.

void CAutoInfo::UpdateListVirus()
{
    Ui::CWindow*    pw;
    Ui::CList*      pl;
    int         i, j, max;
    char        text[100];

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return;

    pl = static_cast< Ui::CList* >(pw->SearchControl(EVENT_OBJECT_GINFO));
    if ( pl == 0 )  return;

    pl->SetState(Ui::STATE_ENABLE);

    pl->Flush();
    for ( i=0 ; i<4 ; i++ )
    {
        max = static_cast< int >(2.0f+Math::Rand()*10.0f);
        for ( j=0 ; j<max ; j++ )
        {
            do
            {
                text[j] = ' '+static_cast< int >(Math::Rand()*94.0f);
            }
            while ( text[j] == '\\' );
        }
        text[j] = 0;

        pl->SetItemName(i, text);
    }
}


// Saves all parameters of the controller.

bool CAutoInfo::Write(char *line)
{
    char    name[100];

    if ( m_phase == AIP_WAIT )  return false;

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

bool CAutoInfo::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoInfoPhase > (OpInt(line, "aPhase", AIP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticle = 0.0f;

    return true;
}

