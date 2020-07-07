/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file object/old_object.h
 * \brief COldObject - legacy CObject code
 */

#pragma once

#include "common/event.h"

#include "object/object.h"

#include "object/implementation/power_container_impl.h"
#include "object/implementation/program_storage_impl.h"
#include "object/implementation/programmable_impl.h"
#include "object/implementation/task_executor_impl.h"

#include "object/interface/carrier_object.h"
#include "object/interface/controllable_object.h"
#include "object/interface/flying_object.h"
#include "object/interface/interactive_object.h"
#include "object/interface/jet_flying_object.h"
#include "object/interface/jostleable_object.h"
#include "object/interface/movable_object.h"
#include "object/interface/power_container_object.h"
#include "object/interface/powered_object.h"
#include "object/interface/programmable_object.h"
#include "object/interface/ranged_object.h"
#include "object/interface/shielded_auto_regen_object.h"
#include "object/interface/task_executor_object.h"
#include "object/interface/trace_drawing_object.h"
#include "object/interface/transportable_object.h"

// The father of all parts must always be the part number zero!
const int OBJECTMAXPART         = 40;

struct ObjectPart
{
    bool         bUsed = false;
    int          object = -1;         // number of the object in CEngine
    int          parentPart = -1;     // number of father part
    int          masterParti = -1;        // master canal of the particle
    Math::Vector position;
    Math::Vector angle;
    Math::Vector zoom;
    bool         bTranslate = false;
    bool         bRotate = false;
    bool         bZoom = false;
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
                   public CProgramStorageObjectImpl,
                   public CProgrammableObjectImpl,
                   public CJostleableObject,
                   public CCarrierObject,
                   public CPoweredObject,
                   public CJetFlyingObject,
                   public CControllableObject,
                   public CPowerContainerObjectImpl,
                   public CRangedObject,
                   public CTraceDrawingObject,
                   public CShieldedAutoRegenObject
{
    friend class CObjectFactory;
    friend class CObjectManager;

protected:
    void        DeleteObject(bool bAll=false);
    void        SetProgrammable();
    void        SetMovable(std::unique_ptr<CMotion> motion, std::unique_ptr<CPhysics> physics);
    void        SetAuto(std::unique_ptr<CAuto> automat);
    void        SetOption(int option);
    void        SetJostlingSphere(const Math::Sphere& jostlingSphere);


public:
    COldObject(int id); // should only be called by CObjectFactory
    ~COldObject();

    void        Simplify() override;

    bool        DamageObject(DamageType type, float force = std::numeric_limits<float>::infinity(), CObject* killer = nullptr) override;
    void        DestroyObject(DestructionType type, CObject* killer = nullptr) override;

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
    Math::Vector GetPowerPosition() override;
    void         SetPowerPosition(const Math::Vector& powerPosition) override;
    void        SetCargo(CObject* cargo) override;
    CObject*    GetCargo() override;
    void        SetTransporter(CObject* transporter) override;
    CObject*    GetTransporter() override;
    void        SetTransporterPart(int part) override;

    Math::Matrix*   GetRotateMatrix(int part);
    Math::Matrix*   GetWorldMatrix(int part) override;

    void        AdjustCamera(Math::Vector &eye, float &dirH, float &dirV,
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

    void        SetReactorRange(float reactorRange) override;
    float       GetReactorRange() override;

    void        SetTransparency(float value) override;

    Math::Sphere GetJostlingSphere() const override;
    bool        JostleObject(float force) override;

    void        SetVirusMode(bool bEnable) override;
    bool        GetVirusMode() override;

    void        SetCameraType(Gfx::CameraType type) override;
    Gfx::CameraType  GetCameraType() override;
    void        SetCameraLock(bool lock) override;
    bool        GetCameraLock() override;

    void        SetHighlight(bool highlight) override;

    void        SetSelect(bool select, bool bDisplayError = true) override;
    bool        GetSelect() override;

    void        SetSelectable(bool bMode);
    bool        GetSelectable() override;

    void        SetUnderground(bool underground);

    void        SetCheckToken(bool bMode);
    bool        GetCheckToken();

    void        SetMagnifyDamage(float factor) override;
    float       GetMagnifyDamage() override;

    void        SetDamaging(bool damaging) override;
    bool        IsDamaging()  override;

    void        SetDying(DeathType deathType) override;
    DeathType   GetDying() override;
    bool        IsDying() override;

    bool        GetActive() override;
    bool        GetDetectable() override;

    void        SetGunGoalV(float gunGoal);
    void        SetGunGoalH(float gunGoal);
    float       GetGunGoalV();
    float       GetGunGoalH();

    float       GetShowLimitRadius() override;

    void        CreateSelectParticle();

    CPhysics*   GetPhysics() override;
    CMotion*    GetMotion() override;
    CAuto*      GetAuto() override;

    bool        CreateShadowCircle(float radius, float intensity, Gfx::EngineShadowType type = Gfx::ENG_SHADOW_NORM);
    bool        CreateShadowLight(float height, Gfx::Color color);

    void        FlatParent() override;

    void SetPosition(const Math::Vector& pos) override;
    Math::Vector GetPosition() const override;

    void SetRotation(const Math::Vector& rotation) override;
    Math::Vector GetRotation() const override;

    using CObject::SetScale; // SetScale(float) version
    void SetScale(const Math::Vector& scale) override;
    Math::Vector GetScale() const override;

    void        UpdateInterface() override;

    void        StopProgram() override;

    bool        GetTraceDown() override;
    void        SetTraceDown(bool down) override;
    TraceColor  GetTraceColor() override;
    void        SetTraceColor(TraceColor color) override;
    float       GetTraceWidth() override;
    void        SetTraceWidth(float width) override;

    bool        IsRepairable() override;
    float       GetShieldFullRegenTime() override;

    float       GetLightningHitProbability() override;

    void        SetBulletWall(bool bulletWall);
    bool        IsBulletWall() override;

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

    /**
     * \brief Check if given object type should be selectable by default
     * \note This is a default value for the selectable= parameter and can still be overriden in the scene file or using the \a selectinsect cheat
     */
    static bool IsSelectableByDefault(ObjectType type);

    /**
     * \brief Check if given object type should have bulletWall enabled by default
     * \note This is a default value for the bulletWall= parameter and can still be overriden in the scene file
     */
    static bool IsBulletWallByDefault(ObjectType type);

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
    Math::Vector    m_linVibration;         // linear vibration
    Math::Vector    m_cirVibration;         // circular vibration
    Math::Vector    m_tilt;          // tilt
    CObject*    m_power;            // battery used by the vehicle
    Math::Vector m_powerPosition;
    CObject*    m_cargo;             // object transported
    CObject*    m_transporter;            // object with the latter
    int     m_transporterLink;            // part
    float       m_lastEnergy;
    float       m_shield;           // shield
    float       m_range;            // flight range
    float       m_aTime;
    float       m_shotTime;         // time since last shot
    bool        m_bVirusMode;           // virus activated/triggered
    float       m_virusTime;            // lifetime of the virus
    float       m_lastVirusParticle;
    bool        m_bSelect;          // object selected
    bool        m_bSelectable;          // selectable object
    bool        m_bCheckToken;          // object with audited tokens
    bool        m_underground;         // object active but undetectable
    bool        m_damaging;
    float       m_damageTime;
    DeathType   m_dying;
    bool        m_bFlat;
    bool        m_bTrainer;         // drive vehicle (without remote)
    bool        m_bToy;             // toy key
    bool        m_bManual;          // manual control (Scribbler)
    float       m_gunGoalV;
    float       m_gunGoalH;
    Gfx::CameraType  m_cameraType;
    bool        m_bCameraLock;
    float       m_magnifyDamage;

    Math::Sphere m_jostlingSphere;
    float       m_shieldRadius;

    int         m_totalPart;
    ObjectPart  m_objectPart[OBJECTMAXPART];

    int         m_partiSel[4];

    EventType   m_buttonAxe;

    float       m_time;
    float       m_burnTime;

    float       m_reactorRange;

    bool        m_traceDown;
    TraceColor  m_traceColor;
    float       m_traceWidth;

    bool        m_bulletWall = false;
};
