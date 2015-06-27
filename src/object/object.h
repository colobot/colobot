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
 * \file object/object.h
 * \brief CObject - base class for all game objects
 */

#pragma once


#include "graphics/engine/engine.h"
#include "graphics/engine/camera.h"

#include "object/object_type.h"

#include "sound/sound.h"

#include <string>

class CApplication;
class CPhysics;
class CBrain;
class CMotion;
class CAuto;
class CDisplayText;
class CRobotMain;
class CBotVar;
class CScript;
class CLevelParserLine;
struct Program;

// The father of all parts must always be the part number zero!
const int OBJECTMAXPART         = 40;
const int MAXCRASHSPHERE        = 40;
const int OBJECTMAXDESELLIST    = 10;
const int OBJECTMAXCMDLINE      = 20;

struct ObjectPart
{
    bool         bUsed;
    int          object;         // number of the object in CEngine
    int          parentPart;     // number of father part
    int          masterParti;        // master canal of the particle
    Math::Vector position;
    Math::Vector angle;
    Math::Vector zoom;
    bool         bTranslate;
    bool         bRotate;
    bool         bZoom;
    Math::Matrix matTranslate;
    Math::Matrix matRotate;
    Math::Matrix matTransform;
    Math::Matrix matWorld;
};

struct Character
{
    float        wheelFront;     // position X of the front wheels
    float        wheelBack;      // position X of the back wheels
    float        wheelLeft;      // position Z of the left wheels
    float        wheelRight;     // position Z of the right wheels
    float        height;         // normal height on top of ground
    Math::Vector posPower;       // position of the battery
};

enum class ExplosionType
{
    Bang  = 1,
    Burn  = 2,
    Water = 3,
};

enum ResetCap
{
    RESET_NONE   = 0,
    RESET_MOVE   = 1,
    RESET_DELETE = 2,
};




class CObject
{
    friend class CObjectFactory;
    friend class CObjectManager;

protected:
    CObject(int id);

    void        DeleteObject(bool bAll=false);
    void        SetPhysics(std::unique_ptr<CPhysics> physics);
    void        SetBrain(std::unique_ptr<CBrain> brain);
    void        SetMotion(std::unique_ptr<CMotion> motion);
    void        SetAuto(std::unique_ptr<CAuto> automat);
    void        SetShowLimitRadius(float radius);
    void        SetCapacity(float capacity);
    float       GetProxyDistance();
    void        SetOption(int option);
    void        SetJostlingSphere(Math::Vector pos, float radius);

public:
    CObject(const CObject&) = delete;
    CObject& operator=(const CObject&) = delete;

    virtual ~CObject();

    void        Simplify();
    bool        ExplodeObject(ExplosionType type, float force, float decay=1.0f);

    bool        EventProcess(const Event &event);
    void        UpdateMapping();

    int         CreatePart();
    void        DeletePart(int part);
    void        SetObjectRank(int part, int objRank);
    int         GetObjectRank(int part);
    void        SetObjectParent(int part, int parent);
    ObjectType  GetType();
    void        SetType(ObjectType type);
    const char* GetName();
    int         GetOption();

    int         GetID();

    virtual void Write(CLevelParserLine* line);
    virtual void Read(CLevelParserLine* line);

    void        SetDrawWorld(bool bDraw);
    void        SetDrawFront(bool bDraw);

    bool        ReadProgram(Program* program, const char* filename);
    bool        WriteProgram(Program* program, const char* filename);

    int         GetShadowLight();
    int         GetEffectLight();

    void        FlushCrashShere();
    int         CreateCrashSphere(Math::Vector pos, float radius, Sound sound, float hardness=0.45f);
    int         GetCrashSphereTotal();
    bool        GetCrashSphere(int rank, Math::Vector &pos, float &radius);
    float       GetCrashSphereHardness(int rank);
    Sound       GetCrashSphereSound(int rank);
    void        DeleteCrashSphere(int rank);
    void        SetGlobalSphere(Math::Vector pos, float radius);
    void        GetGlobalSphere(Math::Vector &pos, float &radius);
    void        GetJostlingSphere(Math::Vector &pos, float &radius);
    void        SetShieldRadius(float radius);
    float       GetShieldRadius();

    void        SetFloorHeight(float height);
    void        FloorAdjust();

    void        SetLinVibration(Math::Vector dir);
    Math::Vector    GetLinVibration();
    void        SetCirVibration(Math::Vector dir);
    Math::Vector    GetCirVibration();
    void        SetTilt(Math::Vector dir);
    Math::Vector    GetTilt();

    void        SetPosition(int part, const Math::Vector &pos);
    Math::Vector    GetPosition(int part);
    void        SetAngle(int part, const Math::Vector &angle);
    Math::Vector    GetAngle(int part);
    void        SetAngleY(int part, float angle);
    void        SetAngleX(int part, float angle);
    void        SetAngleZ(int part, float angle);
    float       GetAngleY(int part);
    float       GetAngleX(int part);
    float       GetAngleZ(int part);
    void        SetZoom(int part, float zoom);
    void        SetZoom(int part, Math::Vector zoom);
    Math::Vector    GetZoom(int part);
    void        SetZoomX(int part, float zoom);
    float       GetZoomX(int part);
    void        SetZoomY(int part, float zoom);
    float       GetZoomY(int part);
    void        SetZoomZ(int part, float zoom);
    float       GetZoomZ(int part);

    float       GetWaterLevel();

    void        SetTrainer(bool bEnable);
    bool        GetTrainer();

    void        SetToy(bool bEnable);
    bool        GetToy();

    void        SetManual(bool bManual);
    bool        GetManual();

    void        SetResetCap(ResetCap cap);
    ResetCap    GetResetCap();
    void        SetResetBusy(bool bBusy);
    bool        GetResetBusy();
    void        SetResetPosition(const Math::Vector &pos);
    Math::Vector    GetResetPosition();
    void        SetResetAngle(const Math::Vector &angle);
    Math::Vector    GetResetAngle();
    void        SetResetRun(Program* run);
    Program*    GetResetRun();

    void        SetMasterParticle(int part, int parti);
    int         GetMasterParticle(int part);

    void        SetPower(CObject* power);
    CObject*    GetPower();
    void        SetFret(CObject* fret);
    CObject*    GetFret();
    void        SetTruck(CObject* truck);
    CObject*    GetTruck();
    void        SetTruckPart(int part);

    bool        SetCmdLine(int rank, float value);
    float       GetCmdLine(int rank);

    Math::Matrix*   GetRotateMatrix(int part);
    Math::Matrix*   GetWorldMatrix(int part);

    void        SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                                Math::Vector &lookat, Math::Vector &upVec,
                                Gfx::CameraType type);

    void        GetCharacter(Character* character);
    Character*  GetCharacter();

    float       GetAbsTime();

    void        SetEnergy(float level);
    float       GetEnergy();

    float       GetCapacity();

    void        SetShield(float level);
    float       GetShield();

    void        SetRange(float delay);
    float       GetRange();

    void        SetTransparency(float value);

    void        SetFixed(bool bFixed);
    bool        GetFixed();

    void        SetClip(bool bClip);
    bool        GetClip();

    void        SetTeam(int team);
    int         GetTeam();

    bool        JostleObject(float force);

    void        StartDetectEffect(CObject *target, bool bFound);

    void        SetVirusMode(bool bEnable);
    bool        GetVirusMode();
    float       GetVirusTime();

    void        SetCameraType(Gfx::CameraType type);
    Gfx::CameraType  GetCameraType();
    void        SetCameraDist(float dist);
    float       GetCameraDist();
    void        SetCameraLock(bool bLock);
    bool        GetCameraLock();

    void        SetHilite(bool bMode);

    void        SetSelect(bool bMode, bool bDisplayError=true);
    bool        GetSelect(bool bReal=false);

    void        SetSelectable(bool bMode);
    bool        GetSelectable();

    void        SetActivity(bool bMode);
    bool        GetActivity();

    void        SetVisible(bool bVisible);

    void        SetEnable(bool bEnable);
    bool        GetEnable();

    void        SetCheckToken(bool bMode);
    bool        GetCheckToken();

    void        SetProxyActivate(bool bActivate);
    bool        GetProxyActivate();
    void        SetProxyDistance(float distance);

    void        SetMagnifyDamage(float factor);
    float       GetMagnifyDamage();

    void        SetParam(float value);
    float       GetParam();

    void        SetIgnoreBuildCheck(bool bIgnoreBuildCheck);
    bool        GetIgnoreBuildCheck();

    void        SetExploding(bool bExplo);
    bool        IsExploding();
    void        SetLock(bool bLock);
    bool        GetLock();
    void        SetCargo(bool bCargo);
    bool        GetCargo();
    void        SetBurn(bool bBurn);
    bool        GetBurn();
    void        SetDead(bool bDead);
    bool        GetDead();
    bool        GetRuin();
    bool        GetActive();

    void        SetGunGoalV(float gunGoal);
    void        SetGunGoalH(float gunGoal);
    float       GetGunGoalV();
    float       GetGunGoalH();

    bool        StartShowLimit();
    void        StopShowLimit();

    bool        IsProgram();
    void        CreateSelectParticle();

    void        SetRunScript(CScript* script);
    CScript*    GetRunScript();
    CBotVar*    GetBotVar();
    CPhysics*   GetPhysics();
    CBrain*     GetBrain();
    CMotion*    GetMotion();
    CAuto*      GetAuto();

    void        SetDefRank(int rank);
    int         GetDefRank();

    bool        GetTooltipName(std::string& name);

    void        AddDeselList(CObject* pObj);
    CObject*    SubDeselList();
    void        DeleteDeselList(CObject* pObj);

    bool        CreateShadowCircle(float radius, float intensity, Gfx::EngineShadowType type = Gfx::ENG_SHADOW_NORM);
    bool        CreateShadowLight(float height, Gfx::Color color);
    bool        CreateEffectLight(float height, Gfx::Color color);

    void        FlatParent();

    // TODO: move to more appropriate place
    //! Set value to be returned by receive() CBOT function
    void SetInfoReturn(float value);
    //! Return value to be returned by receive() CBOT function
    float GetInfoReturn();

    bool        GetTraceDown();
    void        SetTraceDown(bool bDown);
    int         GetTraceColor();
    void        SetTraceColor(int color);
    float       GetTraceWidth();
    void        SetTraceWidth(float width);

protected:
    bool        EventFrame(const Event &event);
    void        VirusFrame(float rTime);
    void        PartiFrame(float rTime);
    void        InitPart(int part);
    void        UpdateTotalPart();
    int         SearchDescendant(int parent, int n);
    void        UpdateEnergyMapping();
    bool        UpdateTransformObject(int part, bool bForceUpdate);
    bool        UpdateTransformObject();
    void        UpdateSelectParticle();

protected:
    CApplication*       m_app;
    Gfx::CEngine*       m_engine;
    Gfx::CLightManager* m_lightMan;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    Gfx::CParticle*     m_particle;
    std::unique_ptr<CPhysics> m_physics;
    std::unique_ptr<CBrain> m_brain;
    std::unique_ptr<CMotion> m_motion;
    std::unique_ptr<CAuto> m_auto;
    CRobotMain*         m_main;
    CSoundInterface*    m_sound;
    CBotVar*            m_botVar;
    CScript*            m_runScript;

    ObjectType  m_type;             // OBJECT_*
    const int     m_id;               // unique identifier
    std::string  m_name;         // name of the object
    Character   m_character;            // characteristic
    int     m_option;           // option
    int     m_partiReactor;         // number of the particle of the reactor
    int     m_shadowLight;          // number of light from the shadows
    float       m_shadowHeight;         // height of light from the shadows
    int     m_effectLight;          // number of light effects
    float       m_effectHeight;         // height of light effects
    Math::Vector    m_linVibration;         // linear vibration
    Math::Vector    m_cirVibration;         // circular vibration
    Math::Vector    m_inclinaison;          // tilt
    CObject*    m_power;            // battery used by the vehicle
    CObject*    m_fret;             // object transported
    CObject*    m_truck;            // object with the latter
    int     m_truckLink;            // part
    float       m_energy;           // energy contained (if battery)
    float       m_lastEnergy;
    float       m_capacity;         // capacity (if battery)
    float       m_shield;           // shield
    float       m_range;            // flight range
    float       m_transparency;         // transparency (0..1)
    float       m_aTime;
    float       m_shotTime;         // time since last shot
    bool        m_bVirusMode;           // virus activated/triggered
    float       m_virusTime;            // lifetime of the virus
    float       m_lastVirusParticle;
    float       m_lastParticle;
    bool        m_bHilite;
    bool        m_bSelect;          // object selected
    bool        m_bSelectable;          // selectable object
    bool        m_bCheckToken;          // object with audited tokens
    bool        m_bVisible;         // object active but undetectable
    bool        m_bEnable;          // dead object
    bool        m_bProxyActivate;       // active object so close
    bool        m_bGadget;          // object nonessential
    bool        m_bLock;
    bool        m_bExplo;
    bool        m_bCargo;
    bool        m_bBurn;
    bool        m_bDead;
    bool        m_bFlat;
    bool        m_bTrainer;         // drive vehicle (without remote)
    bool        m_bToy;             // toy key
    bool        m_bManual;          // manual control (Scribbler)
    bool        m_bIgnoreBuildCheck;
    bool        m_bFixed;
    bool        m_bClip;
    bool        m_bShowLimit;
    float       m_showLimitRadius;
    float       m_gunGoalV;
    float       m_gunGoalH;
    Gfx::CameraType  m_cameraType;
    float       m_cameraDist;
    bool        m_bCameraLock;
    int         m_defRank;
    float       m_magnifyDamage;
    float       m_proxyDistance;
    float       m_param;
    int         m_team;

    int         m_crashSphereUsed;  // number of spheres used
    Math::Vector    m_crashSpherePos[MAXCRASHSPHERE];
    float       m_crashSphereRadius[MAXCRASHSPHERE];
    float       m_crashSphereHardness[MAXCRASHSPHERE];
    Sound       m_crashSphereSound[MAXCRASHSPHERE];
    Math::Vector    m_globalSpherePos;
    float       m_globalSphereRadius;
    Math::Vector    m_jostlingSpherePos;
    float       m_jostlingSphereRadius;
    float       m_shieldRadius;

    int         m_totalPart;
    ObjectPart  m_objectPart[OBJECTMAXPART];

    int         m_totalDesectList;
    CObject*    m_objectDeselectList[OBJECTMAXDESELLIST];

    int         m_partiSel[4];

    ResetCap    m_resetCap;
    bool        m_bResetBusy;
    Math::Vector    m_resetPosition;
    Math::Vector    m_resetAngle;
    Program*        m_resetRun;

    float m_infoReturn;

    float       m_cmdLine[OBJECTMAXCMDLINE];
};

