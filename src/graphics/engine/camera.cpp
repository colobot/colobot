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


#include "graphics/engine/camera.h"

#include "app/app.h"
#include "app/input.h"

#include "common/event.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "math/const.h"
#include "math/geometry.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/interface/carrier_object.h"
#include "object/interface/controllable_object.h"
#include "object/interface/movable_object.h"
#include "object/interface/powered_object.h"
#include "object/interface/transportable_object.h"

#include "physics/physics.h"


// Graphics module namespace
namespace Gfx
{

const float MOUSE_EDGE_MARGIN = 0.01f;

//! Changes the level of transparency of an object and objects transported (battery & cargo)
void SetTransparency(CObject* obj, float value)
{
    obj->SetTransparency(value);

    if (obj->Implements(ObjectInterfaceType::Carrier))
    {
        CObject* cargo = dynamic_cast<CCarrierObject*>(obj)->GetCargo();
        if (cargo != nullptr)
            cargo->SetTransparency(value);
    }

    if (obj->Implements(ObjectInterfaceType::Powered))
    {
        CObject* power = dynamic_cast<CPoweredObject*>(obj)->GetPower();
        if (power != nullptr)
            power->SetTransparency(value);
    }
}



CCamera::CCamera()
{
    m_engine  = CEngine::GetInstancePointer();
    m_water   = m_engine->GetWater();

    m_main    = CRobotMain::GetInstancePointer();
    m_terrain = m_main->GetTerrain();

    m_input   = CInput::GetInstancePointer();

    m_type      = CAM_TYPE_FREE;
    m_smooth    = CAM_SMOOTH_NORM;
    m_cameraObj = nullptr;

    m_initDelay   =  0.0f;

    m_actualEye    = Math::Vector(0.0f, 0.0f, 0.0f);
    m_actualLookat = Math::Vector(0.0f, 0.0f, 0.0f);
    m_finalEye     = Math::Vector(0.0f, 0.0f, 0.0f);
    m_finalLookat  = Math::Vector(0.0f, 0.0f, 0.0f);
    m_prevEye      = Math::Vector(0.0f, 0.0f, 0.0f);
    m_prevLookat   = Math::Vector(0.0f, 0.0f, 0.0f);
    m_focus        = 1.0f;

    m_eyePt        = Math::Vector(0.0f, 0.0f, 0.0f);
    m_directionH   =  0.0f;
    m_directionV   =  0.0f;
    m_heightEye    = 40.0f;
    m_heightLookat =  0.0f;
    m_speed        =  2.0f;

    m_backDist      = 0.0f;
    m_backMin       = 0.0f;
    m_addDirectionH = 0.0f;
    m_addDirectionV = 0.0f;

    m_fixDist       = 0.0f;
    m_fixDirectionH = 0.0f;
    m_fixDirectionV = 0.0f;

    m_visitGoal       = Math::Vector(0.0f, 0.0f, 0.0f);
    m_visitDist       = 0.0f;
    m_visitTime       = 0.0f;
    m_visitType       = CAM_TYPE_NULL;
    m_visitDirectionV = 0.0f;

    m_centeringPhase    = CAM_PHASE_NULL;
    m_centeringAngleH   = 0.0f;
    m_centeringAngleV   = 0.0f;
    m_centeringDist     = 0.0f;
    m_centeringCurrentH = 0.0f;
    m_centeringCurrentV = 0.0f;
    m_centeringTime     = 0.0f;
    m_centeringProgress = 0.0f;

    m_effectType     = CAM_EFFECT_NULL;
    m_effectPos      = Math::Vector(0.0f, 0.0f, 0.0f);
    m_effectForce    = 0.0f;
    m_effectProgress = 0.0f;
    m_effectOffset   = Math::Vector(0.0f, 0.0f, 0.0f);

    m_overType = CAM_OVER_EFFECT_NULL;
    m_overForce = 0.0f;
    m_overTime = 0.0f;
    m_overMode = 0;
    m_overFadeIn = 0.0f;
    m_overFadeOut = 0.0f;

    m_scriptEye    = Math::Vector(0.0f, 0.0f, 0.0f);
    m_scriptLookat = Math::Vector(0.0f, 0.0f, 0.0f);

    m_effect        = true;
    m_blood         = true;
    m_oldCameraScroll = false;
    m_cameraInvertX = false;
    m_cameraInvertY = false;
}

CCamera::~CCamera()
{
}

void CCamera::SetEffect(bool enable)
{
    m_effect = enable;
}

bool CCamera::GetEffect()
{
    return m_effect;
}

void CCamera::SetBlood(bool enable)
{
    m_blood = enable;
}

bool CCamera::GetBlood()
{
    return m_blood;
}

void CCamera::SetOldCameraScroll(bool scroll)
{
    m_oldCameraScroll = scroll;
}

bool CCamera::GetOldCameraScroll()
{
    return m_oldCameraScroll;
}

void CCamera::SetCameraInvertX(bool invert)
{
    m_cameraInvertX = invert;
}

bool CCamera::GetCameraInvertX()
{
    return m_cameraInvertX;
}

void CCamera::SetCameraInvertY(bool invert)
{
    m_cameraInvertY = invert;
}

bool CCamera::GetCameraInvertY()
{
    return m_cameraInvertY;
}

void CCamera::Init(Math::Vector eye, Math::Vector lookat, float delay)
{
    m_initDelay = delay;

    eye.y    += m_terrain->GetFloorLevel(eye,    true);
    lookat.y += m_terrain->GetFloorLevel(lookat, true);

    m_type = CAM_TYPE_FREE;
    m_eyePt = eye;

    m_directionH = Math::RotateAngle(eye.x - lookat.x, eye.z - lookat.z) + Math::PI / 2.0f;
    m_directionV = -Math::RotateAngle(Math::DistanceProjected(eye, lookat), eye.y - lookat.y);

    m_heightLookat = 10.0f;
    m_backDist = 30.0f;
    m_backMin  = 10.0f;
    m_addDirectionH = 0.0f;
    m_addDirectionV = -Math::PI*0.05f;
    m_fixDist = 50.0f;
    m_fixDirectionH = Math::PI*0.25f;
    m_fixDirectionV = -Math::PI*0.10f;
    m_centeringPhase = CAM_PHASE_NULL;
    m_actualEye = m_eyePt;
    m_actualLookat = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);
    m_finalEye = m_actualEye;
    m_finalLookat = m_actualLookat;
    m_scriptEye = m_actualEye;
    m_scriptLookat = m_actualLookat;
    m_focus = 1.00f;

    FlushEffect();
    FlushOver();
    SetType(CAM_TYPE_FREE);
}


void CCamera::SetControllingObject(CObject* object)
{
    m_cameraObj = object;
}

CObject* CCamera::GetControllingObject()
{
    return m_cameraObj;
}

void CCamera::SetType(CameraType type)
{
    if (m_type == CAM_TYPE_BACK)
    {
        for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
        {
            if (IsObjectBeingTransported(obj))
                continue;

            SetTransparency(obj, 0.0f);  // opaque object
        }
    }

    if (type == CAM_TYPE_VISIT)  // *** -> visit ?
    {
        m_prevEye    = m_engine->GetEyePt();
        m_prevLookat = m_engine->GetLookatPt();

        m_engine->SetFocus(1.00f);  // normal
        m_type = type;
        return;
    }

    if (m_type == CAM_TYPE_VISIT)  // visit -> *** ?
    {
        m_engine->SetFocus(m_focus);  // gives initial focus
        m_type = type;

        Math::Vector upVec = Math::Vector(0.0f, 1.0f, 0.0f);
        SetViewParams(m_prevEye, m_prevLookat, upVec);
        return;
    }

    if ( m_type == CAM_TYPE_BACK && type == CAM_TYPE_FREE )  // back -> free ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -50.0f);

    if ( m_type == CAM_TYPE_BACK && type == CAM_TYPE_EDIT )  // back -> edit ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -1.0f);

    if ( m_type == CAM_TYPE_ONBOARD && type == CAM_TYPE_FREE )  // onboard -> free ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);

    if ( m_type == CAM_TYPE_ONBOARD && type == CAM_TYPE_EDIT )  // onboard -> edit ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);

    if ( m_type == CAM_TYPE_ONBOARD && type == CAM_TYPE_EXPLO )  // onboard -> explo ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -50.0f);

    if ( m_type == CAM_TYPE_BACK && type == CAM_TYPE_EXPLO )  // back -> explo ?
        m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -20.0f);

    if ( type == CAM_TYPE_FIX   ||
         type == CAM_TYPE_PLANE )
        AbortCentering();  // Special stops framing

    m_fixDist = 50.0f;
    if ( type == CAM_TYPE_PLANE )
        m_fixDist = 60.0f;

    if ( type == CAM_TYPE_BACK )
    {
        AbortCentering();  // Special stops framing
        m_addDirectionH = 0.0f;
        m_addDirectionV = -Math::PI*0.05f;

        ObjectType oType;
        if ( m_cameraObj == nullptr )  oType = OBJECT_NULL;
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
        if ( oType == OBJECT_MOTHER   )  m_backDist =  55.0f;

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

    //if ( type != CAM_TYPE_ONBOARD && m_cameraObj != 0 )
    //    m_cameraObj->SetGunGoalH(0.0f);  // puts the cannon right

    if ( type == CAM_TYPE_ONBOARD )
        m_focus = 1.50f;  // Wide
    else
        m_focus = 1.00f;  // normal
    m_engine->SetFocus(m_focus);

    m_type = type;

    SetSmooth(CAM_SMOOTH_NORM);
}

CameraType CCamera::GetType()
{
    return m_type;
}

void CCamera::SetSmooth(CameraSmooth type)
{
    m_smooth = type;
}

CameraSmooth CCamera::GetSmooth()
{
    return m_smooth;
}

void CCamera::StartVisit(Math::Vector goal, float dist)
{
    m_visitType = m_type;
    SetType(CAM_TYPE_VISIT);
    m_visitGoal = goal;
    m_visitDist = dist;
    m_visitTime = 0.0f;
    m_visitDirectionV = -Math::PI*0.10f;
}

void CCamera::StopVisit()
{
    SetType(m_visitType);  // presents the initial type
}

void CCamera::GetCamera(Math::Vector &eye, Math::Vector &lookat)
{
    eye = m_eyePt;
    lookat = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);
}

bool CCamera::StartCentering(CObject *object, float angleH, float angleV,
                             float dist, float time)
{
    if (m_type != CAM_TYPE_BACK)
        return false;
    if (object != m_cameraObj)
        return false;

    if (m_centeringPhase != CAM_PHASE_NULL)
        return false;

    if (m_addDirectionH > Math::PI)
        angleH = Math::PI * 2.0f - angleH;

    m_centeringPhase    = CAM_PHASE_START;
    m_centeringAngleH   = angleH;
    m_centeringAngleV   = angleV;
    m_centeringDist     = dist;
    m_centeringCurrentH = 0.0f;
    m_centeringCurrentV = 0.0f;
    m_centeringTime     = time;
    m_centeringProgress = 0.0f;

    return true;
}

bool CCamera::StopCentering(CObject *object, float time)
{
    if (m_type != CAM_TYPE_BACK)
        return false;
    if (object != m_cameraObj)
        return false;

    if (m_centeringPhase != CAM_PHASE_START &&
        m_centeringPhase != CAM_PHASE_WAIT)
        return false;

    m_centeringPhase = CAM_PHASE_STOP;

    if (m_centeringAngleH != 99.9f)
        m_centeringAngleH = m_centeringCurrentH;

    if (m_centeringAngleV != 99.9f)
        m_centeringAngleV = m_centeringCurrentV;

    m_centeringTime     = time;
    m_centeringProgress = 0.0f;

    return true;
}

void CCamera::AbortCentering()
{
    if (m_type == CAM_TYPE_VISIT )
        return;

    if (m_centeringPhase == CAM_PHASE_NULL)
        return;

    m_centeringPhase = CAM_PHASE_NULL;

    if ( m_centeringAngleH != 99.9f )
        m_addDirectionH = m_centeringCurrentH;

    if (m_centeringAngleV != 99.9f)
        m_addDirectionV = m_centeringCurrentV;
}

void CCamera::FlushEffect()
{
    m_effectType     = CAM_EFFECT_NULL;
    m_effectForce    = 0.0f;
    m_effectProgress = 0.0f;
    m_effectOffset   = Math::Vector(0.0f, 0.0f, 0.0f);

    CApplication::GetInstancePointer()->StopForceFeedbackEffect();
}

void CCamera::StartEffect(CameraEffect effect, Math::Vector pos, float force)
{
    if ( !m_effect )  return;

    m_effectType     = effect;
    m_effectPos      = pos;
    m_effectForce    = force;
    m_effectProgress = 0.0f;
}

void CCamera::EffectFrame(const Event &event)
{
    if (m_type == CAM_TYPE_VISIT)
        return;

    if (m_effectType == CAM_EFFECT_NULL)
        return;

    m_effectOffset = Math::Vector(0.0f, 0.0f, 0.0f);

    float force = m_effectForce;

    if ( m_effectType == CAM_EFFECT_TERRAFORM )
    {
        m_effectProgress += event.rTime * 0.7f;
        m_effectOffset.x = (Math::Rand() - 0.5f) * 10.0f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 10.0f;
        m_effectOffset.z = (Math::Rand() - 0.5f) * 10.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == CAM_EFFECT_EXPLO )
    {
        m_effectProgress += event.rTime * 1.0f;
        m_effectOffset.x = (Math::Rand() - 0.5f)  *5.0f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 5.0f;
        m_effectOffset.z = (Math::Rand() - 0.5f) * 5.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == CAM_EFFECT_SHOT )
    {
        m_effectProgress += event.rTime * 1.0f;
        m_effectOffset.x = (Math::Rand() - 0.5f) * 2.0f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 2.0f;
        m_effectOffset.z = (Math::Rand() - 0.5f) * 2.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == CAM_EFFECT_CRASH )
    {
        m_effectProgress += event.rTime * 5.0f;
        m_effectOffset.y = sinf(m_effectProgress * Math::PI) * 1.5f;
        m_effectOffset.x = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
        m_effectOffset.z = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
    }

    if ( m_effectType == CAM_EFFECT_VIBRATION )
    {
        m_effectProgress += event.rTime * 0.1f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
        m_effectOffset.x = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
        m_effectOffset.z = (Math::Rand() - 0.5f) * 1.0f * (1.0f - m_effectProgress);
    }

    if ( m_effectType == CAM_EFFECT_PET )
    {
        m_effectProgress += event.rTime  *5.0f;
        m_effectOffset.x = (Math::Rand() - 0.5f) * 0.2f;
        m_effectOffset.y = (Math::Rand() - 0.5f) * 2.0f;
        m_effectOffset.z = (Math::Rand() - 0.5f) * 0.2f;
    }

    float dist = Math::Distance(m_eyePt, m_effectPos);
    dist = Math::Norm((dist - 100.f) / 100.0f);

    force *= 1.0f-dist;
    m_effectOffset *= force;

    float forceFeedback = force;
    if (m_effectType == CAM_EFFECT_VIBRATION)
    {
        forceFeedback *= 0.5f;
    }
    if (m_effectType == CAM_EFFECT_PET)
    {
        forceFeedback *= 0.75f;
    }
    if (m_effectType == CAM_EFFECT_EXPLO)
    {
        forceFeedback *= 3.0f;
    }
    if (forceFeedback > 1.0f) forceFeedback = 1.0f;
    if (forceFeedback >= 0.1f)
    {
        CApplication::GetInstancePointer()->PlayForceFeedbackEffect(forceFeedback);
    }

    if (m_effectProgress >= 1.0f)
        FlushEffect();
}

void CCamera::FlushOver()
{
    m_overType = CAM_OVER_EFFECT_NULL;
    m_overColorBase = Color(0.0f, 0.0f, 0.0f, 0.0f);  // black
    m_engine->SetOverColor();  // nothing
}

void CCamera::SetOverBaseColor(Color color)
{
    m_overColorBase = color;
}

void CCamera::StartOver(CameraOverEffect effect, Math::Vector pos, float force)
{
    m_overType = effect;
    m_overTime = 0.0f;

    float decay;
    if (m_overType == CAM_OVER_EFFECT_LIGHTNING)
        decay = 400.0f;
    else
        decay = 100.0f;

    float dist = Math::Distance(m_eyePt, pos);
    dist = (dist - decay) / decay;
    if (dist < 0.0f) dist = 0.0f;
    if (dist > 1.0f) dist = 1.0f;

    m_overForce = force * (1.0f - dist);

    if (m_overType == CAM_OVER_EFFECT_BLOOD)
    {
        m_overColor   = Color(0.8f, 0.1f, 0.1f); // red
        m_overMode    = ENG_RSTATE_TCOLOR_BLACK;

        m_overFadeIn  = 0.4f;
        m_overFadeOut = 0.8f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == CAM_OVER_EFFECT_FADEIN_WHITE )
    {
        m_overColor   = Color(1.0f, 1.0f, 1.0f); // white
        m_overMode    = ENG_RSTATE_TCOLOR_BLACK;

        m_overFadeIn  = 0.0f;
        m_overFadeOut = 20.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == CAM_OVER_EFFECT_FADEOUT_WHITE )
    {
        m_overColor   = Color(1.0f, 1.0f, 1.0f); // white
        m_overMode    = ENG_RSTATE_TCOLOR_BLACK;

        m_overFadeIn  = 6.0f;
        m_overFadeOut = 100000.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == CAM_OVER_EFFECT_FADEOUT_BLACK )
    {
        m_overColor   = m_engine->GetFogColor(1); // fog color underwater
        m_overMode    = ENG_RSTATE_TTEXTURE_WHITE;

        m_overFadeIn  = 4.0f;
        m_overFadeOut = 100000.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == CAM_OVER_EFFECT_LIGHTNING )
    {
        m_overColor   = Color(0.9f, 1.0f, 1.0f);  // white-cyan
        m_overMode    = ENG_RSTATE_TCOLOR_BLACK;

        m_overFadeIn  = 0.0f;
        m_overFadeOut = 1.0f;
    }
}

void CCamera::OverFrame(const Event &event)
{
    if (m_type == CAM_TYPE_VISIT)
        return;

    if (m_overType == CAM_OVER_EFFECT_NULL)
        return;

    m_overTime += event.rTime;

    if (m_overType == CAM_OVER_EFFECT_LIGHTNING)
    {
        Color color;
        if (rand() % 2 == 0)
        {
            color.r = m_overColor.r * m_overForce;
            color.g = m_overColor.g * m_overForce;
            color.b = m_overColor.b * m_overForce;
        }
        else
        {
            color = Color(0.0f, 0.0f, 0.0f);
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

            Color color;
            if (m_overMode == ENG_RSTATE_TCOLOR_WHITE)
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

            Color color;
            if (m_overMode == ENG_RSTATE_TCOLOR_WHITE)
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

void CCamera::UpdateCameraAnimation(const Math::Vector &eyePt,
                                    const Math::Vector &lookatPt,
                                    float rTime)
{
    if (m_initDelay > 0.0f)
    {
        m_initDelay -= rTime;
        if (m_initDelay < 0.0f)
            m_initDelay = 0.0f;
        rTime /= 1.0f+m_initDelay;
    }

    m_finalEye    = eyePt;
    m_finalLookat = lookatPt;
    IsCollision(m_finalEye, m_finalLookat);

    float prog = 0.0f;
    float dist = Math::Distance(m_finalEye, m_actualEye);

    if (m_smooth == CAM_SMOOTH_NONE) prog = dist;
    if (m_smooth == CAM_SMOOTH_NORM) prog = powf(dist, 1.5f) * rTime * 0.75f;
    if (m_smooth == CAM_SMOOTH_HARD) prog = dist * rTime * 4.0f;
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
    if ( m_smooth == CAM_SMOOTH_NONE ) prog = dist;
    if ( m_smooth == CAM_SMOOTH_NORM ) prog = powf(dist, 1.5f) * rTime * 3.0f;
    if ( m_smooth == CAM_SMOOTH_HARD ) prog = dist * rTime * 4.0f;
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

    Math::Vector eye = m_effectOffset+m_actualEye;
    m_water->AdjustEye(eye);

    float h = m_terrain->GetFloorLevel(eye);
    if (eye.y < h + 4.0f)
        eye.y = h + 4.0f;

    Math::Vector lookat = m_effectOffset+m_actualLookat;

    SetViewParams(eye, lookat);
}

void CCamera::IsCollision(Math::Vector &eye, Math::Vector lookat)
{
    if (m_type == CAM_TYPE_BACK ) IsCollisionBack();
    if (m_type == CAM_TYPE_FIX  ) IsCollisionFix (eye, lookat);
    if (m_type == CAM_TYPE_PLANE) IsCollisionFix (eye, lookat);
}

void CCamera::IsCollisionBack()
{
    ObjectType iType;
    if (m_cameraObj == nullptr)
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

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (IsObjectBeingTransported(obj))
            continue;

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
             oType == OBJECT_ANT    ||
             oType == OBJECT_SPIDER ||
             oType == OBJECT_BEE    ||
             oType == OBJECT_WORM   )  continue;

        Math::Sphere objSphere = obj->GetCameraCollisionSphere();
        Math::Vector oPos = objSphere.pos;
        float oRadius = objSphere.radius;
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
            angle = Math::Direction(angle, obj->GetRotationY());
            if ( fabs(angle) < 30.0f*Math::PI/180.0f )  continue;  // in the gate?
        }

        float del = Math::Distance(m_actualEye, m_actualLookat);
        if (oType == OBJECT_FACTORY)
            del += oRadius;

        float len = Math::Distance(m_actualEye, proj);
        if (len > del) continue;

        SetTransparency(obj, 1.0f);  // transparent object
    }
}

void CCamera::IsCollisionFix(Math::Vector &eye, Math::Vector lookat)
{
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (obj == m_cameraObj) continue;

        ObjectType type = obj->GetType();
        if ( type == OBJECT_TOTO    ||
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

        Math::Sphere objSphere = obj->GetCameraCollisionSphere();
        Math::Vector objPos = objSphere.pos;
        float objRadius = objSphere.radius;
        if (objRadius == 0.0f) continue;

        float dist = Math::Distance(eye, objPos);
        if (dist < objRadius)
        {
            dist = Math::Distance(eye, lookat);
            Math::Vector proj = Projection(eye, lookat, objPos);
            eye = (lookat - eye) * objRadius / dist + proj;
            return;
        }
    }
}

bool CCamera::EventProcess(const Event &event)
{
    if (event.type == EVENT_MOUSE_MOVE)
    {
        if (m_engine->GetMouseType() == ENG_MOUSE_SCROLLR ||
            m_engine->GetMouseType() == ENG_MOUSE_SCROLLL ||
            m_engine->GetMouseType() == ENG_MOUSE_SCROLLU ||
            m_engine->GetMouseType() == ENG_MOUSE_SCROLLD ||
            m_engine->GetMouseType() == ENG_MOUSE_MOVE    )
        {
            m_engine->SetMouseType(ENG_MOUSE_NORM);
        }

        if ((event.mouseButtonsState & MOUSE_BUTTON_RIGHT) != 0 || (event.mouseButtonsState & MOUSE_BUTTON_MIDDLE) != 0)
        {
            Math::Point newDelta = event.mousePos - m_mousePos;
            if (m_cameraInvertX)
                newDelta.x = -newDelta.x;
            if (m_cameraInvertY)
                newDelta.y = -newDelta.y;
            m_mouseDelta += newDelta;

            m_engine->SetMouseType(ENG_MOUSE_MOVE);
        }

        m_mouseDeltaEdge.LoadZero();
        if (m_oldCameraScroll)
        {
            if (event.mousePos.x < MOUSE_EDGE_MARGIN)
                m_mouseDeltaEdge.x = event.mousePos.x / MOUSE_EDGE_MARGIN - 1.0f;
            if (event.mousePos.x > 1.0f - MOUSE_EDGE_MARGIN)
                m_mouseDeltaEdge.x = 1.0f - (1.0f - event.mousePos.x) / MOUSE_EDGE_MARGIN;
            if (event.mousePos.y < MOUSE_EDGE_MARGIN)
                m_mouseDeltaEdge.y = event.mousePos.y / MOUSE_EDGE_MARGIN - 1.0f;
            if (event.mousePos.y > 1.0f - MOUSE_EDGE_MARGIN)
                m_mouseDeltaEdge.y = 1.0f - (1.0f - event.mousePos.y) / MOUSE_EDGE_MARGIN;

            if (m_type == CAM_TYPE_FREE  ||
                m_type == CAM_TYPE_EDIT  ||
                m_type == CAM_TYPE_BACK  ||
                m_type == CAM_TYPE_FIX   ||
                m_type == CAM_TYPE_PLANE ||
                m_type == CAM_TYPE_EXPLO )
            {
                if (m_mouseDeltaEdge.x > 0.0f)
                    m_engine->SetMouseType(ENG_MOUSE_SCROLLR);
                if (m_mouseDeltaEdge.x < 0.0f)
                    m_engine->SetMouseType(ENG_MOUSE_SCROLLL);
            }
            if (m_type == CAM_TYPE_FREE ||
                m_type == CAM_TYPE_EDIT )
            {
                if (m_mouseDeltaEdge.y > 0.0f)
                    m_engine->SetMouseType(ENG_MOUSE_SCROLLU);
                if (m_mouseDeltaEdge.y < 0.0f)
                    m_engine->SetMouseType(ENG_MOUSE_SCROLLD);
            }

            m_mouseDeltaEdge.x /= 2*Math::PI;
            m_mouseDeltaEdge.y /= Math::PI;
        }

        m_mousePos = event.mousePos;
    }

    if (event.type == EVENT_MOUSE_WHEEL)
    {
        auto dir = event.GetData<MouseWheelEventData>()->y;
        m_mouseWheelDelta -= dir;
    }

    if (event.type == EVENT_MOUSE_BUTTON_DOWN || event.type == EVENT_MOUSE_BUTTON_UP)
    {
        if (event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_RIGHT || event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_MIDDLE)
        {
            if ((event.mouseButtonsState & MOUSE_BUTTON_RIGHT) != 0 || (event.mouseButtonsState & MOUSE_BUTTON_MIDDLE) != 0)
            {
                m_engine->SetMouseType(ENG_MOUSE_MOVE);
            }
            else
            {
                m_engine->SetMouseType(ENG_MOUSE_NORM);
            }
        }
    }

    if (event.type == EVENT_FRAME && !m_freeze)
    {
        Math::Point newDelta = m_mouseDeltaEdge * m_speed * event.rTime;
        if (m_cameraInvertX)
            newDelta.x = -newDelta.x;
        if (m_cameraInvertY)
            newDelta.y = -newDelta.y;
        m_mouseDelta += newDelta;

        EffectFrame(event);
        OverFrame(event);

        if (m_type == CAM_TYPE_FREE ||
            m_type == CAM_TYPE_EDIT)
            return EventFrameFree(event, m_type != CAM_TYPE_EDIT);

        if (m_type == CAM_TYPE_BACK)
            return EventFrameBack(event);

        if (m_type == CAM_TYPE_FIX   ||
            m_type == CAM_TYPE_PLANE)
            return EventFrameFix(event);

        if (m_type == CAM_TYPE_EXPLO)
            return EventFrameExplo(event);

        if (m_type == CAM_TYPE_ONBOARD)
            return EventFrameOnBoard(event);

        if (m_type == CAM_TYPE_SCRIPT)
            return EventFrameScript(event);

        if (m_type == CAM_TYPE_VISIT)
            return EventFrameVisit(event);
    }
    return true;
}

bool CCamera::EventFrameFree(const Event &event, bool keysAllowed)
{
    Math::Vector cameraMove = CalculateCameraMovement(event, keysAllowed);

    float factor = m_heightEye * 0.5f + 30.0f;
    bool secondary = m_input->GetKeyState(INPUT_SLOT_CAM_ALT) || event.mouseButtonsState & MOUSE_BUTTON_MIDDLE; // TODO: make mouse button a keybinding

    // Forward/Backward
    m_eyePt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, -cameraMove.y * factor * 2);

    // Left/Right (pan or rotate)
    if (secondary)
    {
        if (cameraMove.x < 0.0f)
            m_eyePt = Math::LookatPoint(m_eyePt, m_directionH + Math::PI / 2.0f, m_directionV, -cameraMove.x * factor);
        if (cameraMove.x > 0.0f)
            m_eyePt = Math::LookatPoint(m_eyePt, m_directionH - Math::PI / 2.0f, m_directionV,  cameraMove.x * factor);
    }
    else
    {
        m_directionH -= cameraMove.x;
    }

    // Up/Down (eye or lookat point)
    if (secondary)
        m_heightLookat -= cameraMove.z;
    else
        m_heightEye -= cameraMove.z;

    m_heightEye = Math::Clamp(m_heightEye, -2.0f, 500.0f);

    m_terrain->AdjustToBounds(m_eyePt, 10.0f);

    if (m_terrain->AdjustToFloor(m_eyePt, true))
    {
        m_eyePt.y += m_heightEye;

        Math::Vector pos = m_eyePt;
        if (m_terrain->AdjustToFloor(pos, true))
        {
            pos.y -= 2.0f;
            if (m_eyePt.y < pos.y)
                m_eyePt.y = pos.y;
        }

    }

    Math::Vector lookatPt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);

    if (m_terrain->AdjustToFloor(lookatPt, true))
        lookatPt.y += m_heightLookat;

    UpdateCameraAnimation(m_eyePt, lookatPt, event.rTime);

    return true;
}

bool CCamera::EventFrameBack(const Event &event)
{
    Math::Vector cameraMove = CalculateCameraMovement(event);
    m_addDirectionH += cameraMove.x;
    m_addDirectionV += cameraMove.y;
    m_backDist += cameraMove.z;

    m_addDirectionH = Math::NormAngle(m_addDirectionH);
    m_addDirectionV = Math::NormAngle(m_addDirectionV);

    m_backDist = Math::Clamp(m_backDist, m_backMin, 200.0f);

    // Increase the special framework
    float centeringH = 0.0f;
    float centeringV = 0.0f;
    float centeringD = 0.0f;

    if (m_centeringPhase == CAM_PHASE_START)
    {
        m_centeringProgress += event.rTime / m_centeringTime;
        if (m_centeringProgress > 1.0f) m_centeringProgress = 1.0f;
        centeringH = m_centeringProgress;
        centeringV = m_centeringProgress;
        centeringD = m_centeringProgress;
        if (m_centeringProgress >= 1.0f)
            m_centeringPhase = CAM_PHASE_WAIT;
    }

    if (m_centeringPhase == CAM_PHASE_WAIT)
    {
        centeringH = 1.0f;
        centeringV = 1.0f;
        centeringD = 1.0f;
    }

    if (m_centeringPhase == CAM_PHASE_STOP)
    {
        m_centeringProgress += event.rTime / m_centeringTime;
        if (m_centeringProgress > 1.0f) m_centeringProgress = 1.0f;
        centeringH = 1.0f-m_centeringProgress;
        centeringV = 1.0f-m_centeringProgress;
        centeringD = 1.0f-m_centeringProgress;
        if (m_centeringProgress >= 1.0f)
            m_centeringPhase = CAM_PHASE_NULL;
    }

    if (m_centeringAngleH == 99.9f) centeringH = 0.0f;
    if (m_centeringAngleV == 99.9f) centeringV = 0.0f;
    if (m_centeringDist   ==  0.0f) centeringD = 0.0f;

    if (m_cameraObj != nullptr)
    {
        ObjectType type = m_cameraObj->GetType();

        Math::Vector lookatPt = m_cameraObj->GetPosition();
             if (type == OBJECT_BASE ) lookatPt.y += 40.0f;
        else if (type == OBJECT_HUMAN) lookatPt.y +=  1.0f;
        else if (type == OBJECT_TECH ) lookatPt.y +=  1.0f;
        else                           lookatPt.y +=  4.0f;

        float h = -m_cameraObj->GetRotationY();  // angle vehicle / building

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
        h = Math::NormAngle(h);
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

        bool ground = true;
        if (m_cameraObj->Implements(ObjectInterfaceType::Movable))
            ground = dynamic_cast<CMovableObject*>(m_cameraObj)->GetPhysics()->GetLand();
        if ( ground )  // ground?
        {
            Math::Vector pos = lookatPt + (lookatPt - m_eyePt);
            float floor = m_terrain->GetHeightToFloor(pos) - 4.0f;
            if (floor > 0.0f)
                m_eyePt.y += floor;  // shows the descent in front
        }

        m_eyePt = ExcludeTerrain(m_eyePt, lookatPt, h, v);
        m_eyePt = ExcludeObject(m_eyePt, lookatPt, h, v);

        UpdateCameraAnimation(m_eyePt, lookatPt, event.rTime);

        m_directionH = h + Math::PI / 2.0f;
        m_directionV = v;
    }

    return true;
}

bool CCamera::EventFrameFix(const Event &event)
{
    Math::Vector cameraMove = CalculateCameraMovement(event);
    m_fixDirectionH += cameraMove.x;
    m_fixDirectionV += cameraMove.y;
    m_fixDist += cameraMove.z;

    m_fixDirectionH = Math::NormAngle(m_fixDirectionH);

    m_fixDirectionV = Math::Clamp(m_fixDirectionV, -0.5f*Math::PI, 0.25f*Math::PI);
    m_fixDist = Math::Clamp(m_fixDist, 10.0f, 200.0f);

    if (m_cameraObj != nullptr)
    {
        Math::Vector lookatPt = m_cameraObj->GetPosition();

        float h = m_fixDirectionH;
        float v = m_fixDirectionV;

        float d = m_fixDist;
        m_eyePt = RotateView(lookatPt, h, v, d);
        if (m_type == CAM_TYPE_PLANE) m_eyePt.y += m_fixDist / 2.0f;
        m_eyePt = ExcludeTerrain(m_eyePt, lookatPt, h, v);
        m_eyePt = ExcludeObject(m_eyePt, lookatPt, h, v);

        UpdateCameraAnimation(m_eyePt, lookatPt, event.rTime);

        m_directionH = h + Math::PI / 2.0f;
        m_directionV = v;
    }

    return true;
}

bool CCamera::EventFrameExplo(const Event &event)
{
    Math::Vector cameraMove = CalculateCameraMovement(event);
    m_directionH += cameraMove.x;
    m_directionH = Math::NormAngle(m_directionH);

    m_terrain->AdjustToBounds(m_eyePt, 10.0f);

    if ( m_terrain->AdjustToFloor(m_eyePt, false) )
    {
        m_eyePt.y += m_heightEye;

        Math::Vector pos = m_eyePt;
        if ( m_terrain->AdjustToFloor(pos, false) )
        {
            pos.y += 2.0f;
            if ( m_eyePt.y < pos.y )
                m_eyePt.y = pos.y;
        }

    }

    Math::Vector lookatPt = Math::LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);

    if (m_terrain->AdjustToFloor(lookatPt, true))
        lookatPt.y += m_heightLookat;

    UpdateCameraAnimation(m_eyePt, lookatPt, event.rTime);

    return true;
}

bool CCamera::EventFrameOnBoard(const Event &event)
{
    if (m_cameraObj != nullptr)
    {
        assert(m_cameraObj->Implements(ObjectInterfaceType::Controllable));
        Math::Vector lookatPt, upVec;
        dynamic_cast<CControllableObject*>(m_cameraObj)->AdjustCamera(m_eyePt, m_directionH, m_directionV, lookatPt, upVec, m_type);
        Math::Vector eye    = m_effectOffset * 0.3f + m_eyePt;
        Math::Vector lookat = m_effectOffset * 0.3f + lookatPt;

        SetViewParams(eye, lookat, upVec);
        m_actualEye    = eye;
        m_actualLookat = lookat;
    }
    return true;
}

bool CCamera::EventFrameVisit(const Event &event)
{
    m_visitTime += event.rTime;

    Math::Vector cameraMove = CalculateCameraMovement(event);

    // ZoomIn/ZoomOut
    m_visitDist += cameraMove.z;
    m_visitDist = Math::Clamp(m_visitDist, 20.0f, 200.0f);

    // Up/Down
    m_visitDirectionV += cameraMove.y;
    m_visitDirectionV = Math::Clamp(m_visitDirectionV, -Math::PI * 0.40f, 0.0f);

    float angleH = (m_visitTime / 10.0f) * (Math::PI * 2.0f);
    float angleV = m_visitDirectionV;
    Math::Vector eye = RotateView(m_visitGoal, angleH, angleV, m_visitDist);
    eye = ExcludeTerrain(eye, m_visitGoal, angleH, angleV);
    eye = ExcludeObject(eye, m_visitGoal, angleH, angleV);
    UpdateCameraAnimation(eye, m_visitGoal, event.rTime);

    return true;
}

bool CCamera::EventFrameScript(const Event &event)
{
    UpdateCameraAnimation(m_scriptEye + m_effectOffset,
                          m_scriptLookat + m_effectOffset, event.rTime);
    return true;
}

void CCamera::SetScriptCameraAnimateEye(Math::Vector eye)
{
    m_scriptEye = eye;
}

void CCamera::SetScriptCameraAnimateLookat(Math::Vector lookat)
{
    m_scriptLookat = lookat;
}

void CCamera::SetScriptCamera(Math::Vector eye, Math::Vector lookat)
{
    SetScriptCameraAnimate(eye, lookat);

    m_initDelay    = 0.0f;
    m_actualEye    = m_finalEye    = m_scriptEye;
    m_actualLookat = m_finalLookat = m_scriptLookat;
}

void CCamera::SetScriptCameraAnimate(Math::Vector eye, Math::Vector lookat)
{
    m_scriptEye = eye;
    m_scriptLookat = lookat;
}

void CCamera::SetViewParams(const Math::Vector &eye, const Math::Vector &lookat,
                            const Math::Vector &up)
{
    m_engine->SetViewParams(eye, lookat, up);

    bool under = (eye.y < m_water->GetLevel());  // Is it underwater?
    m_engine->SetRankView(under ? 1 : 0);
}

Math::Vector CCamera::ExcludeTerrain(Math::Vector eye, Math::Vector lookat,
                                          float &angleH, float &angleV)
{
    Math::Vector pos = eye;
    if (m_terrain->AdjustToFloor(pos))
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

Math::Vector CCamera::ExcludeObject(Math::Vector eye, Math::Vector lookat,
                                         float &angleH, float &angleV)
{
    return eye;

// TODO: check the commented out code:
/*
    for (int i = 0; i < 1000000; i++)
    {
        CObject* obj = static_cast<CObject*>( iMan->SearchInstance(CLASS_OBJECT, i) );
        if (obj == nullptr)
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

void CCamera::SetCameraSpeed(float speed)
{
    m_speed = speed;
}

Math::Vector CCamera::CalculateCameraMovement(const Event &event, bool keysAllowed)
{
    Math::Vector delta;

    delta.x += m_mouseDelta.x * 2*Math::PI;
    delta.y -= m_mouseDelta.y * Math::PI;
    m_mouseDelta.LoadZero();

    delta.z += m_mouseWheelDelta * 8.0f;
    m_mouseWheelDelta = 0.0f;

    if (keysAllowed)
    {
        delta.x += event.cameraInput.x * event.rTime * 0.5f * m_speed;
        delta.y -= event.cameraInput.y * event.rTime * 0.5f * m_speed;
        delta.z -= event.cameraInput.z * event.rTime * 20.0f * m_speed;

        if (m_cameraObj == nullptr)
        {
            delta.x += event.motionInput.x * event.rTime * 0.5f * m_speed;
            delta.y -= event.motionInput.y * event.rTime * 0.5f * m_speed;
            delta.z -= event.motionInput.z * event.rTime * 20.0f * m_speed;
        }
    }

    if (delta.Length() > 0)
        AbortCentering();  // special stops framing

    return delta;
}

void CCamera::SetFreeze(bool freeze)
{
    m_freeze = freeze;
}

}
