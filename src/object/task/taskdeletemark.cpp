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


#include "object/task/taskdeletemark.h"

#include "common/iman.h"

#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "physics/physics.h"

#include "object/robotmain.h"


CTaskDeleteMark::CTaskDeleteMark(CObject* object) : CTask(object)
{
    m_bExecuted = false;
}

CTaskDeleteMark::~CTaskDeleteMark()
{
}


// Management of an event.

bool CTaskDeleteMark::EventProcess(const Event &event)
{
   
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
    ObjectType	    type;
    CObject*        pObj;
    Math::Vector    oPos=m_object->GetPosition(0);
    int             i;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;
	
	type = pObj->GetType();
	
        if ( type == OBJECT_MARKPOWER || 
	     type == OBJECT_MARKSTONE ||
	     type == OBJECT_MARKURANIUM ||
	     type == OBJECT_MARKKEYa ||
	     type == OBJECT_MARKKEYb ||
	     type == OBJECT_MARKKEYc ||
	     type == OBJECT_MARKKEYd )
        {
	    if ( Math::Distance(oPos, pObj->GetPosition(0)) < 8.0f )
	    {
		pObj->DeleteObject();  // removes the mark
		delete pObj;
		break;
	    }
        }     
    }
}