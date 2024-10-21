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

#include "object/object.h"

#include "common/global.h"
#include "common/restext.h"
#include "common/stringutils.h"

#include "graphics/model/model_crash_sphere.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/const.h"

#include "script/scriptfunc.h"

#include <stdexcept>


CObject::CObject(int id, ObjectType type)
    : m_id(id)
    , m_type(type)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_rotation(0.0f, 0.0f, 0.0f)
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_animateOnReset(false)
    , m_collisions(true)
    , m_team(0)
    , m_proxyActivate(false)
    , m_proxyDistance(60.0f)
    , m_lock(false)
    , m_persistent(true)
{
    m_implementedInterfaces.fill(false);
    m_botVar = CScriptFunctions::CreateObjectVar(this);
}

CObject::~CObject()
{
    CScriptFunctions::DestroyObjectVar(m_botVar, true);
}

ObjectCreateParams CObject::ReadCreateParams(CLevelParserLine* line)
{
    ObjectCreateParams params;
    params.pos = line->GetParam("pos")->AsPoint()*g_unit;
    params.angle = line->GetParam("dir")->AsFloat(0.0f)*Math::PI;
    params.type = line->GetParam("type")->AsObjectType();
    params.power = line->GetParam("power")->AsFloat(1.0f);
    params.height = line->GetParam("h")->AsFloat(0.0f);
    params.trainer = line->GetParam("trainer")->AsBool(false);
    params.toy = line->GetParam("toy")->AsBool(false); // TODO: Remove
    params.option = line->GetParam("option")->AsInt(0);
    params.team = line->GetParam("team")->AsInt(0);
    //params.id = line->GetParam("id")->AsInt(-1);
    return params;
}

void CObject::SetCrashSpheres(const std::vector<Gfx::ModelCrashSphere>& crashSpheres)
{
    for (const auto& crashSphere : crashSpheres)
    {
        SoundType sound = ParseSoundType(crashSphere.sound);
        CrashSphere objectCrashSphere(crashSphere.position, crashSphere.radius, sound, crashSphere.hardness);
        AddCrashSphere(objectCrashSphere);
    }
}

void CObject::AddCrashSphere(const CrashSphere& crashSphere)
{
    m_crashSpheres.push_back(crashSphere);
}

CrashSphere CObject::GetFirstCrashSphere()
{
    assert(m_crashSpheres.size() >= 1);

    CrashSphere transformedFirstCrashSphere = m_crashSpheres[0];
    TransformCrashSphere(transformedFirstCrashSphere.sphere);
    return transformedFirstCrashSphere;
}

std::vector<CrashSphere> CObject::GetAllCrashSpheres()
{
    std::vector<CrashSphere> allCrashSpheres;

    for (const auto& crashSphere : m_crashSpheres)
    {
        CrashSphere transformedCrashSphere = crashSphere;
        TransformCrashSphere(transformedCrashSphere.sphere);
        allCrashSpheres.push_back(transformedCrashSphere);
    }

    return allCrashSpheres;
}

bool CObject::CanCollideWith(CObject* other)
{
    ObjectType otherType = other->GetType();
    if (m_type == OBJECT_WORM) return false;
    if (otherType == OBJECT_WORM) return false;
    if (m_type == OBJECT_MOTHER)
    {
        if (otherType == OBJECT_ANT) return false;
        if (otherType == OBJECT_SPIDER) return false;
        if (otherType == OBJECT_EGG) return false;
    }
    if (otherType == OBJECT_MOTHER)
    {
        if (m_type == OBJECT_ANT) return false;
        if (m_type == OBJECT_SPIDER) return false;
        if (m_type == OBJECT_EGG) return false;
    }
    if ( m_type == OBJECT_MOTHER ||
         m_type == OBJECT_ANT    ||
         m_type == OBJECT_SPIDER ||
         m_type == OBJECT_WORM   ||
         m_type == OBJECT_BEE    )
    {
        if (other->Implements(ObjectInterfaceType::Transportable)) return false;
        if (otherType >= OBJECT_PLANT0 && otherType <= OBJECT_PLANT19) return false;
        if (otherType >= OBJECT_MUSHROOM1 && otherType <= OBJECT_MUSHROOM2) return false;
    }
    return true;
}

glm::vec3 CObject::GetPosition() const
{
    return m_position;
}

void CObject::SetPosition(const glm::vec3& pos)
{
    // TODO: provide default implementation...
    throw std::logic_error("CObject::SetPosition() - not implemented!");
}

glm::vec3 CObject::GetRotation() const
{
    return m_rotation;
}

void CObject::SetRotation(const glm::vec3& rotation)
{
    // TODO: provide default implementation...
    throw std::logic_error("CObject::SetRotation() - not implemented!");
}

void CObject::SetRotationX(float angle)
{
    glm::vec3 rotation = GetRotation();
    rotation.x = angle;
    return SetRotation(rotation);
}

void CObject::SetRotationY(float angle)
{
    glm::vec3 rotation = GetRotation();
    rotation.y = angle;
    return SetRotation(rotation);
}

void CObject::SetRotationZ(float angle)
{
    glm::vec3 rotation = GetRotation();
    rotation.z = angle;
    return SetRotation(rotation);
}

float CObject::GetRotationX()
{
    return GetRotation().x;
}

float CObject::GetRotationY()
{
    return GetRotation().y;
}

float CObject::GetRotationZ()
{
    return GetRotation().z;
}

glm::vec3 CObject::GetScale() const
{
    return m_scale;
}

void CObject::SetScale(const glm::vec3& scale)
{
    // TODO: provide default implementation...
    throw std::logic_error("CObject::SetScale() - not implemented!");
}

void CObject::SetScale(float scale)
{
    SetScale(glm::vec3(scale, scale, scale));
}

void CObject::SetScaleOverride(std::optional<glm::vec3> scale)
{
    m_scaleOverride = scale;
}

glm::vec3 CObject::GetScaleForSave() const
{
    return m_scaleOverride.value_or(GetScale());
}

void CObject::SetScaleX(float angle)
{
    glm::vec3 scale = GetScale();
    scale.x = angle;
    return SetScale(scale);
}

void CObject::SetScaleY(float angle)
{
    glm::vec3 scale = GetScale();
    scale.y = angle;
    return SetScale(scale);
}

void CObject::SetScaleZ(float angle)
{
    glm::vec3 scale = GetScale();
    scale.z = angle;
    return SetScale(scale);
}

float CObject::GetScaleX()
{
    return GetScale().x;
}

float CObject::GetScaleY()
{
    return GetScale().y;
}

float CObject::GetScaleZ()
{
    return GetScale().z;
}

int CObject::GetCrashSphereCount()
{
    return m_crashSpheres.size();
}

void CObject::DeleteAllCrashSpheres()
{
    m_crashSpheres.clear();
}

void CObject::SetCameraCollisionSphere(const Math::Sphere& sphere)
{
    m_cameraCollisionSphere = sphere;
}

Math::Sphere CObject::GetCameraCollisionSphere()
{
    Math::Sphere transformedSphere = m_cameraCollisionSphere;
    TransformCrashSphere(transformedSphere);
    return transformedSphere;
}

bool CObject::GetAnimateOnReset()
{
    return m_animateOnReset;
}

void CObject::SetAnimateOnReset(bool animateOnReset)
{
    m_animateOnReset = animateOnReset;
}

void CObject::SetCollisions(bool collisions)
{
    m_collisions = collisions;
}

bool CObject::GetCollisions()
{
    return m_collisions;
}

void CObject::SetTeam(int team)
{
    m_team = team;
}

int CObject::GetTeam()
{
    return m_team;
}

void CObject::SetProxyActivate(bool activate)
{
    m_proxyActivate = activate;
}

bool CObject::GetProxyActivate()
{
    return m_proxyActivate;
}

void CObject::SetProxyDistance(float distance)
{
    m_proxyDistance = distance;
}

float CObject::GetProxyDistance()
{
    return m_proxyDistance;
}

CBot::CBotVar* CObject::GetBotVar()
{
    return m_botVar;
}

std::string CObject::GetTooltipText()
{
    std::string name;
    GetResource(RES_OBJECT, m_type, name);
    if (GetTeam() != 0)
    {
        name += " ["+CRobotMain::GetInstancePointer()->GetTeamName(GetTeam())+" ("+StrUtils::ToString<int>(GetTeam())+")]";
    }
    return name;
}

// Management of the mode "blocked" of an object.
// For example, a cube of titanium is blocked while it is used to make something,
// or a vehicle is blocked as its construction is not finished.

void CObject::SetLock(bool lock)
{
    m_lock = lock;
}

bool CObject::GetLock() const
{
    return m_lock;
}

void CObject::SetLockOverride(std::optional<bool> lock)
{
    m_lockOverride = lock;
}

bool CObject::GetLockForSave() const
{
    return m_lockOverride.value_or(GetLock());
}

void CObject::SetPersistent(bool persistent)
{
    m_persistent = persistent;
}

bool CObject::GetPersistent() const
{
    return m_persistent;
}
