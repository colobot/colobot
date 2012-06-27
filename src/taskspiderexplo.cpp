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

// taskspiderexplo.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "object.h"
#include "physics.h"
#include "pyro.h"
#include "motion.h"
#include "motionspider.h"
#include "task.h"
#include "taskspiderexplo.h"




// Object's constructor.

CTaskSpiderExplo::CTaskSpiderExplo(CInstanceManager* iMan, CObject* object)
                          : CTask(iMan, object)
{
    m_time = 0.0f;
    m_bError = FALSE;
}

// Object's destructor.

CTaskSpiderExplo::~CTaskSpiderExplo()
{
}


// Management of an event.

BOOL CTaskSpiderExplo::EventProcess(const Event &event)
{
    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;

    // Momentarily stationary object (ant on the back)?
    if ( m_object->RetFixed() )
    {
        m_bError = TRUE;
        return TRUE;
    }

    m_time += event.rTime;

    return TRUE;
}


// Assigns the goal was achieved.

Error CTaskSpiderExplo::Start()
{
    m_motion->SetAction(MSS_EXPLO, 1.0f);  // swells abdominal
    m_time = 0.0f;

    m_physics->SetMotorSpeedX(0.0f);  // stops the advance
    m_physics->SetMotorSpeedZ(0.0f);  // stops the rotation

    m_bError = FALSE;
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskSpiderExplo::IsEnded()
{
    CPyro*      pyro;

    if ( m_engine->RetPause() )  return ERR_CONTINUE;

    if ( m_bError )
    {
        Abort();
        return ERR_STOP;
    }

    if ( m_time < 1.0f )  return ERR_CONTINUE;

    pyro = new CPyro(m_iMan);
    pyro->Create(PT_SPIDER, m_object);  // the spider explodes (suicide)

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

BOOL CTaskSpiderExplo::Abort()
{
    return TRUE;
}

