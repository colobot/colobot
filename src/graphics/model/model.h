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

#include "graphics/model/model_crash_sphere.h"
#include "graphics/model/model_mesh.h"
#include "graphics/model/model_shadow_spot.h"

#include "math/sphere.h"

#include <map>
#include <string>
#include <vector>
#include <optional>

namespace Gfx
{

/**
 * \class CModel
 * \brief 3D model saved in model file
 */
class CModel
{
public:
    //! Returns mesh count
    int GetMeshCount() const;
    //! Return a mesh with given \a name
    CModelMesh* GetMesh(const std::string& name);
    //! Return a mesh with given \a name
    const CModelMesh* GetMesh(const std::string& name) const;
    //! Add new \a mesh with given \a name
    void AddMesh(const std::string& name, CModelMesh&& mesh);
    //! Returns list of mesh names
    std::vector<std::string> GetMeshNames() const;

    //! Returns the model's crash spheres
    const std::vector<ModelCrashSphere>& GetCrashSpheres() const;
    //! Adds a new crash sphere
    void AddCrashSphere(const ModelCrashSphere& crashSphere);
    //! Returns number of crash spheres
    int GetCrashSphereCount() const;

    //! Returns the shadow spot associated with model (assumes it is present)
    const ModelShadowSpot& GetShadowSpot() const;
    //! Sets the shadow spot associated with model
    void SetShadowSpot(const ModelShadowSpot& shadowSpot);
    //! Returns whether there is shadow spot
    bool HasShadowSpot() const;

    //! Returns the optional shadow spot associated with model (assumes it is present)
    const Math::Sphere& GetCameraCollisionSphere() const;
    //! Sets the shadow spot associated with model
    void SetCameraCollisionSphere(const Math::Sphere& sphere);
    //! Returns whether there is camera collision sphere
    bool HasCameraCollisionSphere() const;

private:
    std::map<std::string, CModelMesh> m_meshes;
    std::vector<ModelCrashSphere> m_crashSpheres;
    std::optional<ModelShadowSpot> m_shadowSpot;
    std::optional<Math::Sphere> m_cameraCollisionSphere;
};

} // namespace Gfx
