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
 * \file object/old_object.h
 * \brief COldObject - legacy CObject code
 */

#pragma once

#include "object/interactive_object.h"
#include "object/object.h"

// The father of all parts must always be the part number zero!
const int OBJECTMAXPART         = 40;
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


class COldObject : public CObject, public CInteractiveObject
{
    friend class CObjectFactory;
    friend class CObjectManager;

protected:

    COldObject(int id);

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
    ~COldObject();

    void        Simplify() override;
    bool        ExplodeObject(ExplosionType type, float force, float decay=1.0f) override;

    bool EventProcess(const Event& event) override;
    void        UpdateMapping() override;

    void        DeletePart(int part) override;
    void        SetObjectRank(int part, int objRank) override;
    int         GetObjectRank(int part) override;
    void        SetObjectParent(int part, int parent) override;
    void        SetType(ObjectType type) override;
    const char* GetName() override;
    int         GetOption() override;

    void        Write(CLevelParserLine* line) override;
    void        Read(CLevelParserLine* line) override;

    void        SetDrawWorld(bool bDraw) override;
    void        SetDrawFront(bool bDraw) override;

    bool        ReadProgram(Program* program, const char* filename) override;
    bool        WriteProgram(Program* program, const char* filename) override;

    int         GetShadowLight() override;
    int         GetEffectLight() override;

    void        SetGlobalSphere(Math::Vector pos, float radius) override;
    void        GetGlobalSphere(Math::Vector &pos, float &radius) override;
    void        GetJostlingSphere(Math::Vector &pos, float &radius) override;
    void        SetShieldRadius(float radius) override;
    float       GetShieldRadius() override;

    void        SetFloorHeight(float height) override;
    void        FloorAdjust() override;

    void        SetLinVibration(Math::Vector dir) override;
    Math::Vector    GetLinVibration() override;
    void        SetCirVibration(Math::Vector dir) override;
    Math::Vector    GetCirVibration() override;
    void        SetTilt(Math::Vector dir) override;
    Math::Vector    GetTilt() override;

    void        SetPosition(int part, const Math::Vector &pos) override;
    Math::Vector    GetPosition(int part) override;
    void        SetAngle(int part, const Math::Vector &angle) override;
    Math::Vector    GetAngle(int part) override;
    void        SetAngleY(int part, float angle) override;
    void        SetAngleX(int part, float angle) override;
    void        SetAngleZ(int part, float angle) override;
    float       GetAngleY(int part) override;
    float       GetAngleX(int part) override;
    float       GetAngleZ(int part) override;
    void        SetZoom(int part, float zoom) override;
    void        SetZoom(int part, Math::Vector zoom) override;
    Math::Vector    GetZoom(int part) override;
    void        SetZoomX(int part, float zoom) override;
    float       GetZoomX(int part) override;
    void        SetZoomY(int part, float zoom) override;
    float       GetZoomY(int part) override;
    void        SetZoomZ(int part, float zoom) override;
    float       GetZoomZ(int part) override;

    void        SetTrainer(bool bEnable) override;
    bool        GetTrainer() override;

    void        SetToy(bool bEnable) override;
    bool        GetToy() override;

    void        SetManual(bool bManual) override;
    bool        GetManual() override;

    void        SetResetCap(ResetCap cap) override;
    ResetCap    GetResetCap() override;
    void        SetResetBusy(bool bBusy) override;
    bool        GetResetBusy() override;
    void        SetResetPosition(const Math::Vector &pos) override;
    Math::Vector    GetResetPosition() override;
    void        SetResetAngle(const Math::Vector &angle) override;
    Math::Vector    GetResetAngle() override;
    void        SetResetRun(Program* run) override;
    Program*    GetResetRun() override;

    void        SetMasterParticle(int part, int parti) override;
    int         GetMasterParticle(int part) override;

    void        SetPower(CObject* power) override;
    CObject*    GetPower() override;
    void        SetCargo(CObject* cargo) override;
    CObject*    GetCargo() override;
    void        SetTransporter(CObject* transporter) override;
    CObject*    GetTransporter() override;
    void        SetTransporterPart(int part) override;

    bool        SetCmdLine(int rank, float value) override;
    float       GetCmdLine(int rank) override;

    Math::Matrix*   GetRotateMatrix(int part) override;
    Math::Matrix*   GetWorldMatrix(int part) override;

    void        SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                                Math::Vector &lookat, Math::Vector &upVec,
                                Gfx::CameraType type) override;

    void        GetCharacter(Character* character) override;
    Character*  GetCharacter() override;

    float       GetAbsTime() override;

    void        SetEnergy(float level) override;
    float       GetEnergy() override;

    float       GetCapacity() override;

    void        SetShield(float level) override;
    float       GetShield() override;

    void        SetRange(float delay) override;
    float       GetRange() override;

    void        SetTransparency(float value) override;

    void        SetFixed(bool bFixed) override;
    bool        GetFixed() override;

    void        SetClip(bool bClip) override;
    bool        GetClip() override;

    void        SetTeam(int team) override;
    int         GetTeam() override;

    bool        JostleObject(float force) override;

    void        StartDetectEffect(CObject *target, bool bFound) override;

    void        SetVirusMode(bool bEnable) override;
    bool        GetVirusMode() override;
    float       GetVirusTime() override;

    void        SetCameraType(Gfx::CameraType type) override;
    Gfx::CameraType  GetCameraType() override;
    void        SetCameraDist(float dist) override;
    float       GetCameraDist() override;
    void        SetCameraLock(bool bLock) override;
    bool        GetCameraLock() override;

    void        SetHighlight(bool mode) override;

    void        SetSelect(bool bMode, bool bDisplayError=true) override;
    bool        GetSelect(bool bReal=false) override;

    void        SetSelectable(bool bMode) override;
    bool        GetSelectable() override;

    void        SetActivity(bool bMode) override;
    bool        GetActivity() override;

    void        SetVisible(bool bVisible) override;

    void        SetEnable(bool bEnable) override;
    bool        GetEnable() override;

    void        SetCheckToken(bool bMode) override;
    bool        GetCheckToken() override;

    void        SetProxyActivate(bool bActivate) override;
    bool        GetProxyActivate() override;
    void        SetProxyDistance(float distance) override;

    void        SetMagnifyDamage(float factor) override;
    float       GetMagnifyDamage() override;

    void        SetParam(float value) override;
    float       GetParam() override;
    void        SetIgnoreBuildCheck(bool bIgnoreBuildCheck) override;
    bool        GetIgnoreBuildCheck() override;

    void        SetExploding(bool bExplo) override;
    bool        IsExploding() override;
    void        SetLock(bool bLock) override;
    bool        GetLock() override;
    void        SetSpaceshipCargo(bool bCargo) override;
    bool        IsSpaceshipCargo() override;
    void        SetBurn(bool bBurn) override;
    bool        GetBurn() override;
    void        SetDead(bool bDead) override;
    bool        GetDead() override;
    bool        GetRuin() override;
    bool        GetActive() override;

    void        SetGunGoalV(float gunGoal) override;
    void        SetGunGoalH(float gunGoal) override;
    float       GetGunGoalV() override;
    float       GetGunGoalH() override;

    bool        StartShowLimit() override;
    void        StopShowLimit() override;

    bool        IsProgram() override;
    void        CreateSelectParticle() override;

    void        SetRunScript(CScript* script) override;
    CScript*    GetRunScript() override;
    CBotVar*    GetBotVar() override;
    CPhysics*   GetPhysics() override;
    CBrain*     GetBrain() override;
    CMotion*    GetMotion() override;
    CAuto*      GetAuto() override;

    void        SetDefRank(int rank) override;
    int         GetDefRank() override;

    bool        GetTooltipName(std::string& name) override;

    void        AddDeselList(CObject* pObj) override;
    CObject*    SubDeselList() override;
    void        DeleteDeselList(CObject* pObj) override;

    bool        CreateShadowCircle(float radius, float intensity, Gfx::EngineShadowType type = Gfx::ENG_SHADOW_NORM) override;
    bool        CreateShadowLight(float height, Gfx::Color color) override;
    bool        CreateEffectLight(float height, Gfx::Color color) override;

    void        FlatParent() override;

    void SetInfoReturn(float value) override;
    float GetInfoReturn() override;

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
    void        TransformCrashSphere(Math::Sphere &crashSphere) override;

protected:
    Gfx::CEngine*       m_engine;
    Gfx::CLightManager* m_lightMan;
    Gfx::CTerrain*      m_terrain;
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

    std::string  m_name;         // name of the object
    Character   m_character;            // characteristic
    int     m_option;           // option
    int     m_shadowLight;          // number of light from the shadows
    float       m_shadowHeight;         // height of light from the shadows
    int     m_effectLight;          // number of light effects
    float       m_effectHeight;         // height of light effects
    Math::Vector    m_linVibration;         // linear vibration
    Math::Vector    m_cirVibration;         // circular vibration
    Math::Vector    m_tilt;          // tilt
    CObject*    m_power;            // battery used by the vehicle
    CObject*    m_cargo;             // object transported
    CObject*    m_transporter;            // object with the latter
    int     m_transporterLink;            // part
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
    bool        m_bSelect;          // object selected
    bool        m_bSelectable;          // selectable object
    bool        m_bCheckToken;          // object with audited tokens
    bool        m_bVisible;         // object active but undetectable
    bool        m_bEnable;          // dead object
    bool        m_bProxyActivate;       // active object so close
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
