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


#include "object/motion/motionlevelcontroller.h"

#include "object/old_object.h"

#include "physics/physics.h"

// Object's constructor.

CMotionLevelController::CMotionLevelController(COldObject* object) : CMotion(object)
{
}

// Object's destructor.

CMotionLevelController::~CMotionLevelController()
{
}


// Removes an object.

void CMotionLevelController::DeleteObject(bool bAll)
{
}


// Creates a LevelController motion

void CMotionLevelController::Create(Math::Vector pos, float angle, ObjectType type,
                          float power, Gfx::COldModelManager*)
{
    m_object->SetType(type);

    // Creates the main base.
    int rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
    m_object->SetObjectRank(0, rank);

    // Movement
    m_physics->SetLinMotionX(MO_ADVSPEED, 50.0f);
    m_physics->SetLinMotionX(MO_RECSPEED, 50.0f);
    m_physics->SetLinMotionX(MO_ADVACCEL, 20.0f);
    m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
    m_physics->SetLinMotionX(MO_STOACCEL, 20.0f);
    m_physics->SetLinMotionX(MO_TERSLIDE,  0.0f);
    m_physics->SetLinMotionZ(MO_TERSLIDE,  0.0f);
    m_physics->SetLinMotionX(MO_TERFORCE,  0.0f);
    m_physics->SetLinMotionZ(MO_TERFORCE,  0.0f);
    m_physics->SetLinMotionZ(MO_MOTACCEL, 40.0f);
    m_physics->SetLinMotionY(MO_ADVSPEED, 60.0f);
    m_physics->SetLinMotionY(MO_RECSPEED, 60.0f);
    m_physics->SetLinMotionY(MO_ADVACCEL, 20.0f);
    m_physics->SetLinMotionY(MO_RECACCEL, 50.0f);
    m_physics->SetLinMotionY(MO_STOACCEL, 50.0f);

    m_physics->SetCirMotionY(MO_ADVSPEED,  0.4f*Math::PI);
    m_physics->SetCirMotionY(MO_RECSPEED,  0.4f*Math::PI);
    m_physics->SetCirMotionY(MO_ADVACCEL,  2.0f);
    m_physics->SetCirMotionY(MO_RECACCEL,  2.0f);
    m_physics->SetCirMotionY(MO_STOACCEL,  2.0f);
}
