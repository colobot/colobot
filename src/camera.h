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

// camera.h

#ifndef _CAMERA_H_
#define _CAMERA_H_


#include "d3dengine.h"
#include "struct.h"


class CInstanceManager;
class CD3DEngine;
class CTerrain;
class CWater;
class CObject;


enum CameraType
{
    CAMERA_NULL     = 0,    // camera undefined
    CAMERA_FREE     = 1,    // camera free (never in principle)
    CAMERA_EDIT     = 2,    // camera while editing a program
    CAMERA_ONBOARD      = 3,    // camera on board a robot
    CAMERA_BACK     = 4,    // camera behind a robot
    CAMERA_FIX      = 5,    // static camera following robot
    CAMERA_EXPLO        = 6,    // camera steady after explosion
    CAMERA_SCRIPT       = 7,    // camera during a film script
    CAMERA_INFO     = 8,    // camera for displaying information
    CAMERA_VISIT        = 9,    // visit instead of an error
    CAMERA_DIALOG       = 10,   // camera for dialogue
    CAMERA_PLANE        = 11,   // static camera height
};

enum CameraSmooth
{
    CS_NONE         = 0,    // sharp
    CS_NORM         = 1,    // normal
    CS_HARD         = 2,    // hard
    CS_SPEC         = 3,    // special
};

enum CenteringPhase
{
    CP_NULL         = 0,
    CP_START        = 1,
    CP_WAIT         = 2,
    CP_STOP         = 3,
};

enum CameraEffect
{
    CE_NULL         = 0,    // no effect
    CE_TERRAFORM        = 1,    // digging in
    CE_CRASH        = 2,    // Vehicle driving is severely
    CE_EXPLO        = 3,    // explosion
    CE_SHOT         = 4,    // not mortal shot
    CE_VIBRATION        = 5,    // vibration during construction
    CE_PET          = 6,    // spleen reactor
};

enum OverEffect
{
    OE_NULL         = 0,    // no effect
    OE_BLOOD        = 1,    // flash red
    OE_FADEINw      = 2,    // white -> nothing
    OE_FADEOUTw     = 3,    // nothing -> white
    OE_FADEOUTb     = 4,    // nothing -> blue
    OE_BLITZ        = 5,    // lightning
};



class CCamera
{
public:
    CCamera(CInstanceManager* iMan);
    ~CCamera();

    BOOL        EventProcess(const Event &event);

    void        Init(D3DVECTOR eye, D3DVECTOR lookat, float delay);

    void        SetObject(CObject* object);
    CObject*    RetObject();

    void        SetType(CameraType type);
    CameraType  RetType();

    void        SetSmooth(CameraSmooth type);
    CameraSmooth RetSmoth();

    void        SetDist(float dist);
    float       RetDist();

    void        SetFixDirection(float angle);
    float       RetFixDirection();

    void        SetRemotePan(float value);
    float       RetRemotePan();

    void        SetRemoteZoom(float value);
    float       RetRemoteZoom();

    void        StartVisit(D3DVECTOR goal, float dist);
    void        StopVisit();

    void        RetCamera(D3DVECTOR &eye, D3DVECTOR &lookat);

    BOOL        StartCentering(CObject *object, float angleH, float angleV, float dist, float time);
    BOOL        StopCentering(CObject *object, float time);
    void        AbortCentering();

    void        FlushEffect();
    void        StartEffect(CameraEffect effect, D3DVECTOR pos, float force);

    void        FlushOver();
    void        SetOverBaseColor(D3DCOLORVALUE color);
    void        StartOver(OverEffect effect, D3DVECTOR pos, float force);

    void        FixCamera();
    void        SetScriptEye(D3DVECTOR eye);
    void        SetScriptLookat(D3DVECTOR lookat);

    void        SetEffect(BOOL bEnable);
    void        SetCameraScroll(BOOL bScroll);
    void        SetCameraInvertX(BOOL bInvert);
    void        SetCameraInvertY(BOOL bInvert);

    float       RetMotorTurn();
    D3DMouse    RetMouseDef(FPOINT pos);

protected:
    BOOL        EventMouseMove(const Event &event);
    void        EventMouseWheel(int dir);
    BOOL        EventFrame(const Event &event);
    BOOL        EventFrameFree(const Event &event);
    BOOL        EventFrameEdit(const Event &event);
    BOOL        EventFrameDialog(const Event &event);
    BOOL        EventFrameBack(const Event &event);
    BOOL        EventFrameFix(const Event &event);
    BOOL        EventFrameExplo(const Event &event);
    BOOL        EventFrameOnBoard(const Event &event);
    BOOL        EventFrameInfo(const Event &event);
    BOOL        EventFrameVisit(const Event &event);
    BOOL        EventFrameScript(const Event &event);

    void        SetViewTime(const D3DVECTOR &vEyePt, const D3DVECTOR &vLookatPt, float rTime);
    BOOL        IsCollision(D3DVECTOR &eye, D3DVECTOR lookat);
    BOOL        IsCollisionBack(D3DVECTOR &eye, D3DVECTOR lookat);
    BOOL        IsCollisionFix(D3DVECTOR &eye, D3DVECTOR lookat);

    D3DVECTOR   ExcludeTerrain(D3DVECTOR eye, D3DVECTOR lookat, float &angleH, float &angleV);
    D3DVECTOR   ExcludeObject(D3DVECTOR eye, D3DVECTOR lookat, float &angleH, float &angleV);

    void        SetViewParams(const D3DVECTOR &eye, const D3DVECTOR &lookat, const D3DVECTOR &up);
    void        EffectFrame(const Event &event);
    void        OverFrame(const Event &event);

protected:
    CInstanceManager* m_iMan;
    CD3DEngine* m_engine;
    CTerrain*   m_terrain;
    CWater*     m_water;

    CameraType  m_type;         // the type of camera (CAMERA *)
    CameraSmooth    m_smooth;       // type of smoothing
    CObject*    m_cameraObj;        // object linked to the camera

    float       m_eyeDistance;      // distance between the eyes
    float       m_initDelay;        // time of initial centering

    D3DVECTOR   m_actualEye;        // current eye
    D3DVECTOR   m_actualLookat;     // aim current
    D3DVECTOR   m_finalEye;     // final eye
    D3DVECTOR   m_finalLookat;      // aim final
    D3DVECTOR   m_normEye;      // normal eye
    D3DVECTOR   m_normLookat;       // aim normal
    float       m_focus;

    BOOL        m_bRightDown;
    FPOINT      m_rightPosInit;
    FPOINT      m_rightPosCenter;
    FPOINT      m_rightPosMove;

    D3DVECTOR   m_eyePt;        // CAMERA_FREE: eye
    float       m_directionH;       // CAMERA_FREE: horizontal direction
    float       m_directionV;       // CAMERA_FREE: vertical direction
    float       m_heightEye;        // CAMERA_FREE: height above the ground
    float       m_heightLookat;     // CAMERA_FREE: height above the ground
    float       m_speed;        // CAMERA_FREE: speed of movement

    float       m_backDist;     // CAMERA_BACK: distance
    float       m_backMin;      // CAMERA_BACK: distance minimal
    float       m_addDirectionH;    // CAMERA_BACK: additional direction
    float       m_addDirectionV;    // CAMERA_BACK: additional direction
    BOOL        m_bTransparency;

    float       m_fixDist;      // CAMERA_FIX: distance
    float       m_fixDirectionH;    // CAMERA_FIX: direction
    float       m_fixDirectionV;    // CAMERA_FIX: direction

    D3DVECTOR   m_visitGoal;        // CAMERA_VISIT: target position
    float       m_visitDist;        // CAMERA_VISIT: distance
    float       m_visitTime;        // CAMERA_VISIT: relative time
    CameraType  m_visitType;        // CAMERA_VISIT: initial type
    float       m_visitDirectionH;  // CAMERA_VISIT: direction
    float       m_visitDirectionV;  // CAMERA_VISIT: direction

    float       m_editHeight;       // CAMERA_EDIT: height

    float       m_remotePan;
    float       m_remoteZoom;

    FPOINT      m_mousePos;
    float       m_mouseDirH;
    float       m_mouseDirV;
    float       m_mouseMarging;

    float       m_motorTurn;

    CenteringPhase m_centeringPhase;
    float       m_centeringAngleH;
    float       m_centeringAngleV;
    float       m_centeringDist;
    float       m_centeringCurrentH;
    float       m_centeringCurrentV;
    float       m_centeringTime;
    float       m_centeringProgress;

    CameraEffect m_effectType;
    D3DVECTOR   m_effectPos;
    float       m_effectForce;
    float       m_effectProgress;
    D3DVECTOR   m_effectOffset;

    OverEffect  m_overType;
    float       m_overForce;
    float       m_overTime;
    D3DCOLORVALUE m_overColorBase;
    D3DCOLORVALUE m_overColor;
    int         m_overMode;
    float       m_overFadeIn;
    float       m_overFadeOut;

    D3DVECTOR   m_scriptEye;
    D3DVECTOR   m_scriptLookat;

    BOOL        m_bEffect;      // shocks if explosion?
    BOOL        m_bCameraScroll;    // scroll in the edges?
    BOOL        m_bCameraInvertX;   // X inversion in the edges?
    BOOL        m_bCameraInvertY;   // Y inversion in the edges?
};


#endif //_CAMERA_H_
