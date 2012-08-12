// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

// camera.cpp

#include "graphics/engine/camera.h"

#include "common/iman.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"
#include "math/const.h"
#include "math/geometry.h"
#include "object/object.h"
#include "physics/physics.h"


// TODO temporary stubs for CObject and CPhysics

void CObject::SetTransparency(float)
{
}

CObject* CObject::GetFret()
{
    return nullptr;
}

CObject* CObject::GetPower()
{
    return nullptr;
}

CObject* CObject::GetTruck()
{
    return nullptr;
}

ObjectType CObject::GetType()
{
    return OBJECT_NULL;
}

void CObject::SetGunGoalH(float)
{
}

void CObject::GetGlobalSphere(Math::Vector &pos, float &radius)
{
}

float CObject::GetAngleY(int)
{
    return 0.0f;
}

Math::Vector CObject::GetPosition(int)
{
    return Math::Vector();
}

void CObject::SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                              Math::Vector &lookat, Math::Vector &upVec,
                              Gfx::CameraType type)
{
}

CPhysics* CObject::GetPhysics()
{
    return nullptr;
}

bool CPhysics::GetLand()
{
    return false;
}

//! Changes the level of transparency of an object and objects transported (battery & cargo)
void SetTransparency(CObject* obj, float value)
{
    obj->SetTransparency(value);

    CObject *fret = obj->GetFret();
    if (fret != NULL)
        fret->SetTransparency(value);

    fret = obj->GetPower();
    if (fret != NULL)
        fret->SetTransparency(value);
}



Gfx::CCamera::CCamera(CInstanceManager* iMan)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_CAMERA, this);

    m_engine  = static_cast<Gfx::CEngine*> ( m_iMan->SearchInstance(CLASS_ENGINE) );
    m_terrain = static_cast<Gfx::CTerrain*>( m_iMan->SearchInstance(CLASS_TERRAIN) );
    m_water   = static_cast<Gfx::CWater*>  ( m_iMan->SearchInstance(CLASS_WATER) );

    m_type      = Gfx::CAM_TYPE_FREE;
    m_smooth    = Gfx::CAM_SMOOTH_NORM;
    m_cameraObj = 0;

    m_eyeDistance = 10.0f;
    m_initDelay   =  0.0f;

    m_actualEye    = Math::Vector(0.0f, 0.0f, 0.0f);
    m_actualLookat = Math::Vector(0.0f, 0.0f, 0.0f);
    m_finalEye     = Math::Vector(0.0f, 0.0f, 0.0f);
    m_finalLookat  = Math::Vector(0.0f, 0.0f, 0.0f);
    m_normEye      = Math::Vector(0.0f, 0.0f, 0.0f);
    m_normLookat   = Math::Vector(0.0f, 0.0f, 0.0f);
    m_focus        = 1.0f;

    m_rightDown      = false;
    m_rightPosInit   = Math::Point(0.5f, 0.5f);
    m_rightPosCenter = Math::Point(0.5f, 0.5f);
    m_rightPosMove   = Math::Point(0.5f, 0.5f);

    m_eyePt        = Math::Vector(0.0f, 0.0f, 0.0f);
    m_directionH   =  0.0f;
    m_directionV   =  0.0f;
    m_heightEye    = 20.0f;
    m_heightLookat =  0.0f;
    m_speed        =  2.0f;

    m_backDist      = 0.0f;
    m_backMin       = 0.0f;
    m_addDirectionH = 0.0f;
    m_addDirectionV = 0.0f;
    m_transparency  = false;

    m_fixDist       = 0.0f;
    m_fixDirectionH = 0.0f;
    m_fixDirectionV = 0.0f;

    m_visitGoal       = Math::Vector(0.0f, 0.0f, 0.0f);
    m_visitDist       = 0.0f;
    m_visitTime       = 0.0f;
    m_visitType       = Gfx::CAM_TYPE_NULL;
    m_visitDirectionH = 0.0f;
    m_visitDirectionV = 0.0f;

    m_editHeight = 40.0f;

    m_remotePan  = 0.0f;
    m_remoteZoom = 0.0f;

    m_mouseDirH    = 0.0f;
    m_mouseDirV    = 0.0f;
    m_mouseMarging = 0.01f;

    m_motorTurn = 0.0f;

    m_centeringPhase    = Gfx::CAM_PHASE_NULL;
    m_centeringAngleH   = 0.0f;
    m_centeringAngleV   = 0.0f;
    m_centeringDist     = 0.0f;
    m_centeringCurrentH = 0.0f;
    m_centeringCurrentV = 0.0f;
    m_centeringTime     = 0.0f;
    m_centeringProgress = 0.0f;

    m_effectType     = Gfx::CAM_EFFECT_NULL;
    m_effectPos      = Math::Vector(0.0f, 0.0f, 0.0f);
    m_effectForce    = 0.0f;
    m_effectProgress = 0.0f;
    m_effectOffset   = Math::Vector(0.0f, 0.0f, 0.0f);

    m_scriptEye    = Math::Vector(0.0f, 0.0f, 0.0f);
    m_scriptLookat = Math::Vector(0.0f, 0.0f, 0.0f);

    m_effect        = true;
    m_cameraScroll  = true;
    m_cameraInvertX = false;
    m_cameraInvertY = false;
}

Gfx::CCamera::~CCamera()
{
}

void Gfx::CCamera::SetEffect(bool enable)
{
    m_effect = enable;
}

void Gfx::CCamera::SetCameraScroll(bool scroll)
{
    m_cameraScroll = scroll;
}

void Gfx::CCamera::SetCameraInvertX(bool invert)
{
    m_cameraInvertX = invert;
}

void Gfx::CCamera::SetCameraInvertY(bool invert)
{
    m_cameraInvertY = invert;
}

float Gfx::CCamera::GetMotorTurn()
{
    if (m_type == Gfx::CAM_TYPE_BACK)
        return m_motorTurn;
    return 0.0f;
}

void Gfx::CCamera::Init(Math::Vector eye, Math::Vector lookat, float delay)
{
    m_initDelay = delay;

    eye.y    += m_terrain->GetFloorLevel(eye,    true);
    lookat.y += m_terrain->GetFloorLevel(lookat, true);

    m_type = Gfx::CAM_TYPE_FREE;
    m_eyePt = eye;

    m_directionH = Math::RotateAngle(eye.x - lookat.x, eye.z - lookat.z) + Math::PI / 2.0f;
    m_directionV = -Math::RotateAngle(Math::DistanceProjected(eye, lookat), eye.y - lookat.y);

    m_eyeDistance = 10.0f;
    m_heightLookat = 10.0f;
    m_backDist = 30.0f;
    m_backMin  = 10.0f;
    m_addDirectionH = 0.0f;
    m_addDirectionV = -Math::PI*0.05f;
    m_fixDist = 50.0f;
    m_fixDirectionH = Math::PI*0.25f;
    m_fixDirectionV = -Math::PI*0.10f;
    m_centeringPhase = Gfx::CAM_PHASE_NULL;
    m_actualEye = m_eyePt;
    m_actualLookat = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);
    m_finalEye = m_actualEye;
    m_finalLookat = m_actualLookat;
    m_scriptEye = m_actualEye;
    m_scriptLookat = m_actualLookat;
    m_focus = 1.00f;
    m_remotePan  = 0.0f;
    m_remoteZoom = 0.0f;

    FlushEffect();
    FlushOver();
    SetType(Gfx::CAM_TYPE_FREE);
}


void Gfx::CCamera::SetObject(CObject* object)
{
    m_cameraObj = object;
}

CObject* Gfx::CCamera::GetObject()
{
    return m_cameraObj;
}

void Gfx::CCamera::SetType(CameraType type)
{
    m_remotePan  = 0.0f;
    m_remoteZoom = 0.0f;

    if ( (m_type == Gfx::CAM_TYPE_BACK) && m_transparency )
    {
        for (int i = 0; i < 1000000; i++)
        {
            CObject* obj = static_cast<CObject*>( m_iMan->SearchInstance(CLASS_OBJECT, i) );
            if (obj == NULL)
                break;

            if (obj->GetTruck())
                continue;  // battery or cargo?

            SetTransparency(obj, 0.0f);  // opaque object
        }
    }
    m_transparency = false;

    if (type == Gfx::CAM_TYPE_INFO  ||
        type == Gfx::CAM_TYPE_VISIT)  // xx -> info ?
    {
        m_normEye    = m_engine->GetEyePt();
        m_normLookat = m_engine->GetLookatPt();

        m_engine->SetFocus(1.00f);  // normal
        m_type = type;
        return;
    }

    if (m_type == Gfx::CAM_TYPE_INFO  ||
        m_type == Gfx::CAM_TYPE_VISIT)  // info -> xx ?
    {
        m_engine->SetFocus(m_focus);  // gives initial focus
        m_type = type;

        Math::Vector upVec = Math::Vector(0.0f, 1.0f, 0.0f);
        SetViewParams(m_normEye, m_normLookat, upVec);
        return;
    }

    if ( m_type == Gfx::CAM_TYPE_BACK && type == Gfx::CAM_TYPE_FREE )  // back -> free ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -50.0f);

    if ( m_type == Gfx::CAM_TYPE_BACK && type == Gfx::CAM_TYPE_EDIT )  // back -> edit ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -1.0f);

    if ( m_type == Gfx::CAM_TYPE_ONBOARD && type == Gfx::CAM_TYPE_FREE )  // onboard -> free ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);

    if ( m_type == Gfx::CAM_TYPE_ONBOARD && type == Gfx::CAM_TYPE_EDIT )  // onboard -> edit ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);

    if ( m_type == Gfx::CAM_TYPE_ONBOARD && type == Gfx::CAM_TYPE_EXPLO )  // onboard -> explo ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -50.0f);

    if ( m_type == Gfx::CAM_TYPE_BACK && type == Gfx::CAM_TYPE_EXPLO )  // back -> explo ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -20.0f);

    if ( type == Gfx::CAM_TYPE_FIX   ||
         type == Gfx::CAM_TYPE_PLANE )
        AbortCentering();  // Special stops framing

    m_fixDist = 50.0f;
    if ( type == Gfx::CAM_TYPE_PLANE )
        m_fixDist = 60.0f;

    if ( type == Gfx::CAM_TYPE_BACK )
    {
        AbortCentering();  // Special stops framing
        m_addDirectionH = 0.0f;
        m_addDirectionV = -Math::PI*0.05f;

        ObjectType oType;
        if ( m_cameraObj == 0 )  oType = OBJECT_NULL;
        else                     oType = m_cameraObj->GetType();

        m_backDist = 30.0f;
        if ( oType == OBJECT_BASE     )  m_backDist = 200.0f;
        if ( oType == OBJECT_HUMAN    )  m_backDist =  20.0f;
        if ( oType == OBJECT_TECH     )  m_backDist =  20.0f;
        if ( oType == OBJECT_FACTORY  )  m_backDist =  50.0f;
        if ( oType == OBJECT_RESEARCH )  m_backDist =  40.0f;
        if ( oType == OBJECT_DERRICK  )  m_backDist =  40.0f;
        if ( oType == OBJECT_REPAIR   )  m_backDist =  35.0f;
        if ( oType == OBJECT_DESTROYER)  m_backDist =  35.0f;
        if ( oType == OBJECT_TOWER    )  m_backDist =  45.0f;
        if ( oType == OBJECT_NUCLEAR  )  m_backDist =  70.0f;
        if ( oType == OBJECT_PARA     )  m_backDist = 180.0f;
        if ( oType == OBJECT_SAFE     )  m_backDist =  50.0f;
        if ( oType == OBJECT_HUSTON   )  m_backDist = 120.0f;

        m_backMin = m_backDist/3.0f;
        if ( oType == OBJECT_HUMAN    )  m_backMin =  10.0f;
        if ( oType == OBJECT_TECH     )  m_backMin =  10.0f;
        if ( oType == OBJECT_FACTORY  )  m_backMin =  30.0f;
        if ( oType == OBJECT_RESEARCH )  m_backMin =  20.0f;
        if ( oType == OBJECT_NUCLEAR  )  m_backMin =  32.0f;
        if ( oType == OBJECT_PARA     )  m_backMin =  40.0f;
        if ( oType == OBJECT_SAFE     )  m_backMin =  25.0f;
        if ( oType == OBJECT_HUSTON   )  m_backMin =  80.0f;
    }

    if ( type != Gfx::CAM_TYPE_ONBOARD && m_cameraObj != 0 )
        m_cameraObj->SetGunGoalH(0.0f);  // puts the cannon right

    if ( type == Gfx::CAM_TYPE_ONBOARD )
        m_focus = 1.50f;  // Wide
    else
        m_focus = 1.00f;  // normal
    m_engine->SetFocus(m_focus);

    m_type = type;

    SetSmooth(Gfx::CAM_SMOOTH_NORM);
}

Gfx::CameraType Gfx::CCamera::GetType()
{
    return m_type;
}

void Gfx::CCamera::SetSmooth(CameraSmooth type)
{
    m_smooth = type;
}

Gfx::CameraSmooth Gfx::CCamera::GetSmoth()
{
    return m_smooth;
}

void Gfx::CCamera::SetDist(float dist)
{
    m_fixDist = dist;
}

float Gfx::CCamera::GetDist()
{
    return m_fixDist;
}

void Gfx::CCamera::SetFixDirection(float angle)
{
    m_fixDirectionH = angle;
}

float Gfx::CCamera::GetFixDirection()
{
    return m_fixDirectionH;
}

void Gfx::CCamera::SetRemotePan(float value)
{
    m_remotePan = value;
}

float Gfx::CCamera::GetRemotePan()
{
    return m_remotePan;
}

void Gfx::CCamera::SetRemoteZoom(float value)
{
    value = Math::Norm(value);

    if ( m_type == Gfx::CAM_TYPE_BACK )
        m_backDist = m_backMin + (200.0f - m_backMin) * value;

    if ( m_type == Gfx::CAM_TYPE_FIX   ||
         m_type == Gfx::CAM_TYPE_PLANE )
        m_fixDist = 10.0f + (200.0f - 10.0f) * value;
}

float Gfx::CCamera::GetRemoteZoom()
{
    if ( m_type == Gfx::CAM_TYPE_BACK )
        return (m_backDist - m_backMin) / (200.0f - m_backMin);

    if ( m_type == Gfx::CAM_TYPE_FIX   ||
         m_type == Gfx::CAM_TYPE_PLANE )
        return (m_fixDist - 10.0f) / (200.0f - 10.0f);

    return 0.0f;
}

void Gfx::CCamera::StartVisit(Math::Vector goal, float dist)
{
    m_visitType = m_type;
    SetType(Gfx::CAM_TYPE_VISIT);
    m_visitGoal = goal;
    m_visitDist = dist;
    m_visitTime = 0.0f;
    m_visitDirectionH = 0.0f;
    m_visitDirectionV = -Math::PI*0.10f;
}

void Gfx::CCamera::StopVisit()
{
    SetType(m_visitType);  // presents the initial type
}

void Gfx::CCamera::GetCamera(Math::Vector &eye, Math::Vector &lookat)
{
    eye = m_eyePt;
    lookat = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);
}

bool Gfx::CCamera::StartCentering(CObject *object, float angleH, float angleV,
                             float dist, float time)
{
    if (m_type != Gfx::CAM_TYPE_BACK)
        return false;
    if (object != m_cameraObj)
        return false;

    if (m_centeringPhase != Gfx::CAM_PHASE_NULL)
        return false;

    if (m_addDirectionH > Math::PI)
        angleH = Math::PI * 2.0f - angleH;

    m_centeringPhase    = Gfx::CAM_PHASE_START;
    m_centeringAngleH   = angleH;
    m_centeringAngleV   = angleV;
    m_centeringDist     = dist;
    m_centeringCurrentH = 0.0f;
    m_centeringCurrentV = 0.0f;
    m_centeringTime     = time;
    m_centeringProgress = 0.0f;

    return true;
}

bool Gfx::CCamera::StopCentering(CObject *object, float time)
{
    if (m_type != Gfx::CAM_TYPE_BACK)
        return false;
    if (object != m_cameraObj)
        return false;

    if (m_centeringPhase != Gfx::CAM_PHASE_START &&
        m_centeringPhase != Gfx::CAM_PHASE_WAIT)
        return false;

    m_centeringPhase = Gfx::CAM_PHASE_STOP;

    if (m_centeringAngleH != 99.9f)
        m_centeringAngleH = m_centeringCurrentH;

    if (m_centeringAngleV != 99.9f)
        m_centeringAngleV = m_centeringCurrentV;

    m_centeringTime     = time;
    m_centeringProgress = 0.0f;

    return true;
}

void Gfx::CCamera::AbortCentering()
{
    if (m_type == Gfx::CAM_TYPE_INFO  ||
        m_type == Gfx::CAM_TYPE_VISIT )
        return;

    if (m_centeringPhase == Gfx::CAM_PHASE_NULL)
        return;

    m_centeringPhase = Gfx::CAM_PHASE_NULL;

    if ( m_centeringAngleH != 99.9f )
        m_addDirectionH = m_centeringCurrentH;

    if (m_centeringAngleV != 99.9f)
        m_addDirectionV = m_centeringCurrentV;
}

void Gfx::CCamera::FlushEffect()
{
    m_effectType     = Gfx::CAM_EFFECT_NULL;
    m_effectForce    = 0.0f;
    m_effectProgress = 0.0f;
    m_effectOffset   = Math::Vector(0.0f, 0.0f, 0.0f);
}

void Gfx::CCamera::StartEffect(CameraEffect effect, Math::Vector pos, float force)
{
    if ( !m_effect )  return;

    m_effectType     = effect;
    m_effectPos      = pos;
    m_effectForce    = force;
    m_effectProgress = 0.0f;
}

void Gfx::CCamera::EffectFrame(const Event &event)
{
    if (m_type == Gfx::CAM_TYPE_INFO  ||
        m_type == Gfx::CAM_TYPE_VISIT)
        return;

    if (m_effectType == Gfx::CAM_EFFECT_NULL)
        return;

    m_effectOffset = Math::Vector(0.0f, 0.0f, 0.0f);

    float force = m_effectForce;

    if ( m_effectType == Gfx::CAM_EFFECT_TERRAFORM )
    {
        m_effectProgress += event.rTime * 0.7f;
        m_effectOffset.x = (Math::Rand() - 0.5f) * 10.0f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 10.0f;
        m_effectOffset.z = (Math::Rand() - 0.5f) * 10.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == Gfx::CAM_EFFECT_EXPLO )
    {
        m_effectProgress += event.rTime * 1.0f;
        m_effectOffset.x = (Math::Rand() - 0.5f)  *5.0f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 5.0f;
        m_effectOffset.z = (Math::Rand() - 0.5f) * 5.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == Gfx::CAM_EFFECT_SHOT )
    {
        m_effectProgress += event.rTime * 1.0f;
        m_effectOffset.x = (Math::Rand() - 0.5f) * 2.0f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 2.0f;
        m_effectOffset.z = (Math::Rand() - 0.5f) * 2.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == Gfx::CAM_EFFECT_CRASH )
    {
        m_effectProgress += event.rTime * 5.0f;
        m_effectOffset.y = sinf(m_effectProgress * Math::PI) * 1.5f;
        m_effectOffset.x = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
        m_effectOffset.z = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
    }

    if ( m_effectType == Gfx::CAM_EFFECT_VIBRATION )
    {
        m_effectProgress += event.rTime * 0.1f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
        m_effectOffset.x = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
        m_effectOffset.z = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
    }

    if ( m_effectType == Gfx::CAM_EFFECT_PET )
    {
        m_effectProgress += event.rTime  *5.0f;
        m_effectOffset.x = (Math::Rand() - 0.5f) * 0.2f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 2.0f;
        m_effectOffset.z = (Math::Rand() - 0.5f) * 0.2f;
    }

    float dist = Math::Distance(m_eyePt, m_effectPos);
    dist = Math::Norm((dist - 100.f) / 100.0f);

    force *= 1.0f-dist;
#if _TEEN
    force *= 2.0f;
#endif
    m_effectOffset *= force;

    if (m_effectProgress >= 1.0f)
        FlushEffect();
}

void Gfx::CCamera::FlushOver()
{
    m_overType = Gfx::CAM_OVER_EFFECT_NULL;
    m_overColorBase = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);  // black
    m_engine->SetOverColor();  // nothing
}

void Gfx::CCamera::SetOverBaseColor(Gfx::Color color)
{
    m_overColorBase = color;
}

void Gfx::CCamera::StartOver(Gfx::CameraOverEffect effect, Math::Vector pos, float force)
{
    m_overType = effect;
    m_overTime = 0.0f;

    float decay;
    if (m_overType == Gfx::CAM_OVER_EFFECT_LIGHTNING)
        decay = 400.0f;
    else
        decay = 100.0f;

    float dist = Math::Distance(m_eyePt, pos);
    dist = (dist - decay) / decay;
    if (dist < 0.0f) dist = 0.0f;
    if (dist > 1.0f) dist = 1.0f;

    m_overForce = force * (1.0f - dist);

    if (m_overType == Gfx::CAM_OVER_EFFECT_BLOOD)
    {
        m_overColor   = Gfx::Color(0.8f, 0.1f, 0.1f); // red
        m_overMode    = Gfx::ENG_RSTATE_TCOLOR_BLACK;

        m_overFadeIn  = 0.4f;
        m_overFadeOut = 0.8f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == Gfx::CAM_OVER_EFFECT_FADEIN_WHITE )
    {
        m_overColor   = Gfx::Color(1.0f, 1.0f, 1.0f); // white
        m_overMode    = Gfx::ENG_RSTATE_TCOLOR_BLACK;

        m_overFadeIn  = 0.0f;
        m_overFadeOut = 20.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == Gfx::CAM_OVER_EFFECT_FADEOUT_WHITE )
    {
        m_overColor   = Gfx::Color(1.0f, 1.0f, 1.0f); // white
        m_overMode    = Gfx::ENG_RSTATE_TCOLOR_BLACK;

        m_overFadeIn  = 6.0f;
        m_overFadeOut = 100000.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == Gfx::CAM_OVER_EFFECT_FADEOUT_BLACK )
    {
        m_overColor   = m_engine->GetFogColor(1); // fog color underwater
        m_overMode    = Gfx::ENG_RSTATE_TTEXTURE_WHITE;

        m_overFadeIn  = 4.0f;
        m_overFadeOut = 100000.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == Gfx::CAM_OVER_EFFECT_LIGHTNING )
    {
        m_overColor   = Gfx::Color(0.9f, 1.0f, 1.0f);  // white-cyan
        m_overMode    = Gfx::ENG_RSTATE_TCOLOR_BLACK;

        m_overFadeIn  = 0.0f;
        m_overFadeOut = 1.0f;
    }
}

void Gfx::CCamera::OverFrame(const Event &event)
{
    if (m_type == Gfx::CAM_TYPE_INFO ||
        m_type == Gfx::CAM_TYPE_VISIT)
        return;

    if (m_overType == Gfx::CAM_OVER_EFFECT_NULL)
        return;

    m_overTime += event.rTime;

    if (m_overType == Gfx::CAM_OVER_EFFECT_LIGHTNING)
    {
        Gfx::Color color;
        if (rand() % 2 == 0)
        {
            color.r = m_overColor.r * m_overForce;
            color.g = m_overColor.g * m_overForce;
            color.b = m_overColor.b * m_overForce;
        }
        else
        {
            color = Gfx::Color(0.0f, 0.0f, 0.0f);
        }
        color.a = 0.0f;
        m_engine->SetOverColor(color, m_overMode);
    }
    else
    {
        if ( (m_overFadeIn > 0.0f) && (m_overTime < m_overFadeIn) )
        {
            float intensity = m_overTime / m_overFadeIn;
            intensity *= m_overForce;

            Gfx::Color color;
            if (m_overMode == Gfx::ENG_RSTATE_TCOLOR_WHITE)
            {
                color.r = 1.0f - (1.0f - m_overColor.r) * intensity;
                color.g = 1.0f - (1.0f - m_overColor.g) * intensity;
                color.b = 1.0f - (1.0f - m_overColor.b) * intensity;
            }
            else
            {
                color.r = m_overColor.r * intensity;
                color.g = m_overColor.g * intensity;
                color.b = m_overColor.b * intensity;

                color.r = 1.0f - (1.0f - color.r) * (1.0f - m_overColorBase.r);
                color.g = 1.0f - (1.0f - color.g) * (1.0f - m_overColorBase.g);
                color.b = 1.0f - (1.0f - color.b) * (1.0f - m_overColorBase.b);
            }
            color.a = 0.0f;
            m_engine->SetOverColor(color, m_overMode);
        }
        else if ( (m_overFadeOut > 0.0f) && (m_overTime - m_overFadeIn < m_overFadeOut) )
        {
            float intensity = 1.0f - (m_overTime - m_overFadeIn) / m_overFadeOut;
            intensity *= m_overForce;

            Gfx::Color color;
            if (m_overMode == Gfx::ENG_RSTATE_TCOLOR_WHITE)
            {
                color.r = 1.0f-(1.0f-m_overColor.r) * intensity;
                color.g = 1.0f-(1.0f-m_overColor.g) * intensity;
                color.b = 1.0f-(1.0f-m_overColor.b) * intensity;
            }
            else
            {
                color.r = m_overColor.r * intensity;
                color.g = m_overColor.g * intensity;
                color.b = m_overColor.b * intensity;

                color.r = 1.0f - (1.0f - color.r)*(1.0f - m_overColorBase.r);
                color.g = 1.0f - (1.0f - color.g)*(1.0f - m_overColorBase.g);
                color.b = 1.0f - (1.0f - color.b)*(1.0f - m_overColorBase.b);
            }
            color.a = 0.0f;
            m_engine->SetOverColor(color, m_overMode);
        }
    }

    if ( m_overTime >= m_overFadeIn+m_overFadeOut )
    {
        FlushOver();
        return;
    }
}

void Gfx::CCamera::FixCamera()
{
    m_initDelay = 0.0f;
    m_actualEye    = m_finalEye    = m_scriptEye;
    m_actualLookat = m_finalLookat = m_scriptLookat;
    SetViewTime(m_scriptEye, m_scriptLookat, 0.0f);
}

void Gfx::CCamera::SetViewTime(const Math::Vector &eyePt,
                               const Math::Vector &lookatPt,
                               float rTime)
{
    Math::Vector eye, lookat;

    if (m_type == Gfx::CAM_TYPE_INFO)
    {
        eye    = eyePt;
        lookat = lookatPt;
    }
    else
    {
        if (m_initDelay > 0.0f)
        {
            m_initDelay -= rTime;
            if (m_initDelay < 0.0f)
                m_initDelay = 0.0f;
            rTime /= 1.0f+m_initDelay;
        }

        eye    = eyePt;
        lookat = lookatPt;
        if ( !IsCollision(eye, lookat) )
        {
            m_finalEye    = eye;
            m_finalLookat = lookat;
        }

        float prog = 0.0f;
        float dist = Math::Distance(m_finalEye, m_actualEye);

        if (m_smooth == Gfx::CAM_SMOOTH_NONE) prog = dist;
        if (m_smooth == Gfx::CAM_SMOOTH_NORM) prog = powf(dist, 1.5f) * rTime * 0.5f;
        if (m_smooth == Gfx::CAM_SMOOTH_HARD) prog = powf(dist, 1.0f) * rTime * 4.0f;
        if (m_smooth == Gfx::CAM_SMOOTH_SPEC) prog = powf(dist, 1.0f) * rTime * 0.05f;
        if (dist == 0.0f)
        {
            m_actualEye = m_finalEye;
        }
        else
        {
            if (prog > dist)
                prog = dist;
            m_actualEye = (m_finalEye - m_actualEye) / dist * prog + m_actualEye;
        }

        dist = Math::Distance(m_finalLookat, m_actualLookat);
        if ( m_smooth == Gfx::CAM_SMOOTH_NONE ) prog = dist;
        if ( m_smooth == Gfx::CAM_SMOOTH_NORM ) prog = powf(dist, 1.5f) * rTime * 2.0f;
        if ( m_smooth == Gfx::CAM_SMOOTH_HARD ) prog = powf(dist, 1.0f) * rTime * 4.0f;
        if ( m_smooth == Gfx::CAM_SMOOTH_SPEC ) prog = powf(dist, 1.0f) * rTime * 4.0f;
        if ( dist == 0.0f )
        {
            m_actualLookat = m_finalLookat;
        }
        else
        {
            if (prog > dist)
                prog = dist;
            m_actualLookat = (m_finalLookat - m_actualLookat) / dist * prog + m_actualLookat;
        }

        eye = m_effectOffset+m_actualEye;
        m_water->AdjustEye(eye);

        float h = m_terrain->GetFloorLevel(eye);
        if (eye.y < h + 4.0f)
            eye.y = h + 4.0f;

        lookat = m_effectOffset+m_actualLookat;
    }

    Math::Vector upVec = Math::Vector(0.0f, 1.0f, 0.0f);
    SetViewParams(eye, lookat, upVec);
}

bool Gfx::CCamera::IsCollision(Math::Vector &eye, Math::Vector lookat)
{
    if (m_type == Gfx::CAM_TYPE_BACK )  return IsCollisionBack(eye, lookat);
    if (m_type == Gfx::CAM_TYPE_FIX  )  return IsCollisionFix (eye, lookat);
    if (m_type == Gfx::CAM_TYPE_PLANE)  return IsCollisionFix (eye, lookat);
    return false;
}

bool Gfx::CCamera::IsCollisionBack(Math::Vector &eye, Math::Vector lookat)
{
    ObjectType iType;
    if (m_cameraObj == NULL)
        iType = OBJECT_NULL;
    else
        iType = m_cameraObj->GetType();

    Math::Vector min;
    min.x = Math::Min(m_actualEye.x, m_actualLookat.x);
    min.y = Math::Min(m_actualEye.y, m_actualLookat.y);
    min.z = Math::Min(m_actualEye.z, m_actualLookat.z);

    Math::Vector max;
    max.x = Math::Max(m_actualEye.x, m_actualLookat.x);
    max.y = Math::Max(m_actualEye.y, m_actualLookat.y);
    max.z = Math::Max(m_actualEye.z, m_actualLookat.z);

    m_transparency = false;

    for (int i = 0 ;i < 1000000; i++)
    {
        CObject *obj = static_cast<CObject*>( m_iMan->SearchInstance(CLASS_OBJECT, i) );
        if (obj == NULL) break;

        if (obj->GetTruck()) continue;  // battery or cargo?

        SetTransparency(obj, 0.0f);  // opaque object

        if (obj == m_cameraObj) continue;

        if ( iType == OBJECT_BASE     ||  // building?
             iType == OBJECT_DERRICK  ||
             iType == OBJECT_FACTORY  ||
             iType == OBJECT_STATION  ||
             iType == OBJECT_CONVERT  ||
             iType == OBJECT_REPAIR   ||
             iType == OBJECT_DESTROYER||
             iType == OBJECT_TOWER    ||
             iType == OBJECT_RESEARCH ||
             iType == OBJECT_RADAR    ||
             iType == OBJECT_ENERGY   ||
             iType == OBJECT_LABO     ||
             iType == OBJECT_NUCLEAR  ||
             iType == OBJECT_PARA     ||
             iType == OBJECT_SAFE     ||
             iType == OBJECT_HUSTON   )  continue;

        ObjectType oType = obj->GetType();
        if ( oType == OBJECT_HUMAN  ||
             oType == OBJECT_TECH   ||
             oType == OBJECT_TOTO   ||
             oType == OBJECT_FIX    ||
             oType == OBJECT_FRET   ||
             oType == OBJECT_ANT    ||
             oType == OBJECT_SPIDER ||
             oType == OBJECT_BEE    ||
             oType == OBJECT_WORM   )  continue;

        Math::Vector oPos;
        float oRadius = 0.0f;
        obj->GetGlobalSphere(oPos, oRadius);
        if ( oRadius <= 2.0f )  continue;  // ignores small objects

        if ( oPos.x+oRadius < min.x ||
             oPos.y+oRadius < min.y ||
             oPos.z+oRadius < min.z ||
             oPos.x-oRadius > max.x ||
             oPos.y-oRadius > max.y ||
             oPos.z-oRadius > max.z )  continue;

        Math::Vector proj = Projection(m_actualEye, m_actualLookat, oPos);
        float dpp = Math::Distance(proj, oPos);
        if ( dpp > oRadius )  continue;

        if ( oType == OBJECT_FACTORY )
        {
            float angle = Math::RotateAngle(m_actualEye.x-oPos.x, oPos.z-m_actualEye.z);  // CW !
            angle = Math::Direction(angle, obj->GetAngleY(0));
            if ( fabs(angle) < 30.0f*Math::PI/180.0f )  continue;  // in the gate?
        }

        float del = Math::Distance(m_actualEye, m_actualLookat);
        if (oType == OBJECT_FACTORY)
            del += oRadius;

        float len = Math::Distance(m_actualEye, proj);
        if (len > del) continue;

        SetTransparency(obj, 1.0f);  // transparent object
        m_transparency = true;
    }
    return false;
}

bool Gfx::CCamera::IsCollisionFix(Math::Vector &eye, Math::Vector lookat)
{
    for (int i = 0; i < 1000000; i++)
    {
        CObject *obj = static_cast<CObject*>( m_iMan->SearchInstance(CLASS_OBJECT, i) );
        if (obj == NULL) break;

        if (obj == m_cameraObj) continue;

        ObjectType type = obj->GetType();
        if ( type == OBJECT_TOTO    ||
             type == OBJECT_FRET    ||
             type == OBJECT_STONE   ||
             type == OBJECT_URANIUM ||
             type == OBJECT_METAL   ||
             type == OBJECT_POWER   ||
             type == OBJECT_ATOMIC  ||
             type == OBJECT_BULLET  ||
             type == OBJECT_BBOX    ||
             type == OBJECT_KEYa    ||
             type == OBJECT_KEYb    ||
             type == OBJECT_KEYc    ||
             type == OBJECT_KEYd    ||
             type == OBJECT_ANT     ||
             type == OBJECT_SPIDER  ||
             type == OBJECT_BEE     ||
             type == OBJECT_WORM )  continue;

        Math::Vector objPos;
        float objRadius = 0.0f;
        obj->GetGlobalSphere(objPos, objRadius);
        if (objRadius == 0.0f) continue;

        float dist = Math::Distance(eye, objPos);
        if (dist < objRadius)
        {
            dist = Math::Distance(eye, lookat);
            Math::Vector proj = Projection(eye, lookat, objPos);
            eye = (lookat - eye) * objRadius / dist + proj;
            return false;
        }
    }
    return false;
}

bool Gfx::CCamera::EventProcess(const Event &event)
{
    switch (event.type)
    {
        case EVENT_FRAME:
            EventFrame(event);
            break;

        case EVENT_MOUSE_MOVE:
            EventMouseMove(event);
            break;

        // TODO: mouse wheel event
        /*case EVENT_KEY_DOWN:
            if ( event.param == VK_WHEELUP   )  EventMouseWheel(+1);
            if ( event.param == VK_WHEELDOWN )  EventMouseWheel(-1);
            break;*/

        default:
            break;
    }
    return true;
}

bool Gfx::CCamera::EventMouseMove(const Event &event)
{
    m_mousePos = event.pos;
    return true;
}

void Gfx::CCamera::EventMouseWheel(int dir)
{
    if (m_type == Gfx::CAM_TYPE_BACK)
    {
        if (dir > 0)
        {
            m_backDist -= 8.0f;
            if (m_backDist < m_backMin)
                m_backDist = m_backMin;
        }
        if (dir < 0)
        {
            m_backDist += 8.0f;
            if (m_backDist > 200.0f)
                m_backDist = 200.0f;
        }
    }

    if ( m_type == Gfx::CAM_TYPE_FIX   ||
         m_type == Gfx::CAM_TYPE_PLANE )
    {
        if (dir > 0)
        {
            m_fixDist -= 8.0f;
            if (m_fixDist < 10.0f)
                m_fixDist = 10.0f;
        }
        if (dir < 0)
        {
            m_fixDist += 8.0f;
            if (m_fixDist > 200.0f)
                m_fixDist = 200.0f;
        }
    }

    if ( m_type == Gfx::CAM_TYPE_VISIT )
    {
        if (dir > 0)
        {
            m_visitDist -= 8.0f;
            if (m_visitDist < 20.0f)
                m_visitDist = 20.0f;
        }
        if (dir < 0)
        {
            m_visitDist += 8.0f;
            if (m_visitDist > 200.0f)
                m_visitDist = 200.0f;
        }
    }
}

bool Gfx::CCamera::EventFrame(const Event &event)
{
    EffectFrame(event);
    OverFrame(event);

    if (m_type == Gfx::CAM_TYPE_FREE)
        return EventFrameFree(event);

    if (m_type == Gfx::CAM_TYPE_EDIT)
        return EventFrameEdit(event);

    if (m_type == Gfx::CAM_TYPE_DIALOG)
        return EventFrameDialog(event);

    if (m_type == Gfx::CAM_TYPE_BACK)
        return EventFrameBack(event);

    if (m_type == Gfx::CAM_TYPE_FIX   ||
        m_type == Gfx::CAM_TYPE_PLANE)
        return EventFrameFix(event);

    if (m_type == Gfx::CAM_TYPE_EXPLO)
        return EventFrameExplo(event);

    if (m_type == Gfx::CAM_TYPE_ONBOARD)
        return EventFrameOnBoard(event);

    if (m_type == Gfx::CAM_TYPE_SCRIPT)
        return EventFrameScript(event);

    if (m_type == Gfx::CAM_TYPE_INFO)
        return EventFrameInfo(event);

    if (m_type == Gfx::CAM_TYPE_VISIT)
        return EventFrameVisit(event);

    return true;
}

Gfx::EngineMouseType Gfx::CCamera::GetMouseDef(Math::Point pos)
{
    Gfx::EngineMouseType type = Gfx::ENG_MOUSE_NORM;
    m_mousePos = pos;

    if (m_type == Gfx::CAM_TYPE_INFO)
        return type;

    if (m_rightDown)  // the right button pressed?
    {
        m_rightPosMove.x = pos.x - m_rightPosCenter.x;
        m_rightPosMove.y = pos.y - m_rightPosCenter.y;
        type = Gfx::ENG_MOUSE_MOVE;
    }
    else
    {
        if (!m_cameraScroll)
            return type;

        m_mouseDirH = 0.0f;
        m_mouseDirV = 0.0f;

        if (pos.x < m_mouseMarging)
            m_mouseDirH = pos.x / m_mouseMarging - 1.0f;

        if (pos.x > 1.0f - m_mouseMarging)
            m_mouseDirH = 1.0f - (1.0f - pos.x) / m_mouseMarging;

        if (pos.y < m_mouseMarging)
            m_mouseDirV = pos.y / m_mouseMarging - 1.0f;

        if (pos.y > 1.0f-m_mouseMarging)
            m_mouseDirV = 1.0f - (1.0f - pos.y) / m_mouseMarging;

        if ( m_type == Gfx::CAM_TYPE_FREE  ||
             m_type == Gfx::CAM_TYPE_EDIT  ||
             m_type == Gfx::CAM_TYPE_BACK  ||
             m_type == Gfx::CAM_TYPE_FIX   ||
             m_type == Gfx::CAM_TYPE_PLANE ||
             m_type == Gfx::CAM_TYPE_EXPLO )
        {
            if (m_mouseDirH > 0.0f)
                type = Gfx::ENG_MOUSE_SCROLLR;
            if (m_mouseDirH < 0.0f)
                type = Gfx::ENG_MOUSE_SCROLLL;
        }

        if ( m_type == Gfx::CAM_TYPE_FREE ||
             m_type == Gfx::CAM_TYPE_EDIT )
        {
            if (m_mouseDirV > 0.0f)
                type = Gfx::ENG_MOUSE_SCROLLU;
            if (m_mouseDirV < 0.0f)
                type = Gfx::ENG_MOUSE_SCROLLD;
        }

        if (m_cameraInvertX)
            m_mouseDirH = -m_mouseDirH;
    }

    return type;
}

bool Gfx::CCamera::EventFrameFree(const Event &event)
{
    float factor = m_heightEye * 0.5f + 30.0f;

    if ( m_mouseDirH != 0.0f )
        m_directionH -= m_mouseDirH * event.rTime * 0.7f * m_speed;
    if ( m_mouseDirV != 0.0f )
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, m_mouseDirV * event.rTime * factor * m_speed);

    // Up/Down
    m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, event.axeY * event.rTime * factor * m_speed);

    // Left/Right
    if ( event.keyState & KS_CONTROL )
    {
        if ( event.axeX < 0.0f )
            m_eyePt = Math::LookatPoint(m_eyePt, m_directionH + Math::PI / 2.0f, m_directionV, -event.axeX * event.rTime * factor * m_speed);
        if ( event.axeX > 0.0f )
            m_eyePt = Math::LookatPoint(m_eyePt, m_directionH - Math::PI / 2.0f, m_directionV,  event.axeX * event.rTime * factor * m_speed);
    }
    else
    {
        m_directionH -= event.axeX * event.rTime * 0.7f * m_speed;
    }

    // PageUp/PageDown
    if ( event.keyState & KS_NUMMINUS )
    {
        if (m_heightEye < 500.0f)
            m_heightEye += event.rTime * factor * m_speed;
    }
    if ( event.keyState & KS_NUMPLUS )
    {
        if (m_heightEye > -2.0f)
            m_heightEye -= event.rTime * factor * m_speed;
    }

    m_terrain->ValidPosition(m_eyePt, 10.0f);

    if (m_terrain->MoveOnFloor(m_eyePt, true))
    {
        m_eyePt.y += m_heightEye;

        Math::Vector pos = m_eyePt;
        if (m_terrain->MoveOnFloor(pos, true))
        {
            pos.y -= 2.0f;
            if (m_eyePt.y < pos.y)
                m_eyePt.y = pos.y;
        }

    }

    Math::Vector lookatPt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);

    if (m_terrain->MoveOnFloor(lookatPt, true))
        lookatPt.y += m_heightLookat;

    SetViewTime(m_eyePt, lookatPt, event.rTime);

    return true;
}

bool Gfx::CCamera::EventFrameEdit(const Event &event)
{
    float factor = m_editHeight * 0.5f + 30.0f;

    if (m_mouseDirH != 0.0f)
        m_directionH -= m_mouseDirH * event.rTime * 0.7f * m_speed;
    if (m_mouseDirV != 0.0f)
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, m_mouseDirV * event.rTime * factor * m_speed);

    if (m_cameraScroll)
    {
        // Left/Right.
        m_fixDirectionH += m_mouseDirH * event.rTime * 1.0f * m_speed;
        m_fixDirectionH = Math::NormAngle(m_fixDirectionH);
    }

    m_terrain->ValidPosition(m_eyePt, 10.0f);

    if (m_terrain->MoveOnFloor(m_eyePt, false))
    {
        m_eyePt.y += m_editHeight;

        Math::Vector pos = m_eyePt;
        if (m_terrain->MoveOnFloor(pos, false))
        {
            pos.y += 2.0f;
            if (m_eyePt.y < pos.y)
                m_eyePt.y = pos.y;
        }

    }

    Math::Vector lookatPt = Math::LookatPoint( m_eyePt, m_directionH, m_directionV, 50.0f );

    if ( m_terrain->MoveOnFloor(lookatPt, true))
        lookatPt.y += m_heightLookat;

    SetViewTime(m_eyePt, lookatPt, event.rTime);

    return true;
}

bool Gfx::CCamera::EventFrameDialog(const Event &event)
{
    return true;
}

bool Gfx::CCamera::EventFrameBack(const Event &event)
{
    ObjectType type;
    if (m_cameraObj == NULL)
        type = OBJECT_NULL;
    else
        type = m_cameraObj->GetType();

    // +/-.
    if (event.keyState & KS_NUMPLUS)
    {
        m_backDist -= event.rTime * 30.0f * m_speed;
        if (m_backDist < m_backMin) m_backDist = m_backMin;
    }
    if (event.keyState & KS_NUMMINUS)
    {
        m_backDist += event.rTime * 30.0f * m_speed;
        if (m_backDist > 200.0f) m_backDist = 200.0f;
    }

    m_motorTurn = 0.0f;

    if (m_rightDown)
    {
        m_addDirectionH =  m_rightPosMove.x * 6.0f;
        m_addDirectionV = -m_rightPosMove.y * 2.0f;
    }
    else
    {
        if (m_cameraScroll)
        {
            // Left/Right
            m_addDirectionH += m_mouseDirH * event.rTime * 1.0f * m_speed;
            m_addDirectionH = Math::NormAngle(m_addDirectionH);
        }
    }

    if ((m_mouseDirH != 0) || (m_mouseDirV != 0))
        AbortCentering();  // special stops framing

    // Increase the special framework
    float centeringH = 0.0f;
    float centeringV = 0.0f;
    float centeringD = 0.0f;

    if (m_centeringPhase == Gfx::CAM_PHASE_START)
    {
        m_centeringProgress += event.rTime / m_centeringTime;
        if (m_centeringProgress > 1.0f) m_centeringProgress = 1.0f;
        centeringH = m_centeringProgress;
        centeringV = m_centeringProgress;
        centeringD = m_centeringProgress;
        if (m_centeringProgress >= 1.0f)
            m_centeringPhase = Gfx::CAM_PHASE_WAIT;
    }

    if (m_centeringPhase == Gfx::CAM_PHASE_WAIT)
    {
        centeringH = 1.0f;
        centeringV = 1.0f;
        centeringD = 1.0f;
    }

    if (m_centeringPhase == Gfx::CAM_PHASE_STOP)
    {
        m_centeringProgress += event.rTime / m_centeringTime;
        if (m_centeringProgress > 1.0f) m_centeringProgress = 1.0f;
        centeringH = 1.0f-m_centeringProgress;
        centeringV = 1.0f-m_centeringProgress;
        centeringD = 1.0f-m_centeringProgress;
        if (m_centeringProgress >= 1.0f)
            m_centeringPhase = Gfx::CAM_PHASE_NULL;
    }

    if (m_centeringAngleH == 99.9f) centeringH = 0.0f;
    if (m_centeringAngleV == 99.9f) centeringV = 0.0f;
    if (m_centeringDist   ==  0.0f) centeringD = 0.0f;

    if (m_cameraObj != NULL)
    {
        Math::Vector lookatPt = m_cameraObj->GetPosition(0);
             if (type == OBJECT_BASE ) lookatPt.y += 40.0f;
        else if (type == OBJECT_HUMAN) lookatPt.y +=  1.0f;
        else if (type == OBJECT_TECH ) lookatPt.y +=  1.0f;
        else                           lookatPt.y +=  4.0f;

        float h = -m_cameraObj->GetAngleY(0);  // angle vehicle / building

        if ( type == OBJECT_DERRICK  ||
             type == OBJECT_FACTORY  ||
             type == OBJECT_REPAIR   ||
             type == OBJECT_DESTROYER||
             type == OBJECT_STATION  ||
             type == OBJECT_CONVERT  ||
             type == OBJECT_TOWER    ||
             type == OBJECT_RESEARCH ||
             type == OBJECT_RADAR    ||
             type == OBJECT_INFO     ||
             type == OBJECT_ENERGY   ||
             type == OBJECT_LABO     ||
             type == OBJECT_NUCLEAR  ||
             type == OBJECT_PARA     ||
             type == OBJECT_SAFE     ||
             type == OBJECT_HUSTON   ||
             type == OBJECT_START    ||
             type == OBJECT_END      )  // building?
        {
            h += Math::PI * 0.20f;  // nearly face
        }
        else    // vehicle?
        {
            h += Math::PI;  // back
        }
        h = Math::NormAngle(h)+m_remotePan;
        float v = 0.0f;  //?

        h += m_centeringCurrentH;
        h += m_addDirectionH * (1.0f - centeringH);
        h = Math::NormAngle(h);

        if (type == OBJECT_MOBILEdr)  // designer?
            v -= 0.3f;  // Camera top

        v += m_centeringCurrentV;
        v += m_addDirectionV * (1.0f - centeringV);

        float d = m_backDist;
        d += m_centeringDist * centeringD;

        m_centeringCurrentH = m_centeringAngleH * centeringH;
        m_centeringCurrentV = m_centeringAngleV * centeringV;

        m_eyePt = RotateView(lookatPt, h, v, d);

        CPhysics* physics = m_cameraObj->GetPhysics();
        if ( (physics != NULL) && physics->GetLand() )  // ground?
        {
            Math::Vector pos = lookatPt + (lookatPt - m_eyePt);
            float floor = m_terrain->GetFloorHeight(pos) - 4.0f;
            if (floor > 0.0f)
                m_eyePt.y += floor;  // shows the descent in front
        }

        m_eyePt = ExcludeTerrain(m_eyePt, lookatPt, h, v);
        m_eyePt = ExcludeObject(m_eyePt, lookatPt, h, v);

        SetViewTime(m_eyePt, lookatPt, event.rTime);

        m_directionH = h + Math::PI / 2.0f;
        m_directionV = v;
    }

    return true;
}

bool Gfx::CCamera::EventFrameFix(const Event &event)
{
    // +/-.
    if (event.keyState & KS_NUMPLUS)
    {
        m_fixDist -= event.rTime * 30.0f * m_speed;
        if (m_fixDist < 10.0f) m_fixDist = 10.0f;
    }
    if (event.keyState & KS_NUMMINUS)
    {
        m_fixDist += event.rTime * 30.0f * m_speed;
        if (m_fixDist > 200.0f) m_fixDist = 200.0f;
    }

    if (m_cameraScroll)
    {
        // Left/Right
        m_fixDirectionH += m_mouseDirH * event.rTime * 1.0f * m_speed;
        m_fixDirectionH = Math::NormAngle(m_fixDirectionH);
    }

    if ((m_mouseDirH != 0) || (m_mouseDirV != 0))
        AbortCentering();  // special stops framing

    if (m_cameraObj != NULL)
    {
        Math::Vector lookatPt = m_cameraObj->GetPosition(0);

        float h = m_fixDirectionH + m_remotePan;
        float v = m_fixDirectionV;

        float d = m_fixDist;
        m_eyePt = RotateView(lookatPt, h, v, d);
        if (m_type == Gfx::CAM_TYPE_PLANE) m_eyePt.y += m_fixDist / 2.0f;
        m_eyePt = ExcludeTerrain(m_eyePt, lookatPt, h, v);
        m_eyePt = ExcludeObject(m_eyePt, lookatPt, h, v);

        SetViewTime(m_eyePt, lookatPt, event.rTime);

        m_directionH = h + Math::PI / 2.0f;
        m_directionV = v;
    }

    return true;
}

bool Gfx::CCamera::EventFrameExplo(const Event &event)
{
    if (m_mouseDirH != 0.0f)
        m_directionH -= m_mouseDirH * event.rTime * 0.7f * m_speed;

    m_terrain->ValidPosition(m_eyePt, 10.0f);

    if ( m_terrain->MoveOnFloor(m_eyePt, false) )
    {
        m_eyePt.y += m_heightEye;

        Math::Vector pos = m_eyePt;
        if ( m_terrain->MoveOnFloor(pos, false) )
        {
            pos.y += 2.0f;
            if ( m_eyePt.y < pos.y )
                m_eyePt.y = pos.y;
        }

    }

    Math::Vector lookatPt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);

    if (m_terrain->MoveOnFloor(lookatPt, true))
        lookatPt.y += m_heightLookat;

    SetViewTime(m_eyePt, lookatPt, event.rTime);

    return true;
}

bool Gfx::CCamera::EventFrameOnBoard(const Event &event)
{
    if (m_cameraObj != NULL)
    {
        Math::Vector lookatPt, upVec;
        m_cameraObj->SetViewFromHere(m_eyePt, m_directionH, m_directionV,
                                     lookatPt, upVec, m_type);
        Math::Vector eye    = m_effectOffset * 0.3f + m_eyePt;
        Math::Vector lookat = m_effectOffset * 0.3f + lookatPt;

        SetViewParams(eye, lookat, upVec);
        m_actualEye    = eye;
        m_actualLookat = lookat;
    }
    return true;
}

bool Gfx::CCamera::EventFrameInfo(const Event &event)
{
    SetViewTime(Math::Vector(0.0f, 0.0f, 0.0f),
                Math::Vector(0.0f, 0.0f, 1.0f),
                event.rTime);
    return true;
}

bool Gfx::CCamera::EventFrameVisit(const Event &event)
{
    m_visitTime += event.rTime;

    // +/-.
    if (event.keyState & KS_NUMPLUS)
    {
        m_visitDist -= event.rTime * 50.0f * m_speed;
        if (m_visitDist < 20.0f) m_visitDist = 20.0f;
    }
    if (event.keyState & KS_NUMMINUS)
    {
        m_visitDist += event.rTime * 50.0f * m_speed;
        if (m_visitDist > 200.0f) m_visitDist = 200.0f;
    }

    // PageUp/Down.
    if (event.keyState & KS_PAGEUP)
    {
        m_visitDirectionV -= event.rTime * 1.0f * m_speed;
        if (m_visitDirectionV < -Math::PI * 0.40f) m_visitDirectionV = -Math::PI * 0.40f;
    }
    if (event.keyState & KS_PAGEDOWN)
    {
        m_visitDirectionV += event.rTime * 1.0f * m_speed;
        if (m_visitDirectionV > 0.0f ) m_visitDirectionV = 0.0f;
    }

    if (m_cameraScroll)
    {
        m_visitDist -= m_mouseDirV * event.rTime * 30.0f * m_speed;
        if (m_visitDist <  20.0f)  m_visitDist =  20.0f;
        if (m_visitDist > 200.0f)  m_visitDist = 200.0f;
    }

    float angleH = (m_visitTime / 10.0f) * (Math::PI * 2.0f);
    float angleV = m_visitDirectionV;
    Math::Vector eye = RotateView(m_visitGoal, angleH, angleV, m_visitDist);
    eye = ExcludeTerrain(eye, m_visitGoal, angleH, angleV);
    eye = ExcludeObject(eye, m_visitGoal, angleH, angleV);
    SetViewTime(eye, m_visitGoal, event.rTime);

    return true;
}

bool Gfx::CCamera::EventFrameScript(const Event &event)
{
    SetViewTime(m_scriptEye + m_effectOffset,
                m_scriptLookat + m_effectOffset, event.rTime);
    return true;
}

void Gfx::CCamera::SetScriptEye(Math::Vector eye)
{
    m_scriptEye = eye;
}

void Gfx::CCamera::SetScriptLookat(Math::Vector lookat)
{
    m_scriptLookat = lookat;
}

void Gfx::CCamera::SetViewParams(const Math::Vector &eye, const Math::Vector &lookat,
                            const Math::Vector &up)
{
    m_engine->SetViewParams(eye, lookat, up, m_eyeDistance);

    bool under = (eye.y < m_water->GetLevel());  // Is it underwater?
    if (m_type == Gfx::CAM_TYPE_INFO)
        under = false;

    m_engine->SetRankView(under ? 1 : 0);
}

Math::Vector Gfx::CCamera::ExcludeTerrain(Math::Vector eye, Math::Vector lookat,
                                          float &angleH, float &angleV)
{
    Math::Vector pos = eye;
    if (m_terrain->MoveOnFloor(pos))
    {
        float dist = Math::DistanceProjected(lookat, pos);
        pos.y += 2.0f+dist*0.1f;
        if ( pos.y > eye.y )
        {
            angleV = -Math::RotateAngle(dist, pos.y-lookat.y);
            eye = RotateView(lookat, angleH, angleV, dist);
        }
    }
    return eye;
}

Math::Vector Gfx::CCamera::ExcludeObject(Math::Vector eye, Math::Vector lookat,
                                         float &angleH, float &angleV)
{
    return eye;

// TODO: check the commented out code:
/*
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>( m_iMan->SearchInstance(CLASS_OBJECT, i) );
        if (obj == NULL)
            break;

        int j = 0;
        Math::Vector oPos;
        float oRad;
        while (obj->GetCrashSphere(j++, oPos, oRad))
        {
            float dist = Math::Distance(oPos, eye);
            if (dist < oRad + 2.0f)
                eye.y = oPos.y + oRad + 2.0f;
        }
    }

    return eye;*/
}
