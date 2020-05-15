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

/**
 * \file graphics/engine/camera.h
 * \brief Camera handling - CCamera class
 */

#pragma once

#include "graphics/engine/engine.h"


class CObject;
class CRobotMain;
class CInput;
struct Event;


// Graphics module namespace
namespace Gfx
{


/**
  \enum CameraType
  \brief Type of camera */
enum CameraType
{
    //! Undefined
    CAM_TYPE_NULL = 0,
    //! Free camera
    CAM_TYPE_FREE,
    //! Camera while editing a program
    CAM_TYPE_EDIT,
    //! Camera on board a robot
    CAM_TYPE_ONBOARD,
    //! Camera behind a robot
    CAM_TYPE_BACK,
    //! Static camera following robot
    CAM_TYPE_FIX,
    //! Camera steady after explosion
    CAM_TYPE_EXPLO,
    //! Camera during a cutscene
    CAM_TYPE_SCRIPT,
    //! Visit camera, rotates around given position
    CAM_TYPE_VISIT,
    //! Static camera height
    CAM_TYPE_PLANE,
};

enum CameraSmooth
{
    //! Sharp
    CAM_SMOOTH_NONE         = 0,
    //! Normal
    CAM_SMOOTH_NORM         = 1,
    //! Hard
    CAM_SMOOTH_HARD         = 2,
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
    //! Hard landing
    CAM_EFFECT_CRASH        = 2,
    //! Explosion
    CAM_EFFECT_EXPLO        = 3,
    //! Shot by an enemy
    CAM_EFFECT_SHOT         = 4,
    //! Vibration during construction
    CAM_EFFECT_VIBRATION    = 5,
    //! Overheated reactor
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

  This class manages everything related to animating the camera in 3D scene.
  Calculated values are then passed to Gfx::CEngine.
*/
class CCamera
{
public:
    CCamera();
    ~CCamera();

    //! Management of an event
    bool        EventProcess(const Event &event);

    /**
     * \brief Initializes the camera
     * \param eye Initial eye position
     * \param lookat Initial lookat position
     * \param delay Time of the initial entry animation
     */
    void        Init(Math::Vector eye, Math::Vector lookat, float delay);

    //! Sets the object controlling the camera
    void        SetControllingObject(CObject* object);
    //! Gets the object controlling the camera
    CObject*    GetControllingObject();

    //! Change the type of camera
    void        SetType(CameraType type);
    //! Get the type of the camera
    CameraType  GetType();

    //! Set smoothing mode
    void         SetSmooth(CameraSmooth type);
    //! Get smoothing mode
    CameraSmooth GetSmooth();


    //! Returns the current point of view of the camera
    void        GetCamera(Math::Vector &eye, Math::Vector &lookat);

    //! \name Visit camera management (CAM_TYPE_VISIT) - camera in this mode shows a position, constantly rotating around it
    //@{
    //! Start visit camera
    void        StartVisit(Math::Vector goal, float dist);
    //! Stop visit camera
    void        StopVisit();
    //@}

    //! \name Camera "centering" - moves the camera to show some happening action (e.g. sniffer sniffing)
    //@{
    //! Move camera to show happening action
    bool        StartCentering(CObject *object, float angleH, float angleV, float dist, float time);
    //! Go back to normal position after showing some happening action
    bool        StopCentering(CObject *object, float time);
    //! Abort centering animation in the current position
    void        AbortCentering();
    //@}

    //! \name Camera shake effects
    //@{
    //! Starts a camera shake effect
    void        StartEffect(CameraEffect effect, Math::Vector pos, float force);
    //! Removes the camera shake effect
    void        FlushEffect();
    //@}

    //! \name Camera overlay effects
    //@{
    //! Starts camera overlay effect
    void        StartOver(CameraOverEffect effect, Math::Vector pos, float force);
    //! Removes camera overlay effect
    void        FlushOver();
    //! Specifies camera overlay effect base color
    void        SetOverBaseColor(Color color);
    //@}

    //! \name Script camera - cutscenes controlled by external code
    //@{
    //! Script camera: Set camera position
    void        SetScriptCamera(Math::Vector eye, Math::Vector lookat);
    //! Script camera: Animate to given camera position
    void        SetScriptCameraAnimate(Math::Vector eye, Math::Vector lookat);
    //! Script camera: Animate to given eye position
    void        SetScriptCameraAnimateEye(Math::Vector eye);
    //! Script camera: Animate to given lookat position
    void        SetScriptCameraAnimateLookat(Math::Vector lookat);
    //@}

    //! \name Configuration settings
    //@{
    void        SetEffect(bool enable);
    bool        GetEffect();
    void        SetBlood(bool enable);
    bool        GetBlood();
    void        SetOldCameraScroll(bool scroll);
    bool        GetOldCameraScroll();
    void        SetCameraInvertX(bool invert);
    bool        GetCameraInvertX();
    void        SetCameraInvertY(bool invert);
    bool        GetCameraInvertY();
    //@}

    //! Temporarily freeze camera movement
    void        SetFreeze(bool freeze);

    //! Set camera speed
    void        SetCameraSpeed(float speed);

protected:
    //! Advances the effect of the camera
    void        EffectFrame(const Event &event);
    //! Advanced overlay effect in the foreground
    void        OverFrame(const Event &event);

    bool        EventFrameFree(const Event &event, bool keysAllowed);
    bool        EventFrameBack(const Event &event);
    bool        EventFrameFix(const Event &event);
    bool        EventFrameExplo(const Event &event);
    bool        EventFrameOnBoard(const Event &event);
    bool        EventFrameVisit(const Event &event);
    bool        EventFrameScript(const Event &event);

    /**
     * \brief Calculates camera animation and sends updated camera position to the 3D engine
     * \param eyePt Eye point
     * \param lookatPt Lookat point
     * \param rTime Time since last time this function was called (used to calculate animation)
     * \see SetViewParams
     */
    void        UpdateCameraAnimation(const Math::Vector &eyePt, const Math::Vector &lookatPt, float rTime);

    /**
     * \brief Avoid the obstacles
     *
     * For CAM_TYPE_BACK: make obstacles transparent
     * For CAM_TYPE_FIX or CAM_TYPE_PLANE: adjust eye not to hit the obstacles
     *
     * \param eye Eye position, may be adjusted
     * \param lookat Lookat point
     */
    void        IsCollision(Math::Vector &eye, Math::Vector lookat);
    //! Avoid the obstacles (CAM_TYPE_BACK)
    void        IsCollisionBack();
    //! Avoid the obstacles (CAM_TYPE_FIX or CAM_TYPE_PLANE)
    void        IsCollisionFix(Math::Vector &eye, Math::Vector lookat);

    //! Adjusts the camera not to enter the ground
    Math::Vector ExcludeTerrain(Math::Vector eye, Math::Vector lookat, float &angleH, float &angleV);
    //! Adjusts the camera not to enter an object
    Math::Vector ExcludeObject(Math::Vector eye, Math::Vector lookat, float &angleH, float &angleV);

    /**
     * \brief Updates the location and direction of the camera in the 3D engine
     * \param eye Eye point
     * \param lookat Lookat point
     * \param up Up vector
     * \see CEngine::SetViewParams
     */
    void        SetViewParams(const Math::Vector &eye, const Math::Vector &lookat, const Math::Vector &up = Math::Vector(0.0f, 1.0f, 0.0f));

    /**
     * \brief Calculate camera movement (from user inputs) to apply
     * \return Math::Vector where x, y represent respectively horizontal and vertical angle change in radians and z represents zoom (distance change)
     * \remarks Should not be called more often than once every EVENT_FRAME
     **/
    Math::Vector CalculateCameraMovement(const Event &event, bool keysAllowed = true);

protected:
    CEngine*     m_engine;
    CRobotMain*  m_main;
    CTerrain*    m_terrain;
    CWater*      m_water;
    CInput*      m_input;

    //! The type of camera
    CameraType   m_type;
    //! Type of smoothing
    CameraSmooth m_smooth;
    //! Object linked to the camera
    CObject*     m_cameraObj;

    //! Remaining time of initial camera entry animation
    float        m_initDelay;

    //! Current eye
    Math::Vector    m_actualEye;
    //! Current aim
    Math::Vector    m_actualLookat;
    //! Final eye
    Math::Vector    m_finalEye;
    //! Final lookat
    Math::Vector    m_finalLookat;
    //! Eye position at the moment of entering CAM_TYPE_INFO/CAM_TYPE_VISIT
    Math::Vector    m_prevEye;
    //! Lookat position at the moment of entering CAM_TYPE_INFO/CAM_TYPE_VISIT
    Math::Vector    m_prevLookat;

    float            m_focus;

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
    //! CAM_TYPE_BACK: minimal distance
    float       m_backMin;
    //! CAM_TYPE_BACK: additional horizontal direction
    float       m_addDirectionH;
    //! CAM_TYPE_BACK: additional vertical direction
    float       m_addDirectionV;

    //! CAM_TYPE_FIX: distance
    float       m_fixDist;
    //! CAM_TYPE_FIX: horizontal direction
    float       m_fixDirectionH;
    //! CAM_TYPE_FIX: vertical direction
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
    float        m_visitDirectionV;

    //! Last known mouse position, used to calculate change since last frame
    Math::Point  m_mousePos = Math::Point(0.5f, 0.5f);
    //! Change of mouse position since last frame
    Math::Point  m_mouseDelta = Math::Point(0.0f, 0.0f);
    //! Change of camera position caused by edge camera
    Math::Point  m_mouseDeltaEdge = Math::Point(0.0f, 0.0f);
    //! Change of mouse wheel since last frame
    float        m_mouseWheelDelta = 0.0f;

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

    //! Is camera frozen?
    bool m_freeze = false;

    //! \name Configuration settings
    //@{
    bool        m_effect;
    bool        m_blood;
    bool        m_oldCameraScroll;
    bool        m_cameraInvertX;
    bool        m_cameraInvertY;
    //@}
};


} // namespace Gfx
