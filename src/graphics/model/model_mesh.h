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

#pragma once

#include "graphics/model/model_triangle.h"

#include "math/vector.h"

#include <vector>

namespace Gfx
{

/**
 * \class CModelMesh
 * \brief Mesh data saved in model file
 */
class CModelMesh
{
public:
    //! Adds a new triangle
    void AddTriangle(const ModelTriangle& triangle);
    //! Sets the list of triangles
    void SetTriangles(std::vector<ModelTriangle> &&triangles);
    //! Returns the list of triangles
    const std::vector<ModelTriangle>& GetTriangles() const;
    //! Returns number of triangles
    int GetTriangleCount() const;

    //! Returns the mesh position
    const Math::Vector& GetPosition() const;
    //! Sets the mesh rotation
    void SetPosition(const Math::Vector& position);

    //! Returns the mesh rotation
    const Math::Vector& GetRotation() const;
    //! Sets the mesh rotation
    void SetRotation(const Math::Vector& rotation);

    //! Returns the mesh scale
    const Math::Vector& GetScale() const;
    //! Sets the mesh scale
    void SetScale(const Math::Vector& scale);

    //! Returns the name of parent mesh
    const std::string& GetParent() const;
    //! Sets the name of parent mesh
    void SetParent(const std::string& parent);

private:
    std::vector<ModelTriangle> m_triangles;
    Math::Vector m_position;
    Math::Vector m_rotation;
    Math::Vector m_scale;
    std::string m_parent;
};

} // namespace Gfx
