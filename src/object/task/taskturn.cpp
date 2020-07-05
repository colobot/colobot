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


#include "object/task/taskturn.h"

#include "graphics/engine/terrain.h"

#include "level/robotmain.h"

#include "object/old_object.h"

#include "object/subclass/base_alien.h"

#include "physics/physics.h"


// Object's constructor.

CTaskTurn::CTaskTurn(COldObject* object) : CForegroundTask(object)
{
}

// Object's destructor.

CTaskTurn::~CTaskTurn()
{
}


// Management of an event.

bool CTaskTurn::EventProcess(const Event &event)
{
    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    // Momentarily stationary object (ant on the back)?
    CBaseAlien* alien = dynamic_cast<CBaseAlien*>(m_object);
    if ( alien != nullptr && alien->GetFixed() )
    {
        m_physics->SetMotorSpeedX(0.0f);  // stops the advance
        m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation
        m_bError = true;
        return true;
    }

    return true;
}


// Assigns the goal was achieved.
// A positive angle is turning right.

Error CTaskTurn::Start(float angle)
{
    m_startAngle = m_object->GetRotationY();
    m_finalAngle = m_startAngle+angle;

    if ( angle < 0.0f )
    {
        m_angle = angle+m_physics->GetCirStopLength();
        m_physics->SetMotorSpeedZ(-1.0f);  // turns left
        m_bLeft = true;
    }
    else
    {
        m_angle = angle-m_physics->GetCirStopLength();
        m_physics->SetMotorSpeedZ(1.0f);  // turns right
        m_bLeft = false;
    }
    m_physics->SetMotorSpeedX(0.0f);
    m_physics->SetMotorSpeedY(0.0f);

    m_bError = false;
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskTurn::IsEnded()
{
    float   angle;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;

    if ( m_bError )
    {
        return ERR_STOP;
    }

    angle = m_object->GetRotationY();

    if ( m_bLeft )
    {
        if ( angle <= m_finalAngle )
        {
            m_physics->SetMotorSpeedZ(0.0f);
//?         m_physics->SetCirMotionY(MO_MOTSPEED, 0.0f);
            m_physics->SetCirMotionY(MO_CURSPEED, 0.0f);
//?         m_physics->SetCirMotionY(MO_REASPEED, 0.0f);
            m_object->SetRotationY(m_finalAngle);
            return ERR_STOP;
        }
        if ( !m_bDecel && angle <= m_startAngle+m_angle)
        {
            m_bDecel = true;
            m_physics->SetMotorSpeedZ(-0.5f);
        }
    }
    else
    {
        if ( angle >= m_finalAngle )
        {
            m_physics->SetMotorSpeedZ(0.0f);
//?         m_physics->SetCirMotionY(MO_MOTSPEED, 0.0f);
            m_physics->SetCirMotionY(MO_CURSPEED, 0.0f);
//?         m_physics->SetCirMotionY(MO_REASPEED, 0.0f);
            m_object->SetRotationY(m_finalAngle);
            return ERR_STOP;
        }
        if ( !m_bDecel && angle >= m_startAngle+m_angle)
        {
            m_bDecel = true;
            m_physics->SetMotorSpeedZ(0.5f);
        }
    }

    return ERR_CONTINUE;
}
