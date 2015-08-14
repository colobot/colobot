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

#include "object/old_object_interface.h"

#include <stdexcept>


void COldObjectInterface::Simplify()
{
    throw std::logic_error("Simplify: not implemented!");
}

void COldObjectInterface::DeletePart(int part)
{
    throw std::logic_error("DeletePart: not implemented!");
}

int COldObjectInterface::GetObjectRank(int part)
{
    throw std::logic_error("GetObjectRank: not implemented!");
}

void COldObjectInterface::SetType(ObjectType type)
{
    throw std::logic_error("SetType: not implemented!");
}

int COldObjectInterface::GetOption()
{
    throw std::logic_error("GetOption: not implemented!");
}

void COldObjectInterface::SetDrawFront(bool bDraw)
{
    throw std::logic_error("SetDrawFront: not implemented!");
}

float COldObjectInterface::GetShieldRadius()
{
    throw std::logic_error("GetShieldRadius: not implemented!");
}


void COldObjectInterface::FloorAdjust()
{
    throw std::logic_error("FloorAdjust: not implemented!");
}


void COldObjectInterface::SetLinVibration(Math::Vector dir)
{
    throw std::logic_error("SetLinVibration: not implemented!");
}

void COldObjectInterface::SetCirVibration(Math::Vector dir)
{
    throw std::logic_error("SetCirVibration: not implemented!");
}

Math::Vector COldObjectInterface::GetTilt()
{
    throw std::logic_error("GetTilt: not implemented!");
}

void COldObjectInterface::SetMasterParticle(int part, int parti)
{
    throw std::logic_error("SetMasterParticle: not implemented!");
}

Math::Matrix* COldObjectInterface::GetWorldMatrix(int part)
{
    throw std::logic_error("GetWorldMatrix: not implemented!");
}

void COldObjectInterface::SetViewFromHere(Math::Vector &eye, float &dirH, float &dirV,
                                Math::Vector &lookat, Math::Vector &upVec,
                                Gfx::CameraType type)
{
    throw std::logic_error("SetViewFromHere: not implemented!");
}

Character* COldObjectInterface::GetCharacter()
{
    throw std::logic_error("GetCharacter: not implemented!");
}


void COldObjectInterface::SetFixed(bool bFixed)
{
    throw std::logic_error("SetFixed: not implemented!");
}

bool COldObjectInterface::GetFixed()
{
    throw std::logic_error("GetFixed: not implemented!");
}


void COldObjectInterface::SetVirusMode(bool bEnable)
{
    throw std::logic_error("SetVirusMode: not implemented!");
}

bool COldObjectInterface::GetVirusMode()
{
    throw std::logic_error("GetVirusMode: not implemented!");
}


void COldObjectInterface::SetParam(float value)
{
    throw std::logic_error("SetParam: not implemented!");
}

float COldObjectInterface::GetParam()
{
    throw std::logic_error("GetParam: not implemented!");
}


void COldObjectInterface::SetExploding(bool bExplo)
{
    throw std::logic_error("SetExploding: not implemented!");
}

bool COldObjectInterface::IsExploding()
{
    // TODO: temporary hack
    return false;
    //throw std::logic_error("IsExploding: not implemented!");
}


void COldObjectInterface::SetBurn(bool bBurn)
{
    throw std::logic_error("SetBurn: not implemented!");
}

bool COldObjectInterface::GetBurn()
{
    // TODO: temporary hack
    return false;
    //throw std::logic_error("GetBurn: not implemented!");
}

void COldObjectInterface::SetDead(bool bDead)
{
    throw std::logic_error("SetDead: not implemented!");
}

bool COldObjectInterface::GetDead()
{
    // TODO: temporary hack
    return false;
    //throw std::logic_error("GetDead: not implemented!");
}

bool COldObjectInterface::GetRuin()
{
    // TODO: temporary hack
    return false;
    //throw std::logic_error("GetRuin: not implemented!");
}

bool COldObjectInterface::GetActive()
{
    // TODO: temporary hack
    return false;
    //throw std::logic_error("GetActive: not implemented!");
}


CAuto* COldObjectInterface::GetAuto()
{
    throw std::logic_error("GetAuto: not implemented!");
}


void COldObjectInterface::SetDefRank(int rank)
{
    throw std::logic_error("SetDefRank: not implemented!");
}

int COldObjectInterface::GetDefRank()
{
    throw std::logic_error("GetDefRank: not implemented!");
}


void COldObjectInterface::FlatParent()
{
    throw std::logic_error("FlatParent: not implemented!");
}

float COldObjectInterface::GetInfoReturn()
{
    throw std::logic_error("GetInfoReturn: not implemented!");
}
