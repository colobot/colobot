#pragma once

#include "graphics/model/model_crash_sphere.h"
#include "graphics/model/model_mesh.h"
#include "graphics/model/model_shadow_spot.h"

#include <map>
#include <string>
#include <vector>
#include <boost/optional.hpp>

namespace Gfx {

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

    //! Returns the optional shadow spot associated with model
    const boost::optional<ModelShadowSpot>& GetShadowSpot() const;
    //! Sets the shadow spot associated with model
    void SetShadowSpot(const ModelShadowSpot& shadowSpot);

    //! Returns the model's crash spheres
    const std::vector<ModelCrashSphere>& GetCrashSpheres() const;
    //! Adds a new crash sphere
    void AddCrashSphere(const ModelCrashSphere& crashSphere);

private:
    std::map<std::string, CModelMesh> m_meshes;
    boost::optional<ModelShadowSpot> m_shadowSpot;
    std::vector<ModelCrashSphere> m_crashSpheres;
};

} // namespace Gfx
