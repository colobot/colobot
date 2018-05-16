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

#include "object/object.h"

#include "common/global.h"
#include "common/restext.h"
#include "common/stringutils.h"

#include "graphics/model/model_crash_sphere.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"

#include "script/scriptfunc.h"

#include <stdexcept>


CObject::CObject(int id, ObjectType type)
    : m_id(id)
    , m_type(type)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_rotation(0.0f, 0.0f, 0.0f)
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_crashLineHeight(NAN)
    , m_crashLineMin(Math::Point( 10000.0f,  10000.0f))
    , m_crashLineMax(Math::Point(-10000.0f, -10000.0f))
    , m_animateOnReset(false)
    , m_collisions(true)
    , m_team(0)
    , m_proxyActivate(false)
    , m_proxyDistance(60.0f)
    , m_lock(false)
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

// Déplace une sphère.

void CObject::MoveCrashSphere(int rank, Math::Sphere sphere)
{
    m_crashSpheres[rank].sphere = sphere;
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

Math::Vector CObject::GetPosition() const
{
    return m_position;
}

void CObject::SetPosition(const Math::Vector& pos)
{
    // TODO: provide default implementation...
    throw std::logic_error("CObject::SetPosition() - not implemented!");
}

Math::Vector CObject::GetRotation() const
{
    return m_rotation;
}

void CObject::SetRotation(const Math::Vector& rotation)
{
    // TODO: provide default implementation...
    throw std::logic_error("CObject::SetRotation() - not implemented!");
}

void CObject::SetRotationX(float angle)
{
    Math::Vector rotation = GetRotation();
    rotation.x = angle;
    return SetRotation(rotation);
}

void CObject::SetRotationY(float angle)
{
    Math::Vector rotation = GetRotation();
    rotation.y = angle;
    return SetRotation(rotation);
}

void CObject::SetRotationZ(float angle)
{
    Math::Vector rotation = GetRotation();
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

Math::Vector CObject::GetScale() const
{
    return m_scale;
}

void CObject::SetScale(const Math::Vector& scale)
{
    // TODO: provide default implementation...
    throw std::logic_error("CObject::SetScale() - not implemented!");
}

void CObject::SetScale(float scale)
{
    SetScale(Math::Vector(scale, scale, scale));
}

void CObject::SetScaleX(float angle)
{
    Math::Vector scale = GetScale();
    scale.x = angle;
    return SetScale(scale);
}

void CObject::SetScaleY(float angle)
{
    Math::Vector scale = GetScale();
    scale.y = angle;
    return SetScale(scale);
}

void CObject::SetScaleZ(float angle)
{
    Math::Vector scale = GetScale();
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

bool CObject::GetLock()
{
    return m_lock;
}

// Ajoute une nouvelle ligne.

void CObject::AddCrashLine(const CrashLine &crashLine)
{
    m_crashLines.push_back(crashLine);
}

// Spécifie la hauteur de l'objet (ŕ partir du sol).

void CObject::SetCrashLineHeight(float h)
{
    m_crashLineHeight = h;
}

// Retourne la hauteur de l'objet.

float CObject::GetCrashLineHeight()
{
    return m_crashLineHeight;
}

// Retourne le nombre de lignes.

int CObject::GetCrashLineCount()
{
    return m_crashLines.size();
}

// Retourne une ligne pour les collisions.
// La position est absolue dans le monde.

std::vector<CrashLine> CObject::GetAllCrashLines()
{
    std::vector<CrashLine> allCrashLines;

    for (const auto& crashLine : m_crashLines)
    {
        CrashLine transformedCrashLine = crashLine;

        transformedCrashLine.pos = Math::RotatePoint(-GetRotationY(), transformedCrashLine.pos);
        transformedCrashLine.pos.x += GetPosition().x;
        transformedCrashLine.pos.y += GetPosition().z;

        allCrashLines.push_back(transformedCrashLine);
    }

    return allCrashLines;
}

// Supprime toutes les lignes utilisées pour les collisions.

void CObject::DeleteAllCrashLines()
{
    m_crashLines.clear();
    m_crashLineMin = Math::Point( 10000.0f,  10000.0f);
    m_crashLineMax = Math::Point(-10000.0f, -10000.0f);
    m_crashLineHeight = NAN;
}

// Met ŕ jour la bbox.

void CObject::UpdateBBoxCrashLine()
{
    m_crashLineMin = Math::Point( 10000.0f,  10000.0f);
    m_crashLineMax = Math::Point(-10000.0f, -10000.0f);

    for (const auto& crashLine : GetAllCrashLines())
    {
        if ( crashLine.pos.x < m_crashLineMin.x )  m_crashLineMin.x = crashLine.pos.x;
        if ( crashLine.pos.y < m_crashLineMin.y )  m_crashLineMin.y = crashLine.pos.y;
        if ( crashLine.pos.x > m_crashLineMax.x )  m_crashLineMax.x = crashLine.pos.x;
        if ( crashLine.pos.y > m_crashLineMax.y )  m_crashLineMax.y = crashLine.pos.y;
    }
}

void CObject::GetCrashLineBBox(Math::Point &min, Math::Point &max)
{
    // TODO (krzys_h): There is something wrong with UpdateBBoxCrashLine not being called for objects with IsCrashLineFusion()=false. I have no idea where it's supposed to be called from so I'm just throwing it here for now, but that should be fixed for performance reasons
    UpdateBBoxCrashLine();

    min = m_crashLineMin;
    max = m_crashLineMax;
}

// Indique s'il s'agit d'un objet fusionnable.
// L'objet doit obligatoirement ętre indestructible !

bool CObject::IsCrashLineFusion()
{
    return false;
}

// Essaye de fusionner les lignes de collision de l'objet courant
// avec un autre. Ainsi, plusieurs barričres mises bout ŕ bout ne
// formeront plus qu'un obstacle, regroupé dans la premičre barričre
// crée. L'objet courant n'a alors plus aucune ligne de collision.

void CObject::CrashLineFusion()
{
    if ( !IsCrashLineFusion() )  return;
    if ( GetCrashLineCount() != 5 )  return;

    float iLen = Math::Distance(m_crashLines[0].pos, m_crashLines[1].pos);
    float iWidth = Math::Distance(m_crashLines[1].pos, m_crashLines[2].pos);

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( !pObj->IsCrashLineFusion() )  continue;
        if ( pObj->GetCrashLineCount() != 5 )  continue;
        if ( pObj->GetRotationY() != GetRotationY() )  continue;

        //float oLen = Math::Distance(pObj->m_crashLines[0].pos, pObj->m_crashLines[1].pos);
        float oWidth = Math::Distance(pObj->m_crashLines[1].pos, pObj->m_crashLines[2].pos);

        if ( iWidth != oWidth )  continue;

        auto iPos = GetAllCrashLines();
        auto oPos = pObj->GetAllCrashLines();

        if ( fabs(iPos[1].pos.x-oPos[0].pos.x) < 5.0f &&
             fabs(iPos[1].pos.y-oPos[0].pos.y) < 5.0f &&
             fabs(iPos[2].pos.x-oPos[3].pos.x) < 5.0f &&
             fabs(iPos[2].pos.y-oPos[3].pos.y) < 5.0f )
        {
            pObj->m_crashLines[0].pos.x -= iLen;
            pObj->m_crashLines[3].pos.x -= iLen;
            pObj->m_crashLines[4].pos.x -= iLen;
            pObj->UpdateBBoxCrashLine();
            if ( !std::isnan(pObj->GetCrashLineHeight()) )
            {
                Math::Vector iCenter = GetPosition();
                Math::Vector oCenter = pObj->GetPosition();
                float h = iCenter.y-oCenter.y;
                if (h < 0.0f)  h = 0.0f;
                pObj->SetCrashLineHeight(pObj->GetCrashLineHeight()+h);
            }
            DeleteAllCrashLines();
            return;
        }

        if ( fabs(iPos[0].pos.x-oPos[1].pos.x) < 5.0f &&
             fabs(iPos[0].pos.y-oPos[1].pos.y) < 5.0f &&
             fabs(iPos[3].pos.x-oPos[2].pos.x) < 5.0f &&
             fabs(iPos[3].pos.y-oPos[2].pos.y) < 5.0f )
        {
            pObj->m_crashLines[1].pos.x += iLen;
            pObj->m_crashLines[2].pos.x += iLen;
            pObj->UpdateBBoxCrashLine();
            if ( !std::isnan(pObj->GetCrashLineHeight()) )
            {
                Math::Vector iCenter = GetPosition();
                Math::Vector oCenter = pObj->GetPosition();
                float h = iCenter.y-oCenter.y;
                if (h < 0.0f)  h = 0.0f;
                pObj->SetCrashLineHeight(pObj->GetCrashLineHeight()+h);
            }
            DeleteAllCrashLines();
            return;
        }
    }
}

