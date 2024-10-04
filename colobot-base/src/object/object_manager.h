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

/**
 * \file object/object_manager.h
 * \brief Object manager
 */

#pragma once

#include "common/singleton.h"

#include "math/const.h"

#include "object/object_create_params.h"
#include "object/object_interface_type.h"
#include "object/object_type.h"

#include "object/interface/destroyable_object.h"

#include <glm/glm.hpp>

#include <map>
#include <vector>
#include <memory>

namespace Gfx
{
class CEngine;
class CModelManager;
class COldModelManager;
class CParticle;
class CTerrain;
} // namespace Gfx

class CObject;
class CObjectFactory;

enum RadarFilter
{
    FILTER_NONE        = 0,

    FILTER_ONLYLANDING = 1 << (8+0),
    FILTER_ONLYFLYING  = 1 << (8+1),

    FILTER_FRIENDLY    = 1 << (8+2),
    FILTER_ENEMY       = 1 << (8+3),
    FILTER_NEUTRAL     = 1 << (8+4),
};

using CObjectMap = std::map<int, std::unique_ptr<CObject>>;
using CObjectMapCIt = std::map<int, std::unique_ptr<CObject>>::const_iterator;

class CObjectIteratorProxy
{
private:
    friend class CObjectContainerProxy;

    CObjectIteratorProxy(CObjectMapCIt currentIt, CObjectMapCIt endIt)
     : m_currentIt(currentIt)
     , m_endIt(endIt)
    {
        while (m_currentIt != m_endIt && m_currentIt->second == nullptr)
        {
            ++m_currentIt;
        }
    }

public:
    CObject* operator*()
    {
        return m_currentIt->second.get();
    }

    void operator++()
    {
        do
        {
            ++m_currentIt;
        }
        while (m_currentIt != m_endIt && m_currentIt->second == nullptr);
    }

    bool operator==(const CObjectIteratorProxy& other) const
    {
        return m_currentIt == other.m_currentIt;
    }

private:
    CObjectMapCIt m_currentIt;
    CObjectMapCIt m_endIt;
};

class CObjectContainerProxy
{
private:
    friend class CObjectManager;

    CObjectContainerProxy(const CObjectMap& map, int& activeIteratorsCounter)
     : m_map(map),
       m_activeIteratorsCounter(activeIteratorsCounter)
    {
        ++m_activeIteratorsCounter;
    }

public:
    ~CObjectContainerProxy()
    {
        --m_activeIteratorsCounter;
    }

    CObjectIteratorProxy begin() const
    {
        return CObjectIteratorProxy(m_map.begin(), m_map.end());
    }
    CObjectIteratorProxy end() const
    {
        return CObjectIteratorProxy(m_map.end(), m_map.end());
    }

private:
    const CObjectMap& m_map;
    int& m_activeIteratorsCounter;
};

/**
 * \class CObjectManager
 * \brief Manages CObject instances
 */
class CObjectManager : public CSingleton<CObjectManager>
{
public:
    CObjectManager(Gfx::CEngine* engine,
                   Gfx::CTerrain* terrain,
                   Gfx::COldModelManager* oldModelManager,
                   Gfx::CModelManager* modelManager,
                   Gfx::CParticle* particle);
    virtual ~CObjectManager();

    //! Creates an object
    //@{
    CObject*  CreateObject(ObjectCreateParams params);
    CObject*  CreateObject(glm::vec3 pos, float angle, ObjectType type, float power = -1.0f);
    //@}

    //! Deletes the object
    bool      DeleteObject(CObject* instance);
    //! Deletes all objects
    void      DeleteAllObjects();

    //! Finds object by id (CObject::GetID())
    CObject*  GetObjectById(int id);

    //! Gets object by id in range <0; number of objects - 1>
    CObject*  GetObjectByRank(unsigned int id);

    //! Gets all objects of given team
    std::vector<CObject*> GetObjectsOfTeam(int team);

    //! Checks if any of team's objects exist
    bool TeamExists(int team);

    //! Destroy all objects of team
    // TODO: This should be probably moved to separate class
    void DestroyTeam(int team, DestructionType destructionType = DestructionType::Explosion);

    //! Counts all objects implementing given interface
    int CountObjectsImplementing(ObjectInterfaceType interface);

    //! Returns all objects
    CObjectContainerProxy GetAllObjects()
    {
        CleanRemovedObjectsIfNeeded();
        return CObjectContainerProxy(m_objects, m_activeObjectIterators);
    }

    //! Finds an object, like radar() in CBot
    //@{
    std::vector<CObject*> RadarAll(CObject* pThis,
                    ObjectType type = OBJECT_NULL,
                    float angle = 0.0f,
                    float focus = Math::PI*2.0f,
                    float minDist = 0.0f,
                    float maxDist = 1000.0f,
                    bool furthest = false,
                    RadarFilter filter = FILTER_NONE,
                    bool cbotTypes = false);
    std::vector<CObject*> RadarAll(CObject* pThis,
                    std::vector<ObjectType> type = std::vector<ObjectType>(),
                    float angle = 0.0f,
                    float focus = Math::PI*2.0f,
                    float minDist = 0.0f,
                    float maxDist = 1000.0f,
                    bool furthest = false,
                    RadarFilter filter = FILTER_NONE,
                    bool cbotTypes = false);
    std::vector<CObject*> RadarAll(CObject* pThis,
                       glm::vec3 thisPosition,
                       float thisAngle,
                       ObjectType type = OBJECT_NULL,
                       float angle = 0.0f,
                       float focus = Math::PI*2.0f,
                       float minDist = 0.0f,
                       float maxDist = 1000.0f,
                       bool furthest = false,
                       RadarFilter filter = FILTER_NONE,
                       bool cbotTypes = false);
    std::vector<CObject*> RadarAll(CObject* pThis,
                    glm::vec3 thisPosition,
                    float thisAngle,
                    std::vector<ObjectType> type = std::vector<ObjectType>(),
                    float angle = 0.0f,
                    float focus = Math::PI*2.0f,
                    float minDist = 0.0f,
                    float maxDist = 1000.0f,
                    bool furthest = false,
                    RadarFilter filter = FILTER_NONE,
                    bool cbotTypes = false);
    //@}
    //! Finds an object, like radar() in CBot
    //@{
    CObject*  Radar(CObject* pThis,
                    ObjectType type = OBJECT_NULL,
                    float angle = 0.0f,
                    float focus = Math::PI*2.0f,
                    float minDist = 0.0f,
                    float maxDist = 1000.0f,
                    bool furthest = false,
                    RadarFilter filter = FILTER_NONE,
                    bool cbotTypes = false);
    CObject*  Radar(CObject* pThis,
                    std::vector<ObjectType> type = std::vector<ObjectType>(),
                    float angle = 0.0f,
                    float focus = Math::PI*2.0f,
                    float minDist = 0.0f,
                    float maxDist = 1000.0f,
                    bool furthest = false,
                    RadarFilter filter = FILTER_NONE,
                    bool cbotTypes = false);
    CObject*  Radar(CObject* pThis,
                    glm::vec3 thisPosition,
                    float thisAngle,
                    ObjectType type = OBJECT_NULL,
                    float angle = 0.0f,
                    float focus = Math::PI*2.0f,
                    float minDist = 0.0f,
                    float maxDist = 1000.0f,
                    bool furthest = false,
                    RadarFilter filter = FILTER_NONE,
                    bool cbotTypes = false);
    CObject*  Radar(CObject* pThis,
                    glm::vec3 thisPosition,
                    float thisAngle,
                    std::vector<ObjectType> type = std::vector<ObjectType>(),
                    float angle = 0.0f,
                    float focus = Math::PI*2.0f,
                    float minDist = 0.0f,
                    float maxDist = 1000.0f,
                    bool furthest = false,
                    RadarFilter filter = FILTER_NONE,
                    bool cbotTypes = false);
    //@}
    //! Returns nearest object that's closer than maxDist
    //@{
    CObject*  FindNearest(CObject* pThis,
                          ObjectType type = OBJECT_NULL,
                          float maxDist = 1000.0f,
                          bool cbotTypes = false);
    CObject*  FindNearest(CObject* pThis,
                          std::vector<ObjectType> type = std::vector<ObjectType>(),
                          float maxDist = 1000.0f,
                          bool cbotTypes = false);
    CObject*  FindNearest(CObject* pThis,
                          glm::vec3 thisPosition,
                          ObjectType type = OBJECT_NULL,
                          float maxDist = 1000.0f,
                          bool cbotTypes = false);
    CObject*  FindNearest(CObject* pThis,
                          glm::vec3 thisPosition,
                          std::vector<ObjectType> type = std::vector<ObjectType>(),
                          float maxDist = 1000.0f,
                          bool cbotTypes = false);
    //@}

private:
    //! Prevents creation of overcharged power cells
    float ClampPower(ObjectType type, float power);
    void CleanRemovedObjectsIfNeeded();

private:
    CObjectMap m_objects;
    std::unique_ptr<CObjectFactory> m_objectFactory;
    int m_nextId;
    int m_activeObjectIterators;
    bool m_shouldCleanRemovedObjects;
};
