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
 * \file object/object_factory.h
 * \brief CObjectFactory - factory for game objects
 */

#pragma once

#include "math/vector.h"

#include "object/object_type.h"

#include <memory>

namespace Gfx
{
class CEngine;
class COldModelManager;
class CModelManager;
class CParticle;
class CTerrain;
} // namespace Gfx

class CObject;
class COldObject;
struct ObjectCreateParams;

using CObjectUPtr = std::unique_ptr<CObject>;

class CObjectFactory
{
public:
    CObjectFactory(Gfx::CEngine* engine,
                   Gfx::CTerrain* terrain,
                   Gfx::COldModelManager* oldModelManager,
                   Gfx::CModelManager* modelManager,
                   Gfx::CParticle* particle);

    CObjectUPtr CreateObject(const ObjectCreateParams& params);

private:
    CObjectUPtr CreateResource(const ObjectCreateParams& params);
    CObjectUPtr CreateFlag(const ObjectCreateParams& params);
    CObjectUPtr CreateBarrier(const ObjectCreateParams& params);
    CObjectUPtr CreatePlant(const ObjectCreateParams& params);
    CObjectUPtr CreateMushroom(const ObjectCreateParams& params);
    CObjectUPtr CreateTeen(const ObjectCreateParams& params);
    CObjectUPtr CreateQuartz(const ObjectCreateParams& params);
    CObjectUPtr CreateRoot(const ObjectCreateParams& params);
    CObjectUPtr CreateHome(const ObjectCreateParams& params);
    CObjectUPtr CreateRuin(const ObjectCreateParams& params);
    CObjectUPtr CreateApollo(const ObjectCreateParams& params);
    void AddObjectAuto(COldObject* obj);

private:
    Gfx::CEngine* m_engine;
    Gfx::CTerrain* m_terrain;
    Gfx::COldModelManager* m_oldModelManager;
    Gfx::CModelManager* m_modelManager;
    Gfx::CParticle* m_particle;
};
