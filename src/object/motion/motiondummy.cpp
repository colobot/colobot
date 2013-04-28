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


#include "object/motion/motiondummy.h"

#include "graphics/engine/modelmanager.h"

#include <stdio.h>
#include <string.h>





// Object's constructor.

CMotionDummy::CMotionDummy(CObject* object) : CMotion(object)
{
}

// Object's destructor.

CMotionDummy::~CMotionDummy()
{
}


// Removes an object.

void CMotionDummy::DeleteObject(bool bAll)
{
}


// Creates a Dummy traveling any lands on the ground.

bool CMotionDummy::Create(Math::Vector pos, float angle, ObjectType type,
                          float power)
{
    m_object->SetType(type);

    // Creates the main base.
    int rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICULE);  // this is a moving object
    m_object->SetObjectRank(0, rank);

    return true;
}