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


#include "object/auto/autoegg.h"

#include "math/geometry.h"

#include "object/brain.h"
#include "object/object_manager.h"
#include "object/level/parserline.h"
#include "object/level/parserparam.h"

#include "script/cmdtoken.h"

#include <stdio.h>
#include <string.h>


// Object's constructor.

CAutoEgg::CAutoEgg(CObject* object) : CAuto(object)
{
    m_type = OBJECT_NULL;
    m_value = 0.0f;
    m_string[0] = 0;

    m_param = 0;
    m_phase = AEP_NULL;
    Init();
}

// Object's destructor.

CAutoEgg::~CAutoEgg()
{
}


// Destroys the object.

void CAutoEgg::DeleteObject(bool bAll)
{
    CObject*    alien;

    CAuto::DeleteObject(bAll);

    if ( !bAll )
    {
        alien = SearchAlien();
        if ( alien != 0 )
        {
            // Probably the intended action
            // Original code: ( alien->GetZoom(0) == 1.0f )
            if ( alien->GetZoomY(0) == 1.0f )
            {
                alien->SetLock(false);
                alien->SetActivity(true);  // the insect is active
            }
            else
            {
                alien->DeleteObject();
                delete alien;
            }
        }
    }
}


// Initialize the object.

void CAutoEgg::Init()
{
    CObject*    alien;

    alien = SearchAlien();
    if ( alien == 0 )
    {
        m_phase    = AEP_NULL;
        m_progress = 0.0f;
        m_speed    = 1.0f/5.0f;
        m_time     = 0.0f;
        return;
    }

    m_phase    = AEP_INCUB;
    m_progress = 0.0f;
    m_speed    = 1.0f/5.0f;
    m_time     = 0.0f;

    m_type = alien->GetType();

    if ( m_type == OBJECT_ANT    ||
         m_type == OBJECT_SPIDER ||
         m_type == OBJECT_BEE    )
    {
        alien->SetZoom(0, 0.2f);
    }
    if ( m_type == OBJECT_WORM )
    {
        alien->SetZoom(0, 0.01f);  // invisible !
    }
    alien->SetLock(true);
    alien->SetActivity(false);
}


// Getes a value.

bool CAutoEgg::SetType(ObjectType type)
{
    m_type = type;
    return true;
}

// Getes a value.

bool CAutoEgg::SetValue(int rank, float value)
{
    if ( rank != 0 )  return false;
    m_value = value;
    return true;
}

// Getes the string.

bool CAutoEgg::SetString(char *string)
{
    strcpy(m_string, string);
    return true;
}


// Start object.

void CAutoEgg::Start(int param)
{
    if ( m_type == OBJECT_NULL )  return;
    if ( m_value == 0.0f )  return;

    m_phase    = AEP_DELAY;
    m_progress = 0.0f;
    m_speed    = 1.0f/m_value;

    m_param = param;
}


// Management of an event.

bool CAutoEgg::EventProcess(const Event &event)
{
    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

    if ( event.type != EVENT_FRAME )  return true;
    if ( m_phase == AEP_NULL )  return true;

    if ( m_phase == AEP_DELAY )
    {
        m_progress += event.rTime*m_speed;
        if ( m_progress < 1.0f )  return true;

        Math::Vector pos = m_object->GetPosition(0);
        float angle = m_object->GetAngleY(0);
        CObject* alien = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, m_type);

        alien->SetActivity(false);
        CBrain* brain = alien->GetBrain();
        if(brain != nullptr)
        {
            Program* program = brain->AddProgram();
            brain->ReadProgram(program, m_string);
            brain->RunProgram(program);
        }
        Init();
    }

    CObject* alien = SearchAlien();
    if ( alien == nullptr )  return true;
    alien->SetActivity(false);

    m_progress += event.rTime*m_speed;

    if ( m_phase == AEP_ZOOM )
    {
        if ( m_type == OBJECT_ANT    ||
             m_type == OBJECT_SPIDER ||
             m_type == OBJECT_BEE    )
        {
            alien->SetZoom(0, 0.2f+m_progress*0.8f);  // Others push
        }
    }

    return true;
}

// Indicates whether the controller has completed its activity.

Error CAutoEgg::IsEnded()
{
    CObject*    alien;
    Gfx::CPyro*      pyro;

    if ( m_phase == AEP_DELAY )
    {
        return ERR_CONTINUE;
    }

    alien = SearchAlien();
    if ( alien == 0 )  return ERR_STOP;

    if ( m_phase == AEP_INCUB )
    {
        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        m_phase    = AEP_ZOOM;
        m_progress = 0.0f;
        m_speed    = 1.0f/5.0f;
    }

    if ( m_phase == AEP_ZOOM )
    {
        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        pyro = new Gfx::CPyro();
        pyro->Create(Gfx::PT_EGG, m_object);  // exploding egg

        alien->SetZoom(0, 1.0f);  // this is a big boy now

        m_phase    = AEP_WAIT;
        m_progress = 0.0f;
        m_speed    = 1.0f/3.0f;
    }

    if ( m_phase == AEP_WAIT )
    {
        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        alien->SetLock(false);
        alien->SetActivity(true);  // the insect is active
    }

    return ERR_STOP;
}


// Returns an error due the state of the automation.

Error CAutoEgg::GetError()
{
    return ERR_OK;
}


// Seeking the insect that starts in the egg.

CObject* CAutoEgg::SearchAlien()
{
    CObject*    pObj;
    CObject*    pBest;
    Math::Vector    cPos, oPos;
    ObjectType  type;
    float       dist, min;

    cPos = m_object->GetPosition(0);
    min = 100000.0f;
    pBest = 0;
    for(auto it : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        pObj = it.second;

        if ( pObj->GetTruck() != 0 )  continue;

        type = pObj->GetType();
        if ( type != OBJECT_ANT    &&
             type != OBJECT_BEE    &&
             type != OBJECT_SPIDER &&
             type != OBJECT_WORM   )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::DistanceProjected(oPos, cPos);
        if ( dist < 8.0f && dist < min )
        {
            min = dist;
            pBest = pObj;
        }
    }
    return pBest;
}


// Saves all parameters of the controller.

bool CAutoEgg::Write(CLevelParserLine* line)
{
    if ( m_phase == AEP_NULL )  return false;
    
    line->AddParam("aExist", new CLevelParserParam(true));
    CAuto::Write(line);
    line->AddParam("aPhase", new CLevelParserParam(static_cast<int>(m_phase)));
    line->AddParam("aProgress", new CLevelParserParam(m_progress));
    line->AddParam("aSpeed", new CLevelParserParam(m_speed));
    line->AddParam("aParamType", new CLevelParserParam(m_type));
    line->AddParam("aParamValue1", new CLevelParserParam(m_value));
    line->AddParam("aParamString", new CLevelParserParam(std::string(m_string)));

    return true;
}

// Restores all parameters of the controller.

bool CAutoEgg::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoEggPhase >(line->GetParam("aPhase")->AsInt(AEP_NULL));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);
    m_type = line->GetParam("aParamType")->AsObjectType(OBJECT_NULL);
    m_value = line->GetParam("aParamValue1")->AsFloat(0.0f);
    strcpy(m_string, line->GetParam("aParamString")->AsString("").c_str());

    return true;
}

