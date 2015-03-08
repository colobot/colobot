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

#include "object/objman.h"


#include "math/all.h"

#include "object/object.h"
#include "object/auto/auto.h"

#include "physics/physics.h"

#include <algorithm>


template<> CObjectManager* CSingleton<CObjectManager>::m_instance = nullptr;


CObjectManager::CObjectManager()
{
}

CObjectManager::~CObjectManager()
{
}

bool CObjectManager::AddObject(CObject* instance)
{
    assert(instance != nullptr);
    assert(m_table[instance->GetID()] == nullptr);
    m_table[instance->GetID()] = instance;
    return true;
}

bool CObjectManager::DeleteObject(CObject* instance)
{
    assert(instance != nullptr);
    for(auto it = m_table.begin(); it != m_table.end(); ++it)
    {
        if(it->second == instance)
        {
            m_table.erase(it);
            return true;
        }
    }

    return false;
}

CObject* CObjectManager::GetObjectById(unsigned int id)
{
    if(m_table.count(id) == 0) return nullptr;
    return m_table[id];
}

CObject* CObjectManager::GetObjectByRank(unsigned int id)
{
    if(id >= m_table.size()) return nullptr;
    auto it = m_table.begin();
    for(unsigned int i = 0; i < id; i++, ++it);
    return it->second;
}

const std::map<unsigned int, CObject*>& CObjectManager::GetAllObjects()
{
    return m_table;
}

void CObjectManager::Flush()
{
    m_table.clear();
}

CObject* CObjectManager::CreateObject(Math::Vector pos, float angle, ObjectType type,
                                      float power, float zoom, float height,
                                      bool trainer, bool toy, int option)
{
    CObject* object = nullptr;

    if ( type == OBJECT_NULL ) return nullptr;

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        trainer = false;  // necessarily
    }

    if ( type == OBJECT_PORTICO  ||
         type == OBJECT_BASE     ||
         type == OBJECT_DERRICK  ||
         type == OBJECT_FACTORY  ||
         type == OBJECT_STATION  ||
         type == OBJECT_CONVERT  ||
         type == OBJECT_REPAIR   ||
         type == OBJECT_DESTROYER||
         type == OBJECT_TOWER    ||
         type == OBJECT_NEST     ||
         type == OBJECT_RESEARCH ||
         type == OBJECT_RADAR    ||
         type == OBJECT_INFO     ||
         type == OBJECT_ENERGY   ||
         type == OBJECT_LABO     ||
         type == OBJECT_NUCLEAR  ||
         type == OBJECT_PARA     ||
         type == OBJECT_SAFE     ||
         type == OBJECT_HUSTON   ||
         type == OBJECT_TARGET1  ||
         type == OBJECT_TARGET2  ||
         type == OBJECT_START    ||
         type == OBJECT_END      )
    {
        object = new CObject();
        object->CreateBuilding(pos, angle, height, type, power);

        CAuto* automat = object->GetAuto();
        if (automat != nullptr)
        {
            automat->Init();
        }
    }
    else
    if ( type == OBJECT_FRET        ||
         type == OBJECT_STONE       ||
         type == OBJECT_URANIUM     ||
         type == OBJECT_METAL       ||
         type == OBJECT_POWER       ||
         type == OBJECT_ATOMIC      ||
         type == OBJECT_BULLET      ||
         type == OBJECT_BBOX        ||
         type == OBJECT_KEYa        ||
         type == OBJECT_KEYb        ||
         type == OBJECT_KEYc        ||
         type == OBJECT_KEYd        ||
         type == OBJECT_TNT         ||
         type == OBJECT_SCRAP1      ||
         type == OBJECT_SCRAP2      ||
         type == OBJECT_SCRAP3      ||
         type == OBJECT_SCRAP4      ||
         type == OBJECT_SCRAP5      ||
         type == OBJECT_BOMB        ||
         type == OBJECT_WAYPOINT    ||
         type == OBJECT_SHOW        ||
         type == OBJECT_WINFIRE     ||
         type == OBJECT_BAG         ||
         type == OBJECT_MARKPOWER   ||
         type == OBJECT_MARKSTONE   ||
         type == OBJECT_MARKURANIUM ||
         type == OBJECT_MARKKEYa    ||
         type == OBJECT_MARKKEYb    ||
         type == OBJECT_MARKKEYc    ||
         type == OBJECT_MARKKEYd    ||
         type == OBJECT_EGG         )
    {
        object = new CObject();
        object->CreateResource(pos, angle, type, power);
    }
    else
    if ( type == OBJECT_FLAGb ||
         type == OBJECT_FLAGr ||
         type == OBJECT_FLAGg ||
         type == OBJECT_FLAGy ||
         type == OBJECT_FLAGv )
    {
        object = new CObject();
        object->CreateFlag(pos, angle, type);
    }
    else
    if ( type == OBJECT_BARRIER0 ||
         type == OBJECT_BARRIER1 ||
         type == OBJECT_BARRIER2 ||
         type == OBJECT_BARRIER3 )
    {
        object = new CObject();
        object->CreateBarrier(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_PLANT0  ||
         type == OBJECT_PLANT1  ||
         type == OBJECT_PLANT2  ||
         type == OBJECT_PLANT3  ||
         type == OBJECT_PLANT4  ||
         type == OBJECT_PLANT5  ||
         type == OBJECT_PLANT6  ||
         type == OBJECT_PLANT7  ||
         type == OBJECT_PLANT8  ||
         type == OBJECT_PLANT9  ||
         type == OBJECT_PLANT10 ||
         type == OBJECT_PLANT11 ||
         type == OBJECT_PLANT12 ||
         type == OBJECT_PLANT13 ||
         type == OBJECT_PLANT14 ||
         type == OBJECT_PLANT15 ||
         type == OBJECT_PLANT16 ||
         type == OBJECT_PLANT17 ||
         type == OBJECT_PLANT18 ||
         type == OBJECT_PLANT19 ||
         type == OBJECT_TREE0   ||
         type == OBJECT_TREE1   ||
         type == OBJECT_TREE2   ||
         type == OBJECT_TREE3   ||
         type == OBJECT_TREE4   ||
         type == OBJECT_TREE5   )
    {
        object = new CObject();
        object->CreatePlant(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_MUSHROOM1 ||
         type == OBJECT_MUSHROOM2 )
    {
        object = new CObject();
        object->CreateMushroom(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_TEEN0  ||
         type == OBJECT_TEEN1  ||
         type == OBJECT_TEEN2  ||
         type == OBJECT_TEEN3  ||
         type == OBJECT_TEEN4  ||
         type == OBJECT_TEEN5  ||
         type == OBJECT_TEEN6  ||
         type == OBJECT_TEEN7  ||
         type == OBJECT_TEEN8  ||
         type == OBJECT_TEEN9  ||
         type == OBJECT_TEEN10 ||
         type == OBJECT_TEEN11 ||
         type == OBJECT_TEEN12 ||
         type == OBJECT_TEEN13 ||
         type == OBJECT_TEEN14 ||
         type == OBJECT_TEEN15 ||
         type == OBJECT_TEEN16 ||
         type == OBJECT_TEEN17 ||
         type == OBJECT_TEEN18 ||
         type == OBJECT_TEEN19 ||
         type == OBJECT_TEEN20 ||
         type == OBJECT_TEEN21 ||
         type == OBJECT_TEEN22 ||
         type == OBJECT_TEEN23 ||
         type == OBJECT_TEEN24 ||
         type == OBJECT_TEEN25 ||
         type == OBJECT_TEEN26 ||
         type == OBJECT_TEEN27 ||
         type == OBJECT_TEEN28 ||
         type == OBJECT_TEEN29 ||
         type == OBJECT_TEEN30 ||
         type == OBJECT_TEEN31 ||
         type == OBJECT_TEEN32 ||
         type == OBJECT_TEEN33 ||
         type == OBJECT_TEEN34 ||
         type == OBJECT_TEEN35 ||
         type == OBJECT_TEEN36 ||
         type == OBJECT_TEEN37 ||
         type == OBJECT_TEEN38 ||
         type == OBJECT_TEEN39 ||
         type == OBJECT_TEEN40 ||
         type == OBJECT_TEEN41 ||
         type == OBJECT_TEEN42 ||
         type == OBJECT_TEEN43 ||
         type == OBJECT_TEEN44 )
    {
        object = new CObject();
        object->SetOption(option);
        object->CreateTeen(pos, angle, zoom, height, type);
    }
    else
    if ( type == OBJECT_QUARTZ0 ||
         type == OBJECT_QUARTZ1 ||
         type == OBJECT_QUARTZ2 ||
         type == OBJECT_QUARTZ3 )
    {
        object = new CObject();
        object->CreateQuartz(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_ROOT0 ||
         type == OBJECT_ROOT1 ||
         type == OBJECT_ROOT2 ||
         type == OBJECT_ROOT3 ||
         type == OBJECT_ROOT4 ||
         type == OBJECT_ROOT5 )
    {
        object = new CObject();
        object->CreateRoot(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_HOME1 )
    {
        object = new CObject();
        object->CreateHome(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_RUINmobilew1 ||
         type == OBJECT_RUINmobilew2 ||
         type == OBJECT_RUINmobilet1 ||
         type == OBJECT_RUINmobilet2 ||
         type == OBJECT_RUINmobiler1 ||
         type == OBJECT_RUINmobiler2 ||
         type == OBJECT_RUINfactory  ||
         type == OBJECT_RUINdoor     ||
         type == OBJECT_RUINsupport  ||
         type == OBJECT_RUINradar    ||
         type == OBJECT_RUINconvert  ||
         type == OBJECT_RUINbase     ||
         type == OBJECT_RUINhead     )
    {
        object = new CObject();
        object->CreateRuin(pos, angle, height, type);
    }
    else
    if ( type == OBJECT_APOLLO1 ||
         type == OBJECT_APOLLO3 ||
         type == OBJECT_APOLLO4 ||
         type == OBJECT_APOLLO5 )
    {
        object = new CObject();
        object->CreateApollo(pos, angle, type);
    }
    else
    if ( type == OBJECT_MOTHER ||
         type == OBJECT_ANT    ||
         type == OBJECT_SPIDER ||
         type == OBJECT_BEE    ||
         type == OBJECT_WORM   )
    {
        object = new CObject();
        object->CreateInsect(pos, angle, type);  // no eggs
    }
    else
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TECH     ||
         type == OBJECT_TOTO     ||
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_APOLLO2  )
    {
        object = new CObject();
        object->SetOption(option);
        object->CreateVehicle(pos, angle, type, power, trainer, toy);
    }

    return object;
}

bool CObjectManager::DestroyObject(int id)
{
    CObject* obj = GetObjectById(id);
    if(obj == nullptr) return false;
    delete obj; // Destructor calls CObjectManager::DeleteObject
    return true;
}

CObject* CObjectManager::Radar(CObject* pThis, ObjectType type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    std::vector<ObjectType> types;
    if(type != OBJECT_NULL)
        types.push_back(type);
    return Radar(pThis, types, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
}

CObject* CObjectManager::Radar(CObject* pThis, std::vector<ObjectType> type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    Math::Vector iPos;
    float iAngle;
    if(pThis != nullptr)
    {
        iPos   = pThis->GetPosition(0);
        iAngle = pThis->GetAngleY(0);
        iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI
    } else {
        iPos   = Math::Vector();
        iAngle = 0.0f;
    }
    return Radar(pThis, iPos, iAngle, type, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
}

CObject* CObjectManager::Radar(CObject* pThis, Math::Vector thisPosition, float thisAngle, ObjectType type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    std::vector<ObjectType> types;
    if(type != OBJECT_NULL)
        types.push_back(type);
    return Radar(pThis, thisPosition, thisAngle, types, angle, focus, minDist, maxDist, furthest, filter, cbotTypes);
}

CObject* CObjectManager::Radar(CObject* pThis, Math::Vector thisPosition, float thisAngle, std::vector<ObjectType> type, float angle, float focus, float minDist, float maxDist, bool furthest, RadarFilter filter, bool cbotTypes)
{
    CObject     *pObj, *pBest;
    CPhysics*   physics;
    Math::Vector    iPos, oPos;
    float       best, iAngle, d, a;
    ObjectType  oType;
    
    minDist *= g_unit;
    maxDist *= g_unit;
    
    iPos   = thisPosition;
    iAngle = thisAngle+angle;
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI
    
    if ( !furthest )  best = 100000.0f;
    else              best = 0.0f;
    pBest = nullptr;
    for ( auto it = m_table.begin() ; it != m_table.end() ; ++it )
    {
        pObj = it->second;
        if ( pObj == pThis )  continue; // pThis may be nullptr but it doesn't matter
        
        if ( pObj->GetTruck() != 0 )  continue;  // object transported?
        if ( !pObj->GetActif() )  continue;
        if ( pObj->GetProxyActivate() )  continue;
        
        oType = pObj->GetType();
        if ( oType == OBJECT_TOTO || oType == OBJECT_CONTROLLER )  continue;
        
        if(cbotTypes) {
            // TODO: handle this differently (new class describing types? CObjectType::GetBaseType()?)
            if ( oType == OBJECT_RUINmobilew2 ||
                oType == OBJECT_RUINmobilet1 ||
                oType == OBJECT_RUINmobilet2 ||
                oType == OBJECT_RUINmobiler1 ||
                oType == OBJECT_RUINmobiler2 )
            {
                oType = OBJECT_RUINmobilew1;  // any ruin
            }
            
            if ( oType == OBJECT_SCRAP2 ||
                oType == OBJECT_SCRAP3 ||
                oType == OBJECT_SCRAP4 ||
                oType == OBJECT_SCRAP5 )  // wastes?
            {
                oType = OBJECT_SCRAP1;  // any waste
            }
            
            if ( oType == OBJECT_BARRIER2 ||
                oType == OBJECT_BARRIER3 )  // barriers?
            {
                oType = OBJECT_BARRIER1;  // any barrier
            }
            // END OF TODO
        }
        
        if ( filter == FILTER_ONLYLANDING )
        {
            physics = pObj->GetPhysics();
            if ( physics != nullptr && !physics->GetLand() )  continue;
        }
        if ( filter == FILTER_ONLYFLYING )
        {
            physics = pObj->GetPhysics();
            if ( physics != nullptr && physics->GetLand() )  continue;
        }
        
        if ( std::find(type.begin(), type.end(), oType) == type.end() && type.size() > 0 )  continue;
        
        oPos = pObj->GetPosition(0);
        d = Math::DistanceProjected(iPos, oPos);
        if ( d < minDist || d > maxDist )  continue;  // too close or too far?
        
        a = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
        if ( Math::TestAngle(a, iAngle-focus/2.0f, iAngle+focus/2.0f) || focus >= Math::PI*2.0f )
        {
            if ( (!furthest && d < best) ||
                (furthest && d > best) )
            {
                best = d;
                pBest = pObj;
            }
        }
    }
    
    return pBest;
}

CObject*  CObjectManager::FindNearest(CObject* pThis, ObjectType type, float maxDist, bool cbotTypes)
{
    return Radar(pThis, type, 0.0f, Math::PI*2.0f, 0.0f, maxDist, false, FILTER_NONE, cbotTypes);
}

CObject*  CObjectManager::FindNearest(CObject* pThis, std::vector<ObjectType> type, float maxDist, bool cbotTypes)
{
    return Radar(pThis, type, 0.0f, Math::PI*2.0f, 0.0f, maxDist, false, FILTER_NONE, cbotTypes);
}

CObject*  CObjectManager::FindNearest(CObject* pThis, Math::Vector thisPosition, ObjectType type, float maxDist, bool cbotTypes)
{
    return Radar(pThis, thisPosition, 0.0f, type, 0.0f, Math::PI*2.0f, 0.0f, maxDist, false, FILTER_NONE, cbotTypes);
}

CObject*  CObjectManager::FindNearest(CObject* pThis, Math::Vector thisPosition, std::vector<ObjectType> type, float maxDist, bool cbotTypes)
{
    return Radar(pThis, thisPosition, 0.0f, type, 0.0f, Math::PI*2.0f, 0.0f, maxDist, false, FILTER_NONE, cbotTypes);
}