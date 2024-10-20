/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "object/object.h"

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>
#include <filesystem>

namespace Gfx
{
class CModelManager;
class CEngine;
class CModel;
class CTerrain;
} // namespace Gfx

class CStaticObject;
using CStaticObjectUPtr = std::unique_ptr<CStaticObject>;

class CStaticObject : public CObject
{
public:
    CStaticObject(int id,
                  ObjectType type,
                  const std::string& key,
                  const glm::vec3& position,
                  float angleY,
                  const Gfx::CModel* model,
                  Gfx::CEngine* engine);

    virtual ~CStaticObject();

    void Read(CLevelParserLine* line) override;
    void Write(CLevelParserLine* line) override;

    void SetGhostMode(bool enabled) override;

public:
    static bool IsStaticObject(ObjectType type);

    static CStaticObjectUPtr Create(int id,
                                    ObjectType type,
                                    const glm::vec3& position,
                                    float angleY,
                                    float height,
                                    Gfx::CEngine* engine,
                                    Gfx::CModelManager* modelManager,
                                    Gfx::CTerrain* terrain);

protected:
    void TransformCrashSphere(Math::Sphere& crashSphere) override;
    void TransformCameraCollisionSphere(Math::Sphere& collisionSphere) override;

private:
    static glm::mat4 ComputeWorldMatrix(const glm::vec3& position, float angleY);

private:
    Gfx::CEngine* m_engine;
    int m_meshHandle;
    static const std::unordered_map<ObjectType, std::filesystem::path, ObjectTypeHash> m_staticModelNames;
};
