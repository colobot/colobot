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


#include "object/task/taskadvance.h"

#include "math/geometry.h"

#include "physics/physics.h"




// Object's constructor.

CTaskAdvance::CTaskAdvance(CObject* object) : CTask(object)
{
}

// Object's destructor.

CTaskAdvance::~CTaskAdvance()
{
}


// Management of an event.

bool CTaskAdvance::EventProcess(const Event &event)
{
    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_fixTime += event.rTime;

    // Momentarily stationary object (ant on the back)?
    if ( m_object->GetFixed() )
    {
        m_physics->SetMotorSpeedX(0.0f);  // stops the advance
        m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
        m_bError = true;
        return true;
    }

    m_timeLimit -= event.rTime;
    return true;
}


// Assigns the goal was achieved.

Error CTaskAdvance::Start(float length)
{
    m_direction = (length>=0.0f)?1.0f:-1.0f;
    m_totalLength = fabs(length);
    m_advanceLength = m_physics->GetLinLength(length);
    m_startPos = m_object->GetPosition(0);
    m_lastDist = 0.0f;
    m_fixTime = 0.0f;

    m_timeLimit = m_physics->GetLinTimeLength(m_totalLength, m_direction)*3.0f;
    if ( m_timeLimit < 2.0f )  m_timeLimit = 2.0f;

    m_physics->SetMotorSpeedX(m_direction*1.0f);  // forward/backward
    m_physics->SetMotorSpeedY(0.0f);
    m_physics->SetMotorSpeedZ(0.0f);

    m_bError = false;
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskAdvance::IsEnded()
{
    Math::Vector    pos;
    float       length;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;

    if ( m_bError )
    {
        return ERR_STOP;
    }

    if ( m_timeLimit < 0.0f )
    {
        m_physics->SetMotorSpeedX(0.0f);
        return ERR_MOVE_IMPOSSIBLE;
    }

    pos = m_object->GetPosition(0);
    length = Math::DistanceProjected(pos, m_startPos);

    if ( length > m_lastDist )  // forward?
    {
        m_fixTime = 0.0f;
    }
    else    // still stands?
    {
        if ( m_fixTime > 1.0f )  // for more than a second?
        {
            m_physics->SetMotorSpeedX(0.0f);
            return ERR_MOVE_IMPOSSIBLE;
        }
    }
    m_lastDist = length;

    if ( length >= m_totalLength )
    {
        m_physics->SetMotorSpeedX(0.0f);
        m_physics->SetLinMotionX(MO_CURSPEED, 0.0f);

        if ( length != 0.0f )
        {
            pos = m_startPos+((pos-m_startPos)*m_totalLength/length);
            m_object->SetPosition(0, pos);
        }
        return ERR_STOP;
    }

    if ( length >= m_advanceLength )
    {
        m_physics->SetMotorSpeedX(m_direction*0.1f);
    }
    return ERR_CONTINUE;
}

