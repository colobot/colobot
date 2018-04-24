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


#include "object/task/taskdeletemark.h"

#include "common/global.h"

#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "physics/physics.h"


CTaskDeleteMark::CTaskDeleteMark(COldObject* object) : CForegroundTask(object)
{
    m_bExecuted = false;
}

CTaskDeleteMark::~CTaskDeleteMark()
{
}

// Management of an event.
bool CTaskDeleteMark::EventProcess(const Event &event)
{
    return true;
}

Error CTaskDeleteMark::Start()
{
    DeleteMark();

    m_bExecuted = true;

    return ERR_OK;
}

// Indicates whether the action is finished.
Error CTaskDeleteMark::IsEnded()
{
    if ( m_bExecuted )
        return ERR_STOP;
    else
        return ERR_CONTINUE;
}

// Suddenly ends the current action.
bool CTaskDeleteMark::Abort()
{
    return true;
}

void CTaskDeleteMark::DeleteMark()
{
    CObject* obj = CObjectManager::GetInstancePointer()->FindNearest(m_object, {
        OBJECT_MARKPOWER,
        OBJECT_MARKSTONE,
        OBJECT_MARKURANIUM,
        OBJECT_MARKKEYa,
        OBJECT_MARKKEYb,
        OBJECT_MARKKEYc,
        OBJECT_MARKKEYd
    }, 8.0f/g_unit);

    if (obj != nullptr)
    {
        CObjectManager::GetInstancePointer()->DeleteObject(obj);
    }
}
