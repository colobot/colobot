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

/**
 * \file object/object_manager.h
 * \brief Object manager
 */

#pragma once

#include "common/singleton.h"

#include "math/const.h"
#include "math/vector.h"

#include "object/object_type.h"

#include <map>
#include <vector>
#include <memory>

namespace Gfx {
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
public:
    inline CObject* operator*()
    {
        return m_it->second.get();
    }

    inline void operator++()
    {
        ++m_it;
    }

    inline bool operator!=(const CObjectIteratorProxy& other)
    {
        return m_it != other.m_it;
    }

private:
    friend class CObjectContainerProxy;

    CObjectIteratorProxy(CObjectMapCIt it)
     : m_it(it)
    {}

private:
    CObjectMapCIt m_it;
};

class CObjectContainerProxy
{
private:
    friend class CObjectManager;

    inline CObjectContainerProxy(const CObjectMap& map, int& userCount)
     : m_map(map)
     , m_mapUserCount(userCount)
    {
        m_mapUserCount++;
    }

public:
    inline ~CObjectContainerProxy()
    {
        m_mapUserCount--;
    }

    inline CObjectIteratorProxy begin() const
    {
        return CObjectIteratorProxy(m_map.begin());
    }
    inline CObjectIteratorProxy end() const
    {
        return CObjectIteratorProxy(m_map.end());
    }

private:
    const CObjectMap& m_map;
    int& m_mapUserCount;
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
    CObject*  CreateObject(Math::Vector pos,
                           float angle,
                           ObjectType type,
                           float power = -1.f,
                           float zoom = 1.f,
                           float height = 0.f,
                           bool trainer = false,
                           bool toy = false,
                           int option = 0,
                           int id = -1);

    //! Deletes the object
    bool      DeleteObject(CObject* instance);
    //! Deletes all objects
    void      DeleteAllObjects();

    //! Finds object by id (CObject::GetID())
    CObject*  GetObjectById(unsigned int id);

    //! Gets object by id in range <0; number of objects - 1>
    CObject*  GetObjectByRank(unsigned int id);

    //! Gets all objects of given team
    std::vector<CObject*> GetObjectsOfTeam(int team);

    //! Checks if any of team's objects exist
    bool TeamExists(int team);

    //! Destroy all objects of team
    // TODO: This should be probably moved to separate class
    void DestroyTeam(int team);

    //! Returns all objects
    inline CObjectContainerProxy GetAllObjects()
    {
        return CObjectContainerProxy(m_objects, m_objectsIteratingUserCount);
    }

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
                    Math::Vector thisPosition,
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
                    Math::Vector thisPosition,
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
                          Math::Vector thisPosition,
                          ObjectType type = OBJECT_NULL,
                          float maxDist = 1000.0f,
                          bool cbotTypes = false);
    CObject*  FindNearest(CObject* pThis,
                          Math::Vector thisPosition,
                          std::vector<ObjectType> type = std::vector<ObjectType>(),
                          float maxDist = 1000.0f,
                          bool cbotTypes = false);
    //@}

protected:
    CObjectMap m_objects;
    int m_objectsIteratingUserCount;
    std::unique_ptr<CObjectFactory> m_objectFactory;
    int m_nextId;
};
