#include "object/object.h"

#include "graphics/model/model_crash_sphere.h"


CObject::CObject(int id, ObjectType type)
    : m_id(id)
    , m_type(type)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_rotation(0.0f, 0.0f, 0.0f)
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_animateOnReset(false)
{
    m_implementedInterfaces.fill(false);
}

CObject::~CObject()
{
}

void CObject::SetCrashSpheres(const std::vector<Gfx::ModelCrashSphere>& crashSpheres)
{
    for (const auto& crashSphere : crashSpheres)
    {
        SoundType sound = ParseSoundType(crashSphere.sound);
        CrashSphere objectCrashSphere(crashSphere.position, crashSphere.radius, sound, crashSphere.hardness);
        AddCrashSphere(objectCrashSphere);
    }
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

Math::Vector CObject::GetPosition() const
{
    return m_position;
}

Math::Vector CObject::GetRotation() const
{
    return m_rotation;
}

Math::Vector CObject::GetScale() const
{
    return m_scale;
}

int CObject::GetCrashSphereCount()
{
    return m_crashSpheres.size();
}

void CObject::DeleteAllCrashSpheres()
{
    m_crashSpheres.clear();
}

void CObject::SetCameraCollisionSphere(const Math::Sphere& sphere)
{
    m_cameraCollisionSphere = sphere;
}

Math::Sphere CObject::GetCameraCollisionSphere()
{
    Math::Sphere transformedSphere = m_cameraCollisionSphere;
    TransformCrashSphere(transformedSphere);
    return transformedSphere;
}

bool CObject::GetAnimateOnReset()
{
    return m_animateOnReset;
}

void CObject::SetAnimateOnReset(bool animateOnReset)
{
    m_animateOnReset = animateOnReset;
}


