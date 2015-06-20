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
 * \file object/object_factory.h
 * \brief CObjectFactory - factory for game objects
 */

#pragma once

#include "math/vector.h"

#include "object/object_type.h"

#include <memory>

namespace Gfx {
class CEngine;
class CModelManager;
class CParticle;
class CTerrain;
} // namespace Gfx

class CObject;
class CRobotMain;

class CObjectFactory
{
public:
    CObjectFactory(Gfx::CEngine* engine,
                   Gfx::CTerrain* terrain,
                   Gfx::CModelManager* modelManager,
                   Gfx::CParticle* particle,
                   CRobotMain* main);

    CObject* CreateObject(Math::Vector pos, float angle, ObjectType type,
                          float power, float zoom, float height,
                          bool trainer, bool toy, int option);

    CObject* CreateBuilding(Math::Vector pos, float angle, float height, ObjectType type, float power=1.0f);
    CObject* CreateResource(Math::Vector pos, float angle, ObjectType type, float power=1.0f);
    CObject* CreateVehicle(Math::Vector pos, float angle, ObjectType type, float power, bool trainer, bool toy, int option);
    CObject* CreateInsect(Math::Vector pos, float angle, ObjectType type);
    CObject* CreateFlag(Math::Vector pos, float angle, ObjectType type);
    CObject* CreateBarrier(Math::Vector pos, float angle, float height, ObjectType type);
    CObject* CreatePlant(Math::Vector pos, float angle, float height, ObjectType type);
    CObject* CreateMushroom(Math::Vector pos, float angle, float height, ObjectType type);
    CObject* CreateTeen(Math::Vector pos, float angle, float zoom, float height, ObjectType type, int option);
    CObject* CreateQuartz(Math::Vector pos, float angle, float height, ObjectType type);
    CObject* CreateRoot(Math::Vector pos, float angle, float height, ObjectType type);
    CObject* CreateHome(Math::Vector pos, float angle, float height, ObjectType type);
    CObject* CreateRuin(Math::Vector pos, float angle, float height, ObjectType type);
    CObject* CreateApollo(Math::Vector pos, float angle, ObjectType type);

private:
    void AddObjectAuto(CObject* obj);

private:
    Gfx::CEngine* m_engine;
    Gfx::CTerrain* m_terrain;
    Gfx::CModelManager* m_modelManager;
    Gfx::CParticle* m_particle;
    CRobotMain* m_main;
};
