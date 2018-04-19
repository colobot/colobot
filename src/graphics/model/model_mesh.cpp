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

#include "graphics/model/model_mesh.h"

namespace Gfx
{

void CModelMesh::AddTriangle(const ModelTriangle& triangle)
{
    m_triangles.push_back(triangle);
}

void CModelMesh::SetTriangles(std::vector<ModelTriangle>&& triangles)
{
    m_triangles = triangles;
}

const std::vector<ModelTriangle>& CModelMesh::GetTriangles() const
{
    return m_triangles;
}

int CModelMesh::GetTriangleCount() const
{
    return m_triangles.size();
}

const Math::Vector& CModelMesh::GetPosition() const
{
    return m_position;
}

void CModelMesh::SetPosition(const Math::Vector& position)
{
    m_position = position;
}

const Math::Vector& CModelMesh::GetRotation() const
{
    return m_rotation;
}

void CModelMesh::SetRotation(const Math::Vector& rotation)
{
    m_rotation = rotation;
}

const Math::Vector& CModelMesh::GetScale() const
{
    return m_scale;
}

void CModelMesh::SetScale(const Math::Vector& scale)
{
    m_scale = scale;
}

const std::string& CModelMesh::GetParent() const
{
    return m_parent;
}

void CModelMesh::SetParent(const std::string& parent)
{
    m_parent = parent;
}

} // namespace Gfx
