// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "physics.h"
#include "camera.h"




// Object's constructor.

CCamera::CCamera(CInstanceManager* iMan)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_CAMERA, this);

    m_engine  = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
    m_water   = (CWater*)m_iMan->SearchInstance(CLASS_WATER);

    m_type      = CAMERA_FREE;
    m_smooth    = CS_NORM;
    m_cameraObj = 0;

    m_eyeDistance = 10.0f;
    m_initDelay   =  0.0f;

    m_actualEye    = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_actualLookat = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_finalEye     = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_finalLookat  = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_normEye      = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_normLookat   = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_focus        = 1.0f;

    m_bRightDown     = FALSE;
    m_rightPosInit   = FPOINT(0.5f, 0.5f);
    m_rightPosCenter = FPOINT(0.5f, 0.5f);
    m_rightPosMove   = FPOINT(0.5f, 0.5f);

    m_eyePt        = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_directionH   =  0.0f;
    m_directionV   =  0.0f;
    m_heightEye    = 20.0f;
    m_heightLookat =  0.0f;
    m_speed        =  2.0f;

    m_backDist      = 0.0f;
    m_backMin       = 0.0f;
    m_addDirectionH = 0.0f;
    m_addDirectionV = 0.0f;
    m_bTransparency = FALSE;

    m_fixDist       = 0.0f;
    m_fixDirectionH = 0.0f;
    m_fixDirectionV = 0.0f;

    m_visitGoal       = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_visitDist       = 0.0f;
    m_visitTime       = 0.0f;
    m_visitType       = CAMERA_NULL;
    m_visitDirectionH = 0.0f;
    m_visitDirectionV = 0.0f;

    m_editHeight = 40.0f;

    m_remotePan  = 0.0f;
    m_remoteZoom = 0.0f;

    m_mouseDirH    = 0.0f;
    m_mouseDirV    = 0.0f;
    m_mouseMarging = 0.01f;

    m_motorTurn = 0.0f;

    m_centeringPhase    = CP_NULL;
    m_centeringAngleH   = 0.0f;
    m_centeringAngleV   = 0.0f;
    m_centeringDist     = 0.0f;
    m_centeringCurrentH = 0.0f;
    m_centeringCurrentV = 0.0f;
    m_centeringTime     = 0.0f;
    m_centeringProgress = 0.0f;

    m_effectType     = CE_NULL;
    m_effectPos      = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_effectForce    = 0.0f;
    m_effectProgress = 0.0f;
    m_effectOffset   = D3DVECTOR(0.0f, 0.0f, 0.0f);

    m_scriptEye    = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_scriptLookat = D3DVECTOR(0.0f, 0.0f, 0.0f);

    m_bEffect        = TRUE;
    m_bCameraScroll  = TRUE;
    m_bCameraInvertX = FALSE;
    m_bCameraInvertY = FALSE;
}

// Object's constructor.

CCamera::~CCamera()
{
}


void CCamera::SetEffect(BOOL bEnable)
{
    m_bEffect = bEnable;
}

void CCamera::SetCameraScroll(BOOL bScroll)
{
    m_bCameraScroll = bScroll;
}

void CCamera::SetCameraInvertX(BOOL bInvert)
{
    m_bCameraInvertX = bInvert;
}

void CCamera::SetCameraInvertY(BOOL bInvert)
{
    m_bCameraInvertY = bInvert;
}


// Returns an additional force to turn.

float CCamera::RetMotorTurn()
{
    if ( m_type == CAMERA_BACK )  return m_motorTurn;
    return 0.0f;
}



// Initializes the camera.

void CCamera::Init(D3DVECTOR eye, D3DVECTOR lookat, float delay)
{
    D3DVECTOR   vUpVec;

    m_initDelay = delay;

    eye.y    += m_terrain->RetFloorLevel(eye,    TRUE);
    lookat.y += m_terrain->RetFloorLevel(lookat, TRUE);

    m_type = CAMERA_FREE;
    m_eyePt = eye;

    m_directionH = RotateAngle(eye.x-lookat.x, eye.z-lookat.z)+PI/2.0f;
    m_directionV = -RotateAngle(Length2d(eye, lookat), eye.y-lookat.y);

    m_eyeDistance = 10.0f;
    m_heightLookat = 10.0f;
    m_backDist = 30.0f;
    m_backMin  = 10.0f;
    m_addDirectionH = 0.0f;
    m_addDirectionV = -PI*0.05f;
    m_fixDist = 50.0f;
    m_fixDirectionH = PI*0.25f;
    m_fixDirectionV = -PI*0.10f;
    m_centeringPhase = CP_NULL;
    m_actualEye = m_eyePt;
    m_actualLookat = LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);
    m_finalEye = m_actualEye;
    m_finalLookat = m_actualLookat;
    m_scriptEye = m_actualEye;
    m_scriptLookat = m_actualLookat;
    m_focus = 1.00f;
    m_remotePan  = 0.0f;
    m_remoteZoom = 0.0f;

    FlushEffect();
    FlushOver();
    SetType(CAMERA_FREE);
}


// Gives the object controlling the camera.

void CCamera::SetObject(CObject* object)
{
    m_cameraObj = object;
}

CObject* CCamera::RetObject()
{
    return m_cameraObj;
}


// Changes the level of transparency of an object and objects
// transported (battery & cargo).

void SetTransparency(CObject* pObj, float value)
{
    CObject*    pFret;

    pObj->SetTransparency(value);

    pFret = pObj->RetFret();
    if ( pFret != 0 )
    {
        pFret->SetTransparency(value);
    }

    pFret = pObj->RetPower();
    if ( pFret != 0 )
    {
        pFret->SetTransparency(value);
    }
}

// Change the type of camera.

void CCamera::SetType(CameraType type)
{
    CObject*    pObj;
    ObjectType  oType;
    D3DVECTOR   vUpVec;
    int         i;

    m_remotePan  = 0.0f;
    m_remoteZoom = 0.0f;

    if ( m_type == CAMERA_BACK && m_bTransparency )
    {
        for ( i=0 ; i<1000000 ; i++ )
        {
            pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
            if ( pObj == 0 )  break;

            if ( pObj->RetTruck() )  continue;  // battery or cargo?

            SetTransparency(pObj, 0.0f);  // opaque object
        }
    }
    m_bTransparency = FALSE;

    if ( type == CAMERA_INFO  ||
         type == CAMERA_VISIT )  // xx -> info ?
    {
        m_normEye    = m_engine->RetEyePt();
        m_normLookat = m_engine->RetLookatPt();

        m_engine->SetFocus(1.00f);  // normal
        m_type = type;
        return;
    }

    if ( m_type == CAMERA_INFO  ||
         m_type == CAMERA_VISIT )  // info -> xx ?
    {
        m_engine->SetFocus(m_focus);  // gives initial focus
        m_type = type;

        vUpVec = D3DVECTOR(0.0f, 1.0f, 0.0f);
        SetViewParams(m_normEye, m_normLookat, vUpVec);
        return;
    }

    if ( m_type == CAMERA_BACK && type == CAMERA_FREE )  // back -> free ?
    {
        m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -50.0f);
    }

    if ( m_type == CAMERA_BACK && type == CAMERA_EDIT )  // back -> edit ?
    {
        m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -1.0f);
    }

    if ( m_type == CAMERA_ONBOARD && type == CAMERA_FREE )  // onboard -> free ?
    {
        m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);
    }

    if ( m_type == CAMERA_ONBOARD && type == CAMERA_EDIT )  // onboard -> edit ?
    {
        m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -30.0f);
    }

    if ( m_type == CAMERA_ONBOARD && type == CAMERA_EXPLO )  // onboard -> explo ?
    {
        m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -50.0f);
    }

    if ( m_type == CAMERA_BACK && type == CAMERA_EXPLO )  // back -> explo ?
    {
        m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, -20.0f);
    }

    if ( type == CAMERA_FIX   ||
         type == CAMERA_PLANE )
    {
        AbortCentering();  // Special stops framing
    }

    m_fixDist = 50.0f;
    if ( type == CAMERA_PLANE )
    {
        m_fixDist = 60.0f;
    }

    if ( type == CAMERA_BACK )
    {
        AbortCentering();  // Special stops framing
        m_addDirectionH = 0.0f;
        m_addDirectionV = -PI*0.05f;

        if ( m_cameraObj == 0 )  oType = OBJECT_NULL;
        else                     oType = m_cameraObj->RetType();

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

    if ( type != CAMERA_ONBOARD && m_cameraObj != 0 )
    {
        m_cameraObj->SetGunGoalH(0.0f);  // puts the cannon right
    }

    if ( type == CAMERA_ONBOARD )
    {
        m_focus = 1.50f;  // Wide
    }
    else
    {
        m_focus = 1.00f;  // normal
    }
    m_engine->SetFocus(m_focus);

    m_type = type;

    SetSmooth(CS_NORM);
}

CameraType CCamera::RetType()
{
    return m_type;
}


// Management of the smoothing mode.

void CCamera::SetSmooth(CameraSmooth type)
{
    m_smooth = type;
}

CameraSmooth CCamera::RetSmoth()
{
    return m_smooth;
}


// Management of the setback distance.

void CCamera::SetDist(float dist)
{
    m_fixDist = dist;
}

float CCamera::RetDist()
{
    return m_fixDist;
}


// Manage angle mode CAMERA_FIX.

void CCamera::SetFixDirection(float angle)
{
    m_fixDirectionH = angle;
}

float CCamera::RetFixDirection()
{
    return m_fixDirectionH;
}


// Managing the triggering mode of the camera panning.

void CCamera::SetRemotePan(float value)
{
    m_remotePan = value;
}

float CCamera::RetRemotePan()
{
    return m_remotePan;
}

// Management of the remote zoom (0 .. 1) of the camera.

void CCamera::SetRemoteZoom(float value)
{
    value = Norm(value);

    if ( m_type == CAMERA_BACK )
    {
        m_backDist = m_backMin+(200.0f-m_backMin)*value;
    }

    if ( m_type == CAMERA_FIX   ||
         m_type == CAMERA_PLANE )
    {
        m_fixDist = 10.0f+(200.0f-10.0f)*value;
    }
}

float CCamera::RetRemoteZoom()
{
    if ( m_type == CAMERA_BACK )
    {
        return (m_backDist-m_backMin)/(200.0f-m_backMin);
    }

    if ( m_type == CAMERA_FIX   ||
         m_type == CAMERA_PLANE )
    {
        return (m_fixDist-10.0f)/(200.0f-10.0f);
    }
    return 0.0f;
}



// Start with a tour round the camera.

void CCamera::StartVisit(D3DVECTOR goal, float dist)
{
    m_visitType = m_type;
    SetType(CAMERA_VISIT);
    m_visitGoal = goal;
    m_visitDist = dist;
    m_visitTime = 0.0f;
    m_visitDirectionH = 0.0f;
    m_visitDirectionV = -PI*0.10f;
}

// Circular end of a visit with the camera.

void CCamera::StopVisit()
{
    SetType(m_visitType);  // presents the initial type
}


// Returns the point of view of the camera.

void CCamera::RetCamera(D3DVECTOR &eye, D3DVECTOR &lookat)
{
    eye = m_eyePt;
    lookat = LookatPoint(m_eyePt, m_directionH, m_directionV, 50.0f);
}


// Specifies a special movement of camera to frame action.

BOOL CCamera::StartCentering(CObject *object, float angleH, float angleV,
                             float dist, float time)
{
    if ( m_type != CAMERA_BACK )  return FALSE;
    if ( object != m_cameraObj )  return FALSE;

    if ( m_centeringPhase != CP_NULL )  return FALSE;

    if ( m_addDirectionH > PI )
    {
        angleH = PI*2.0f-angleH;
    }

    m_centeringPhase    = CP_START;
    m_centeringAngleH   = angleH;
    m_centeringAngleV   = angleV;
    m_centeringDist     = dist;
    m_centeringCurrentH = 0.0f;
    m_centeringCurrentV = 0.0f;
    m_centeringTime     = time;
    m_centeringProgress = 0.0f;

    return TRUE;
}

// Ends a special movement of camera to frame action.

BOOL CCamera::StopCentering(CObject *object, float time)
{
    if ( m_type != CAMERA_BACK )  return FALSE;
    if ( object != m_cameraObj )  return FALSE;

    if ( m_centeringPhase != CP_START &&
         m_centeringPhase != CP_WAIT  )  return FALSE;

    m_centeringPhase = CP_STOP;

    if ( m_centeringAngleH != 99.9f )
    {
        m_centeringAngleH = m_centeringCurrentH;
    }
    if ( m_centeringAngleV != 99.9f )
    {
        m_centeringAngleV = m_centeringCurrentV;
    }

    m_centeringTime     = time;
    m_centeringProgress = 0.0f;

    return TRUE;
}

// Stop framing special in the current position.

void CCamera::AbortCentering()
{
    if ( m_type == CAMERA_INFO  ||
         m_type == CAMERA_VISIT )  return;

    if ( m_centeringPhase == CP_NULL )  return;

    m_centeringPhase = CP_NULL;

    if ( m_centeringAngleH != 99.9f )
    {
        m_addDirectionH = m_centeringCurrentH;
    }
    if ( m_centeringAngleV != 99.9f )
    {
        m_addDirectionV = m_centeringCurrentV;
    }
}



// Removes the special effect with the camera

void CCamera::FlushEffect()
{
    m_effectType     = CE_NULL;
    m_effectForce    = 0.0f;
    m_effectProgress = 0.0f;
    m_effectOffset   = D3DVECTOR(0.0f, 0.0f, 0.0f);
}

// Starts a special effect with the camera.

void CCamera::StartEffect(CameraEffect effect, D3DVECTOR pos, float force)
{
    if ( !m_bEffect )  return;

    m_effectType     = effect;
    m_effectPos      = pos;
    m_effectForce    = force;
    m_effectProgress = 0.0f;
}

// Advances the effect of the camera.

void CCamera::EffectFrame(const Event &event)
{
    float       dist, force;

    if ( m_type == CAMERA_INFO  ||
         m_type == CAMERA_VISIT )  return;

    if ( m_effectType == CE_NULL )  return;

    m_effectOffset = D3DVECTOR(0.0f, 0.0f, 0.0f);
    force = m_effectForce;

    if ( m_effectType == CE_TERRAFORM )
    {
        m_effectProgress += event.rTime*0.7f;
        m_effectOffset.x = (Rand()-0.5f)*10.0f;
        m_effectOffset.y = (Rand()-0.5f)*10.0f;
        m_effectOffset.z = (Rand()-0.5f)*10.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == CE_EXPLO )
    {
        m_effectProgress += event.rTime*1.0f;
        m_effectOffset.x = (Rand()-0.5f)*5.0f;
        m_effectOffset.y = (Rand()-0.5f)*5.0f;
        m_effectOffset.z = (Rand()-0.5f)*5.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == CE_SHOT )
    {
        m_effectProgress += event.rTime*1.0f;
        m_effectOffset.x = (Rand()-0.5f)*2.0f;
        m_effectOffset.y = (Rand()-0.5f)*2.0f;
        m_effectOffset.z = (Rand()-0.5f)*2.0f;

        force *= 1.0f-m_effectProgress;
    }

    if ( m_effectType == CE_CRASH )
    {
        m_effectProgress += event.rTime*5.0f;
        m_effectOffset.y = sinf(m_effectProgress*PI)*1.5f;
        m_effectOffset.x = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
        m_effectOffset.z = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
    }

    if ( m_effectType == CE_VIBRATION )
    {
        m_effectProgress += event.rTime*0.1f;
        m_effectOffset.y = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
        m_effectOffset.x = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
        m_effectOffset.z = (Rand()-0.5f)*1.0f*(1.0f-m_effectProgress);
    }

    if ( m_effectType == CE_PET )
    {
        m_effectProgress += event.rTime*5.0f;
        m_effectOffset.x = (Rand()-0.5f)*0.2f;
        m_effectOffset.y = (Rand()-0.5f)*2.0f;
        m_effectOffset.z = (Rand()-0.5f)*0.2f;
    }

    dist = Length(m_eyePt, m_effectPos);
    dist = Norm((dist-100.f)/100.0f);

    force *= 1.0f-dist;
#if _TEEN
    force *= 2.0f;
#endif
    m_effectOffset *= force;

    if ( m_effectProgress >= 1.0f )
    {
        FlushEffect();
    }
}


// Removes the effect of superposition in the foreground.

void CCamera::FlushOver()
{
    m_overType = OE_NULL;
    m_overColorBase.r = 0.0f;  // black
    m_overColorBase.g = 0.0f;
    m_overColorBase.b = 0.0f;
    m_overColorBase.a = 0.0f;
    m_engine->SetOverColor();  // nothing
}

// Specifies the base color.

void CCamera::SetOverBaseColor(D3DCOLORVALUE color)
{
    m_overColorBase = color;
}

// Starts a layering effect in the foreground.

void CCamera::StartOver(OverEffect effect, D3DVECTOR pos, float force)
{
    D3DCOLOR    color;
    float       dist, decay;

    m_overType = effect;
    m_overTime = 0.0f;

    if ( m_overType == OE_BLITZ )  decay = 400.0f;
    else                           decay = 100.0f;
    dist = Length(m_eyePt, pos);
    dist = (dist-decay)/decay;
    if ( dist < 0.0f )  dist = 0.0f;
    if ( dist > 1.0f )  dist = 1.0f;

    m_overForce = force * (1.0f-dist);

    if ( m_overType == OE_BLOOD )
    {
        m_overColor.r = 0.8f;
        m_overColor.g = 0.1f;
        m_overColor.b = 0.1f;  // red
        m_overMode    = D3DSTATETCb;

        m_overFadeIn  = 0.4f;
        m_overFadeOut = 0.8f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == OE_FADEINw )
    {
        m_overColor.r = 1.0f;
        m_overColor.g = 1.0f;
        m_overColor.b = 1.0f;  // white
        m_overMode    = D3DSTATETCb;

        m_overFadeIn  = 0.0f;
        m_overFadeOut =20.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == OE_FADEOUTw )
    {
        m_overColor.r = 1.0f;
        m_overColor.g = 1.0f;
        m_overColor.b = 1.0f;  // white
        m_overMode    = D3DSTATETCb;

        m_overFadeIn  = 6.0f;
        m_overFadeOut = 100000.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == OE_FADEOUTb )
    {
        color = m_engine->RetFogColor(1);  // fog color underwater
        m_overColor = RetColor(color);
        m_overMode = D3DSTATETCw;

        m_overFadeIn  = 4.0f;
        m_overFadeOut = 100000.0f;
        m_overForce   = 1.0f;
    }

    if ( m_overType == OE_BLITZ )
    {
        m_overColor.r = 0.9f;
        m_overColor.g = 1.0f;
        m_overColor.b = 1.0f;  // white-cyan
        m_overMode    = D3DSTATETCb;

        m_overFadeIn  = 0.0f;
        m_overFadeOut = 1.0f;
    }
}

// Advanced overlay effect in the foreground.

void CCamera::OverFrame(const Event &event)
{
    D3DCOLORVALUE   color;
    float           intensity;

    if ( m_type == CAMERA_INFO  ||
         m_type == CAMERA_VISIT )  return;

    if ( m_overType == OE_NULL )
    {
        return;
    }

    m_overTime += event.rTime;

    if ( m_overType == OE_BLITZ )
    {
        if ( rand()%2 == 0 )
        {
            color.r = m_overColor.r*m_overForce;
            color.g = m_overColor.g*m_overForce;
            color.b = m_overColor.b*m_overForce;
        }
        else
        {
            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
        }
        color.a = 0.0f;
        m_engine->SetOverColor(RetColor(color), m_overMode);
    }
    else
    {
        if ( m_overFadeIn > 0.0f && m_overTime < m_overFadeIn )
        {
            intensity = m_overTime/m_overFadeIn;
            intensity *= m_overForce;

            if ( m_overMode == D3DSTATETCw )
            {
                color.r = 1.0f-(1.0f-m_overColor.r)*intensity;
                color.g = 1.0f-(1.0f-m_overColor.g)*intensity;
                color.b = 1.0f-(1.0f-m_overColor.b)*intensity;
            }
            else
            {
                color.r = m_overColor.r*intensity;
                color.g = m_overColor.g*intensity;
                color.b = m_overColor.b*intensity;

                color.r = 1.0f-(1.0f-color.r)*(1.0f-m_overColorBase.r);
                color.g = 1.0f-(1.0f-color.g)*(1.0f-m_overColorBase.g);
                color.b = 1.0f-(1.0f-color.b)*(1.0f-m_overColorBase.b);
            }
            color.a = 0.0f;
            m_engine->SetOverColor(RetColor(color), m_overMode);
        }
        else if ( m_overFadeOut > 0.0f && m_overTime-m_overFadeIn < m_overFadeOut )
        {
            intensity = 1.0f-(m_overTime-m_overFadeIn)/m_overFadeOut;
            intensity *= m_overForce;

            if ( m_overMode == D3DSTATETCw )
            {
                color.r = 1.0f-(1.0f-m_overColor.r)*intensity;
                color.g = 1.0f-(1.0f-m_overColor.g)*intensity;
                color.b = 1.0f-(1.0f-m_overColor.b)*intensity;
            }
            else
            {
                color.r = m_overColor.r*intensity;
                color.g = m_overColor.g*intensity;
                color.b = m_overColor.b*intensity;

                color.r = 1.0f-(1.0f-color.r)*(1.0f-m_overColorBase.r);
                color.g = 1.0f-(1.0f-color.g)*(1.0f-m_overColorBase.g);
                color.b = 1.0f-(1.0f-color.b)*(1.0f-m_overColorBase.b);
            }
            color.a = 0.0f;
            m_engine->SetOverColor(RetColor(color), m_overMode);
        }
    }

    if ( m_overTime >= m_overFadeIn+m_overFadeOut )
    {
        FlushOver();
        return;
    }
}



// Sets the soft movement of the camera.

void CCamera::FixCamera()
{
    m_initDelay = 0.0f;
    m_actualEye    = m_finalEye    = m_scriptEye;
    m_actualLookat = m_finalLookat = m_scriptLookat;
    SetViewTime(m_scriptEye, m_scriptLookat, 0.0f);
}

// Specifies the location and direction of view to the 3D engine.

void CCamera::SetViewTime(const D3DVECTOR &vEyePt,
                          const D3DVECTOR &vLookatPt,
                          float rTime)
{
    D3DVECTOR   vUpVec, eye, lookat;
    float       prog, dist, h;

    if ( m_type == CAMERA_INFO )
    {
        eye    = vEyePt;
        lookat = vLookatPt;
    }
    else
    {
        if ( m_initDelay > 0.0f )
        {
            m_initDelay -= rTime;
            if ( m_initDelay < 0.0f )  m_initDelay = 0.0f;
            rTime /= 1.0f+m_initDelay;
        }

        eye    = vEyePt;
        lookat = vLookatPt;
        if ( !IsCollision(eye, lookat) )
        {
            m_finalEye    = eye;
            m_finalLookat = lookat;
        }

        dist = Length(m_finalEye, m_actualEye);
        if ( m_smooth == CS_NONE )  prog = dist;
        if ( m_smooth == CS_NORM )  prog = powf(dist, 1.5f)*rTime*0.5f;
        if ( m_smooth == CS_HARD )  prog = powf(dist, 1.0f)*rTime*4.0f;
        if ( m_smooth == CS_SPEC )  prog = powf(dist, 1.0f)*rTime*0.05f;
        if ( dist == 0.0f )
        {
            m_actualEye = m_finalEye;
        }
        else
        {
            if ( prog > dist )  prog = dist;
            m_actualEye = (m_finalEye-m_actualEye)/dist*prog + m_actualEye;
        }

        dist = Length(m_finalLookat, m_actualLookat);
        if ( m_smooth == CS_NONE )  prog = dist;
        if ( m_smooth == CS_NORM )  prog = powf(dist, 1.5f)*rTime*2.0f;
        if ( m_smooth == CS_HARD )  prog = powf(dist, 1.0f)*rTime*4.0f;
        if ( m_smooth == CS_SPEC )  prog = powf(dist, 1.0f)*rTime*4.0f;
        if ( dist == 0.0f )
        {
            m_actualLookat = m_finalLookat;
        }
        else
        {
            if ( prog > dist )  prog = dist;
            m_actualLookat = (m_finalLookat-m_actualLookat)/dist*prog + m_actualLookat;
        }

        eye = m_effectOffset+m_actualEye;
        m_water->AdjustEye(eye);

        h = m_terrain->RetFloorLevel(eye);
        if ( eye.y < h+4.0f )
        {
            eye.y = h+4.0f;
        }

        lookat = m_effectOffset+m_actualLookat;
    }

    vUpVec = D3DVECTOR(0.0f, 1.0f, 0.0f);
    SetViewParams(eye, lookat, vUpVec);
}


// Avoid the obstacles.

BOOL CCamera::IsCollision(D3DVECTOR &eye, D3DVECTOR lookat)
{
    if ( m_type == CAMERA_BACK  )  return IsCollisionBack(eye, lookat);
    if ( m_type == CAMERA_FIX   )  return IsCollisionFix(eye, lookat);
    if ( m_type == CAMERA_PLANE )  return IsCollisionFix(eye, lookat);
    return FALSE;
}

// Avoid the obstacles.

BOOL CCamera::IsCollisionBack(D3DVECTOR &eye, D3DVECTOR lookat)
{
#if 0
    CObject     *pObj;
    D3DVECTOR   oPos, min, max, proj;
    ObjectType  oType, iType;
    float       oRadius, dpp, dpl, del, dist, len, prox;
    int         i;

    if ( m_cameraObj == 0 )
    {
        iType = OBJECT_NULL;
    }
    else
    {
        iType = m_cameraObj->RetType();
    }

    min.x = Min(eye.x, lookat.x);
    min.y = Min(eye.y, lookat.y);
    min.z = Min(eye.z, lookat.z);

    max.x = Max(eye.x, lookat.x);
    max.y = Max(eye.y, lookat.y);
    max.z = Max(eye.z, lookat.z);

    prox = 8.0f;  // maximum proximity of the vehicle

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj == m_cameraObj )  continue;

        oType = pObj->RetType();
        if ( oType == OBJECT_TOTO      ||
             oType == OBJECT_FIX       ||
             oType == OBJECT_FRET      ||
             oType == OBJECT_STONE     ||
             oType == OBJECT_URANIUM   ||
             oType == OBJECT_METAL     ||
             oType == OBJECT_POWER     ||
             oType == OBJECT_ATOMIC    ||
             oType == OBJECT_BULLET    ||
             oType == OBJECT_BBOX      ||
             oType == OBJECT_TNT       ||
             oType == OBJECT_BOMB      ||
             oType == OBJECT_WAYPOINTb ||
             oType == OBJECT_WAYPOINTr ||
             oType == OBJECT_WAYPOINTg ||
             oType == OBJECT_WAYPOINTy ||
             oType == OBJECT_WAYPOINTv ||
             oType == OBJECT_FLAGb     ||
             oType == OBJECT_FLAGr     ||
             oType == OBJECT_FLAGg     ||
             oType == OBJECT_FLAGy     ||
             oType == OBJECT_FLAGv     ||
             oType == OBJECT_ANT       ||
             oType == OBJECT_SPIDER    ||
             oType == OBJECT_BEE       ||
             oType == OBJECT_WORM      )  continue;

        pObj->GetGlobalSphere(oPos, oRadius);
        if ( oRadius <= 0.0f )  continue;

        if ( oPos.x+oRadius < min.x ||
             oPos.y+oRadius < min.y ||
             oPos.z+oRadius < min.z ||
             oPos.x-oRadius > max.x ||
             oPos.y-oRadius > max.y ||
             oPos.z-oRadius > max.z )  continue;

        if ( iType == OBJECT_FACTORY )
        {
            dpl = Length(oPos, lookat);
            if ( dpl < oRadius )  continue;
        }

        proj = Projection(eye, lookat, oPos);
        dpp = Length(proj, oPos);
        if ( dpp > oRadius )  continue;

        del = Length(eye, lookat);
        len = Length(eye, proj);
        if ( len > del )  continue;

        dist = sqrtf(oRadius*oRadius + dpp*dpp)-3.0f;
        if ( dist < 0.0f )  dist = 0.0f;
        proj = (lookat-eye)*dist/del + proj;
        len  = Length(eye, proj);

        if ( len < del-prox )
        {
            eye = proj;
            eye.y += len/5.0f;
            return FALSE;
        }
        else
        {
            eye = (eye-lookat)*prox/del + lookat;
            eye.y += (del-prox)/5.0f;
            return FALSE;
        }
    }
    return FALSE;
#else
    CObject     *pObj;
    D3DVECTOR   oPos, min, max, proj;
    ObjectType  oType, iType;
    float       oRadius, dpp, del, len, angle;
    int         i;

    if ( m_cameraObj == 0 )
    {
        iType = OBJECT_NULL;
    }
    else
    {
        iType = m_cameraObj->RetType();
    }

    min.x = Min(m_actualEye.x, m_actualLookat.x);
    min.y = Min(m_actualEye.y, m_actualLookat.y);
    min.z = Min(m_actualEye.z, m_actualLookat.z);

    max.x = Max(m_actualEye.x, m_actualLookat.x);
    max.y = Max(m_actualEye.y, m_actualLookat.y);
    max.z = Max(m_actualEye.z, m_actualLookat.z);

    m_bTransparency = FALSE;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj->RetTruck() )  continue;  // battery or cargo?

        SetTransparency(pObj, 0.0f);  // opaque object

        if ( pObj == m_cameraObj )  continue;

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

        oType = pObj->RetType();
        if ( oType == OBJECT_HUMAN  ||
             oType == OBJECT_TECH   ||
             oType == OBJECT_TOTO   ||
             oType == OBJECT_FIX    ||
             oType == OBJECT_FRET   ||
             oType == OBJECT_ANT    ||
             oType == OBJECT_SPIDER ||
             oType == OBJECT_BEE    ||
             oType == OBJECT_WORM   )  continue;

        pObj->GetGlobalSphere(oPos, oRadius);
        if ( oRadius <= 2.0f )  continue;  // ignores small objects

        if ( oPos.x+oRadius < min.x ||
             oPos.y+oRadius < min.y ||
             oPos.z+oRadius < min.z ||
             oPos.x-oRadius > max.x ||
             oPos.y-oRadius > max.y ||
             oPos.z-oRadius > max.z )  continue;

        proj = Projection(m_actualEye, m_actualLookat, oPos);
        dpp = Length(proj, oPos);
        if ( dpp > oRadius )  continue;

        if ( oType == OBJECT_FACTORY )
        {
            angle = RotateAngle(m_actualEye.x-oPos.x, oPos.z-m_actualEye.z);  // CW !
            angle = Direction(angle, pObj->RetAngleY(0));
            if ( Abs(angle) < 30.0f*PI/180.0f )  continue;  // in the gate?
        }

        del = Length(m_actualEye, m_actualLookat);
        if ( oType == OBJECT_FACTORY )
        {
            del += oRadius;
        }

        len = Length(m_actualEye, proj);
        if ( len > del )  continue;

        SetTransparency(pObj, 1.0f);  // transparent object
        m_bTransparency = TRUE;
    }
    return FALSE;
#endif
}

// Avoid the obstacles.

BOOL CCamera::IsCollisionFix(D3DVECTOR &eye, D3DVECTOR lookat)
{
    CObject     *pObj;
    D3DVECTOR   oPos, proj;
    ObjectType  type;
    float       oRadius, dist;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj == m_cameraObj )  continue;

        type = pObj->RetType();
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
             type == OBJECT_WORM    )  continue;

        pObj->GetGlobalSphere(oPos, oRadius);
        if ( oRadius == 0.0f )  continue;

        dist = Length(eye, oPos);
        if ( dist < oRadius )
        {
            dist = Length(eye, lookat);
            proj = Projection(eye, lookat, oPos);
            eye = (lookat-eye)*oRadius/dist + proj;
            return FALSE;
        }
    }
    return FALSE;
}


// Management of an event.

BOOL CCamera::EventProcess(const Event &event)
{
    switch( event.event )
    {
        case EVENT_FRAME:
            EventFrame(event);
            break;

#if 0
        case EVENT_RBUTTONDOWN:
            m_bRightDown = TRUE;
            m_rightPosInit = event.pos;
            m_rightPosCenter = FPOINT(0.5f, 0.5f);
            m_engine->MoveMousePos(m_rightPosCenter);
//?         m_engine->SetMouseHide(TRUE);  // cache la souris
            break;

        case EVENT_RBUTTONUP:
            m_bRightDown = FALSE;
            m_engine->MoveMousePos(m_rightPosInit);
//?         m_engine->SetMouseHide(FALSE);  // remontre la souris
            m_addDirectionH = 0.0f;
            m_addDirectionV = -PI*0.05f;
            break;
#endif

        case EVENT_MOUSEMOVE:
            EventMouseMove(event);
            break;

        case EVENT_KEYDOWN:
            if ( event.param == VK_WHEELUP   )  EventMouseWheel(+1);
            if ( event.param == VK_WHEELDOWN )  EventMouseWheel(-1);
            break;
    }
    return TRUE;
}

// Changed the camera according to the mouse moved.

BOOL CCamera::EventMouseMove(const Event &event)
{
    m_mousePos = event.pos;
    return TRUE;
}

// Mouse wheel operated.

void CCamera::EventMouseWheel(int dir)
{
    if ( m_type == CAMERA_BACK )
    {
        if ( dir > 0 )
        {
            m_backDist -= 8.0f;
            if ( m_backDist < m_backMin )  m_backDist = m_backMin;
        }
        if ( dir < 0 )
        {
            m_backDist += 8.0f;
            if ( m_backDist > 200.0f )  m_backDist = 200.0f;
        }
    }

    if ( m_type == CAMERA_FIX   ||
         m_type == CAMERA_PLANE )
    {
        if ( dir > 0 )
        {
            m_fixDist -= 8.0f;
            if ( m_fixDist < 10.0f )  m_fixDist = 10.0f;
        }
        if ( dir < 0 )
        {
            m_fixDist += 8.0f;
            if ( m_fixDist > 200.0f )  m_fixDist = 200.0f;
        }
    }

    if ( m_type == CAMERA_VISIT )
    {
        if ( dir > 0 )
        {
            m_visitDist -= 8.0f;
            if ( m_visitDist < 20.0f )  m_visitDist = 20.0f;
        }
        if ( dir < 0 )
        {
            m_visitDist += 8.0f;
            if ( m_visitDist > 200.0f )  m_visitDist = 200.0f;
        }
    }
}

// Changed the camera according to the time elapsed.

BOOL CCamera::EventFrame(const Event &event)
{
    EffectFrame(event);
    OverFrame(event);

    if ( m_type == CAMERA_FREE )
    {
        return EventFrameFree(event);
    }
    if ( m_type == CAMERA_EDIT )
    {
        return EventFrameEdit(event);
    }
    if ( m_type == CAMERA_DIALOG )
    {
        return EventFrameDialog(event);
    }
    if ( m_type == CAMERA_BACK )
    {
        return EventFrameBack(event);
    }
    if ( m_type == CAMERA_FIX   ||
         m_type == CAMERA_PLANE )
    {
        return EventFrameFix(event);
    }
    if ( m_type == CAMERA_EXPLO )
    {
        return EventFrameExplo(event);
    }
    if ( m_type == CAMERA_ONBOARD )
    {
        return EventFrameOnBoard(event);
    }
    if ( m_type == CAMERA_SCRIPT )
    {
        return EventFrameScript(event);
    }
    if ( m_type == CAMERA_INFO )
    {
        return EventFrameInfo(event);
    }
    if ( m_type == CAMERA_VISIT )
    {
        return EventFrameVisit(event);
    }

    return TRUE;
}


// Returns the default sprite to use for the mouse.

D3DMouse CCamera::RetMouseDef(FPOINT pos)
{
    D3DMouse    type;

    type = D3DMOUSENORM;
    m_mousePos = pos;

    if ( m_type == CAMERA_INFO )  return type;

    if ( m_bRightDown )  // the right button pressed?
    {
        m_rightPosMove.x = pos.x - m_rightPosCenter.x;
        m_rightPosMove.y = pos.y - m_rightPosCenter.y;
        type = D3DMOUSEMOVE;
    }
    else
    {
        if ( !m_bCameraScroll )  return type;

        m_mouseDirH = 0.0f;
        m_mouseDirV = 0.0f;

        if ( pos.x < m_mouseMarging )
        {
            m_mouseDirH = pos.x/m_mouseMarging - 1.0f;
        }

        if ( pos.x > 1.0f-m_mouseMarging )
        {
            m_mouseDirH = 1.0f - (1.0f-pos.x)/m_mouseMarging;
        }

        if ( pos.y < m_mouseMarging )
        {
            m_mouseDirV = pos.y/m_mouseMarging - 1.0f;
        }

        if ( pos.y > 1.0f-m_mouseMarging )
        {
            m_mouseDirV = 1.0f - (1.0f-pos.y)/m_mouseMarging;
        }

        if ( m_type == CAMERA_FREE  ||
             m_type == CAMERA_EDIT  ||
             m_type == CAMERA_BACK  ||
             m_type == CAMERA_FIX   ||
             m_type == CAMERA_PLANE ||
             m_type == CAMERA_EXPLO )
        {
            if ( m_mouseDirH > 0.0f )
            {
                type = D3DMOUSESCROLLR;
            }
            if ( m_mouseDirH < 0.0f )
            {
                type = D3DMOUSESCROLLL;
            }
        }

        if ( m_type == CAMERA_FREE ||
             m_type == CAMERA_EDIT )
        {
            if ( m_mouseDirV > 0.0f )
            {
                type = D3DMOUSESCROLLU;
            }
            if ( m_mouseDirV < 0.0f )
            {
                type = D3DMOUSESCROLLD;
            }
        }

        if ( m_bCameraInvertX )
        {
            m_mouseDirH = -m_mouseDirH;
        }
    }

    return type;
}



// Moves the point of view.

BOOL CCamera::EventFrameFree(const Event &event)
{
    D3DVECTOR   pos, vLookatPt;
    float       factor;

    factor = m_heightEye*0.5f+30.0f;

    if ( m_mouseDirH != 0.0f )
    {
        m_directionH -= m_mouseDirH*event.rTime*0.7f*m_speed;
    }
    if ( m_mouseDirV != 0.0f )
    {
        m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, m_mouseDirV*event.rTime*factor*m_speed);
    }

    // Up/Down.
    m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, event.axeY*event.rTime*factor*m_speed);

    // Left/Right.
    if ( event.keyState & KS_CONTROL )
    {
        if ( event.axeX < 0.0f )
        {
            m_eyePt = LookatPoint(m_eyePt, m_directionH+PI/2.0f, m_directionV, -event.axeX*event.rTime*factor*m_speed);
        }
        if ( event.axeX > 0.0f )
        {
            m_eyePt = LookatPoint(m_eyePt, m_directionH-PI/2.0f, m_directionV, event.axeX*event.rTime*factor*m_speed);
        }
    }
    else
    {
        m_directionH -= event.axeX*event.rTime*0.7f*m_speed;
    }

    // PageUp/PageDown.
    if ( event.keyState & KS_NUMMINUS )
    {
        if ( m_heightEye < 500.0f )
        {
            m_heightEye += event.rTime*factor*m_speed;
        }
    }
    if ( event.keyState & KS_NUMPLUS )
    {
        if ( m_heightEye > -2.0f )
        {
            m_heightEye -= event.rTime*factor*m_speed;
        }
    }

    m_terrain->ValidPosition(m_eyePt, 10.0f);

    if ( m_terrain->MoveOnFloor(m_eyePt, TRUE) )
    {
        m_eyePt.y += m_heightEye;

        pos = m_eyePt;
        if ( m_terrain->MoveOnFloor(pos, TRUE) )
        {
            pos.y -= 2.0f;
            if ( m_eyePt.y < pos.y )
            {
                m_eyePt.y = pos.y;
            }
        }

    }

    vLookatPt = LookatPoint( m_eyePt, m_directionH, m_directionV, 50.0f );

    if ( m_terrain->MoveOnFloor(vLookatPt, TRUE) )
    {
        vLookatPt.y += m_heightLookat;
    }

    SetViewTime(m_eyePt, vLookatPt, event.rTime);

    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameEdit(const Event &event)
{
    D3DVECTOR   pos, vLookatPt;
    float       factor;

    factor = m_editHeight*0.5f+30.0f;

    if ( m_mouseDirH != 0.0f )
    {
        m_directionH -= m_mouseDirH*event.rTime*0.7f*m_speed;
    }
    if ( m_mouseDirV != 0.0f )
    {
        m_eyePt = LookatPoint(m_eyePt, m_directionH, m_directionV, m_mouseDirV*event.rTime*factor*m_speed);
    }

    if ( m_bCameraScroll )
    {
    // Left/Right.
        m_fixDirectionH += m_mouseDirH*event.rTime*1.0f*m_speed;
        m_fixDirectionH = NormAngle(m_fixDirectionH);

        // Up/Down.
//?     m_fixDirectionV -= m_mouseDirV*event.rTime*0.5f*m_speed;
//?     if ( m_fixDirectionV < -PI*0.40f )  m_fixDirectionV = -PI*0.40f;
//?     if ( m_fixDirectionV >  PI*0.20f )  m_fixDirectionV =  PI*0.20f;
    }

    m_terrain->ValidPosition(m_eyePt, 10.0f);

    if ( m_terrain->MoveOnFloor(m_eyePt, FALSE) )
    {
        m_eyePt.y += m_editHeight;

        pos = m_eyePt;
        if ( m_terrain->MoveOnFloor(pos, FALSE) )
        {
            pos.y += 2.0f;
            if ( m_eyePt.y < pos.y )
            {
                m_eyePt.y = pos.y;
            }
        }

    }

    vLookatPt = LookatPoint( m_eyePt, m_directionH, m_directionV, 50.0f );

    if ( m_terrain->MoveOnFloor(vLookatPt, TRUE) )
    {
        vLookatPt.y += m_heightLookat;
    }

    SetViewTime(m_eyePt, vLookatPt, event.rTime);

    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameDialog(const Event &event)
{
    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameBack(const Event &event)
{
    CPhysics*   physics;
    ObjectType  type;
    D3DVECTOR   pos, vLookatPt;
    FPOINT      mouse;
    float       centeringH, centeringV, centeringD, h, v, d, floor;

    if ( m_cameraObj == 0 )
    {
        type = OBJECT_NULL;
    }
    else
    {
        type = m_cameraObj->RetType();
    }

    // +/-.
    if ( event.keyState & KS_NUMPLUS )
    {
        m_backDist -= event.rTime*30.0f*m_speed;
        if ( m_backDist < m_backMin )  m_backDist = m_backMin;
    }
    if ( event.keyState & KS_NUMMINUS )
    {
        m_backDist += event.rTime*30.0f*m_speed;
        if ( m_backDist > 200.0f )  m_backDist = 200.0f;
    }

    m_motorTurn = 0.0f;

    if ( m_bRightDown )
    {
        m_addDirectionH =  m_rightPosMove.x*6.0f;
        m_addDirectionV = -m_rightPosMove.y*2.0f;
    }
    else
    {
        if ( m_bCameraScroll )
        {
#if 1
            // Left/Right.
            m_addDirectionH += m_mouseDirH*event.rTime*1.0f*m_speed;
            m_addDirectionH = NormAngle(m_addDirectionH);

            // Up/Down.
//?         m_backDist -= m_mouseDirV*event.rTime*30.0f*m_speed;
//?         if ( m_backDist <  10.0f )  m_backDist =  10.0f;
//?         if ( m_backDist > 200.0f )  m_backDist = 200.0f;
#else
            if ( m_mousePos.y >= 0.18f && m_mousePos.y <= 0.93f )
            {
//?             m_addDirectionH = -(m_mousePos.x-0.5f)*4.0f;
                m_addDirectionV =  (m_mousePos.y-0.5f)*2.0f;
//?             if ( m_bCameraInvertX )  m_addDirectionH = -m_addDirectionH;
                if ( m_bCameraInvertY )  m_addDirectionV = -m_addDirectionV;

                if ( m_mousePos.x < 0.5f )  m_motorTurn = -1.0f;
                if ( m_mousePos.x > 0.5f )  m_motorTurn =  1.0f;

                mouse = m_mousePos;
                mouse.x = 0.5f;
                m_engine->MoveMousePos(mouse);
            }
            else
            {
                m_addDirectionH = 0.0f;
                m_addDirectionV = 0.0f;
            }
#endif
        }
    }

    if ( m_mouseDirH != 0 || m_mouseDirV != 0 )
    {
        AbortCentering();  // special stops framing
    }

    // Increase the special framework.
    centeringH = 0.0f;
    centeringV = 0.0f;
    centeringD = 0.0f;

    if ( m_centeringPhase == CP_START )
    {
        m_centeringProgress += event.rTime/m_centeringTime;
        if ( m_centeringProgress > 1.0f )  m_centeringProgress = 1.0f;
        centeringH = m_centeringProgress;
        centeringV = m_centeringProgress;
        centeringD = m_centeringProgress;
        if ( m_centeringProgress >= 1.0f )
        {
            m_centeringPhase = CP_WAIT;
        }
    }

    if ( m_centeringPhase == CP_WAIT )
    {
        centeringH = 1.0f;
        centeringV = 1.0f;
        centeringD = 1.0f;
    }

    if ( m_centeringPhase == CP_STOP )
    {
        m_centeringProgress += event.rTime/m_centeringTime;
        if ( m_centeringProgress > 1.0f )  m_centeringProgress = 1.0f;
        centeringH = 1.0f-m_centeringProgress;
        centeringV = 1.0f-m_centeringProgress;
        centeringD = 1.0f-m_centeringProgress;
        if ( m_centeringProgress >= 1.0f )
        {
            m_centeringPhase = CP_NULL;
        }
    }

    if ( m_centeringAngleH == 99.9f )  centeringH = 0.0f;
    if ( m_centeringAngleV == 99.9f )  centeringV = 0.0f;
    if ( m_centeringDist   ==  0.0f )  centeringD = 0.0f;

    if ( m_cameraObj != 0 )
    {
        vLookatPt = m_cameraObj->RetPosition(0);
             if ( type == OBJECT_BASE  )  vLookatPt.y += 40.0f;
        else if ( type == OBJECT_HUMAN )  vLookatPt.y +=  1.0f;
        else if ( type == OBJECT_TECH  )  vLookatPt.y +=  1.0f;
        else                              vLookatPt.y +=  4.0f;

        h = -m_cameraObj->RetAngleY(0);  // angle vehicle / building

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
            h += PI*0.20f;  // nearly face
        }
        else    // vehicle?
        {
            h += PI;  // back
        }
        h = NormAngle(h)+m_remotePan;
        v = 0.0f;  //?

        h += m_centeringCurrentH;
        h += m_addDirectionH*(1.0f-centeringH);
        h = NormAngle(h);

        if ( type == OBJECT_MOBILEdr )  // designer?
        {
            v -= 0.3f;  // Camera top
        }

        v += m_centeringCurrentV;
        v += m_addDirectionV*(1.0f-centeringV);

        d = m_backDist;
        d += m_centeringDist*centeringD;

        m_centeringCurrentH = m_centeringAngleH*centeringH;
        m_centeringCurrentV = m_centeringAngleV*centeringV;

        m_eyePt = RotateView(vLookatPt, h, v, d);

        physics = m_cameraObj->RetPhysics();
        if ( physics != 0 && physics->RetLand() )  // ground?
        {
            pos = vLookatPt+(vLookatPt-m_eyePt);
            floor = m_terrain->RetFloorHeight(pos)-4.0f;
            if ( floor > 0.0f )
            {
                m_eyePt.y += floor;  // shows the descent in front
            }
        }

        m_eyePt = ExcludeTerrain(m_eyePt, vLookatPt, h, v);
        m_eyePt = ExcludeObject(m_eyePt, vLookatPt, h, v);

        SetViewTime(m_eyePt, vLookatPt, event.rTime);

        m_directionH = h+PI/2.0f;
        m_directionV = v;
    }

    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameFix(const Event &event)
{
    D3DVECTOR   pos, vLookatPt;
    float       h, v, d;

    // +/-.
    if ( event.keyState & KS_NUMPLUS )
    {
        m_fixDist -= event.rTime*30.0f*m_speed;
        if ( m_fixDist < 10.0f )  m_fixDist = 10.0f;
    }
    if ( event.keyState & KS_NUMMINUS )
    {
        m_fixDist += event.rTime*30.0f*m_speed;
        if ( m_fixDist > 200.0f )  m_fixDist = 200.0f;
    }

    if ( m_bCameraScroll )
    {
        // Left/Right.
        m_fixDirectionH += m_mouseDirH*event.rTime*1.0f*m_speed;
        m_fixDirectionH = NormAngle(m_fixDirectionH);

        // Up/Down.
//?     m_fixDist -= m_mouseDirV*event.rTime*30.0f*m_speed;
//?     if ( m_fixDist <  10.0f )  m_fixDist =  10.0f;
//?     if ( m_fixDist > 200.0f )  m_fixDist = 200.0f;
    }

    if ( m_mouseDirH != 0 || m_mouseDirV != 0 )
    {
        AbortCentering();  // special stops framing
    }

    if ( m_cameraObj != 0 )
    {
        vLookatPt = m_cameraObj->RetPosition(0);

        h = m_fixDirectionH+m_remotePan;
        v = m_fixDirectionV;

        d = m_fixDist;
//-     if ( m_type == CAMERA_PLANE )  d += 20.0f;
        m_eyePt = RotateView(vLookatPt, h, v, d);
//-     if ( m_type == CAMERA_PLANE )  m_eyePt.y += 50.0f;
        if ( m_type == CAMERA_PLANE )  m_eyePt.y += m_fixDist/2.0f;
        m_eyePt = ExcludeTerrain(m_eyePt, vLookatPt, h, v);
        m_eyePt = ExcludeObject(m_eyePt, vLookatPt, h, v);

        SetViewTime(m_eyePt, vLookatPt, event.rTime);

        m_directionH = h+PI/2.0f;
        m_directionV = v;
    }

    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameExplo(const Event &event)
{
    D3DVECTOR   pos, vLookatPt;
    float       factor;

    factor = m_heightEye*0.5f+30.0f;

    if ( m_mouseDirH != 0.0f )
    {
        m_directionH -= m_mouseDirH*event.rTime*0.7f*m_speed;
    }

    m_terrain->ValidPosition(m_eyePt, 10.0f);

    if ( m_terrain->MoveOnFloor(m_eyePt, FALSE) )
    {
        m_eyePt.y += m_heightEye;

        pos = m_eyePt;
        if ( m_terrain->MoveOnFloor(pos, FALSE) )
        {
            pos.y += 2.0f;
            if ( m_eyePt.y < pos.y )
            {
                m_eyePt.y = pos.y;
            }
        }

    }

    vLookatPt = LookatPoint( m_eyePt, m_directionH, m_directionV, 50.0f );

    if ( m_terrain->MoveOnFloor(vLookatPt, TRUE) )
    {
        vLookatPt.y += m_heightLookat;
    }

    SetViewTime(m_eyePt, vLookatPt, event.rTime);

    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameOnBoard(const Event &event)
{
    D3DVECTOR   vLookatPt, vUpVec, eye, lookat, pos;

    if ( m_cameraObj != 0 )
    {
        m_cameraObj->SetViewFromHere(m_eyePt, m_directionH, m_directionV,
                                     vLookatPt, vUpVec, m_type);
        eye    = m_effectOffset*0.3f+m_eyePt;
        lookat = m_effectOffset*0.3f+vLookatPt;

        SetViewParams(eye, lookat, vUpVec);
        m_actualEye    = eye;
        m_actualLookat = lookat;
    }
    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameInfo(const Event &event)
{
    SetViewTime(D3DVECTOR(0.0f, 0.0f, 0.0f),
                D3DVECTOR(0.0f, 0.0f, 1.0f),
                event.rTime);
    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameVisit(const Event &event)
{
    D3DVECTOR   eye;
    float       angleH, angleV;

    m_visitTime += event.rTime;

    // +/-.
    if ( event.keyState & KS_NUMPLUS )
    {
        m_visitDist -= event.rTime*50.0f*m_speed;
        if ( m_visitDist < 20.0f )  m_visitDist = 20.0f;
    }
    if ( event.keyState & KS_NUMMINUS )
    {
        m_visitDist += event.rTime*50.0f*m_speed;
        if ( m_visitDist > 200.0f )  m_visitDist = 200.0f;
    }

    // PageUp/Down.
    if ( event.keyState & KS_PAGEUP )
    {
        m_visitDirectionV -= event.rTime*1.0f*m_speed;
        if ( m_visitDirectionV < -PI*0.40f )  m_visitDirectionV = -PI*0.40f;
    }
    if ( event.keyState & KS_PAGEDOWN )
    {
        m_visitDirectionV += event.rTime*1.0f*m_speed;
        if ( m_visitDirectionV > 0.0f )  m_visitDirectionV = 0.0f;
    }

    if ( m_bCameraScroll )
    {
        m_visitDist -= m_mouseDirV*event.rTime*30.0f*m_speed;
        if ( m_visitDist <  20.0f )  m_visitDist =  20.0f;
        if ( m_visitDist > 200.0f )  m_visitDist = 200.0f;
    }

    angleH = (m_visitTime/10.0f)*(PI*2.0f);
    angleV = m_visitDirectionV;
    eye = RotateView(m_visitGoal, angleH, angleV, m_visitDist);
    eye = ExcludeTerrain(eye, m_visitGoal, angleH, angleV);
    eye = ExcludeObject(eye, m_visitGoal, angleH, angleV);
    SetViewTime(eye, m_visitGoal, event.rTime);

    return TRUE;
}

// Moves the point of view.

BOOL CCamera::EventFrameScript(const Event &event)
{
    SetViewTime(m_scriptEye+m_effectOffset,
                m_scriptLookat+m_effectOffset, event.rTime);
    return TRUE;
}

void CCamera::SetScriptEye(D3DVECTOR eye)
{
    m_scriptEye = eye;
}

void CCamera::SetScriptLookat(D3DVECTOR lookat)
{
    m_scriptLookat = lookat;
}


// Specifies the location and direction of view.

void CCamera::SetViewParams(const D3DVECTOR &eye, const D3DVECTOR &lookat,
                            const D3DVECTOR &up)
{
    BOOL        bUnder;

    m_engine->SetViewParams(eye, lookat, up, m_eyeDistance);

    bUnder = (eye.y < m_water->RetLevel());  // Is it underwater?
    if ( m_type == CAMERA_INFO )  bUnder = FALSE;
    m_engine->SetRankView(bUnder?1:0);
}


// Adjusts the camera not to enter the field.

D3DVECTOR CCamera::ExcludeTerrain(D3DVECTOR eye, D3DVECTOR lookat,
                                  float &angleH, float &angleV)
{
    D3DVECTOR   pos;
    float       dist;

    pos = eye;
    if ( m_terrain->MoveOnFloor(pos) )
    {
        dist = Length2d(lookat, pos);
        pos.y += 2.0f+dist*0.1f;
        if ( pos.y > eye.y )
        {
            angleV = -RotateAngle(dist, pos.y-lookat.y);
            eye = RotateView(lookat, angleH, angleV, dist);
        }
    }
    return eye;
}

// Adjusts the camera not to enter an object.

D3DVECTOR CCamera::ExcludeObject(D3DVECTOR eye, D3DVECTOR lookat,
                                 float &angleH, float &angleV)
{
    CObject*    pObj;
    D3DVECTOR   oPos;
    float       oRad, dist;
    int         i, j;

return eye;
//?
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        j = 0;
        while ( pObj->GetCrashSphere(j++, oPos, oRad) )
        {
            dist = Length(oPos, eye);
            if ( dist < oRad+2.0f )
            {
                eye.y = oPos.y+oRad+2.0f;
            }
        }
    }

    return eye;
}


