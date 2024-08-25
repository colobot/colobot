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


#include "object/motion/motionvehicle.h"

#include "app/app.h"

#include "common/stringutils.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/programmable_object.h"
#include "object/interface/slotted_object.h"
#include "object/interface/transportable_object.h"

#include "physics/physics.h"

#include <stdio.h>
#include <string.h>





// Object's constructor.

CMotionVehicle::CMotionVehicle(COldObject* object)
    : CMotion(object),
      m_wheelTurn(),
      m_flyPaw()
{
    m_posTrackLeft  = 0.0f;
    m_posTrackRight = 0.0f;
    m_partiReactor  = -1;
    m_armTimeAbs    = 1000.0f;
    m_armMember     = 1000.0f;
    m_canonTime     = 0.0f;
    m_wheelLastPos   = glm::vec3(0.0f, 0.0f, 0.0f);
    m_wheelLastAngle = glm::vec3(0.0f, 0.0f, 0.0f);
    m_posKey         = glm::vec3(0.0f, 0.0f, 0.0f);
    m_bFlyFix = false;
}

// Object's destructor.

CMotionVehicle::~CMotionVehicle()
{
}


// Removes an object.

void CMotionVehicle::DeleteObject(bool bAll)
{
    if ( m_partiReactor != -1 )
    {
        m_particle->DeleteParticle(m_partiReactor);
        m_partiReactor = -1;
    }
}


// Creates a vehicle traveling any lands on the ground.

void CMotionVehicle::Create(glm::vec3 pos, float angle, ObjectType type,
                            float power, Gfx::COldModelManager* modelManager)
{
    int             rank, i, j, parent;
    Gfx::Color      color;
    std::array<char, 50> name;

    m_object->SetType(type);

    // Creates the main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
    m_object->SetObjectRank(0, rank);

    if ((m_object->GetTrainer() && type == OBJECT_MOBILEsa) || type == OBJECT_MOBILEst)
    {
        modelManager->AddModelReference("trainers", false, rank, m_object->GetTeam());
    }
    else if ((m_object->GetTrainer() &&
            ( type == OBJECT_MOBILErt ||
              type == OBJECT_MOBILErc ||
              type == OBJECT_MOBILErr ||
              type == OBJECT_MOBILErs)) || type == OBJECT_MOBILErp)
    {
        modelManager->AddModelReference("trainerr", false, rank, m_object->GetTeam());
    }
    else if (m_object->GetTrainer()  ||
             type == OBJECT_MOBILEwt ||
             type == OBJECT_MOBILEtt ||
             type == OBJECT_MOBILEft ||
             type == OBJECT_MOBILEit)
    {
        modelManager->AddModelReference("trainer", false, rank, m_object->GetTeam());
    }

    if (type == OBJECT_MOBILEfa ||
        type == OBJECT_MOBILEfb ||
        type == OBJECT_MOBILEfc ||
        type == OBJECT_MOBILEfi ||
        type == OBJECT_MOBILEfs)
    {
        if (!m_object->GetTrainer())
            modelManager->AddModelReference("lem1f", false, rank, m_object->GetTeam());
        else
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(28, rank);
            m_object->SetObjectParent(28, 0);
            modelManager->AddModelReference("trainerf", false, rank, m_object->GetTeam());

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(29, rank);
            m_object->SetObjectParent(29, 0);
            modelManager->AddModelReference("trainera", false, rank, m_object->GetTeam());
        }
    }
    else if (type == OBJECT_MOBILEta ||
             type == OBJECT_MOBILEtb ||
             type == OBJECT_MOBILEtc ||
             type == OBJECT_MOBILEti ||
             type == OBJECT_MOBILEts)
    {
        if (!m_object->GetTrainer())
            modelManager->AddModelReference("lem1t", false, rank, m_object->GetTeam());
        else
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(28, rank);
            m_object->SetObjectParent(28, 0);
            modelManager->AddModelReference("trainert", false, rank, m_object->GetTeam());

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(29, rank);
            m_object->SetObjectParent(29, 0);
            modelManager->AddModelReference("trainera", false, rank, m_object->GetTeam());
        }
    }
    else if (type == OBJECT_MOBILEwa ||
             type == OBJECT_MOBILEwb ||
             type == OBJECT_MOBILEwc ||
             type == OBJECT_MOBILEwi ||
             type == OBJECT_MOBILEws)
    {
        if (!m_object->GetTrainer())
            modelManager->AddModelReference("lem1w", false, rank, m_object->GetTeam());
        else
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(28, rank);
            m_object->SetObjectParent(28, 0);
            modelManager->AddModelReference("trainerw", false, rank, m_object->GetTeam());

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(29, rank);
            m_object->SetObjectParent(29, 0);
            modelManager->AddModelReference("trainera", false, rank, m_object->GetTeam());
        }
    }
    else if (type == OBJECT_MOBILEia ||
             type == OBJECT_MOBILEib ||
             type == OBJECT_MOBILEic ||
             type == OBJECT_MOBILEii ||
             type == OBJECT_MOBILEis)
    {
        if (!m_object->GetTrainer())
            modelManager->AddModelReference("lem1i", false, rank, m_object->GetTeam());
        else
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(28, rank);
            m_object->SetObjectParent(28, 0);
            modelManager->AddModelReference("traineri", false, rank, m_object->GetTeam());

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(29, rank);
            m_object->SetObjectParent(29, 0);
            modelManager->AddModelReference("trainera", false, rank, m_object->GetTeam());
        }
    }
    else if (!m_object->GetTrainer() &&
            (type == OBJECT_MOBILErt ||
             type == OBJECT_MOBILErc ||
             type == OBJECT_MOBILErr ||
             type == OBJECT_MOBILErs))
    {
        modelManager->AddModelReference("roller1", false, rank, m_object->GetTeam());
    }
    else if (type == OBJECT_MOBILEsa && !m_object->GetTrainer())
    {
        modelManager->AddModelReference("subm1", false, rank, m_object->GetTeam());
    }
    else if (type == OBJECT_MOBILEtg)
    {
        modelManager->AddModelReference("target", false, rank, m_object->GetTeam());
    }
    else if (type == OBJECT_MOBILEwt)
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(28, rank);
        m_object->SetObjectParent(28, 0);
        modelManager->AddModelReference("trainerw", false, rank, m_object->GetTeam());
    }
    else if (type == OBJECT_MOBILEft)
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(28, rank);
        m_object->SetObjectParent(28, 0);
        modelManager->AddModelReference("trainerf", false, rank, m_object->GetTeam());
    }
    else if (type == OBJECT_MOBILEtt)
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(28, rank);
        m_object->SetObjectParent(28, 0);
        modelManager->AddModelReference("trainert", false, rank, m_object->GetTeam());
    }
    else if (type == OBJECT_MOBILEit)
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(28, rank);
        m_object->SetObjectParent(28, 0);
        modelManager->AddModelReference("traineri", false, rank, m_object->GetTeam());
    }
    else if (type == OBJECT_MOBILEdr)
    {
        modelManager->AddModelReference("drawer1", false, rank, m_object->GetTeam());
    }
    else if (type == OBJECT_APOLLO2)
    {
        modelManager->AddModelReference("apolloj1", false, rank, m_object->GetTeam());
    }

    m_object->SetPosition(pos);
    m_object->SetRotationY(angle);

    // A vehicle must have a obligatory collision
    // with a sphere of center (0, y, 0) (see GetCrashSphere).
    if (type == OBJECT_MOBILErt ||
        type == OBJECT_MOBILErc ||
        type == OBJECT_MOBILErr ||
        type == OBJECT_MOBILErs ||
        type == OBJECT_MOBILErp)
    {
        m_object->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 4.0f, 0.0f), 6.5f, SOUND_BOUMm, 0.45f));
        m_object->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 3.0f, 0.0f), 7.0f));
    }
    else if (type == OBJECT_MOBILEsa ||
             type == OBJECT_MOBILEst)
    {
        m_object->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 3.0f, 0.0f), 4.5f, SOUND_BOUMm, 0.45f));
        m_object->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 3.0f, 0.0f), 6.0f));
    }
    else if (type == OBJECT_MOBILEdr)
    {
        m_object->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        m_object->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 3.0f, 0.0f), 7.0f));
    }
    else if (type == OBJECT_APOLLO2)
    {
        m_object->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 0.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f));
    }
    else
    {
        m_object->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 3.0f, 0.0f), 4.5f, SOUND_BOUMm, 0.45f));
        m_object->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 4.0f, 0.0f), 6.0f));
    }

    if (type == OBJECT_MOBILEfa ||
        type == OBJECT_MOBILEta ||
        type == OBJECT_MOBILEwa ||
        type == OBJECT_MOBILEia)
    {
        // Creates the arm.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("lem2", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(0.0f, 5.3f, 0.0f));
        m_object->SetPartRotationZ(1, ARM_NEUTRAL_ANGLE1);

        // Creates the forearm.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 1);
        modelManager->AddModelReference("lem3", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, glm::vec3(5.0f, 0.0f, 0.0f));
        m_object->SetPartRotationZ(2, ARM_NEUTRAL_ANGLE2);

        // Creates the hand.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(3, rank);
        m_object->SetObjectParent(3, 2);
        modelManager->AddModelReference("lem4", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(3, glm::vec3(3.5f, 0.0f, 0.0f));
        m_object->SetPartRotationZ(3, ARM_NEUTRAL_ANGLE3);
        m_object->SetPartRotationX(3, Math::PI/2.0f);

        // Creates the close clamp.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(4, rank);
        m_object->SetObjectParent(4, 3);
        modelManager->AddModelReference("lem5", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(4, glm::vec3(1.5f, 0.0f, 0.0f));
        m_object->SetPartRotationZ(4, -Math::PI*0.10f);

        // Creates the remote clamp.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(5, rank);
        m_object->SetObjectParent(5, 3);
        modelManager->AddModelReference("lem6", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(5, glm::vec3(1.5f, 0.0f, 0.0f));
        m_object->SetPartRotationZ(5, Math::PI*0.10f);
    }

    if (type == OBJECT_MOBILEfs ||
        type == OBJECT_MOBILEts ||
        type == OBJECT_MOBILEws ||
        type == OBJECT_MOBILEis)
    {
        // Creates the arm.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("lem2", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(0.0f, 5.3f, 0.0f));
        m_object->SetPartRotationZ(1, 110.0f*Math::PI/180.0f);

        // Creates the forearm.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 1);
        modelManager->AddModelReference("lem3", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, glm::vec3(5.0f, 0.0f, 0.0f));
        m_object->SetPartRotationZ(2, -110.0f*Math::PI/180.0f);

        // Creates the sensor.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(3, rank);
        m_object->SetObjectParent(3, 2);
        modelManager->AddModelReference("lem4s", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(3, glm::vec3(3.5f, 0.0f, 0.0f));
        m_object->SetPartRotationZ(3, -65.0f*Math::PI/180.0f);
    }

    if (type == OBJECT_MOBILEfc ||
        type == OBJECT_MOBILEtc ||
        type == OBJECT_MOBILEwc ||
        type == OBJECT_MOBILEic)
    {
        // Creates the cannon.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("canon", false, rank, m_object->GetTeam());
//?     m_object->SetPartPosition(1, glm::vec3(0.0f, 5.3f, 0.0f));
        m_object->SetPartPosition(1, glm::vec3(0.0f, 5.3f, 0.0f));
        m_object->SetPartRotationZ(1, 0.0f);
    }

    if (type == OBJECT_MOBILEfi ||
        type == OBJECT_MOBILEti ||
        type == OBJECT_MOBILEwi ||
        type == OBJECT_MOBILEii)
    {
        // Creates the insect cannon.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("canoni1", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(0.0f, 5.3f, 0.0f));
        m_object->SetPartRotationZ(1, 0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 1);
        modelManager->AddModelReference("canoni2", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, glm::vec3(0.0f, 2.5f, 0.0f));
        m_object->SetPartRotationZ(2, 0.0f);
    }

    if (type == OBJECT_MOBILEfb ||
        type == OBJECT_MOBILEtb ||
        type == OBJECT_MOBILEwb ||
        type == OBJECT_MOBILEib)
    {
        // Creates the neutron gun.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("neutron", false, rank, m_object->GetTeam());
//?     m_object->SetPartPosition(1, glm::vec3(0.0f, 5.3f, 0.0f));
        m_object->SetPartPosition(1, glm::vec3(0.0f, 5.3f, 0.0f));
        m_object->SetPartRotationZ(1, 0.0f);
    }

    if (type == OBJECT_MOBILEwa ||
        type == OBJECT_MOBILEwb ||
        type == OBJECT_MOBILEwc ||
        type == OBJECT_MOBILEws ||
        type == OBJECT_MOBILEwi ||
        type == OBJECT_MOBILEwt)
    {
        // Creates the right-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelReference("lem2w", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(6, glm::vec3(-3.0f, 1.0f, -3.0f));

        // Creates the left-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("lem2w", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, glm::vec3(-3.0f, 1.0f, 3.0f));
        m_object->SetPartRotationY(7, Math::PI);

        // Creates the right-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("lem2w", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, glm::vec3(2.0f, 1.0f, -3.0f));

        // Creates the left-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("lem2w", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, glm::vec3(2.0f, 1.0f, 3.0f));
        m_object->SetPartRotationY(9, Math::PI);
    }

    if (type == OBJECT_MOBILEtg)
    {
        // Creates the right-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelReference("lem2w", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(6, glm::vec3(-2.0f, 1.0f, -3.0f));

        // Creates the left-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("lem2w", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, glm::vec3(-2.0f, 1.0f, 3.0f));
        m_object->SetPartRotationY(7, Math::PI);

        // Creates the right-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("lem2w", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, glm::vec3(3.0f, 1.0f, -3.0f));

        // Creates the left-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("lem2w", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(9, glm::vec3(3.0f, 1.0f, 3.0f));
        m_object->SetPartRotationY(9, Math::PI);
    }

    if (type == OBJECT_MOBILEta ||
        type == OBJECT_MOBILEtb ||
        type == OBJECT_MOBILEtc ||
        type == OBJECT_MOBILEti ||
        type == OBJECT_MOBILEts ||
        type == OBJECT_MOBILEtt)  // caterpillars?
    {
        // Creates the right caterpillar.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelCopy("lem2t", false, rank, m_object->GetTeam());
        if (m_object->GetTrainer() || type == OBJECT_MOBILEtt)
        {
            m_object->SetPartPosition(6, glm::vec3(0.0f, 2.0f, -3.55f));
            m_object->SetPartScaleZ(6, 0.725f);
        }
        else
            m_object->SetPartPosition(6, glm::vec3(0.0f, 2.0f, -3.0f));

        // Creates the left caterpillar.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelCopy("lem3t", false, rank, m_object->GetTeam());
        if (m_object->GetTrainer() || type == OBJECT_MOBILEtt)
        {
            m_object->SetPartPosition(7, glm::vec3(0.0f, 2.0f, 3.55f));
            m_object->SetPartScaleZ(7, 0.725f);
        }
        else
            m_object->SetPartPosition(7, glm::vec3(0.0f, 2.0f, 3.0f));
    }

    if (type == OBJECT_MOBILErt ||
        type == OBJECT_MOBILErc ||
        type == OBJECT_MOBILErr ||
        type == OBJECT_MOBILErs ||
        type == OBJECT_MOBILErp)  // large caterpillars?
    {
        // Creates the right caterpillar.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelCopy("roller2", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(6, glm::vec3(0.0f, 2.0f, -3.0f));

        // Creates the left caterpillar.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelCopy("roller3", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, glm::vec3(0.0f, 2.0f, 3.0f));
    }

    if (type == OBJECT_MOBILEsa ||
        type == OBJECT_MOBILEst)  // underwater caterpillars?
    {
        // Creates the right caterpillar.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelCopy("subm4", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(6, glm::vec3(0.0f, 1.0f, -3.0f));

        // Creates the left caterpillar.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelCopy("subm5", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, glm::vec3(0.0f, 1.0f, 3.0f));
    }

    if (type == OBJECT_MOBILEdr)  // caterpillars?
    {
        // Creates the right caterpillar.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelCopy("drawer2", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(6, glm::vec3(0.0f, 1.0f, -3.0f));

        // Creates the left caterpillar.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelCopy("drawer3", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, glm::vec3(0.0f, 1.0f, 3.0f));
    }

    if (type == OBJECT_MOBILEfa ||
        type == OBJECT_MOBILEfb ||
        type == OBJECT_MOBILEfc ||
        type == OBJECT_MOBILEfs ||
        type == OBJECT_MOBILEfi ||
        type == OBJECT_MOBILEft)  // flying?
    {
        // Creates the front foot.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelReference("lem2f", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(6, glm::vec3(1.7f, 3.0f, 0.0f));

        // Creates the right-back foot.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("lem2f", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(7, glm::vec3(-1.8f, 3.0f, -1.5f));
        m_object->SetPartRotationY(7, 120.0f*Math::PI/180.0f);

        // Creates the left-back foot.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("lem2f", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(8, glm::vec3(-1.8f, 3.0f, 1.5f));
        m_object->SetPartRotationY(8, -120.0f*Math::PI/180.0f);
    }

    if (type == OBJECT_MOBILEia ||
        type == OBJECT_MOBILEib ||
        type == OBJECT_MOBILEic ||
        type == OBJECT_MOBILEis ||
        type == OBJECT_MOBILEii ||
        type == OBJECT_MOBILEit)  // insect legs?
    {
        float       table[] =
        {
        //    x       y       z
            -1.5f,   1.2f,  -0.7f,  // back leg
             0.0f,   0.0f,  -1.0f,
             0.0f,   0.0f,  -2.0f,

             0.0f,   1.2f,  -0.9f,  // middle leg
             0.0f,   0.0f,  -1.0f,
             0.0f,   0.0f,  -2.0f,

             1.5f,   1.2f,  -0.7f,  // front leg
             0.0f,   0.0f,  -1.0f,
             0.0f,   0.0f,  -2.0f,
        };

        for ( i=0 ; i<3 ; i++ )
        {
            for ( j=0 ; j<3 ; j++ )
            {
                snprintf(name.data(), name.size(), "ant%d", j+4);  // 4..6

                // Creates the right leg.
                rank = m_engine->CreateObject();
                m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
                m_object->SetObjectRank(6+i*3+j, rank);
                if ( j == 0 )  parent = 0;
                else           parent = 6+i*3+j-1;
                m_object->SetObjectParent(6+i*3+j, parent);
                modelManager->AddModelReference(name.data(), false, rank, m_object->GetTeam());
                pos.x = table[i*9+j*3+0];
                pos.y = table[i*9+j*3+1];
                pos.z = table[i*9+j*3+2];
                m_object->SetPartPosition(6+i*3+j, pos);

                // Creates the left leg.
                rank = m_engine->CreateObject();
                m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
                m_object->SetObjectRank(15+i*3+j, rank);
                if ( j == 0 )  parent = 0;
                else           parent = 15+i*3+j-1;
                m_object->SetObjectParent(15+i*3+j, parent);
                modelManager->AddModelReference(name.data(), true, rank, m_object->GetTeam());
                pos.x =  table[i*9+j*3+0];
                pos.y =  table[i*9+j*3+1];
                pos.z = -table[i*9+j*3+2];
                m_object->SetPartPosition(15+i*3+j, pos);
            }
        }
    }

    if (type == OBJECT_MOBILErt)
    {
        // Creates the holder.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("roller2t", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(0.0f, 0.0f, 0.0f));
        m_object->SetPartRotationZ(1, 0.0f);

        // Creates the pestle.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("roller3t", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, glm::vec3(9.0f, 4.0f, 0.0f));
        m_object->SetPartRotationZ(2, 0.0f);
    }

    if (type == OBJECT_MOBILErc)
    {
        // Creates the holder.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("roller2c", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(3.0f, 4.6f, 0.0f));
        m_object->SetPartRotationZ(1, Math::PI/8.0f);

        // Creates the cannon.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("roller3p", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, glm::vec3(7.0f, 6.5f, 0.0f));
        m_object->SetPartRotationZ(2, 0.0f);
    }

    if (type == OBJECT_MOBILErr)
    {
        // Creates the holder.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("recover1", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(2.0f, 5.0f, 0.0f));

        // Creates the right arm.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 1);
        modelManager->AddModelReference("recover2", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, glm::vec3(0.1f, 0.0f, -5.0f));
        m_object->SetPartRotationZ(2, 126.0f*Math::PI/180.0f);

        // Creates the right forearm.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(3, rank);
        m_object->SetObjectParent(3, 2);
        modelManager->AddModelReference("recover3", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(3, glm::vec3(5.0f, 0.0f, -0.5f));
        m_object->SetPartRotationZ(3, -144.0f*Math::PI/180.0f);

        // Creates the left arm.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(4, rank);
        m_object->SetObjectParent(4, 1);
        modelManager->AddModelReference("recover2", true, rank, m_object->GetTeam());
        m_object->SetPartPosition(4, glm::vec3(0.1f, 0.0f, 5.0f));
        m_object->SetPartRotationZ(4, 126.0f*Math::PI/180.0f);

        // Creates the left forearm.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(5, rank);
        m_object->SetObjectParent(5, 4);
        modelManager->AddModelReference("recover3", true, rank, m_object->GetTeam());
        m_object->SetPartPosition(5, glm::vec3(5.0f, 0.0f, 0.5f));
        m_object->SetPartRotationZ(5, -144.0f*Math::PI/180.0f);
    }

    if (type == OBJECT_MOBILErs)
    {
        // Creates the holder.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("roller2s", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(0.0f, 0.0f, 0.0f));
        m_object->SetPartRotationZ(1, 0.0f);

        // Creates the intermediate piston.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 1);
        modelManager->AddModelReference("roller3s", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, glm::vec3(7.0f, 4.5f, 0.0f));
        m_object->SetPartRotationZ(2, 0.0f);

        // Creates the piston with the sphere.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(3, rank);
        m_object->SetObjectParent(3, 2);
        modelManager->AddModelReference("roller4s", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(3, glm::vec3(0.0f, 1.0f, 0.0f));
        m_object->SetPartRotationZ(3, 0.0f);
    }

    if (type == OBJECT_MOBILEsa)
    {
        // Creates the holder.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("subm2", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(4.2f, 3.0f, 0.0f));

        // Creates the right tong.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 1);
        modelManager->AddModelReference("subm3", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(2, glm::vec3(0.5f, 0.0f, -1.5f));

        // Creates the left tong.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(3, rank);
        m_object->SetObjectParent(3, 1);
        modelManager->AddModelReference("subm3", true, rank, m_object->GetTeam());
        m_object->SetPartPosition(3, glm::vec3(0.5f, 0.0f, 1.5f));

        if (m_object->GetTrainer())
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(4, rank);
            m_object->SetObjectParent(4, 0);
            modelManager->AddModelReference("trainerg", true, rank, m_object->GetTeam());
        }
    }

    if (type == OBJECT_MOBILEdr)
    {
        // Creates the carousel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("drawer4", false, rank, m_object->GetTeam());
        m_object->SetPartPosition(1, glm::vec3(-3.0f, 3.0f, 0.0f));

        // Creates the key.
        if ( m_object->GetToy() )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(2, rank);
            m_object->SetObjectParent(2, 0);
            modelManager->AddModelReference("drawer5", false, rank, m_object->GetTeam());
            m_posKey = glm::vec3(3.0f, 5.7f, 0.0f);
            m_object->SetPartPosition(2, m_posKey);
            m_object->SetPartRotationY(2, 90.0f*Math::PI/180.0f);
        }

        // Creates pencils.
        for ( i=0 ; i<8 ; i++ )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(10+i, rank);
            m_object->SetObjectParent(10+i, 1);
            snprintf(name.data(), name.size(), "drawer%d", 10+i);
            modelManager->AddModelReference(name.data(), false, rank, m_object->GetTeam());
            m_object->SetPartPosition(10+i, glm::vec3(0.0f, 0.0f, 0.0f));
            m_object->SetPartRotationY(10+i, 45.0f*Math::PI/180.0f*i);
        }
    }

    if (type == OBJECT_MOBILEwt)
    {
        // Creates the key.
        if ( m_object->GetToy() )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(2, rank);
            m_object->SetObjectParent(2, 0);
            modelManager->AddModelReference("drawer5", false, rank, m_object->GetTeam());
            m_posKey = glm::vec3(0.2f, 4.1f, 0.0f);
            m_object->SetPartPosition(2, m_posKey);
            m_object->SetPartRotationY(2, 90.0f*Math::PI/180.0f);
        }
    }

    if (type == OBJECT_APOLLO2)
    {
        // Creates the accessories.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(1, rank);
        m_object->SetObjectParent(1, 0);
        modelManager->AddModelReference("apolloj2", false, rank, m_object->GetTeam());  // antenna
        m_object->SetPartPosition(1, glm::vec3(5.5f, 8.8f, 2.0f));
        m_object->SetPartRotationY(1, -120.0f*Math::PI/180.0f);
        m_object->SetPartRotationZ(1,   45.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(2, rank);
        m_object->SetObjectParent(2, 0);
        modelManager->AddModelReference("apolloj3", false, rank, m_object->GetTeam());  // camera
        m_object->SetPartPosition(2, glm::vec3(5.5f, 2.8f, -2.0f));
        m_object->SetPartRotationY(2, 30.0f*Math::PI/180.0f);

        // Creates the wheels.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(6, rank);
        m_object->SetObjectParent(6, 0);
        modelManager->AddModelReference("apolloj4", false, rank, m_object->GetTeam());  // wheel
        m_object->SetPartPosition(6, glm::vec3(-5.75f, 1.65f, -5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(7, rank);
        m_object->SetObjectParent(7, 0);
        modelManager->AddModelReference("apolloj4", false, rank, m_object->GetTeam());  // wheel
        m_object->SetPartPosition(7, glm::vec3(-5.75f, 1.65f, 5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(8, rank);
        m_object->SetObjectParent(8, 0);
        modelManager->AddModelReference("apolloj4", false, rank, m_object->GetTeam());  // wheel
        m_object->SetPartPosition(8, glm::vec3(5.75f, 1.65f, -5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(9, rank);
        m_object->SetObjectParent(9, 0);
        modelManager->AddModelReference("apolloj4", false, rank, m_object->GetTeam());  // wheel
        m_object->SetPartPosition(9, glm::vec3(5.75f, 1.65f, 5.00f));

        // Creates mud guards.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(10, rank);
        m_object->SetObjectParent(10, 0);
        modelManager->AddModelReference("apolloj6", false, rank, m_object->GetTeam());  // wheel
        m_object->SetPartPosition(10, glm::vec3(-5.75f, 1.65f, -5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(11, rank);
        m_object->SetObjectParent(11, 0);
        modelManager->AddModelReference("apolloj6", false, rank, m_object->GetTeam());  // wheel
        m_object->SetPartPosition(11, glm::vec3(-5.75f, 1.65f, 5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(12, rank);
        m_object->SetObjectParent(12, 0);
        modelManager->AddModelReference("apolloj5", false, rank, m_object->GetTeam());  // wheel
        m_object->SetPartPosition(12, glm::vec3(5.75f, 1.65f, -5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(13, rank);
        m_object->SetObjectParent(13, 0);
        modelManager->AddModelReference("apolloj5", false, rank, m_object->GetTeam());  // wheel
        m_object->SetPartPosition(13, glm::vec3(5.75f, 1.65f, 5.00f));
    }

    if (type == OBJECT_MOBILErt ||
        type == OBJECT_MOBILErc ||
        type == OBJECT_MOBILErr ||
        type == OBJECT_MOBILErs ||
        type == OBJECT_MOBILErp)
    {
        m_object->CreateShadowCircle(6.0f, 1.0f);
    }
    else if (type == OBJECT_MOBILEta ||
             type == OBJECT_MOBILEtb ||
             type == OBJECT_MOBILEtc ||
             type == OBJECT_MOBILEti ||
             type == OBJECT_MOBILEts ||
             type == OBJECT_MOBILEtt ||
             type == OBJECT_MOBILEsa ||
             type == OBJECT_MOBILEst)
    {
        m_object->CreateShadowCircle(5.0f, 1.0f);
    }
    else if (type == OBJECT_MOBILEdr)
    {
        m_object->CreateShadowCircle(4.5f, 1.0f);
    }
    else if (type == OBJECT_APOLLO2)
    {
        m_object->CreateShadowCircle(7.0f, 0.8f);
    }
    else
    {
        m_object->CreateShadowCircle(4.0f, 1.0f);
    }

    if (type == OBJECT_MOBILEfa ||
        type == OBJECT_MOBILEfb ||
        type == OBJECT_MOBILEfc ||
        type == OBJECT_MOBILEfi ||
        type == OBJECT_MOBILEfs ||
        type == OBJECT_MOBILEft)  // flying?
    {
//?     color.r = 0.5f-1.0f;
//?     color.g = 0.2f-1.0f;
//?     color.b = 0.0f-1.0f;  // orange
//?     color.r = 0.8f;
//?     color.g = 0.6f;
//?     color.b = 0.0f;  // yellow-orange
        color.r = 0.0f;
        color.g = 0.4f;
        color.b = 0.8f;  // blue
        color.a = 0.0f;
        m_object->CreateShadowLight(50.0f, color);
    }

    CreatePhysics(type);
    m_object->SetFloorHeight(0.0f);

    if (power > 0.0f            &&
        type != OBJECT_MOBILEdr &&
        type != OBJECT_APOLLO2)
    {
        CObject* powerCell = nullptr;
        int powerSlotIndex = m_object->MapPseudoSlot(CSlottedObject::Pseudoslot::POWER);
        glm::vec3 powerCellPos = m_object->GetSlotPosition(powerSlotIndex);
        float powerCellAngle = 0.0f;
        if (power <= 1.0f)
        {
            powerCell = CObjectManager::GetInstancePointer()->CreateObject(powerCellPos, powerCellAngle, OBJECT_POWER, power);
        }
        else
        {
            powerCell = CObjectManager::GetInstancePointer()->CreateObject(powerCellPos, powerCellAngle, OBJECT_ATOMIC, power / 100.0f);
        }
        assert(powerCell->Implements(ObjectInterfaceType::Transportable));

        powerCell->SetPosition(powerCellPos);
        powerCell->SetRotation(glm::vec3(0.0f, powerCellAngle, 0.0f));
        dynamic_cast<CTransportableObject&>(*powerCell).SetTransporter(m_object);
        m_object->SetSlotContainedObjectReq(CSlottedObject::Pseudoslot::POWER, powerCell);
    }

    pos = m_object->GetPosition();
    m_object->SetPosition(pos);  //to display the shadows immediately

    m_engine->LoadAllTextures();
}

// Creates the physics of the object.

void CMotionVehicle::CreatePhysics(ObjectType type)
{
    Character*  character;

    character = m_object->GetCharacter();

    if ( type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEwt )  // wheels?
    {
        character->wheelFront = 3.0f;
        character->wheelBack  = 4.0f;
        character->wheelLeft  = 4.0f;
        character->wheelRight = 4.0f;
        m_object->SetPowerPosition(glm::vec3(-3.2f, 3.0f, 0.0f));

        m_physics->SetLinMotionX(MO_ADVSPEED, 20.0f);
        m_physics->SetLinMotionX(MO_RECSPEED, 10.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.8f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.8f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
        m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
    }

    if ( type == OBJECT_MOBILEtg )
    {
        character->wheelFront = 4.0f;
        character->wheelBack  = 3.0f;
        character->wheelLeft  = 4.0f;
        character->wheelRight = 4.0f;
        m_object->SetPowerPosition(glm::vec3(-3.2f, 3.0f, 0.0f));

        m_physics->SetLinMotionX(MO_ADVSPEED, 20.0f);
        m_physics->SetLinMotionX(MO_RECSPEED, 10.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 20.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.8f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.8f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL, 10.0f);
        m_physics->SetCirMotionY(MO_RECACCEL, 10.0f);
        m_physics->SetCirMotionY(MO_STOACCEL, 15.0f);
    }

    if ( type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEtt )  // caterpillars?
    {
        character->wheelFront = 4.0f;
        character->wheelBack  = 4.0f;
        character->wheelLeft  = 4.8f;
        character->wheelRight = 4.8f;
        m_object->SetPowerPosition(glm::vec3(-3.2f, 3.0f, 0.0f));

        m_physics->SetLinMotionX(MO_ADVSPEED, 15.0f);
        m_physics->SetLinMotionX(MO_RECSPEED,  8.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 15.0f);
        m_physics->SetLinMotionX(MO_RECACCEL,  8.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 20.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 40.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.5f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.5f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL, 10.0f);
        m_physics->SetCirMotionY(MO_RECACCEL, 10.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);
    }

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILEit)  // legs?
    {
        character->wheelFront = 4.0f;
        character->wheelBack  = 4.0f;
        character->wheelLeft  = 5.0f;
        character->wheelRight = 5.0f;
        m_object->SetPowerPosition(glm::vec3(-3.2f, 3.0f, 0.0f));

        m_physics->SetLinMotionX(MO_ADVSPEED, 15.0f);
        m_physics->SetLinMotionX(MO_RECSPEED,  8.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 10.0f);
//?     m_physics->SetLinMotionX(MO_TERFORCE, 15.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 40.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.5f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.5f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL, 10.0f);
        m_physics->SetCirMotionY(MO_RECACCEL, 10.0f);
        m_physics->SetCirMotionY(MO_STOACCEL, 15.0f);
    }

    if ( type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEft )  // flying?
    {
        character->wheelFront = 5.0f;
        character->wheelBack  = 4.0f;
        character->wheelLeft  = 4.5f;
        character->wheelRight = 4.5f;
        m_object->SetPowerPosition(glm::vec3(-3.2f, 3.0f, 0.0f));

        m_physics->SetLinMotionX(MO_ADVSPEED, 50.0f);
        m_physics->SetLinMotionX(MO_RECSPEED, 50.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 50.0f);
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

    if ( type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILErp)  // large caterpillars?
    {
        character->wheelFront = 5.0f;
        character->wheelBack  = 5.0f;
        character->wheelLeft  = 6.0f;
        character->wheelRight = 6.0f;
        m_object->SetPowerPosition(glm::vec3(-5.8f, 4.0f, 0.0f));

        m_physics->SetLinMotionX(MO_ADVSPEED, 10.0f);
        m_physics->SetLinMotionX(MO_RECSPEED,  5.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 10.0f);
        m_physics->SetLinMotionX(MO_RECACCEL,  5.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 20.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 40.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.3f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.3f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  2.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  2.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  4.0f);
    }

    if ( type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEst )
    {
        character->wheelFront = 4.0f;
        character->wheelBack  = 4.0f;
        character->wheelLeft  = 4.0f;
        character->wheelRight = 4.0f;
        m_object->SetPowerPosition(glm::vec3(-5.0f, 3.0f, 0.0f));

        m_physics->SetLinMotionX(MO_ADVSPEED, 15.0f);
        m_physics->SetLinMotionX(MO_RECSPEED, 10.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_RECACCEL, 10.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 20.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 40.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.5f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.5f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  5.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  5.0f);
        m_physics->SetCirMotionY(MO_STOACCEL, 10.0f);
    }

    if ( type == OBJECT_MOBILEdr )
    {
        character->wheelFront = 4.0f;
        character->wheelBack  = 4.0f;
        character->wheelLeft  = 4.0f;
        character->wheelRight = 4.0f;
        m_object->SetPowerPosition(glm::vec3(-5.0f, 3.0f, 0.0f));

        m_physics->SetLinMotionX(MO_ADVSPEED, 15.0f);
        m_physics->SetLinMotionX(MO_RECSPEED, 10.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_RECACCEL, 10.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 20.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 40.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.5f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.5f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  5.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  5.0f);
        m_physics->SetCirMotionY(MO_STOACCEL, 10.0f);
    }

    if ( type == OBJECT_APOLLO2 )  // jeep?
    {
        character->wheelFront = 6.0f;
        character->wheelBack  = 6.0f;
        character->wheelLeft  = 5.0f;
        character->wheelRight = 5.0f;

        m_physics->SetLinMotionX(MO_ADVSPEED, 15.0f);
        m_physics->SetLinMotionX(MO_RECSPEED, 10.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 40.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  2.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  2.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 30.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.4f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.4f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  2.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  2.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  4.0f);
    }
}


// Management of an event.

bool CMotionVehicle::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    if ( event.type == EVENT_KEY_DOWN )
    {
    }

    return true;
}

// Management of an event.

bool CMotionVehicle::EventFrame(const Event &event)
{
    Character*  character;
    glm::vec3    pos, angle;
    ObjectType  type;
    float       s, a, speedBL, speedBR, speedFL, speedFR, h, a1, a2;
    float       back, front, dist, radius, limit[2];

    if ( m_engine->GetPause() )  return true;
    if ( !m_engine->IsVisiblePoint(m_object->GetPosition()) )  return true;

    type = m_object->GetType();

    if ( type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_APOLLO2  )  // wheels?
    {
        s = m_physics->GetLinMotionX(MO_MOTSPEED)*1.0f;
        a = m_physics->GetCirMotionY(MO_MOTSPEED)*3.0f;

        if ( type == OBJECT_APOLLO2 )  s *= 0.5f;

        speedBR = -s+a;
        speedBL =  s+a;
        speedFR = -s+a;
        speedFL =  s+a;

        m_object->SetPartRotationZ(6, m_object->GetPartRotationZ(6)+event.rTime*speedBR);  // turning the wheels
        m_object->SetPartRotationZ(7, m_object->GetPartRotationZ(7)+event.rTime*speedBL);
        m_object->SetPartRotationZ(8, m_object->GetPartRotationZ(8)+event.rTime*speedFR);
        m_object->SetPartRotationZ(9, m_object->GetPartRotationZ(9)+event.rTime*speedFL);

        if ( s > 0.0f )
        {
            m_wheelTurn[0] = -a*0.05f;
            m_wheelTurn[1] = -a*0.05f+Math::PI;
            m_wheelTurn[2] =  a*0.05f;
            m_wheelTurn[3] =  a*0.05f+Math::PI;
        }
        else if ( s < 0.0f )
        {
            m_wheelTurn[0] =  a*0.05f;
            m_wheelTurn[1] =  a*0.05f+Math::PI;
            m_wheelTurn[2] = -a*0.05f;
            m_wheelTurn[3] = -a*0.05f+Math::PI;
        }
        else
        {
            m_wheelTurn[0] =  fabs(a)*0.05f;
            m_wheelTurn[1] = -fabs(a)*0.05f+Math::PI;
            m_wheelTurn[2] = -fabs(a)*0.05f;
            m_wheelTurn[3] =  fabs(a)*0.05f+Math::PI;
        }
        m_object->SetPartRotationY(6, m_object->GetPartRotationY(6)+(m_wheelTurn[0]-m_object->GetPartRotationY(6))*event.rTime*8.0f);
        m_object->SetPartRotationY(7, m_object->GetPartRotationY(7)+(m_wheelTurn[1]-m_object->GetPartRotationY(7))*event.rTime*8.0f);
        m_object->SetPartRotationY(8, m_object->GetPartRotationY(8)+(m_wheelTurn[2]-m_object->GetPartRotationY(8))*event.rTime*8.0f);
        m_object->SetPartRotationY(9, m_object->GetPartRotationY(9)+(m_wheelTurn[3]-m_object->GetPartRotationY(9))*event.rTime*8.0f);

        if ( type == OBJECT_APOLLO2 )
        {
            m_object->SetPartRotationY(10, m_object->GetPartRotationY(6)+(m_wheelTurn[0]-m_object->GetPartRotationY(6))*event.rTime*8.0f);
            m_object->SetPartRotationY(11, m_object->GetPartRotationY(7)+(m_wheelTurn[1]-m_object->GetPartRotationY(7))*event.rTime*8.0f+Math::PI);
            m_object->SetPartRotationY(12, m_object->GetPartRotationY(8)+(m_wheelTurn[2]-m_object->GetPartRotationY(8))*event.rTime*8.0f);
            m_object->SetPartRotationY(13, m_object->GetPartRotationY(9)+(m_wheelTurn[3]-m_object->GetPartRotationY(9))*event.rTime*8.0f+Math::PI);
        }

        pos = m_object->GetPosition();
        angle = m_object->GetRotation();
        if ( pos.x   != m_wheelLastPos.x   ||
             pos.y   != m_wheelLastPos.y   ||
             pos.z   != m_wheelLastPos.z   ||
             angle.x != m_wheelLastAngle.x ||
             angle.y != m_wheelLastAngle.y ||
             angle.z != m_wheelLastAngle.z )
        {
            m_wheelLastPos = pos;
            m_wheelLastAngle = angle;

            if ( type == OBJECT_MOBILEtg )
            {
                back   = -2.0f;  // back wheels position
                front  =  3.0f;  // front wheels position
                dist   =  3.0f;  // distancing wheels Z
                radius =  1.0f;
            }
            else if ( type == OBJECT_APOLLO2 )
            {
                back   = -5.75f;  // back wheels position
                front  =  5.75f;  // front wheels position
                dist   =  5.00f;  // distancing wheels Z
                radius =  1.65f;
            }
            else
            {
                back   = -3.0f;  // back wheels position
                front  =  2.0f;  // front wheels position
                dist   =  3.0f;  // distancing wheels Z
                radius =  1.0f;
            }

            if ( glm::distance(pos, m_engine->GetEyePt()) < 50.0f )  // suspension?
            {
                character = m_object->GetCharacter();
                glm::mat4 mat = m_object->GetWorldMatrix(0);

                pos.x = -character->wheelBack;  // right back wheel
                pos.z = -character->wheelRight;
                pos.y =  0.0f;
                pos = Math::Transform(mat, pos);
                h = m_terrain->GetHeightToFloor(pos);
                if ( h >  0.5f )  h =  0.5f;
                if ( h < -0.5f )  h = -0.5f;
                pos.x =  back;
                pos.y =  radius-h;
                pos.z = -dist;
                m_object->SetPartPosition(6, pos);
                if ( type == OBJECT_APOLLO2 )  m_object->SetPartPosition(10, pos);

                pos.x = -character->wheelBack;  // left back wheel
                pos.z =  character->wheelLeft;
                pos.y =  0.0f;
                pos = Math::Transform(mat, pos);
                h = m_terrain->GetHeightToFloor(pos);
                if ( h >  0.5f )  h =  0.5f;
                if ( h < -0.5f )  h = -0.5f;
                pos.x =  back;
                pos.y =  radius-h;
                pos.z =  dist;
                m_object->SetPartPosition(7, pos);
                if ( type == OBJECT_APOLLO2 )  m_object->SetPartPosition(11, pos);

                pos.x =  character->wheelFront;  // right front wheel
                pos.z = -character->wheelRight;
                pos.y =  0.0f;
                pos = Math::Transform(mat, pos);
                h = m_terrain->GetHeightToFloor(pos);
                if ( h >  0.5f )  h =  0.5f;
                if ( h < -0.5f )  h = -0.5f;
                pos.x =  front;
                pos.y =  radius-h;
                pos.z = -dist;
                m_object->SetPartPosition(8, pos);
                if ( type == OBJECT_APOLLO2 )  m_object->SetPartPosition(12, pos);

                pos.x =  character->wheelFront;  // left front wheel
                pos.z =  character->wheelLeft;
                pos.y =  0.0f;
                pos = Math::Transform(mat, pos);
                h = m_terrain->GetHeightToFloor(pos);
                if ( h >  0.5f )  h =  0.5f;
                if ( h < -0.5f )  h = -0.5f;
                pos.x =  front;
                pos.y =  radius-h;
                pos.z =  dist;
                m_object->SetPartPosition(9, pos);
                if ( type == OBJECT_APOLLO2 )  m_object->SetPartPosition(13, pos);
            }
            else
            {
                m_object->SetPartPosition(6, glm::vec3(back,  radius, -dist));
                m_object->SetPartPosition(7, glm::vec3(back,  radius,  dist));
                m_object->SetPartPosition(8, glm::vec3(front, radius, -dist));
                m_object->SetPartPosition(9, glm::vec3(front, radius,  dist));

                if ( type == OBJECT_APOLLO2 )
                {
                    m_object->SetPartPosition(10, glm::vec3(back,  radius, -dist));
                    m_object->SetPartPosition(11, glm::vec3(back,  radius,  dist));
                    m_object->SetPartPosition(12, glm::vec3(front, radius, -dist));
                    m_object->SetPartPosition(13, glm::vec3(front, radius,  dist));
                }
            }
        }
    }

    if ( type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILErp ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEst ||
         type == OBJECT_MOBILEdr )  // caterpillars?
    {
        s = m_physics->GetLinMotionX(MO_MOTSPEED)*0.7f;
        a = m_physics->GetCirMotionY(MO_MOTSPEED)*2.5f;

        m_posTrackLeft  += event.rTime*(s+a);
        m_posTrackRight += event.rTime*(s-a);

        UpdateTrackMapping(m_posTrackLeft, m_posTrackRight, type);

        pos = m_object->GetPosition();
        angle = m_object->GetRotation();
        if ( pos.x   != m_wheelLastPos.x   ||
             pos.y   != m_wheelLastPos.y   ||
             pos.z   != m_wheelLastPos.z   ||
             angle.x != m_wheelLastAngle.x ||
             angle.y != m_wheelLastAngle.y ||
             angle.z != m_wheelLastAngle.z )
        {
            m_wheelLastPos = pos;
            m_wheelLastAngle = angle;

            if ( type == OBJECT_MOBILEta ||
                 type == OBJECT_MOBILEtc ||
                 type == OBJECT_MOBILEti ||
                 type == OBJECT_MOBILEts ||
                 type == OBJECT_MOBILEtt )
            {
                limit[0] =   8.0f*Math::PI/180.0f;
                limit[1] = -12.0f*Math::PI/180.0f;
            }
            else if ( type == OBJECT_MOBILEsa ||
                      type == OBJECT_MOBILEst )
            {
                limit[0] =  15.0f*Math::PI/180.0f;
                limit[1] = -15.0f*Math::PI/180.0f;
            }
            else if ( type == OBJECT_MOBILEdr )
            {
                limit[0] =  10.0f*Math::PI/180.0f;
                limit[1] = -10.0f*Math::PI/180.0f;
            }
            else
            {
                limit[0] =  15.0f*Math::PI/180.0f;
                limit[1] = -10.0f*Math::PI/180.0f;
            }

            if ( glm::distance(pos, m_engine->GetEyePt()) < 50.0f )  // suspension?
            {
                character = m_object->GetCharacter();
                glm::mat4 mat = m_object->GetWorldMatrix(0);

                pos.x =  character->wheelFront;  // right front wheel
                pos.z = -character->wheelRight;
                pos.y =  0.0f;
                pos = Math::Transform(mat, pos);
                a1 = atanf(m_terrain->GetHeightToFloor(pos)/character->wheelFront);

                pos.x = -character->wheelBack;  // right back wheel
                pos.z = -character->wheelRight;
                pos.y =  0.0f;
                pos = Math::Transform(mat, pos);
                a2 = atanf(m_terrain->GetHeightToFloor(pos)/character->wheelBack);

                a = (a2-a1)/2.0f;
                if ( a > limit[0] )  a = limit[0];
                if ( a < limit[1] )  a = limit[1];
                m_object->SetPartRotationZ(6, a);

                pos.x =  character->wheelFront;  // left front wheel
                pos.z =  character->wheelLeft;
                pos.y =  0.0f;
                pos = Math::Transform(mat, pos);
                a1 = atanf(m_terrain->GetHeightToFloor(pos)/character->wheelFront);

                pos.x = -character->wheelBack;  // left back wheel
                pos.z =  character->wheelLeft;
                pos.y =  0.0f;
                pos = Math::Transform(mat, pos);
                a2 = atanf(m_terrain->GetHeightToFloor(pos)/character->wheelBack);

                a = (a2-a1)/2.0f;
                if ( a > limit[0] )  a = limit[0];
                if ( a < limit[1] )  a = limit[1];
                m_object->SetPartRotationZ(7, a);
            }
            else
            {
                m_object->SetPartRotationZ(6, 0.0f);
                m_object->SetPartRotationZ(7, 0.0f);
            }
        }
    }

    if ( type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEdr )  // toy is key?
    {
        pos = m_posKey;
        if ( m_object->GetSelect() &&
             m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD )
        {
            pos.y += 10.0f;  // out of sight!
        }
        m_object->SetPartPosition(2, pos);

        s  = -fabs(m_physics->GetLinMotionX(MO_MOTSPEED)*0.1f);
        s += -fabs(m_physics->GetCirMotionY(MO_MOTSPEED)*1.5f);
        m_object->SetPartRotationY(2, m_object->GetPartRotationY(2)+event.rTime*s);  // turns the key
    }

    if ( type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEft )  // flying?
    {
        EventFrameFly(event);
    }

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILEit )  // legs?
    {
        EventFrameInsect(event);
    }

    if ( type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEii )  // insect cannon?
    {
        EventFrameCanoni(event);
    }

    return true;
}

// Managing an event for a flying robot.

bool CMotionVehicle::EventFrameFly(const Event &event)
{
    glm::vec3    pos, angle, paw[3];
    float       hope[3], actual, final, h, a;
    int         i;

    pos = m_object->GetPosition();
    angle = m_object->GetRotation();
    if ( m_bFlyFix                     &&
         pos.x   == m_wheelLastPos.x   &&
         pos.y   == m_wheelLastPos.y   &&
         pos.z   == m_wheelLastPos.z   &&
         angle.x == m_wheelLastAngle.x &&
         angle.y == m_wheelLastAngle.y &&
         angle.z == m_wheelLastAngle.z )  return true;

    m_wheelLastPos = pos;
    m_wheelLastAngle = angle;

    if ( m_physics->GetLand() )  // on the ground?
    {
        glm::mat4 mat = m_object->GetWorldMatrix(0);
        paw[0] = Math::Transform(mat, glm::vec3( 4.2f, 0.0f,  0.0f));  // front
        paw[1] = Math::Transform(mat, glm::vec3(-3.0f, 0.0f, -3.7f));  // right back
        paw[2] = Math::Transform(mat, glm::vec3(-3.0f, 0.0f,  3.7f));  // left back

        for ( i=0 ; i<3 ; i++ )
        {
            h = m_terrain->GetHeightToFloor(paw[i]);
            a = -atanf(h*0.5f);
            if ( a >  Math::PI*0.2f )  a =  Math::PI*0.2f;
            if ( a < -Math::PI*0.2f )  a = -Math::PI*0.2f;
            hope[i] = a;
        }
    }
    else    // in flight?
    {
        hope[0] = 0.0f;  // front
        hope[1] = 0.0f;  // right back
        hope[2] = 0.0f;  // left back
    }

    m_bFlyFix = true;
    for ( i=0 ; i<3 ; i++ )
    {
        actual = m_object->GetPartRotationZ(6+i);
        final = Math::Smooth(actual, hope[i], event.rTime*5.0f);
        if ( final != actual )
        {
            m_bFlyFix = false;  // it is moving
            m_object->SetPartRotationZ(6+i, final);
        }
    }

    return true;
}

// Event management for insect legs.

bool CMotionVehicle::EventFrameInsect(const Event &event)
{
    glm::vec3    dir;
    float       s, a, prog = 0.0f, time;
    int         i, st, nd, action;
    bool        bStop, bOnBoard;

    static int table[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // in the air:
        60,25,0,    60,0,0,     60,-25,0,   // t0: thighs 1..4
        -35,0,0,    -35,0,0,    -35,0,0,    // t0: legs 1..4
        -65,0,0,    -65,0,0,    -65,0,0,    // t0: feet 1..4
                                            // on the ground:
        30,10,0,    30,-15,0,   30,-40,0,   // t1: thighs 1..4
        -45,0,0,    -45,0,0,    -45,0,0,    // t1: legs 1..4
        -20,0,0,    -20,0,0,    -20,0,0,    // t1: feet 1..4
                                            // on the ground back:
        35,40,0,    40,15,0,    40,-10,0,   // t2: thighs 1..4
        -35,0,0,    -35,0,0,    -35,0,0,    // t2: legs 1..4
        -50,0,0,    -65,0,0,    -65,0,0,    // t2: feet 1..4
                                            // stop:
        35,35,0,    40,10,0,    40,-15,0,   // s0: thighs 1..4
        -35,0,0,    -35,0,0,    -35,0,0,    // s0: legs 1..4
        -50,0,0,    -65,0,0,    -65,0,0,    // s0: feet 1..4
    };

    bOnBoard = false;
    if ( m_object->GetSelect() &&
         m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD )
    {
        bOnBoard = true;
    }

    s =     m_physics->GetLinMotionX(MO_MOTSPEED)*1.5f;
    a = fabs(m_physics->GetCirMotionY(MO_MOTSPEED)*2.0f);

    if ( s == 0.0f && a != 0.0f )  a *= 1.5f;

    m_armTimeAbs += event.rTime;
    m_armMember += (s+a)*event.rTime*0.15f;

    bStop = ( a == 0.0f && s == 0.0f );  // stop?

    action = 0;  // walking
    if ( s == 0.0f && a == 0.0f )
    {
        action = 3;  // stop
    }

    if ( bStop )
    {
        prog = Math::Mod(m_armTimeAbs, 2.0f)/10.0f;
        a = Math::Mod(m_armMember, 1.0f);
        a = (prog-a)*event.rTime*2.0f;  // stop position is pleasantly
        m_armMember += a;
    }

    assert(m_object->Implements(ObjectInterfaceType::Destroyable));
    if ( dynamic_cast<CDestroyableObject*>(m_object)->IsDying() )  // burn or explode?
    {
        action = 3;
    }

    for ( i=0 ; i<6 ; i++ )  // the six legs
    {
        if ( action != 0 )  // special action in progress?
        {
            st = 3*3*3*action + (i%3)*3;
            nd = st;
            time = event.rTime*5.0f;
        }
        else
        {
            if ( i < 3 )  prog = Math::Mod(m_armMember+(2.0f-(i%3))*0.33f+0.0f, 1.0f);
            else          prog = Math::Mod(m_armMember+(2.0f-(i%3))*0.33f+0.3f, 1.0f);
            if ( prog < 0.33f )  // t0..t1 ?
            {
                prog = prog/0.33f;  // 0..1
                st = 0;  // index start
                nd = 1;  // index end
            }
            else if ( prog < 0.67f )  // t1..t2 ?
            {
                prog = (prog-0.33f)/0.33f;  // 0..1
                st = 1;  // index start
                nd = 2;  // index end
            }
            else    // t2..t0 ?
            {
                prog = (prog-0.67f)/0.33f;  // 0..1
                st = 2;  // index start
                nd = 0;  // index end
            }
            st = 3*3*3*action + st*3*3*3 + (i%3)*3;
            nd = 3*3*3*action + nd*3*3*3 + (i%3)*3;

            // Less and less soft ...
            time = event.rTime*20.0f;
        }

        if ( i < 3 )  // right leg (1..3) ?
        {
            m_object->SetPartRotationX(6+3*i+0, Math::Smooth(m_object->GetPartRotationX(6+3*i+0), Math::PropAngle(table[st+ 0], table[nd+ 0], prog), time));
            m_object->SetPartRotationY(6+3*i+0, Math::Smooth(m_object->GetPartRotationY(6+3*i+0), Math::PropAngle(table[st+ 1], table[nd+ 1], prog), time));
            m_object->SetPartRotationZ(6+3*i+0, Math::Smooth(m_object->GetPartRotationZ(6+3*i+0), Math::PropAngle(table[st+ 2], table[nd+ 2], prog), time));
            m_object->SetPartRotationX(6+3*i+1, Math::Smooth(m_object->GetPartRotationX(6+3*i+1), Math::PropAngle(table[st+ 9], table[nd+ 9], prog), time));
            m_object->SetPartRotationY(6+3*i+1, Math::Smooth(m_object->GetPartRotationY(6+3*i+1), Math::PropAngle(table[st+10], table[nd+10], prog), time));
            m_object->SetPartRotationZ(6+3*i+1, Math::Smooth(m_object->GetPartRotationZ(6+3*i+1), Math::PropAngle(table[st+11], table[nd+11], prog), time));
            m_object->SetPartRotationX(6+3*i+2, Math::Smooth(m_object->GetPartRotationX(6+3*i+2), Math::PropAngle(table[st+18], table[nd+18], prog), time));
            m_object->SetPartRotationY(6+3*i+2, Math::Smooth(m_object->GetPartRotationY(6+3*i+2), Math::PropAngle(table[st+19], table[nd+19], prog), time));
            m_object->SetPartRotationZ(6+3*i+2, Math::Smooth(m_object->GetPartRotationZ(6+3*i+2), Math::PropAngle(table[st+20], table[nd+20], prog), time));
        }
        else    // left leg (4..6) ?
        {
            m_object->SetPartRotationX(6+3*i+0, Math::Smooth(m_object->GetPartRotationX(6+3*i+0), Math::PropAngle(-table[st+ 0], -table[nd+ 0], prog), time));
            m_object->SetPartRotationY(6+3*i+0, Math::Smooth(m_object->GetPartRotationY(6+3*i+0), Math::PropAngle(-table[st+ 1], -table[nd+ 1], prog), time));
            m_object->SetPartRotationZ(6+3*i+0, Math::Smooth(m_object->GetPartRotationZ(6+3*i+0), Math::PropAngle( table[st+ 2],  table[nd+ 2], prog), time));
            m_object->SetPartRotationX(6+3*i+1, Math::Smooth(m_object->GetPartRotationX(6+3*i+1), Math::PropAngle(-table[st+ 9], -table[nd+ 9], prog), time));
            m_object->SetPartRotationY(6+3*i+1, Math::Smooth(m_object->GetPartRotationY(6+3*i+1), Math::PropAngle(-table[st+10], -table[nd+10], prog), time));
            m_object->SetPartRotationZ(6+3*i+1, Math::Smooth(m_object->GetPartRotationZ(6+3*i+1), Math::PropAngle( table[st+11],  table[nd+11], prog), time));
            m_object->SetPartRotationX(6+3*i+2, Math::Smooth(m_object->GetPartRotationX(6+3*i+2), Math::PropAngle(-table[st+18], -table[nd+18], prog), time));
            m_object->SetPartRotationY(6+3*i+2, Math::Smooth(m_object->GetPartRotationY(6+3*i+2), Math::PropAngle(-table[st+19], -table[nd+19], prog), time));
            m_object->SetPartRotationZ(6+3*i+2, Math::Smooth(m_object->GetPartRotationZ(6+3*i+2), Math::PropAngle( table[st+20],  table[nd+20], prog), time));
        }
    }

    if ( bStop )
    {
    }
    else
    {
        a = Math::Mod(m_armMember, 1.0f);
        if ( a < 0.5f )  a = -1.0f+4.0f*a;  // -1..1
        else             a =  3.0f-4.0f*a;  // 1..-1
        dir.x = sinf(a)*0.05f;

        s = Math::Mod(m_armMember/2.0f, 1.0f);
        if ( s < 0.5f )  s = -1.0f+4.0f*s;  // -1..1
        else             s =  3.0f-4.0f*s;  // 1..-1
        dir.z = sinf(s)*0.1f;

        dir.y = 0.0f;

        if ( bOnBoard )  dir *= 0.6f;
        SetTilt(dir);
    }

    return true;
}

// Event management for a insect cannon.

bool CMotionVehicle::EventFrameCanoni(const Event &event)
{
    float       zoom, angle, factor;
    bool        bOnBoard = false;

    m_canonTime += event.rTime;

    if ( m_object->GetSelect() &&
         m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD )
    {
        bOnBoard = true;
    }

    float energy = GetObjectEnergyLevel(m_object);
    if (energy == 0.0f)  return true;

    factor = 0.5f+energy*0.5f;
    if ( bOnBoard )  factor *= 0.8f;

    zoom = 1.3f+
           sinf(m_canonTime*Math::PI*0.31f)*0.10f+
           sinf(m_canonTime*Math::PI*0.52f)*0.08f+
           sinf(m_canonTime*Math::PI*1.53f)*0.05f;
    zoom *= factor;
    m_object->SetPartScaleY(2, zoom);

    zoom = 1.0f+
           sinf(m_canonTime*Math::PI*0.27f)*0.07f+
           sinf(m_canonTime*Math::PI*0.62f)*0.06f+
           sinf(m_canonTime*Math::PI*1.73f)*0.03f;
    zoom *= factor;
    m_object->SetPartScaleZ(2, zoom);

    angle = sinf(m_canonTime*1.0f)*0.10f+
            sinf(m_canonTime*1.3f)*0.15f+
            sinf(m_canonTime*2.7f)*0.05f;
    m_object->SetPartRotationX(2, angle);

    return true;
}


// Updates the mapping of the texture of the caterpillars.

void CMotionVehicle::UpdateTrackMapping(float left, float right, ObjectType type)
{
    int rRank = m_object->GetObjectRank(6);
    int lRank = m_object->GetObjectRank(7);

    std::string teamStr = StrUtils::ToString<int>(m_object->GetTeam());
    if(m_object->GetTeam() == 0) teamStr = "";

    constexpr float scale = 0.25f / 8.0f;

    left = left - std::floor(left);
    right = right - std::floor(right);

    if (type == OBJECT_MOBILEdr)
    {
        m_engine->SetUVTransform(rRank, "tracker_right",
            { (1.0f - right) * scale, 0.0f }, { 1.0f, 1.0f });

        m_engine->SetUVTransform(lRank, "tracker_left",
            { (1.0f - left) * scale, 0.0f }, { 1.0f, 1.0f });
    }
    else
    {
        m_engine->SetUVTransform(rRank, "tracker_right",
            { (1.0f - right) * scale, 0.0f }, { 1.0f, 1.0f });

        m_engine->SetUVTransform(lRank, "tracker_left",
            { (1.0f - left) * scale, 0.0f }, { 1.0f, 1.0f });
    }
}
