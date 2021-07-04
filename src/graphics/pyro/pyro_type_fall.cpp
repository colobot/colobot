/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "graphics/pyro/pyro.h"
#include "graphics/pyro/pyro_manager.h"

#include "object/object.h"
#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/subclass/shielder.h"

#include "sound/sound.h"

using namespace Gfx;

CFallPyro::CFallPyro(CObject *obj)
    : CPyro(PT_OTHER, obj)
{}

void CFallPyro::AfterCreate()
{
    m_object->SetLock(true);  // usable

    Math::Vector pos = m_object->GetPosition();
    m_fallFloor = m_terrain->GetFloorLevel(pos);
    m_fallSpeed = 0.0f;
    m_fallBulletTime = 0.0f;
    m_fallEnding = false;
}

CObject* CFallPyro::FallSearchBeeExplo()
{
    auto bulletCrashSphere = m_object->GetFirstCrashSphere();

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (obj == m_object) continue;
        if (obj->GetType() == OBJECT_BEE) continue;
        if ( !obj->Implements(ObjectInterfaceType::Destroyable) )  continue;

        if (IsObjectBeingTransported(obj)) continue;

        Math::Vector oPos = obj->GetPosition();

        if (obj->GetType() == OBJECT_MOBILErs)
        {
            float shieldRadius = dynamic_cast<CShielder&>(*obj).GetActiveShieldRadius();
            if ( shieldRadius > 0.0f )
            {
                float distance = Math::Distance(oPos, bulletCrashSphere.sphere.pos);
                if (distance <= shieldRadius)
                    return obj;
            }
        }

        if ( obj->GetType() == OBJECT_BASE )
        {
            float distance = Math::Distance(oPos, bulletCrashSphere.sphere.pos);
            if (distance < 25.0f)
                return obj;
        }

        // Test the center of the object, which is necessary for objects
        // that have no sphere in the center (station).
        float distance = Math::Distance(oPos, bulletCrashSphere.sphere.pos)-4.0f;
        if (distance < 5.0f)
            return obj;

        // Test with all spheres of the object.
        for (const auto& objCrashSphere : obj->GetAllCrashSpheres())
        {
            if (Math::DistanceBetweenSpheres(objCrashSphere.sphere, bulletCrashSphere.sphere) <= 0.0f)
            {
                return obj;
            }
        }
    }

    return nullptr;
}

void CFallPyro::FallProgress(float rTime)
{
    if (m_object == nullptr) return;

    m_fallSpeed += rTime*50.0f;  // v2 = v1 + a*dt
    Math::Vector pos;
    pos = m_object->GetPosition();
    pos.y -= m_fallSpeed*rTime;  // dd -= v2*dt

    bool floor = false;

    if (pos.y <= m_fallFloor)  // below the ground level?
    {
        pos.y = m_fallFloor;
        floor = true;
    }
    m_object->SetPosition(pos);

    if (m_object->GetType() == OBJECT_BULLET)
    {
        m_fallBulletTime += rTime;

        if (m_fallBulletTime > 0.2f || floor)
        {
            m_fallBulletTime = 0.0f;

            CObject* obj = FallSearchBeeExplo();
            if (obj == nullptr)
            {
                if (floor)  // reaches the ground?
                {
                    assert(m_object->Implements(ObjectInterfaceType::Destroyable));
                    // TODO: implement "killer"?
                    dynamic_cast<CDestroyableObject&>(*m_object).DestroyObject(DestructionType::Explosion);
                }
            }
            else
            {
                if (obj->GetType() == OBJECT_MOBILErs && dynamic_cast<CShielder&>(*obj).GetActiveShieldRadius() > 0.0f)  // protected by shield?
                {
                    m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f),
                                                Math::Point(6.0f, 6.0f), PARTIGUNDEL, 2.0f, 0.0f, 0.0f);
                    m_sound->Play(SOUND_GUNDEL);

                    DeleteObject(true, true);  // removes the ball
                }
                else
                {
                    assert(obj->Implements(ObjectInterfaceType::Damageable));
                    if (dynamic_cast<CDamageableObject&>(*obj).DamageObject(DamageType::FallingObject))
                    {
                        DeleteObject(true, true);  // removes the ball
                    }
                    else
                    {
                        assert(m_object->Implements(ObjectInterfaceType::Destroyable));
                        // TODO: implement "killer"?
                        dynamic_cast<CDestroyableObject&>(*m_object).DestroyObject(DestructionType::Explosion);
                    }
                }
            }

            if (floor || obj != nullptr)
            {
                m_fallEnding = true;
            }
        }
    }
}

Error CFallPyro::IsEnded()
{
    if (m_fallEnding || m_object == nullptr) return ERR_STOP;

    Math::Vector pos = m_object->GetPosition();
    if (pos.y > m_fallFloor) return ERR_CONTINUE;

    return ERR_STOP;
}

void CFallPyro::AfterEnd()
{
    if (m_object != nullptr)
    {
        m_sound->Play(SOUND_BOUM, m_object->GetPosition());
        m_object->SetLock(false);  // usable again
    }
}

bool CFallPyro::EventProcess(const Event& event)
{
    CPyro::EventProcess(event);

    if ( event.type != EVENT_FRAME ) return true;
    if ( m_engine->GetPause() ) return true;

    // don't use UpdateEffect since we want to access event.rTime
    FallProgress(event.rTime);

    return true;
}
