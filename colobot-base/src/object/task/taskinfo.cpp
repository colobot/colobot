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


#include "object/task/taskinfo.h"

#include "common/global.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/particle.h"

#include "object/object_manager.h"

#include "object/subclass/exchange_post.h"

#include <string.h>



// Object's constructor.

CTaskInfo::CTaskInfo(COldObject* object) : CForegroundTask(object)
    , m_progress(0.0f)
    , m_error(false)
{
}

// Object's destructor.

CTaskInfo::~CTaskInfo()
{
}


// Management of an event.

bool CTaskInfo::EventProcess(const Event &event)
{
    if (m_engine->GetPause())  return true;
    if (event.type != EVENT_FRAME)  return true;
    if (m_error)  return false;

    m_progress += event.rTime;  // other advance

    return true;
}


// Assigns the goal was achieved.

Error CTaskInfo::Start(const char *name, float value, float power, bool send)
{
    m_error = true;

    CExchangePost* exchangePost = FindExchangePost(power);
    if (exchangePost == nullptr)
    {
        return ERR_INFO_NULL;
    }

    int op = 1;  // transmission impossible
    if (send)  // send?
    {
        bool infoValueSet = exchangePost->SetInfo(name, value);
        if (infoValueSet)
        {
            op = 2;  // start of reception (for terminal)
        }
    }
    else    // receive?
    {
        if (exchangePost->HasInfo(name))
        {
            op = 0;  // beginning of transmission (for terminal)
        }
    }

    exchangePost->GetAuto()->Start(op);

    glm::vec3 pos, goal;
    if (op == 0)  // transmission?
    {
        pos = exchangePost->GetPosition();
        pos.y += 9.5f;
        goal = m_object->GetPosition();
        goal.y += 4.0f;
        m_particle->CreateRay(pos, goal, Gfx::PARTIRAY3, { 2.0f, 2.0f }, 1.0f);
    }
    if (op == 2)  // reception?
    {
        goal = exchangePost->GetPosition();
        goal.y += 9.5f;
        pos = m_object->GetPosition();
        pos.y += 4.0f;
        m_particle->CreateRay(pos, goal, Gfx::PARTIRAY3, { 2.0f, 2.0f }, 1.0f);
    }

    m_progress = 0.0f;

    m_error = false;  // ok

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskInfo::IsEnded()
{
    if (m_engine->GetPause())  return ERR_CONTINUE;
    if (m_error)  return ERR_STOP;

    if (m_progress < 1.0f)  return ERR_CONTINUE;
    m_progress = 0.0f;

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskInfo::Abort()
{
    return true;
}


// Seeks the nearest information terminal.

CExchangePost* CTaskInfo::FindExchangePost(float power)
{
    return dynamic_cast<CExchangePost*>(
        CObjectManager::GetInstancePointer()->FindNearest(m_object, OBJECT_INFO, power/g_unit));
}
