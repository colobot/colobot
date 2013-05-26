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

/**
 * \file graphics/engine/camera.h
 * \brief Camera handling - CCamera class
 */

#pragma once


#include "common/event.h"

#include "graphics/engine/engine.h"


class CObject;
class CRobotMain;


// Graphics module namespace
namespace Gfx {


/**
  \enum CameraType
  \brief Type of camera */
enum CameraType
{
    //! Undefined
    CAM_TYPE_NULL     = 0,
    //! Free camera (? never in principle ?)
    CAM_TYPE_FREE     = 1,
    //! Camera while editing a program
    CAM_TYPE_EDIT     = 2,
    //! Camera on board a robot
    CAM_TYPE_ONBOARD  = 3,
    //! Camera behind a robot
    CAM_TYPE_BACK     = 4,
    //! Static camera following robot
    CAM_TYPE_FIX      = 5,
    //! Camera steady after explosion
    CAM_TYPE_EXPLO    = 6,
    //! Camera during a film script
    CAM_TYPE_SCRIPT   = 7,
    //! Camera for displaying information
    CAM_TYPE_INFO     = 8,
    //! Visit instead of an error
    CAM_TYPE_VISIT    = 9,
    //! Camera for dialog
    CAM_TYPE_DIALOG   = 10,
    //! Static camera height
    CAM_TYPE_PLANE    = 11,
};

enum CameraSmooth
{
    //! Sharp
    CAM_SMOOTH_NONE         = 0,
    //! Normal
    CAM_SMOOTH_NORM         = 1,
    //! Hard
    CAM_SMOOTH_HARD         = 2,
    //! Special
    CAM_SMOOTH_SPEC         = 3,
};

enum CenteringPhase
{
    CAM_PHASE_NULL         = 0,
    CAM_PHASE_START        = 1,
    CAM_PHASE_WAIT         = 2,
    CAM_PHASE_STOP         = 3,
};

enum CameraEffect
{
    //! No effect
    CAM_EFFECT_NULL         = 0,
    //! Digging in
    CAM_EFFECT_TERRAFORM    = 1,
    //! ? Vehicle driving is severely ?
    CAM_EFFECT_CRASH        = 2,
    //! Explosion
    CAM_EFFECT_EXPLO        = 3,
    //! ? Not mortal shot ?
    CAM_EFFECT_SHOT         = 4,
    //! Vibration during construction
    CAM_EFFECT_VIBRATION    = 5,
    //! ? Spleen reactor ?
    CAM_EFFECT_PET          = 6,
};

enum CameraOverEffect
{
    //! No effect
    CAM_OVER_EFFECT_NULL           = 0,
    //! Flash red
    CAM_OVER_EFFECT_BLOOD          = 1,
    //! White -> nothing
    CAM_OVER_EFFECT_FADEIN_WHITE   = 2,
    //! Nothing -> white
    CAM_OVER_EFFECT_FADEOUT_WHITE  = 3,
    //! Nothing -> blue
    CAM_OVER_EFFECT_FADEOUT_BLACK  = 4,
    //! Lightning
    CAM_OVER_EFFECT_LIGHTNING      = 5,
};


/**
  \class CCamera
  \brief Camera moving in 3D scene

  ... */
class CCamera {
public:
    CCamera();
    ~CCamera();

    //! Management of an event
    bool        EventProcess(const Event &event);

    //! Initializes the camera
    void        Init(Math::Vector eye, Math::Vector lookat, float delay);

    //! Sets the object controlling the camera
    void        SetControllingObject(CObject* object);
    CObject*    GetControllingObject();

    //! Change the type of camera
    void            SetType(CameraType type);
    CameraType GetType();

    //! Management of the smoothing mode
    void              SetSmooth(CameraSmooth type);
    CameraSmooth GetSmoth();

    //! Management of the setback distance
    void        SetDist(float dist);
    float       GetDist();

    //! Manage angle mode CAM_TYPE_FIX
    void        SetFixDirection(float angle);
    float       GetFixDirection();

    //! Managing the triggering mode of the camera panning
    void        SetRemotePan(float value);
    float       GetRemotePan();

    //! Management of the remote zoom (0 .. 1) of the camera
    void        SetRemoteZoom(float value);
    float       GetRemoteZoom();

    //! Start with a tour round the camera
    void        StartVisit(Math::Vector goal, float dist);
    //! Circular end of a visit with the camera
    void        StopVisit();

    //! Returns the point of view of the camera
    void        GetCamera(Math::Vector &eye, Math::Vector &lookat);

    //! Specifies a special movement of camera to frame action
    bool        StartCentering(CObject *object, float angleH, float angleV, float dist, float time);
    //! Ends a special movement of camera to frame action
    bool        StopCentering(CObject *object, float time);
    //! Stop framing special in the current position
    void        AbortCentering();

    //! Removes the special effect with the camera
    void        FlushEffect();
    //! Starts a special effect with the camera
    void        StartEffect(CameraEffect effect, Math::Vector pos, float force);

    //! Removes the effect of superposition in the foreground
    void        FlushOver();
    //! Specifies the base color
    void        SetOverBaseColor(Color color);
    void        StartOver(CameraOverEffect effect, Math::Vector pos, float force);

    //! Sets the soft movement of the camera
    void        FixCamera();
    void        SetScriptEye(Math::Vector eye);
    void        SetScriptLookat(Math::Vector lookat);

    void        SetEffect(bool enable);
    void        SetCameraScroll(bool scroll);
    void        SetCameraInvertX(bool invert);
    void        SetCameraInvertY(bool invert);

    //! Returns an additional force to turn
    float       GetMotorTurn();
    //! Returns the default sprite to use for the mouse
    EngineMouseType GetMouseDef(Math::Point pos);

protected:
    //! Changes the camera according to the mouse moved
    bool        EventMouseMove(const Event &event);
    //! Mouse wheel operation
    void        EventMouseWheel(WheelDirection dir);
    //! Changes the camera according to the time elapsed
    bool        EventFrame(const Event &event);
    //! Moves the point of view
    bool        EventFrameFree(const Event &event);
    //! Moves the point of view
    bool        EventFrameEdit(const Event &event);
    //! Moves the point of view
    bool        EventFrameDialog(const Event &event);
    //! Moves the point of view
    bool        EventFrameBack(const Event &event);
    //! Moves the point of view
    bool        EventFrameFix(const Event &event);
    //! Moves the point of view
    bool        EventFrameExplo(const Event &event);
    //! Moves the point of view
    bool        EventFrameOnBoard(const Event &event);
    //! Moves the point of view
    bool        EventFrameInfo(const Event &event);
    //! Moves the point of view
    bool        EventFrameVisit(const Event &event);
    //! Moves the point of view
    bool        EventFrameScript(const Event &event);

    //! Specifies the location and direction of view to the 3D engine
    void        SetViewTime(const Math::Vector &vEyePt, const Math::Vector &vLookatPt, float rTime);
    //! Avoid the obstacles
    bool        IsCollision(Math::Vector &eye, Math::Vector lookat);
    //! Avoid the obstacles
    bool        IsCollisionBack(Math::Vector &eye, Math::Vector lookat);
    //! Avoid the obstacles
    bool        IsCollisionFix(Math::Vector &eye, Math::Vector lookat);

    //! Adjusts the camera not to enter the ground
    Math::Vector ExcludeTerrain(Math::Vector eye, Math::Vector lookat, float &angleH, float &angleV);
    //! Adjusts the camera not to enter an object
    Math::Vector ExcludeObject(Math::Vector eye, Math::Vector lookat, float &angleH, float &angleV);

    //! Specifies the location and direction of view
    void        SetViewParams(const Math::Vector &eye, const Math::Vector &lookat, const Math::Vector &up);
    //! Advances the effect of the camera
    void        EffectFrame(const Event &event);
    //! Advanced overlay effect in the foreground
    void        OverFrame(const Event &event);

protected:
    CEngine*     m_engine;
    CRobotMain*  m_main;
    CTerrain*    m_terrain;
    CWater*      m_water;

    //! The type of camera
    CameraType   m_type;
    //! Type of smoothing
    CameraSmooth m_smooth;
    //! Object linked to the camera
    CObject*          m_cameraObj;

    //! Distance between the eyes
    float       m_eyeDistance;
    //! Time of initial centering
    float       m_initDelay;

    //! Current eye
    Math::Vector    m_actualEye;
    //! Current aim
    Math::Vector    m_actualLookat;
    //! Final eye
    Math::Vector    m_finalEye;
    //! Final aim
    Math::Vector    m_finalLookat;
    //! Normal eye
    Math::Vector    m_normEye;
    //! Normal aim
    Math::Vector    m_normLookat;

    float       m_focus;

    bool            m_rightDown;
    Math::Point     m_rightPosInit;
    Math::Point     m_rightPosCenter;
    Math::Point     m_rightPosMove;

    //! CAM_TYPE_FREE: eye
    Math::Vector    m_eyePt;
    //! CAM_TYPE_FREE: horizontal direction
    float       m_directionH;
    //! CAM_TYPE_FREE: vertical direction
    float       m_directionV;
    //! CAM_TYPE_FREE: height above the ground
    float       m_heightEye;
    //! CAM_TYPE_FREE: height above the ground
    float       m_heightLookat;
    //! CAM_TYPE_FREE: speed of movement
    float       m_speed;

    //! CAM_TYPE_BACK: distance
    float       m_backDist;
    //! CAM_TYPE_BACK: distance minimal
    float       m_backMin;
    //! CAM_TYPE_BACK: additional direction
    float       m_addDirectionH;
    //! CAM_TYPE_BACK: additional direction
    float       m_addDirectionV;
    bool        m_transparency;

    //! CAM_TYPE_FIX: distance
    float       m_fixDist;
    //! CAM_TYPE_FIX: direction
    float       m_fixDirectionH;
    //! CAM_TYPE_FIX: direction
    float       m_fixDirectionV;

    //! CAM_TYPE_VISIT: target position
    Math::Vector m_visitGoal;
    //! CAM_TYPE_VISIT: distance
    float        m_visitDist;
    //! CAM_TYPE_VISIT: relative time
    float        m_visitTime;
    //! CAM_TYPE_VISIT: initial type
    CameraType   m_visitType;
    //! CAM_TYPE_VISIT: direction
    float        m_visitDirectionH;
    //! CAM_TYPE_VISIT: direction
    float        m_visitDirectionV;

    //! CAM_TYPE_EDIT: height
    float        m_editHeight;

    float        m_remotePan;
    float        m_remoteZoom;

    Math::Point  m_mousePos;
    float        m_mouseDirH;
    float        m_mouseDirV;
    float        m_mouseMarging;

    float        m_motorTurn;

    CenteringPhase m_centeringPhase;
    float       m_centeringAngleH;
    float       m_centeringAngleV;
    float       m_centeringDist;
    float       m_centeringCurrentH;
    float       m_centeringCurrentV;
    float       m_centeringTime;
    float       m_centeringProgress;

    CameraEffect m_effectType;
    Math::Vector m_effectPos;
    float        m_effectForce;
    float        m_effectProgress;
    Math::Vector m_effectOffset;

    CameraOverEffect  m_overType;
    float       m_overForce;
    float       m_overTime;
    Color  m_overColorBase;
    Color  m_overColor;
    int         m_overMode;
    float       m_overFadeIn;
    float       m_overFadeOut;

    Math::Vector m_scriptEye;
    Math::Vector m_scriptLookat;

    //! Shocks if explosion?
    bool        m_effect;
    //! Scroll in the edges?
    bool        m_cameraScroll;
    //! X inversion in the edges?
    bool        m_cameraInvertX;
    //! Y inversion in the edges?
    bool        m_cameraInvertY;

};


} // namespace Gfx

