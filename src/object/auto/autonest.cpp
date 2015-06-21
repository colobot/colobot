/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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


#include "object/auto/autonest.h"

#include "graphics/engine/terrain.h"

#include "object/object_manager.h"
#include "object/level/parserline.h"
#include "object/level/parserparam.h"

#include "script/cmdtoken.h"

#include <stdio.h>
#include <string.h>


// Object's constructor.

CAutoNest::CAutoNest(CObject* object) : CAuto(object)
{
    Init();
}

// Object's destructor.

CAutoNest::~CAutoNest()
{
}


// Destroys the object.

void CAutoNest::DeleteObject(bool all)
{
    if ( !all )
    {
        CObject* fret = SearchFret();
        if ( fret != nullptr )
        {
            CObjectManager::GetInstancePointer()->DeleteObject(fret);
        }
    }

    CAuto::DeleteObject(all);
}


// Initialize the object.

void CAutoNest::Init()
{
    Math::Vector    pos;

    m_phase    = ANP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/4.0f;

    m_time     = 0.0f;
    m_lastParticle = 0.0f;

    pos = m_object->GetPosition(0);
    m_terrain->AdjustToFloor(pos);
    m_fretPos = pos;
}


// Management of an event.

bool CAutoNest::EventProcess(const Event &event)
{
    CObject*    fret;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;

    if ( m_phase == ANP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            if ( !SearchFree(m_fretPos) )
            {
                m_phase    = ANP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/4.0f;
            }
            else
            {
                CreateFret(m_fretPos, 0.0f, OBJECT_BULLET);
                m_phase    = ANP_BIRTH;
                m_progress = 0.0f;
                m_speed    = 1.0f/5.0f;
            }
        }
    }

    if ( m_phase == ANP_BIRTH )
    {
        fret = SearchFret();

        if ( m_progress < 1.0f )
        {
            if ( fret != 0 )
            {
                fret->SetZoom(0, m_progress);
            }
        }
        else
        {
            if ( fret != 0 )
            {
                fret->SetZoom(0, 1.0f);
                fret->SetLock(false);
            }

            m_phase    = ANP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/5.0f;
        }
    }

    return true;
}


// Seeks if a site is free.

bool CAutoNest::SearchFree(Math::Vector pos)
{
    CObject*    pObj;
    Math::Vector    sPos;
    ObjectType  type;
    float       sRadius, distance;
    int         j;
    
    for(auto& it : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        pObj = it.second.get();

        type = pObj->GetType();
        if ( type == OBJECT_NEST )  continue;

        j = 0;
        while ( pObj->GetCrashSphere(j++, sPos, sRadius) )
        {
            distance = Math::Distance(sPos, pos);
            distance -= sRadius;
            if ( distance < 2.0f )  return false;  // location occupied
        }
    }

    return true;  // free location
}

// Create a transportable object.

void CAutoNest::CreateFret(Math::Vector pos, float angle, ObjectType type)
{
    CObject* fret = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type);
    fret->SetLock(true);  // not usable
    fret->SetZoom(0, 0.0f);
}

// Looking for the ball during manufacture.

CObject* CAutoNest::SearchFret()
{
    CObject*    pObj;
    Math::Vector    oPos;
    ObjectType  type;
    
    for(auto& it : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        pObj = it.second.get();

        if ( !pObj->GetLock() )  continue;

        type = pObj->GetType();
        if ( type != OBJECT_BULLET )  continue;

        oPos = pObj->GetPosition(0);
        if ( oPos.x == m_fretPos.x &&
             oPos.z == m_fretPos.z )
        {
            return pObj;
        }
    }

    return 0;
}


// Returns an error due the state of the automation.

Error CAutoNest::GetError()
{
    return ERR_OK;
}


// Saves all parameters of the controller.

bool CAutoNest::Write(CLevelParserLine* line)
{
    if ( m_phase == ANP_WAIT )  return false;
    
    line->AddParam("aExist", new CLevelParserParam(true));
    CAuto::Write(line);
    line->AddParam("aPhase", new CLevelParserParam(static_cast<int>(m_phase)));
    line->AddParam("aProgress", new CLevelParserParam(m_progress));
    line->AddParam("aSpeed", new CLevelParserParam(m_speed));

    return true;
}

// Restores all parameters of the controller.

bool CAutoNest::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoNestPhase >(line->GetParam("aPhase")->AsInt(ANP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;

    return true;
}

