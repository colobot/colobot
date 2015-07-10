/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file object/old_object_interface.h
 * \brief Legacy CObject interface
 */

#pragma once


#include "graphics/engine/engine.h"
#include "graphics/engine/camera.h"

#include "object/object_type.h"

#include "sound/sound_type.h"

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

class COldObjectInterface
{
public:
    virtual ~COldObjectInterface() {}

    virtual void        Simplify() = 0;
    virtual bool        ExplodeObject(ExplosionType type, float force, float decay=1.0f) = 0;

    virtual void        UpdateMapping() = 0;

    virtual void        DeletePart(int part) = 0;
    virtual void        SetObjectRank(int part, int objRank) = 0;
    virtual int         GetObjectRank(int part) = 0;
    virtual void        SetObjectParent(int part, int parent) = 0;
    virtual void        SetType(ObjectType type) = 0;
    virtual const char* GetName() = 0;
    virtual int         GetOption() = 0;

    virtual void        SetDrawWorld(bool bDraw) = 0;
    virtual void        SetDrawFront(bool bDraw) = 0;

    virtual bool        ReadProgram(Program* program, const char* filename) = 0;
    virtual bool        WriteProgram(Program* program, const char* filename) = 0;

    virtual int         GetShadowLight() = 0;
    virtual int         GetEffectLight() = 0;

    virtual void        SetGlobalSphere(Math::Vector pos, float radius) = 0;
    virtual void        GetGlobalSphere(Math::Vector &pos, float &radius) = 0;
    virtual void        GetJostlingSphere(Math::Vector &pos, float &radius) = 0;
    virtual void        SetShieldRadius(float radius) = 0;
    virtual float       GetShieldRadius() = 0;

    virtual void        SetFloorHeight(float height) = 0;
    virtual void        FloorAdjust() = 0;

    virtual void        SetLinVibration(Math::Vector dir) = 0;
    virtual Math::Vector    GetLinVibration() = 0;
    virtual void        SetCirVibration(Math::Vector dir) = 0;
    virtual Math::Vector    GetCirVibration() = 0;
    virtual void        SetTilt(Math::Vector dir) = 0;
    virtual Math::Vector    GetTilt() = 0;

    virtual void        SetPosition(int part, const Math::Vector &pos) = 0;
    virtual Math::Vector    GetPosition(int part) = 0;
    virtual void        SetAngle(int part, const Math::Vector &angle) = 0;
    virtual Math::Vector    GetAngle(int part) = 0;
    virtual void        SetAngleY(int part, float angle) = 0;
    virtual void        SetAngleX(int part, float angle) = 0;
    virtual void        SetAngleZ(int part, float angle) = 0;
    virtual float       GetAngleY(int part) = 0;
    virtual float       GetAngleX(int part) = 0;
    virtual float       GetAngleZ(int part) = 0;
    virtual void        SetZoom(int part, float zoom) = 0;
    virtual void        SetZoom(int part, Math::Vector zoom) = 0;
    virtual Math::Vector    GetZoom(int part) = 0;
    virtual void        SetZoomX(int part, float zoom) = 0;
    virtual float       GetZoomX(int part) = 0;
    virtual void        SetZoomY(int part, float zoom) = 0;
    virtual float       GetZoomY(int part) = 0;
    virtual void        SetZoomZ(int part, float zoom) = 0;
    virtual float       GetZoomZ(int part) = 0;

    virtual void        SetTrainer(bool bEnable) = 0;
    virtual bool        GetTrainer() = 0;

    virtual void        SetToy(bool bEnable) = 0;
    virtual bool        GetToy() = 0;

    virtual void        SetManual(bool bManual) = 0;
    virtual bool        GetManual() = 0;

    virtual void        SetResetCap(ResetCap cap) = 0;
    virtual ResetCap    GetResetCap() = 0;
    virtual void        SetResetBusy(bool bBusy) = 0;
    virtual bool        GetResetBusy() = 0;
    virtual void        SetResetPosition(const Math::Vector &pos) = 0;
    virtual Math::Vector    GetResetPosition() = 0;
    virtual void        SetResetAngle(const Math::Vector &angle) = 0;
    virtual Math::Vector    GetResetAngle() = 0;
    virtual void        SetResetRun(Program* run) = 0;
    virtual Program*    GetResetRun() = 0;

    virtual void        SetMasterParticle(int part, int parti) = 0;
    virtual int         GetMasterParticle(int part) = 0;

    virtual void        SetPower(CObject* power) = 0;
    virtual CObject*    GetPower() = 0;
    virtual void        SetCargo(CObject* cargo) = 0;
    virtual CObject*    GetCargo() = 0;
    virtual void        SetTransporter(CObject* transporter) = 0;
    virtual CObject*    GetTransporter() = 0;
    virtual void        SetTransporterPart(int part) = 0;

    virtual void        SetCmdLine(unsigned int rank, float value) = 0;
    virtual float       GetCmdLine(unsigned int rank) = 0;

    virtual Math::Matrix*   GetRotateMatrix(int part) = 0;
    virtual Math::Matrix*   GetWorldMatrix(int part) = 0;

    virtual void        SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                                Math::Vector &lookat, Math::Vector &upVec,
                                Gfx::CameraType type) = 0;

    virtual void        GetCharacter(Character* character) = 0;
    virtual Character*  GetCharacter() = 0;

    virtual float       GetAbsTime() = 0;

    virtual void        SetEnergy(float level) = 0;
    virtual float       GetEnergy() = 0;

    virtual float       GetCapacity() = 0;

    virtual void        SetShield(float level) = 0;
    virtual float       GetShield() = 0;

    virtual void        SetRange(float delay) = 0;
    virtual float       GetRange() = 0;

    virtual void        SetTransparency(float value) = 0;

    virtual void        SetFixed(bool bFixed) = 0;
    virtual bool        GetFixed() = 0;

    virtual void        SetClip(bool bClip) = 0;
    virtual bool        GetClip() = 0;

    virtual void        SetTeam(int team) = 0;
    virtual int         GetTeam() = 0;

    virtual bool        JostleObject(float force) = 0;

    virtual void        StartDetectEffect(CObject *target, bool bFound) = 0;

    virtual void        SetVirusMode(bool bEnable) = 0;
    virtual bool        GetVirusMode() = 0;
    virtual float       GetVirusTime() = 0;

    virtual void        SetCameraType(Gfx::CameraType type) = 0;
    virtual Gfx::CameraType  GetCameraType() = 0;
    virtual void        SetCameraDist(float dist) = 0;
    virtual float       GetCameraDist() = 0;
    virtual void        SetCameraLock(bool bLock) = 0;
    virtual bool        GetCameraLock() = 0;

    virtual void        SetHighlight(bool mode) = 0;

    virtual void        SetSelect(bool bMode, bool bDisplayError=true) = 0;
    virtual bool        GetSelect(bool bReal=false) = 0;

    virtual void        SetSelectable(bool bMode) = 0;
    virtual bool        GetSelectable() = 0;

    virtual void        SetActivity(bool bMode) = 0;
    virtual bool        GetActivity() = 0;

    virtual void        SetVisible(bool bVisible) = 0;

    virtual void        SetEnable(bool bEnable) = 0;
    virtual bool        GetEnable() = 0;

    virtual void        SetCheckToken(bool bMode) = 0;
    virtual bool        GetCheckToken() = 0;

    virtual void        SetProxyActivate(bool bActivate) = 0;
    virtual bool        GetProxyActivate() = 0;
    virtual void        SetProxyDistance(float distance) = 0;

    virtual void        SetMagnifyDamage(float factor) = 0;
    virtual float       GetMagnifyDamage() = 0;

    virtual void        SetParam(float value) = 0;
    virtual float       GetParam() = 0;
    virtual void        SetIgnoreBuildCheck(bool bIgnoreBuildCheck) = 0;
    virtual bool        GetIgnoreBuildCheck() = 0;

    virtual void        SetExploding(bool bExplo) = 0;
    virtual bool        IsExploding() = 0;
    virtual void        SetLock(bool bLock) = 0;
    virtual bool        GetLock() = 0;
    virtual void        SetSpaceshipCargo(bool bCargo) = 0;
    virtual bool        IsSpaceshipCargo() = 0;
    virtual void        SetBurn(bool bBurn) = 0;
    virtual bool        GetBurn() = 0;
    virtual void        SetDead(bool bDead) = 0;
    virtual bool        GetDead() = 0;
    virtual bool        GetRuin() = 0;
    virtual bool        GetActive() = 0;

    virtual void        SetGunGoalV(float gunGoal) = 0;
    virtual void        SetGunGoalH(float gunGoal) = 0;
    virtual float       GetGunGoalV() = 0;
    virtual float       GetGunGoalH() = 0;

    virtual bool        StartShowLimit() = 0;
    virtual void        StopShowLimit() = 0;

    virtual bool        IsProgram() = 0;
    virtual void        CreateSelectParticle() = 0;

    virtual void        SetRunScript(CScript* script) = 0;
    virtual CScript*    GetRunScript() = 0;
    virtual CBotVar*    GetBotVar() = 0;
    virtual CPhysics*   GetPhysics() = 0;
    virtual CBrain*     GetBrain() = 0;
    virtual CMotion*    GetMotion() = 0;
    virtual CAuto*      GetAuto() = 0;

    virtual void        SetDefRank(int rank) = 0;
    virtual int         GetDefRank() = 0;

    virtual bool        GetTooltipName(std::string& name) = 0;

    virtual void        AddDeselList(CObject* pObj) = 0;
    virtual CObject*    SubDeselList() = 0;
    virtual void        DeleteDeselList(CObject* pObj) = 0;

    virtual bool        CreateShadowCircle(float radius, float intensity, Gfx::EngineShadowType type = Gfx::ENG_SHADOW_NORM) = 0;
    virtual bool        CreateShadowLight(float height, Gfx::Color color) = 0;
    virtual bool        CreateEffectLight(float height, Gfx::Color color) = 0;

    virtual void        FlatParent() = 0;

    virtual void SetInfoReturn(float value) = 0;
    virtual float GetInfoReturn() = 0;
};

