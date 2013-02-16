#include "graphics/engine/particle.h"

#include "common/logger.h"


// Graphics module namespace
namespace Gfx {


CParticle::CParticle(CEngine* /*engine*/)
{
}

CParticle::~CParticle()
{
}

void CParticle::SetDevice(CDevice* /*device*/)
{
}

void CParticle::FlushParticle()
{
}

void CParticle::FlushParticle(int /*sheet*/)
{
}

int CParticle::CreateParticle(Math::Vector /*pos*/, Math::Vector /*speed*/, Math::Point /*dim*/,
                        ParticleType /*type*/, float /*duration*/, float /*mass*/,
                        float /*windSensitivity*/, int /*sheet*/)
{
    return 0;
}

int CParticle::CreateFrag(Math::Vector /*pos*/, Math::Vector /*speed*/, EngineTriangle */*triangle*/,
                           ParticleType /*type*/, float /*duration*/, float /*mass*/,
                           float /*windSensitivity*/, int /*sheet*/)
{
    return 0;
}

int CParticle::CreatePart(Math::Vector /*pos*/, Math::Vector /*speed*/, ParticleType /*type*/,
                               float /*duration*/, float /*mass*/, float /*weight*/,
                               float /*windSensitivity*/, int /*sheet*/)
{
    return 0;
}

int CParticle::CreateRay(Math::Vector /*pos*/, Math::Vector /*goal*/, ParticleType /*type*/, Math::Point /*dim*/,
                              float /*duration*/, int /*sheet*/)
{
    return 0;
}

int CParticle::CreateTrack(Math::Vector /*pos*/, Math::Vector /*speed*/, Math::Point /*dim*/, ParticleType /*type*/,
                                float /*duration*/, float /*mass*/, float /*length*/, float /*width*/)
{
    return 0;
}

void CParticle::CreateWheelTrace(const Math::Vector &/*p1*/, const Math::Vector &/*p2*/, const Math::Vector &/*p3*/,
                                      const Math::Vector &/*p4*/, ParticleType /*type*/)
{
}

void CParticle::DeleteParticle(ParticleType /*type*/)
{
}

void CParticle::DeleteParticle(int /*channel*/)
{
}

void CParticle::SetObjectLink(int /*channel*/, CObject */*object*/)
{
}

void CParticle::SetObjectFather(int /*channel*/, CObject */*object*/)
{
}

void CParticle::SetPosition(int /*channel*/, Math::Vector /*pos*/)
{
}

void CParticle::SetDimension(int /*channel*/, Math::Point /*dim*/)
{
}

void CParticle::SetZoom(int /*channel*/, float /*zoom*/)
{
}

void CParticle::SetAngle(int /*channel*/, float /*angle*/)
{
}

void CParticle::SetIntensity(int /*channel*/, float /*intensity*/)
{
}

void CParticle::SetParam(int /*channel*/, Math::Vector /*pos*/, Math::Point /*dim*/, float /*zoom*/, float /*angle*/, float /*intensity*/)
{
}

void CParticle::SetPhase(int /*channel*/, ParticlePhase /*phase*/, float /*duration*/)
{
}

bool CParticle::GetPosition(int /*channel*/, Math::Vector &/*pos*/)
{
    return true;
}

Color CParticle::GetFogColor(Math::Vector /*pos*/)
{
    return Color();
}

void CParticle::SetFrameUpdate(int /*sheet*/, bool /*update*/)
{
}

void CParticle::FrameParticle(float /*rTime*/)
{
}

void CParticle::DrawParticle(int /*sheet*/)
{
}

bool CParticle::WriteWheelTrace(const char */*filename*/, int /*width*/, int /*height*/, Math::Vector /*dl*/, Math::Vector /*ur*/)
{
    return true;
}

void CParticle::DeleteRank(int /*rank*/)
{
}

bool CParticle::CheckChannel(int &/*channel*/)
{
    return true;
}

void CParticle::DrawParticleTriangle(int /*i*/)
{
}

void CParticle::DrawParticleNorm(int /*i*/)
{
}

void CParticle::DrawParticleFlat(int /*i*/)
{
}

void CParticle::DrawParticleFog(int /*i*/)
{
}

void CParticle::DrawParticleRay(int /*i*/)
{
}

void CParticle::DrawParticleSphere(int /*i*/)
{
}

void CParticle::DrawParticleCylinder(int /*i*/)
{
}

void CParticle::DrawParticleWheel(int /*i*/)
{
}

CObject* CParticle::SearchObjectGun(Math::Vector /*old*/, Math::Vector /*pos*/, ParticleType /*type*/, CObject */*father*/)
{
    return nullptr;
}

CObject* CParticle::SearchObjectRay(Math::Vector /*pos*/, Math::Vector /*goal*/, ParticleType /*type*/, CObject */*father*/)
{
    return nullptr;
}

void CParticle::Play(Sound /*sound*/, Math::Vector /*pos*/, float /*amplitude*/)
{
}

bool CParticle::TrackMove(int /*i*/, Math::Vector /*pos*/, float /*progress*/)
{
    return true;
}

void CParticle::TrackDraw(int /*i*/, ParticleType /*type*/)
{
}


} // namespace Gfx

