#include "object/object.h"

CObject::CObject(int id, ObjectType type)
    : m_id(id)
    , m_type(type)
{
    m_implementedInterfaces.fill(false);
}

CObject::~CObject()
{
}

void CObject::AddCrashSphere(const CrashSphere& crashSphere)
{
    m_crashSpheres.push_back(crashSphere);
}

CrashSphere CObject::GetFirstCrashSphere()
{
    assert(m_crashSpheres.size() >= 1);

    CrashSphere transformedFirstCrashSphere = m_crashSpheres[0];
    TransformCrashSphere(transformedFirstCrashSphere.sphere);
    return transformedFirstCrashSphere;
}

std::vector<CrashSphere> CObject::GetAllCrashSpheres()
{
    std::vector<CrashSphere> allCrashSpheres;

    for (const auto& crashSphere : m_crashSpheres)
    {
        CrashSphere transformedCrashSphere = crashSphere;
        TransformCrashSphere(transformedCrashSphere.sphere);
        allCrashSpheres.push_back(transformedCrashSphere);
    }

    return allCrashSpheres;
}

int CObject::GetCrashSphereCount()
{
    return m_crashSpheres.size();
}

void CObject::DeleteAllCrashSpheres()
{
    m_crashSpheres.clear();
}
