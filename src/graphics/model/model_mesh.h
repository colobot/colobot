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

#pragma once

#include "graphics/model/model_triangle.h"

#include <vector>

namespace Gfx
{

/**
 * \class CModelPart
 * \brief Part of mesh with a common material
 */
class CModelPart
{
public:
    //! Creates new part for given material
    CModelPart(const Material& material);

    //! Returns this part's material
    const Material& GetMaterial() const;

    //! Returns true if this part is indexed
    bool IsIndexed() const;
    //! Returns the number of vertices in this part
    size_t GetVertexCount() const;
    //! Returns the vertices in this part
    const std::vector<Vertex3D>& GetVertices() const;
    //! Returns the number of indices in this part
    size_t GetIndexCount() const;
    //! Returns the indices in this part
    const std::vector<unsigned int>& GetIndices() const;

    //! Adds a vertex
    void AddVertex(const Vertex3D& vertex);
    //! Adds an index
    void AddIndex(unsigned int index);

private:
    //! Material
    Material m_material;
    //! Vertices
    std::vector<Vertex3D> m_vertices;
    //! Indices
    std::vector<unsigned int> m_indices;
};

/**
 * \class CModelMesh
 * \brief Mesh data saved in model file
 */
class CModelMesh
{
public:
    //! Adds a new triangle
    void AddTriangle(const ModelTriangle& triangle);
    //! Adds a new triangle
    void AddTriangle(const Triangle& triangle, const Material& material);

    //! Returns the number of parts
    size_t GetPartCount() const;
    //! Returns a part with given index
    const CModelPart& GetPart(size_t index) const;
    //! Adds a new part with given material or returns an existing one
    CModelPart& AddPart(const Material& material);

    //! Returns the mesh position
    const glm::vec3& GetPosition() const;
    //! Sets the mesh rotation
    void SetPosition(const glm::vec3& position);

    //! Returns the mesh rotation
    const glm::vec3& GetRotation() const;
    //! Sets the mesh rotation
    void SetRotation(const glm::vec3& rotation);

    //! Returns the mesh scale
    const glm::vec3& GetScale() const;
    //! Sets the mesh scale
    void SetScale(const glm::vec3& scale);

    //! Returns the name of parent mesh
    const std::string& GetParent() const;
    //! Sets the name of parent mesh
    void SetParent(const std::string& parent);

    //! Returns all model triangles of this mesh
    std::vector<ModelTriangle> GetTriangles() const;

private:
    std::vector<CModelPart> m_parts;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
    std::string m_parent;
};

} // namespace Gfx
