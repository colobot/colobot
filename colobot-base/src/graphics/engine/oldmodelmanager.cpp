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

#include "graphics/engine/oldmodelmanager.h"

#include "app/app.h"

#include "common/logger.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/resourcemanager.h"

#include "graphics/engine/engine.h"

#include "graphics/model/model_input.h"
#include "graphics/model/model_io_exception.h"

#include <cstdio>

namespace Gfx
{

COldModelManager::COldModelManager(CEngine* engine)
{
    m_engine = engine;
}

COldModelManager::~COldModelManager()
{
}

bool COldModelManager::LoadModel(const std::filesystem::path& name, bool mirrored, int team)
{
    std::unique_ptr<CModel> model;
    try
    {
        auto extension = name.extension();

        if (!extension.empty())
        {
            GetLogger()->Debug("Loading model '%%'", name);

            model = ModelInput::Read("models" / name);

            if (model->GetMeshCount() == 0)
                return false;

            goto skip;
        }

        auto gltf_path = "models" / name;
        gltf_path.replace_extension("gltf");

        if (CResourceManager::Exists(gltf_path))
        {
            GetLogger()->Debug("Loading model '%%'", gltf_path);

            model = ModelInput::Read(gltf_path);

            if (model->GetMeshCount() > 0)
                goto skip;
        }

        auto mod_path = "models" / name;
        mod_path.replace_extension("mod");

        if (CResourceManager::Exists(mod_path))
        {
            GetLogger()->Debug("Loading model '%%'", mod_path);

            model = ModelInput::Read(mod_path);

            if (model->GetMeshCount() > 0)
                goto skip;
        }

        return false;
    }
    catch (const CModelIOException& e)
    {
        GetLogger()->Error("Loading model '%%' failed: %%", name, e.what());
        return false;
    }

skip:
    CModelMesh* mesh = model->GetMesh();
    assert(mesh != nullptr);

    ModelInfo modelInfo;
    modelInfo.baseObjRank = m_engine->CreateBaseObject();
    modelInfo.triangles = mesh->GetTriangles();

    if (mirrored)
        Mirror(modelInfo.triangles);

    FileInfo fileInfo(StrUtils::ToString(name), mirrored, team);
    m_models[fileInfo] = modelInfo;

    m_engine->AddBaseObjTriangles(modelInfo.baseObjRank, modelInfo.triangles);

    return true;
}

bool COldModelManager::AddModelReference(const std::string& fileName, bool mirrored, int objRank, int team)
{
    auto it = m_models.find(FileInfo(fileName, mirrored, team));
    if (it == m_models.end())
    {
        if (!LoadModel(StrUtils::ToPath(fileName), mirrored, team))
            return false;

        it = m_models.find(FileInfo(fileName, mirrored, team));
    }

    m_engine->SetObjectBaseRank(objRank, (*it).second.baseObjRank);
    m_engine->SetObjectTeam(objRank, team);

    return true;
}

bool COldModelManager::AddModelCopy(const std::string& fileName, bool mirrored, int objRank, int team)
{
    auto it = m_models.find(FileInfo(fileName, mirrored, team));
    if (it == m_models.end())
    {
        if (!LoadModel(StrUtils::ToPath(fileName), mirrored, team))
            return false;

        it = m_models.find(FileInfo(fileName, mirrored, team));
    }

    int copyBaseObjRank = m_engine->CreateBaseObject();
    m_engine->CopyBaseObject((*it).second.baseObjRank, copyBaseObjRank);
    m_engine->SetObjectBaseRank(objRank, copyBaseObjRank);
    m_engine->SetObjectTeam(objRank, team);

    m_copiesBaseRanks.push_back(copyBaseObjRank);

    return true;
}

bool COldModelManager::IsModelLoaded(const std::string& fileName, bool mirrored, int team)
{
    return m_models.count(FileInfo(fileName, mirrored, team)) > 0;
}

int COldModelManager::GetModelBaseObjRank(const std::string& fileName, bool mirrored, int team)
{
    auto it = m_models.find(FileInfo(fileName, mirrored, team));
    if (it == m_models.end())
        return -1;

    return (*it).second.baseObjRank;
}

void COldModelManager::DeleteAllModelCopies()
{
    for (int baseObjRank : m_copiesBaseRanks)
    {
        m_engine->DeleteBaseObject(baseObjRank);
    }

    m_copiesBaseRanks.clear();
}

void COldModelManager::UnloadModel(const std::string& fileName, bool mirrored, int team)
{
    auto it = m_models.find(FileInfo(fileName, mirrored, team));
    if (it == m_models.end())
        return;

    m_engine->DeleteBaseObject((*it).second.baseObjRank);

    m_models.erase(it);
}

void COldModelManager::UnloadAllModels()
{
    for (auto& mf : m_models)
        m_engine->DeleteBaseObject(mf.second.baseObjRank);

    m_models.clear();
}

void COldModelManager::Mirror(std::vector<ModelTriangle>& triangles)
{
    for (int i = 0; i < static_cast<int>( triangles.size() ); i++)
    {
        Vertex3D t = triangles[i].p1;
        triangles[i].p1 = triangles[i].p2;
        triangles[i].p2 = t;

        triangles[i].p1.position.z = -triangles[i].p1.position.z;
        triangles[i].p2.position.z = -triangles[i].p2.position.z;
        triangles[i].p3.position.z = -triangles[i].p3.position.z;

        triangles[i].p1.normal.z = -triangles[i].p1.normal.z;
        triangles[i].p2.normal.z = -triangles[i].p2.normal.z;
        triangles[i].p3.normal.z = -triangles[i].p3.normal.z;
    }
}

}
