/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/model/model.h"

#include "common/logger.h"


namespace Gfx
{

int CModel::GetMeshCount() const
{
    return m_meshes.size();
}

CModelMesh* CModel::GetMesh(const std::string& name)
{
    auto it = m_meshes.find(name);
    if (it == m_meshes.end())
    {
        GetLogger()->Error("Mesh named '%s' not found in model!\n", name.c_str());
        return nullptr;
    }
    return &(it->second);
}

const CModelMesh* CModel::GetMesh(const std::string& name) const
{
    auto it = m_meshes.find(name);
    if (it == m_meshes.end())
    {
        GetLogger()->Error("Mesh named '%s' not found in model!\n", name.c_str());
        return nullptr;
    }
    return &(it->second);
}

void CModel::AddMesh(const std::string& name, CModelMesh&& mesh)
{
    m_meshes[name] = mesh;
}

std::vector<std::string> CModel::GetMeshNames() const
{
    std::vector<std::string> meshNames;

    for (const auto& v : m_meshes)
        meshNames.push_back(v.first);

    return meshNames;
}

const ModelShadowSpot& CModel::GetShadowSpot() const
{
    return *m_shadowSpot;
}

void CModel::SetShadowSpot(const ModelShadowSpot& shadowSpot)
{
    m_shadowSpot = shadowSpot;
}

bool CModel::HasShadowSpot() const
{
    return m_shadowSpot.has_value();
}

const std::vector<ModelCrashSphere>& CModel::GetCrashSpheres() const
{
    return m_crashSpheres;
}

void CModel::AddCrashSphere(const ModelCrashSphere& crashSphere)
{
    m_crashSpheres.push_back(crashSphere);
}

int CModel::GetCrashSphereCount() const
{
    return m_crashSpheres.size();
}

const Math::Sphere& CModel::GetCameraCollisionSphere() const
{
    return *m_cameraCollisionSphere;
}

void CModel::SetCameraCollisionSphere(const Math::Sphere& sphere)
{
    m_cameraCollisionSphere = sphere;
}

bool CModel::HasCameraCollisionSphere() const
{
    return m_cameraCollisionSphere.has_value();
}

} // namespace Gfx
