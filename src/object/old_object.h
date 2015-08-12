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

#include "object/object.h"

#include "object/interface/carrier_object.h"
#include "object/interface/controllable_object.h"
#include "object/interface/interactive_object.h"
#include "object/interface/jostleable_object.h"
#include "object/interface/movable_object.h"
#include "object/interface/power_container_object.h"
#include "object/interface/powered_object.h"
#include "object/interface/programmable_object.h"
#include "object/interface/task_executor_object.h"
#include "object/interface/transportable_object.h"

#include "object/implementation/power_container_impl.h"
#include "object/implementation/programmable_impl.h"
#include "object/implementation/task_executor_impl.h"

// The father of all parts must always be the part number zero!
const int OBJECTMAXPART         = 40;

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

namespace Ui
{
class CObjectInterface;
}


class COldObject : public CObject,
                   public CInteractiveObject,
                   public CTransportableObject,
                   public CTaskExecutorObjectImpl,
                   public CProgrammableObjectImpl,
                   public CJostleableObject,
                   public CCarrierObject,
                   public CPoweredObject,
                   public CMovableObject,
                   public CControllableObject,
                   public CPowerContainerObjectImpl
{
    friend class CObjectFactory;
    friend class CObjectManager;

protected:
    void        DeleteObject(bool bAll=false);
    void        SetPhysics(std::unique_ptr<CPhysics> physics);
    void        SetProgrammable(bool programmable);
    void        SetMotion(std::unique_ptr<CMotion> motion);
    void        SetAuto(std::unique_ptr<CAuto> automat);
    void        SetOption(int option);
    void        SetJostlingSphere(const Math::Sphere& sphere);


public:
    COldObject(int id); // should only be called by CObjectFactory
    ~COldObject();

    void        Simplify() override;
    bool        ExplodeObject(ExplosionType type, float force, float decay=1.0f) override;

    bool EventProcess(const Event& event) override;
    void        UpdateMapping();

    void        DeletePart(int part) override;
    void        SetObjectRank(int part, int objRank);
    int         GetObjectRank(int part) override;
    void        SetObjectParent(int part, int parent);
    void        SetType(ObjectType type) override;
    const char* GetName();
    int         GetOption() override;

    void        Write(CLevelParserLine* line) override;
    void        Read(CLevelParserLine* line) override;

    void        SetDrawFront(bool bDraw) override;

    int         GetShadowLight();
    int         GetEffectLight();

    void        SetShieldRadius(float radius);
    float       GetShieldRadius() override;

    void        SetFloorHeight(float height);
    void        FloorAdjust() override;

    void        SetLinVibration(Math::Vector dir) override;
    Math::Vector    GetLinVibration();
    void        SetCirVibration(Math::Vector dir) override;
    Math::Vector    GetCirVibration();
    void        SetTilt(Math::Vector dir);
    Math::Vector    GetTilt() override;

    void        SetPartPosition(int part, const Math::Vector &pos);
    Math::Vector    GetPartPosition(int part) const;

    void        SetPartRotation(int part, const Math::Vector &angle);
    Math::Vector    GetPartRotation(int part) const;
    void        SetPartRotationY(int part, float angle);
    void        SetPartRotationX(int part, float angle);
    void        SetPartRotationZ(int part, float angle);
    float       GetPartRotationY(int part);
    float       GetPartRotationX(int part);
    float       GetPartRotationZ(int part);

    void        SetPartScale(int part, float zoom);
    void        SetPartScale(int part, Math::Vector zoom);
    Math::Vector    GetPartScale(int part) const;
    void        SetPartScaleX(int part, float zoom);
    float       GetPartScaleX(int part);
    void        SetPartScaleY(int part, float zoom);
    float       GetPartScaleY(int part);
    void        SetPartScaleZ(int part, float zoom);
    float       GetPartScaleZ(int part);

    void        SetTrainer(bool bEnable) override;
    bool        GetTrainer() override;

    void        SetToy(bool bEnable);
    bool        GetToy();

    void        SetManual(bool bManual);
    bool        GetManual();

    void        SetMasterParticle(int part, int parti) override;

    void        SetPower(CObject* power) override;
    CObject*    GetPower() override;
    void        SetCargo(CObject* cargo) override;
    CObject*    GetCargo() override;
    void        SetTransporter(CObject* transporter) override;
    CObject*    GetTransporter() override;
    void        SetTransporterPart(int part) override;

    Math::Matrix*   GetRotateMatrix(int part);
    Math::Matrix*   GetWorldMatrix(int part) override;

    void        SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                                Math::Vector &lookat, Math::Vector &upVec,
                                Gfx::CameraType type) override;

    Character*  GetCharacter() override;

    float       GetAbsTime();

    float       GetCapacity() override;

    bool        IsRechargeable() override;

    void        SetShield(float level) override;
    float       GetShield() override;

    void        SetRange(float delay) override;
    float       GetRange() override;

    void        SetTransparency(float value) override;

    void        SetFixed(bool bFixed) override;
    bool        GetFixed() override;

    Math::Sphere GetJostlingSphere() const override;
    bool        JostleObject(float force) override;

    void        SetVirusMode(bool bEnable) override;
    bool        GetVirusMode() override;

    void        SetCameraType(Gfx::CameraType type) override;
    Gfx::CameraType  GetCameraType() override;
    void        SetCameraDist(float dist) override;
    float       GetCameraDist() override;
    void        SetCameraLock(bool lock) override;
    bool        GetCameraLock() override;

    void        SetHighlight(bool highlight) override;

    void        SetSelect(bool select, bool bDisplayError = true) override;
    bool        GetSelect() override;

    void        SetSelectable(bool bMode);
    bool        GetSelectable() override;

    void        SetVisible(bool bVisible);

    void        SetCheckToken(bool bMode);
    bool        GetCheckToken();

    void        SetMagnifyDamage(float factor) override;
    float       GetMagnifyDamage() override;

    void        SetParam(float value) override;
    float       GetParam() override;

    void        SetExploding(bool bExplo) override;
    bool        IsExploding() override;
    void        SetBurn(bool bBurn) override;
    bool        GetBurn() override;
    void        SetDead(bool bDead) override;
    bool        GetDead() override;
    bool        GetRuin() override;
    bool        GetActive() override;

    void        SetGunGoalV(float gunGoal);
    void        SetGunGoalH(float gunGoal);
    float       GetGunGoalV();
    float       GetGunGoalH();

    float       GetShowLimitRadius() override;

    void        CreateSelectParticle();

    CPhysics*   GetPhysics() override;
    CMotion*    GetMotion() override;
    CAuto*      GetAuto() override;

    void        SetDefRank(int rank) override;
    int         GetDefRank() override;

    bool        CreateShadowCircle(float radius, float intensity, Gfx::EngineShadowType type = Gfx::ENG_SHADOW_NORM);
    bool        CreateShadowLight(float height, Gfx::Color color);
    bool        CreateEffectLight(float height, Gfx::Color color);

    void        FlatParent() override;

    void SetInfoReturn(float value);
    float GetInfoReturn() override;

    void SetPosition(const Math::Vector& pos) override;
    Math::Vector GetPosition() const override;

    void SetRotation(const Math::Vector& rotation) override;
    Math::Vector GetRotation() const override;

    using CObject::SetScale; // SetScale(float) version
    void SetScale(const Math::Vector& scale) override;
    Math::Vector GetScale() const override;

    Error       StartTaskTake() override;
    Error       StartTaskManip(TaskManipOrder order, TaskManipArm arm) override;
    Error       StartTaskFlag(TaskFlagOrder order, int rank) override;
    Error       StartTaskBuild(ObjectType type) override;
    Error       StartTaskSearch() override;
    Error       StartTaskDeleteMark() override;
    Error       StartTaskTerraform() override;
    Error       StartTaskRecover() override;
    Error       StartTaskFire(float delay) override;
    Error       StartTaskFireAnt(Math::Vector impact) override;
    Error       StartTaskSpiderExplo() override;
    Error       StartTaskPen(bool down, TraceColor color = TraceColor::Default) override;

    Error       StartTaskWait(float time) override;
    Error       StartTaskAdvance(float length) override;
    Error       StartTaskTurn(float angle) override;
    Error       StartTaskGoto(Math::Vector pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode) override;
    Error       StartTaskInfo(const char *name, float value, float power, bool bSend) override;

    Error       StartTaskShield(TaskShieldMode mode, float delay = 1000.0f) override;
    Error       StartTaskGunGoal(float dirV, float dirH) override;

    void        UpdateInterface() override;

    void        StopProgram() override;

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
    void TransformCameraCollisionSphere(Math::Sphere& collisionSphere) override;

    Error       EndedTask();

protected:
    Gfx::CEngine*       m_engine;
    Gfx::CLightManager* m_lightMan;
    Gfx::CTerrain*      m_terrain;
    Gfx::CCamera*       m_camera;
    Gfx::CParticle*     m_particle;
    CRobotMain*         m_main;
    CSoundInterface*    m_sound;

    std::unique_ptr<CPhysics> m_physics;
    std::unique_ptr<CMotion> m_motion;
    std::unique_ptr<CAuto> m_auto;
    std::unique_ptr<Ui::CObjectInterface> m_objectInterface;

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
    float       m_lastEnergy;
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
    bool        m_bExplo;
    bool        m_bCargo;
    bool        m_bBurn;
    bool        m_bDead;
    bool        m_bFlat;
    bool        m_bTrainer;         // drive vehicle (without remote)
    bool        m_bToy;             // toy key
    bool        m_bManual;          // manual control (Scribbler)
    bool        m_bFixed;
    float       m_gunGoalV;
    float       m_gunGoalH;
    Gfx::CameraType  m_cameraType;
    float       m_cameraDist;
    bool        m_bCameraLock;
    int         m_defRank;
    float       m_magnifyDamage;
    float       m_param;

    Math::Sphere m_jostlingSphere;
    float       m_shieldRadius;

    int         m_totalPart;
    ObjectPart  m_objectPart[OBJECTMAXPART];

    int         m_partiSel[4];

    float       m_infoReturn;

    EventType   m_buttonAxe;

    float       m_time;
    float       m_burnTime;
};
