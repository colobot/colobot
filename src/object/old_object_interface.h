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

#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"

#include "object/object_type.h"

#include "sound/sound_type.h"

#include <string>

class CPhysics;
class CMotion;
class CAuto;
class CBotVar;
class CScript;


struct Character
{
    float        wheelFront = 0.0f;     // position X of the front wheels
    float        wheelBack = 0.0f;      // position X of the back wheels
    float        wheelLeft = 0.0f;      // position Z of the left wheels
    float        wheelRight = 0.0f;     // position Z of the right wheels
    float        height = 0.0f;         // normal height on top of ground
    Math::Vector posPower;       // position of the battery
};

enum class ExplosionType
{
    Bang  = 1,
    Burn  = 2,
    Water = 3,
};

class COldObjectInterface
{
public:
    virtual ~COldObjectInterface() {}

    virtual void        Simplify();
    virtual bool        ExplodeObject(ExplosionType type, float force = 1.0f);
    virtual void        DeletePart(int part);
    virtual void        SetType(ObjectType type);

    virtual int         GetObjectRank(int part);

    virtual int         GetOption();

    virtual void        SetDrawFront(bool bDraw);


    virtual void        FloorAdjust();

    virtual void        SetLinVibration(Math::Vector dir);
    virtual void        SetCirVibration(Math::Vector dir);

    virtual Math::Vector    GetTilt();

    virtual void        SetMasterParticle(int part, int parti);

    virtual Math::Matrix*   GetWorldMatrix(int part);

    virtual void        SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                                Math::Vector &lookat, Math::Vector &upVec,
                                Gfx::CameraType type);

    virtual Character*  GetCharacter();

    virtual void        FlatParent();

    // This goes to CBaseAlien or something like that
    virtual void        SetFixed(bool bFixed);
    virtual bool        GetFixed();

    // Not sure. Maybe a separate interface, or maybe CControllableObject (buildings can have viruses too)
    virtual void        SetVirusMode(bool bEnable);
    virtual bool        GetVirusMode();

    // These go to Shielder subclass
    //! Shielder radius (only while active) [0 or RADIUS_SHIELD_MIN..RADIUS_SHIELD_MAX]
    virtual float       GetShieldRadius();
    //! Shielder radius [0..1]
    //@{
    virtual void        SetParam(float value);
    virtual float       GetParam();
    //@}

    // TODO: What to do with these?
    virtual void        SetExploding(bool bExplo);
    virtual bool        IsExploding();
    virtual void        SetBurn(bool bBurn);
    virtual bool        GetBurn();
    virtual void        SetDead(bool bDead);
    virtual bool        GetDead();
    virtual bool        GetRuin();
    virtual bool        GetActive();

    // This will be eventually removed after refactoring to subclasses
    virtual CAuto*      GetAuto();

    // TODO: We'll see if this is still needed after I refactor program storage later
    virtual void        SetDefRank(int rank);
    virtual int         GetDefRank();

    // CProgrammableObject or refactor
    virtual float GetInfoReturn();
};
