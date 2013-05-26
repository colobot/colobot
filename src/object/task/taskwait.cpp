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


#include "object/task/taskwait.h"

#include "graphics/engine/engine.h"

// Object's constructor.

CTaskWait::CTaskWait(CObject* object) : CTask(object)
{
}

// Object's destructor.

CTaskWait::~CTaskWait()
{
}


// Management of an event.

bool CTaskWait::EventProcess(const Event &event)
{
    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_passTime += event.rTime;
    m_bEnded = (m_passTime >= m_waitTime);
    return true;
}


// Assigns the goal was achieved.

Error CTaskWait::Start(float time)
{
    m_waitTime = time;  // duration to wait
    m_passTime = 0.0f;  // time elapsed
    m_bEnded = false;
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskWait::IsEnded()
{
    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bEnded )  return ERR_STOP;
    return ERR_CONTINUE;
}

