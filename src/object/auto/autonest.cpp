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


#include "object/auto/autonest.h"

#include "common/make_unique.h"

#include "graphics/engine/terrain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "object/object_manager.h"
#include "object/old_object.h"


// Object's constructor.

CAutoNest::CAutoNest(COldObject* object) : CAuto(object)
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
        CObject* cargo = SearchCargo();
        if ( cargo != nullptr )
        {
            CObjectManager::GetInstancePointer()->DeleteObject(cargo);
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

    pos = m_object->GetPosition();
    m_terrain->AdjustToFloor(pos);
    m_cargoPos = pos;
}


// Management of an event.

bool CAutoNest::EventProcess(const Event &event)
{
    CObject*    cargo;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;

    if ( m_phase == ANP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            if ( !SearchFree(m_cargoPos) )
            {
                m_phase    = ANP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/4.0f;
            }
            else
            {
                CreateCargo(m_cargoPos, 0.0f, OBJECT_BULLET);
                m_phase    = ANP_BIRTH;
                m_progress = 0.0f;
                m_speed    = 1.0f/5.0f;
            }
        }
    }

    if ( m_phase == ANP_BIRTH )
    {
        cargo = SearchCargo();

        if ( m_progress < 1.0f )
        {
            if ( cargo != nullptr )
            {
                cargo->SetScale(m_progress);
            }
        }
        else
        {
            if ( cargo != nullptr )
            {
                cargo->SetScale(1.0f);
                cargo->SetLock(false);
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
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType type = obj->GetType();
        if ( type == OBJECT_NEST )  continue;

        for (const auto& crashSphere : obj->GetAllCrashSpheres())
        {
            Math::Vector sPos = crashSphere.sphere.pos;
            float sRadius = crashSphere.sphere.radius;

            float distance = Math::Distance(sPos, pos);
            distance -= sRadius;
            if ( distance < 2.0f )  return false;  // location occupied
        }
    }

    return true;  // free location
}

// Create a transportable object.

void CAutoNest::CreateCargo(Math::Vector pos, float angle, ObjectType type)
{
    CObject* cargo = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type);
    cargo->SetLock(true);  // not usable
    cargo->SetScale(0.0f);
}

// Looking for the ball during manufacture.

CObject* CAutoNest::SearchCargo()
{
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( !obj->GetLock() )  continue;

        ObjectType type = obj->GetType();
        if ( type != OBJECT_BULLET )  continue;

        Math::Vector oPos = obj->GetPosition();
        if ( oPos.x == m_cargoPos.x &&
             oPos.z == m_cargoPos.z )
        {
            return obj;
        }
    }

    return nullptr;
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

    line->AddParam("aExist", MakeUnique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", MakeUnique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", MakeUnique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", MakeUnique<CLevelParserParam>(m_speed));

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
