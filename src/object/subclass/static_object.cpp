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

#include "object/subclass/static_object.h"

#include "common/make_unique.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"

#include "graphics/model/model.h"
#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_manager.h"

#include "math/geometry.h"

#include "object/object_create_exception.h"


const std::unordered_map<ObjectType, std::string, ObjectTypeHash> CStaticObject::m_staticModelNames{};
// TODO: commenting out temporarily
//=
//{
//   { OBJECT_TREE0, "tree0" }
//};


CStaticObject::CStaticObject(int id,
                             ObjectType type,
                             const std::string& key,
                             const Math::Vector& position,
                             float angleY,
                             const Gfx::CModel& model,
                             Gfx::CEngine* engine)
    : CObject(id, type)
    , m_engine(engine)
{
    const Gfx::CModelMesh* mesh = model.GetMesh("main");
    assert(mesh != nullptr);

    m_position = position;
    m_rotation.y = angleY;

    Math::Matrix worldMatrix = ComputeWorldMatrix(position, angleY);
    m_meshHandle = m_engine->AddStaticMesh(key, mesh, worldMatrix);

    if (model.HasShadowSpot())
        m_engine->AddStaticMeshShadowSpot(m_meshHandle, model.GetShadowSpot());

    SetCrashSpheres(model.GetCrashSpheres());

    if (model.HasCameraCollisionSphere())
        SetCameraCollisionSphere(model.GetCameraCollisionSphere());
}

CStaticObject::~CStaticObject()
{
    m_engine->DeleteStaticMesh(m_meshHandle);
}

Math::Matrix CStaticObject::ComputeWorldMatrix(const Math::Vector& position, float angleY)
{
    Math::Matrix translationMatrix;
    Math::LoadTranslationMatrix(translationMatrix, position);

    Math::Matrix rotationMatrix;
    Math::LoadRotationZXYMatrix(rotationMatrix, Math::Vector(0.0f, angleY, 0.0f));

    return Math::MultiplyMatrices(translationMatrix, rotationMatrix);
}

void CStaticObject::Read(CLevelParserLine* line)
{
}

void CStaticObject::Write(CLevelParserLine* line)
{
}

void CStaticObject::TransformCrashSphere(Math::Sphere& crashSphere)
{
    Math::Matrix worldMatrix = m_engine->GetStaticMeshWorldMatrix(m_meshHandle);
    Math::Transform(worldMatrix, crashSphere.pos);
}

void CStaticObject::TransformCameraCollisionSphere(Math::Sphere& collisionSphere)
{
    Math::Matrix worldMatrix = m_engine->GetStaticMeshWorldMatrix(m_meshHandle);
    Math::Transform(worldMatrix, collisionSphere.pos);
}

void CStaticObject::SetTransparency(float value)
{
    m_engine->SetStaticMeshTransparency(m_meshHandle, value);
}

bool CStaticObject::IsStaticObject(ObjectType type)
{
    return m_staticModelNames.count(type) > 0;
}

CStaticObjectUPtr CStaticObject::Create(int id,
                                        ObjectType type,
                                        const Math::Vector& position,
                                        float angleY,
                                        float height,
                                        Gfx::CEngine* engine,
                                        Gfx::CModelManager* modelManager,
                                        Gfx::CTerrain* terrain)
{
    auto it = m_staticModelNames.find(type);
    if (it == m_staticModelNames.end())
        throw CObjectCreateException("Object type is not static object", type);

    std::string modelFile = it->second;

    Math::Vector adjustedPosition = position;
    terrain->AdjustToFloor(adjustedPosition);
    adjustedPosition.y += height;

    try
    {
        Gfx::CModel& model = modelManager->GetModel(modelFile);

        if (model.GetMeshCount() != 1 || model.GetMesh("main") == nullptr)
            throw CObjectCreateException("Unexpected mesh configuration", type, modelFile);

        return MakeUnique<CStaticObject>(id, type, modelFile, adjustedPosition, angleY, model, engine);
    }
    catch (const Gfx::CModelIOException& e)
    {
        throw CObjectCreateException(std::string("Error loading model file: ") + e.what(), type, modelFile);
    }
}
