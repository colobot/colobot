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


#include "object/auto/autoegg.h"

#include "common/stringutils.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/pyro_manager.h"

#include "level/parser/parser.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/program_storage_object.h"
#include "object/interface/programmable_object.h"
#include "object/interface/transportable_object.h"


// Object's constructor.

CAutoEgg::CAutoEgg(COldObject* object) : CAuto(object)
{
    m_type = OBJECT_NULL;
    m_value = 0.0f;

    m_param = 0;
    m_phase = AEP_NULL;
    Init();
}

// Object's destructor.

CAutoEgg::~CAutoEgg()
{
}


// Destroys the object.

void CAutoEgg::DeleteObject(bool all)
{
    CAuto::DeleteObject(all);

    if ( !all )
    {
        CObject* alien = SearchAlien();
        if ( alien != nullptr )
        {
            // Probably the intended action
            // Original code: ( alien->GetScale() == 1.0f )
            if ( alien->GetScaleY() == 1.0f )
            {
                alien->SetLock(false);
                if (alien->Implements(ObjectInterfaceType::Programmable))
                {
                    dynamic_cast<CProgrammableObject&>(*alien).SetActivity(true);  // the insect is active
                }
            }
            else
            {
                CObjectManager::GetInstancePointer()->DeleteObject(alien);
            }
        }
    }
}


// Initialize the object.

void CAutoEgg::Init()
{
    CObject*    alien;

    alien = SearchAlien();
    if ( alien == nullptr )
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
        alien->SetScale(0.2f);
    }
    if ( m_type == OBJECT_WORM )
    {
        alien->SetScale(0.01f);  // invisible !
    }
    alien->SetLock(true);

    if (alien->Implements(ObjectInterfaceType::Programmable))
    {
        dynamic_cast<CProgrammableObject&>(*alien).SetActivity(false);
    }
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

bool CAutoEgg::SetString(char *string)
{
    m_alienProgramName = string;
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

        glm::vec3 pos = m_object->GetPosition();
        float angle = m_object->GetRotationY();
        CObject* alien = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, m_type);

        if (alien->Implements(ObjectInterfaceType::Programmable))
        {
            CProgrammableObject* programmable = dynamic_cast<CProgrammableObject*>(alien);
            programmable->SetActivity(false);

            CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(alien);
            Program* program = programStorage->AddProgram();
            programStorage->ReadProgram(program, StrUtils::ToString(InjectLevelPathsForCurrentLevel(m_alienProgramName, "ai")));
            program->readOnly = true;
            program->filename = m_alienProgramName;
            programmable->RunProgram(program);
        }
        Init();
    }

    CObject* alien = SearchAlien();
    if ( alien == nullptr )  return true;
    if (alien->Implements(ObjectInterfaceType::Programmable))
    {
        dynamic_cast<CProgrammableObject&>(*alien).SetActivity(false);
    }

    m_progress += event.rTime*m_speed;

    if ( m_phase == AEP_ZOOM )
    {
        if ( m_type == OBJECT_ANT    ||
             m_type == OBJECT_SPIDER ||
             m_type == OBJECT_BEE    )
        {
            alien->SetScale(0.2f+m_progress*0.8f);  // Others push
        }
    }

    return true;
}

// Indicates whether the controller has completed its activity.

Error CAutoEgg::IsEnded()
{
    CObject*    alien;

    if ( m_phase == AEP_DELAY )
    {
        return ERR_CONTINUE;
    }

    alien = SearchAlien();
    if ( alien == nullptr )  return ERR_STOP;

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

        m_engine->GetPyroManager()->Create(Gfx::PT_EGG, m_object);  // exploding egg

        alien->SetScale(1.0f);  // this is a big boy now

        m_phase    = AEP_WAIT;
        m_progress = 0.0f;
        m_speed    = 1.0f/3.0f;
    }

    if ( m_phase == AEP_WAIT )
    {
        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        alien->SetLock(false);
        if(alien->Implements(ObjectInterfaceType::Programmable))
        {
            dynamic_cast<CProgrammableObject&>(*alien).SetActivity(true);  // the insect is active
        }
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
    glm::vec3 cPos = m_object->GetPosition();
    float min = 100000.0f;
    CObject* best = nullptr;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (IsObjectBeingTransported(obj))  continue;

        ObjectType type = obj->GetType();
        if ( type != OBJECT_ANT    &&
             type != OBJECT_BEE    &&
             type != OBJECT_SPIDER &&
             type != OBJECT_WORM   )  continue;

        glm::vec3 oPos = obj->GetPosition();
        float dist = Math::DistanceProjected(oPos, cPos);
        if ( dist < 8.0f && dist < min )
        {
            min = dist;
            best = obj;
        }
    }
    return best;
}


// Saves all parameters of the controller.

bool CAutoEgg::Write(CLevelParserLine* line)
{
    if ( m_phase == AEP_NULL )  return false;

    line->AddParam("aExist", std::make_unique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", std::make_unique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", std::make_unique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", std::make_unique<CLevelParserParam>(m_speed));
    line->AddParam("aParamType", std::make_unique<CLevelParserParam>(m_type));
    line->AddParam("aParamValue1", std::make_unique<CLevelParserParam>(m_value));
    line->AddParam("aParamString", std::make_unique<CLevelParserParam>(m_alienProgramName));

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
    m_alienProgramName = line->GetParam("aParamString")->AsString("");

    return true;
}
