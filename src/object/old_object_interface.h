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
    RESET_MOVE   = 1
};

class COldObjectInterface
{
public:
    virtual ~COldObjectInterface() {}

    virtual void        Simplify();
    virtual bool        ExplodeObject(ExplosionType type, float force, float decay=1.0f);
    virtual void        DeletePart(int part);
    virtual void        SetType(ObjectType type);

    virtual int         GetObjectRank(int part);

    virtual int         GetOption();

    virtual void        SetDrawFront(bool bDraw);

    virtual float       GetShieldRadius();

    virtual void        FloorAdjust();

    virtual void        SetLinVibration(Math::Vector dir);
    virtual void        SetCirVibration(Math::Vector dir);

    virtual Math::Vector    GetTilt();

    virtual void        SetPosition(int part, const Math::Vector &pos);
    virtual Math::Vector    GetPosition(int part) const;
    virtual void        SetAngle(int part, const Math::Vector &angle);
    virtual Math::Vector    GetAngle(int part) const;
    virtual void        SetAngleY(int part, float angle);
    virtual void        SetAngleX(int part, float angle);
    virtual void        SetAngleZ(int part, float angle);
    virtual float       GetAngleY(int part);
    virtual float       GetAngleX(int part);
    virtual float       GetAngleZ(int part);
    virtual void        SetZoom(int part, float zoom);
    virtual void        SetZoom(int part, Math::Vector zoom);
    virtual Math::Vector    GetZoom(int part) const;
    virtual void        SetZoomX(int part, float zoom);
    virtual float       GetZoomX(int part);
    virtual void        SetZoomY(int part, float zoom);
    virtual float       GetZoomY(int part);
    virtual void        SetZoomZ(int part, float zoom);
    virtual float       GetZoomZ(int part);

    virtual void        SetTrainer(bool bEnable);
    virtual bool        GetTrainer();

    virtual ResetCap    GetResetCap();
    virtual void        SetResetCap(ResetCap resetCap);

    virtual void        SetMasterParticle(int part, int parti);

    virtual float       GetCmdLine(unsigned int rank);

    virtual Math::Matrix*   GetWorldMatrix(int part);

    virtual void        SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                                Math::Vector &lookat, Math::Vector &upVec,
                                Gfx::CameraType type);

    virtual Character*  GetCharacter();

    virtual void        SetEnergy(float level);
    virtual float       GetEnergy();
    virtual float       GetCapacity();

    virtual void        SetShield(float level);
    virtual float       GetShield();

    virtual void        SetRange(float delay);
    virtual float       GetRange();

    virtual void        SetFixed(bool bFixed);
    virtual bool        GetFixed();

    virtual void        SetClip(bool bClip);
    virtual bool        GetClip();
    virtual void        SetTeam(int team);
    virtual int         GetTeam();

    virtual void        StartDetectEffect(CObject *target, bool bFound);

    virtual void        SetVirusMode(bool bEnable);
    virtual bool        GetVirusMode();

    virtual void        SetCameraType(Gfx::CameraType type);
    virtual Gfx::CameraType  GetCameraType();
    virtual void        SetCameraDist(float dist);
    virtual float       GetCameraDist();
    virtual void        SetCameraLock(bool bLock);
    virtual bool        GetCameraLock();

    virtual void        SetHighlight(bool mode);

    virtual void        SetSelect(bool bMode, bool bDisplayError=true);
    virtual bool        GetSelect(bool bReal=false);
    virtual bool        GetSelectable();

    virtual void        SetActivity(bool bMode);
    virtual bool        GetActivity();

    virtual void        SetEnable(bool bEnable);
    virtual bool        GetEnable();

    virtual void        SetProxyActivate(bool bActivate);
    virtual bool        GetProxyActivate();

    virtual void        SetMagnifyDamage(float factor);
    virtual float       GetMagnifyDamage();

    virtual void        SetParam(float value);
    virtual float       GetParam();
    virtual void        SetIgnoreBuildCheck(bool bIgnoreBuildCheck);
    virtual bool        GetIgnoreBuildCheck();

    virtual void        SetExploding(bool bExplo);
    virtual bool        IsExploding();

    virtual void        SetLock(bool bLock);
    virtual bool        GetLock();

    virtual void        SetBurn(bool bBurn);
    virtual bool        GetBurn();

    virtual void        SetDead(bool bDead);
    virtual bool        GetDead();

    virtual bool        GetRuin();

    virtual bool        GetActive();

    virtual bool        StartShowLimit();
    virtual void        StopShowLimit();

    virtual bool        IsProgram();

    virtual CScript*    GetRunScript();
    virtual CBotVar*    GetBotVar();
    virtual CPhysics*   GetPhysics();
    virtual CMotion*    GetMotion();
    virtual CAuto*      GetAuto();

    virtual void        SetDefRank(int rank);
    virtual int         GetDefRank();

    virtual bool        GetTooltipName(std::string& name);

    virtual void        AddDeselList(CObject* pObj);
    virtual CObject*    SubDeselList();
    virtual void        DeleteDeselList(CObject* pObj);

    virtual void        FlatParent();

    virtual float GetInfoReturn();
};

