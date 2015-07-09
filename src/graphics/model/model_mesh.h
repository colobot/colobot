#pragma once

#include "math/vector.h"

#include "graphics/model/model_triangle.h"

#include <vector>

namespace Gfx {

struct ModelTriangle;

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
