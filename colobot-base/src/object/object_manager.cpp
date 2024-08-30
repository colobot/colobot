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

#include "object/object_manager.h"

#include "common/global.h"

#include "math/all.h"

#include "object/object.h"
#include "object/object_create_exception.h"
#include "object/object_create_params.h"
#include "object/object_factory.h"
#include "object/old_object.h"

#include "object/auto/auto.h"

#include "physics/physics.h"

#include <algorithm>

CObjectManager::CObjectManager(Gfx::CEngine* engine,
                               Gfx::CTerrain* terrain,
                               Gfx::COldModelManager* oldModelManager,
                               Gfx::CModelManager* modelManager,
                               Gfx::CParticle* particle)
  : m_objectFactory(std::make_unique<CObjectFactory>(engine,
                                               terrain,
                                               oldModelManager,
                                               modelManager,
                                               particle)),
    m_nextId(0),
    m_activeObjectIterators(0),
    m_shouldCleanRemovedObjects(false)
{
}

CObjectManager::~CObjectManager()
{
}

bool CObjectManager::DeleteObject(CObject* instance)
{
    assert(instance != nullptr);

    // TODO: temporarily...
    auto oldObj = dynamic_cast<COldObject*>(instance);
    if (oldObj != nullptr)
        oldObj->DeleteObject();

    auto it = m_objects.find(instance->GetID());
    if (it != m_objects.end())
    {
        it->second.reset();
        m_shouldCleanRemovedObjects = true;
        return true;
    } else assert(false);

    return false;
}

void CObjectManager::CleanRemovedObjectsIfNeeded()
{
    if (m_activeObjectIterators != 0)
        return;

    if (! m_shouldCleanRemovedObjects)
        return;

    auto it = m_objects.begin();
    if (it != m_objects.end())
    {
        if (it->second == nullptr)
            it = m_objects.erase(it);
    }

    m_shouldCleanRemovedObjects = false;
}

void CObjectManager::DeleteAllObjects()
{
    for (auto& it : m_objects)
    {
        // TODO: temporarily...
        auto oldObj = dynamic_cast<COldObject*>(it.second.get());
        if (oldObj != nullptr)
        {
            bool all = true;
            oldObj->DeleteObject(all);
        }
    }

    m_objects.clear();

    m_nextId = 0;
}

CObject* CObjectManager::GetObjectById(int id)
{
    if (m_objects.count(id) == 0) return nullptr;
    return m_objects[id].get();
}

CObject* CObjectManager::GetObjectByRank(unsigned int id)
{
    auto objects = GetAllObjects();
    auto it = objects.begin();
    for (unsigned int i = 0; i < id && it != objects.end(); i++, ++it);
    if (it == objects.end()) return nullptr;
    return *it;
}

CObject* CObjectManager::CreateObject(ObjectCreateParams params)
{
    if (params.id < 0)
    {
        params.id = m_nextId;
        m_nextId++;
    }
    else
    {
        if (params.id >= m_nextId)
        {
            m_nextId = params.id + 1;
        }
    }

    params.power = ClampPower(params.type,params.power);

    assert(m_objects.find(params.id) == m_objects.end());

    auto objectUPtr = m_objectFactory->CreateObject(params);

    if (objectUPtr == nullptr)
        throw CObjectCreateException("Something went wrong in CObjectFactory", params.type);

    CObject* objectPtr = objectUPtr.get();

    m_objects[params.id] = std::move(objectUPtr);

    return objectPtr;
}

CObject* CObjectManager::CreateObject(glm::vec3 pos, float angle, ObjectType type, float power)
{
    ObjectCreateParams params;
    params.pos = pos;
    params.angle = angle;
    params.type = type;
    params.power = power;
    return CreateObject(params);
}

float CObjectManager::ClampPower(ObjectType type, float power)
{
    float min = 0;
    float max = 100;
    if (type == OBJECT_POWER || type == OBJECT_ATOMIC)
    {
        max = 1;
    }
    return glm::clamp(power, min, max);
}

std::vector<CObject*> CObjectManager::GetObjectsOfTeam(int team)
{
    std::vector<CObject*> result;
    for (CObject* object : GetAllObjects())
    {
        if (object->GetTeam() == team)
        {
            result.push_back(object);
        }
    }
    return result;
}

bool CObjectManager::TeamExists(int team)
{
    if(team == 0) return true;

    for (CObject* object : GetAllObjects())
    {
        if (!object->GetActive())
            continue;

        if (object->GetTeam() == team)
            return true;
    }
    return false;
}

void CObjectManager::DestroyTeam(int team, DestructionType destructionType)
{
    assert(team != 0);

    for (CObject* object : GetAllObjects())
    {
        if (object->GetTeam() == team)
        {
            if (object->Implements(ObjectInterfaceType::Destroyable))
            {
                dynamic_cast<CDestroyableObject&>(*object).DestroyObject(destructionType);
            }
            else
            {
                DeleteObject(object);
            }
        }
    }
}

int CObjectManager::CountObjectsImplementing(ObjectInterfaceType interface)
{
    int count = 0;
    for (CObject* object : GetAllObjects())
    {
        if (object->Implements(interface))
            count++;
    }
    return count;
}

std::vector<CObject*> CObjectManager::RadarAll(CObject* pThis, ObjectType type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    std::vector<ObjectType> types;
    if (type != OBJECT_NULL)
        types.push_back(type);
    return RadarAll(pThis, types, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
}

std::vector<CObject*> CObjectManager::RadarAll(CObject* pThis, std::vector<ObjectType> type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    glm::vec3 iPos{};
    float iAngle;
    if (pThis != nullptr)
    {
        iPos   = pThis->GetPosition();
        iAngle = pThis->GetRotationY();
        iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI
    }
    else
    {
        iPos   = glm::vec3(0, 0, 0);
        iAngle = 0.0f;
    }
    return RadarAll(pThis, iPos, iAngle, type, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
}

std::vector<CObject*> CObjectManager::RadarAll(CObject* pThis, glm::vec3 thisPosition, float thisAngle, ObjectType type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    std::vector<ObjectType> types;
    if (type != OBJECT_NULL)
        types.push_back(type);
    return RadarAll(pThis, thisPosition, thisAngle, types, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
}

std::vector<CObject*> CObjectManager::RadarAll(CObject* pThis, glm::vec3 thisPosition, float thisAngle, std::vector<ObjectType> type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    CObject     *pObj;
    glm::vec3    iPos{ 0, 0, 0 }, oPos{ 0, 0, 0 };
    float       iAngle, d, a;
    ObjectType  oType;

    minDist *= g_unit;
    maxDist *= g_unit;

    iPos   = thisPosition;
    iAngle = thisAngle+angle;
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    int filter_team = filter & 0xFF;
    RadarFilter filter_flying = static_cast<RadarFilter>(filter & (FILTER_ONLYLANDING | FILTER_ONLYFLYING));
    RadarFilter filter_enemy = static_cast<RadarFilter>(filter & (FILTER_FRIENDLY | FILTER_ENEMY | FILTER_NEUTRAL));

    // Use a multimap to allow for multiple objects at exactly the same distance
    // from the origin to be returned.
    std::multimap<float, CObject*> best;

    for ( auto it = m_objects.begin() ; it != m_objects.end() ; ++it )
    {
        pObj = it->second.get();
        if ( pObj == pThis )  continue; // pThis may be nullptr but it doesn't matter

        if (pObj == nullptr) continue;
        if (IsObjectBeingTransported(pObj))  continue;
        if ( !pObj->GetDetectable() )  continue;
        if ( pObj->GetProxyActivate() )  continue;

        oType = pObj->GetType();

        if (cbotTypes)
        {
            // TODO: handle this differently (new class describing types? CObjectType::GetBaseType()?)
            if ( oType == OBJECT_RUINmobilew2 ||
                oType == OBJECT_RUINmobilet1 ||
                oType == OBJECT_RUINmobilet2 ||
                oType == OBJECT_RUINmobiler1 ||
                oType == OBJECT_RUINmobiler2 )
            {
                oType = OBJECT_RUINmobilew1;  // any wreck
            }

            if ( oType == OBJECT_BARRIER2 ||
                 oType == OBJECT_BARRIER3 ||
                 oType == OBJECT_BARRICADE0 ||
                 oType == OBJECT_BARRICADE1 )  // barriers?
            {
                oType = OBJECT_BARRIER1;  // any barrier
            }

            if ( oType == OBJECT_RUINdoor    ||
                 oType == OBJECT_RUINsupport ||
                 oType == OBJECT_RUINradar   ||
                 oType == OBJECT_RUINconvert )  // ruins?
            {
                oType = OBJECT_RUINfactory;  // any ruin
            }

            if ( oType == OBJECT_PLANT1  ||
                 oType == OBJECT_PLANT2  ||
                 oType == OBJECT_PLANT3  ||
                 oType == OBJECT_PLANT4  ||
                 oType == OBJECT_PLANT15 ||
                 oType == OBJECT_PLANT16 ||
                 oType == OBJECT_PLANT17 ||
                 oType == OBJECT_PLANT18 )  // bushes?
            {
                oType = OBJECT_PLANT0;  // any bush
            }

            if ( oType == OBJECT_QUARTZ1 ||
                 oType == OBJECT_QUARTZ2 ||
                 oType == OBJECT_QUARTZ3 )  // crystals?
            {
                oType = OBJECT_QUARTZ0;  // any crystal
            }
            // END OF TODO
        }

        if ( std::find(type.begin(), type.end(), oType) == type.end() && type.size() > 0 )  continue;

        if ( (oType == OBJECT_TOTO || oType == OBJECT_CONTROLLER) && type.size() == 0 )  continue; // allow OBJECT_TOTO and OBJECT_CONTROLLER only if explicitly asked in type parameter

        if ( filter_flying == FILTER_ONLYLANDING )
        {
            if ( pObj->Implements(ObjectInterfaceType::Movable) )
            {
                CPhysics* physics = dynamic_cast<CMovableObject&>(*pObj).GetPhysics();
                if ( physics != nullptr )
                {
                    if ( !physics->GetLand() )  continue;
                }
            }
        }
        if ( filter_flying == FILTER_ONLYFLYING )
        {
            if ( !pObj->Implements(ObjectInterfaceType::Movable) ) continue;
            CPhysics* physics = dynamic_cast<CMovableObject&>(*pObj).GetPhysics();
            if ( physics == nullptr ) continue;
            if ( physics->GetLand() ) continue;
        }

        if ( filter_team != 0 && pObj->GetTeam() != filter_team )
            continue;

        if( pThis != nullptr )
        {
            RadarFilter enemy = FILTER_NONE;
            if ( pObj->GetTeam() == 0 ) enemy = static_cast<RadarFilter>(enemy | FILTER_NEUTRAL);
            if ( pObj->GetTeam() != 0 && pObj->GetTeam() == pThis->GetTeam() ) enemy = static_cast<RadarFilter>(enemy | FILTER_FRIENDLY);
            if ( pObj->GetTeam() != 0 && pObj->GetTeam() != pThis->GetTeam() ) enemy = static_cast<RadarFilter>(enemy | FILTER_ENEMY);
            if ( filter_enemy != 0 && (filter_enemy & enemy) == 0 ) continue;
        }

        oPos = pObj->GetPosition();
        d = Math::DistanceProjected(iPos, oPos);
        if ( d < minDist || d > maxDist )  continue;  // too close or too far?

        a = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
        if ( Math::TestAngle(a, iAngle-focus/2.0f, iAngle+focus/2.0f) || focus >= Math::PI*2.0f )
        {
            best.insert(std::make_pair(d, pObj));
        }
    }

    std::vector<CObject*> sortedBest;
    if (!furthest)
    {
        for (auto it = best.begin(); it != best.end(); ++it)
        {
            sortedBest.push_back(it->second);
        }
    }
    else
    {
        for (auto it = best.rbegin(); it != best.rend(); ++it)
        {
            sortedBest.push_back(it->second);
        }
    }

    return sortedBest;
}

CObject* CObjectManager::Radar(CObject* pThis, ObjectType type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    std::vector<CObject*> best = RadarAll(pThis, type, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
    return best.size() > 0 ? best[0] : nullptr;
}

CObject* CObjectManager::Radar(CObject* pThis, std::vector<ObjectType> type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    std::vector<CObject*> best = RadarAll(pThis, type, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
    return best.size() > 0 ? best[0] : nullptr;
}

CObject* CObjectManager::Radar(CObject* pThis, glm::vec3 thisPosition, float thisAngle, ObjectType type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    std::vector<CObject*> best = RadarAll(pThis, thisPosition, thisAngle, type, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
    return best.size() > 0 ? best[0] : nullptr;
}

CObject* CObjectManager::Radar(CObject* pThis, glm::vec3 thisPosition, float thisAngle, std::vector<ObjectType> type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    std::vector<CObject*> best = RadarAll(pThis, thisPosition, thisAngle, type, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
    return best.size() > 0 ? best[0] : nullptr;
}

CObject*  CObjectManager::FindNearest(CObject* pThis, ObjectType type, float maxDist, bool cbotTypes)
{
    return Radar(pThis, type, 0.0f, Math::PI*2.0f, 0.0f, maxDist, false, FILTER_NONE, cbotTypes);
}

CObject*  CObjectManager::FindNearest(CObject* pThis, std::vector<ObjectType> type, float maxDist, bool cbotTypes)
{
    return Radar(pThis, type, 0.0f, Math::PI*2.0f, 0.0f, maxDist, false, FILTER_NONE, cbotTypes);
}

CObject*  CObjectManager::FindNearest(CObject* pThis, glm::vec3 thisPosition, ObjectType type, float maxDist, bool cbotTypes)
{
    return Radar(pThis, thisPosition, 0.0f, type, 0.0f, Math::PI*2.0f, 0.0f, maxDist, false, FILTER_NONE, cbotTypes);
}

CObject*  CObjectManager::FindNearest(CObject* pThis, glm::vec3 thisPosition, std::vector<ObjectType> type, float maxDist, bool cbotTypes)
{
    return Radar(pThis, thisPosition, 0.0f, type, 0.0f, Math::PI*2.0f, 0.0f, maxDist, false, FILTER_NONE, cbotTypes);
}
