#include "graphics/model/model.h"

#include "common/logger.h"


namespace Gfx {

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

const boost::optional<ModelShadowSpot>& CModel::GetShadowSpot() const
{
    return m_shadowSpot;
}

void CModel::SetShadowSpot(const ModelShadowSpot& shadowSpot)
{
    m_shadowSpot = shadowSpot;
}

const std::vector<ModelCrashSphere>& CModel::GetCrashSpheres() const
{
    return m_crashSpheres;
}

void CModel::AddCrashSphere(const ModelCrashSphere& crashSphere)
{
    m_crashSpheres.push_back(crashSphere);
}

} // namespace Gfx
