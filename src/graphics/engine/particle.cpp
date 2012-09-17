// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// particle.cpp (aka particule.cpp)

#include "graphics/engine/particle.h"

#include "common/logger.h"


Gfx::CParticle::CParticle(CInstanceManager* iMan, Gfx::CEngine* engine)
{
    GetLogger()->Trace("CParticle::CParticle() stub!\n");
    // TODO!
}

Gfx::CParticle::~CParticle()
{
    GetLogger()->Trace("CParticle::~CParticle() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetDevice(Gfx::CDevice* device)
{
    GetLogger()->Trace("CParticle::SetDevice() stub!\n");
    // TODO!
}

void Gfx::CParticle::FlushParticle()
{
    GetLogger()->Trace("CParticle::FlushParticle() stub!\n");
    // TODO!
}

void Gfx::CParticle::FlushParticle(int sheet)
{
    GetLogger()->Trace("CParticle::FlushParticle() stub!\n");
    // TODO!
}

int Gfx::CParticle::CreateParticle(Math::Vector pos, Math::Vector speed, Math::Point dim,
                        Gfx::ParticleType type, float duration, float mass,
                        float windSensitivity, int sheet)
{
    GetLogger()->Trace("CParticle::CreateParticle() stub!\n");
    // TODO!
    return 0;
}

int Gfx::CParticle::CreateFrag(Math::Vector pos, Math::Vector speed, Gfx::EngineTriangle *triangle,
                           Gfx::ParticleType type, float duration, float mass,
                           float windSensitivity, int sheet)
{
    GetLogger()->Trace("CParticle::CreateFrag() stub!\n");
    // TODO!
    return 0;
}

int Gfx::CParticle::CreatePart(Math::Vector pos, Math::Vector speed, Gfx::ParticleType type,
                               float duration, float mass, float weight,
                               float windSensitivity, int sheet)
{
    GetLogger()->Trace("CParticle::CreatePart() stub!\n");
    // TODO!
    return 0;
}

int Gfx::CParticle::CreateRay(Math::Vector pos, Math::Vector goal, Gfx::ParticleType type, Math::Point dim,
                              float duration, int sheet)
{
    GetLogger()->Trace("CParticle::CreateRay() stub!\n");
    // TODO!
    return 0;
}

int Gfx::CParticle::CreateTrack(Math::Vector pos, Math::Vector speed, Math::Point dim, Gfx::ParticleType type,
                                float duration, float mass, float length, float width)
{
    GetLogger()->Trace("CParticle::CreateTrack() stub!\n");
    // TODO!
    return 0;
}

void Gfx::CParticle::CreateWheelTrace(const Math::Vector &p1, const Math::Vector &p2, const Math::Vector &p3,
                                      const Math::Vector &p4, Gfx::ParticleType type)
{
    GetLogger()->Trace("CParticle::CreateWheelTrace() stub!\n");
    // TODO!
}

void Gfx::CParticle::DeleteParticle(Gfx::ParticleType type)
{
    GetLogger()->Trace("CParticle::DeleteParticle() stub!\n");
    // TODO!
}

void Gfx::CParticle::DeleteParticle(int channel)
{
    GetLogger()->Trace("CParticle::DeleteParticle() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetObjectLink(int channel, CObject *object)
{
    GetLogger()->Trace("CParticle::SetObjectLink() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetObjectFather(int channel, CObject *object)
{
    GetLogger()->Trace("CParticle::SetObjectFather() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetPosition(int channel, Math::Vector pos)
{
    GetLogger()->Trace("CParticle::SetPosition() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetDimension(int channel, Math::Point dim)
{
    GetLogger()->Trace("CParticle::SetDimension() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetZoom(int channel, float zoom)
{
    GetLogger()->Trace("CParticle::SetZoom() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetAngle(int channel, float angle)
{
    GetLogger()->Trace("CParticle::SetAngle() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetIntensity(int channel, float intensity)
{
    GetLogger()->Trace("CParticle::SetIntensity() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetParam(int channel, Math::Vector pos, Math::Point dim, float zoom, float angle, float intensity)
{
    GetLogger()->Trace("CParticle::SetParam() stub!\n");
    // TODO!
}

void Gfx::CParticle::SetPhase(int channel, Gfx::ParticlePhase phase, float duration)
{
    GetLogger()->Trace("CParticle::SetPhase() stub!\n");
    // TODO!
}

bool Gfx::CParticle::GetPosition(int channel, Math::Vector &pos)
{
    GetLogger()->Trace("CParticle::GetPosition() stub!\n");
    // TODO!
    return true;
}

Gfx::Color Gfx::CParticle::GetFogColor(Math::Vector pos)
{
    GetLogger()->Trace("CParticle::GetFogColor() stub!\n");
    // TODO!
    return Gfx::Color();
}

void Gfx::CParticle::SetFrameUpdate(int sheet, bool update)
{
    GetLogger()->Trace("CParticle::SetFrameUpdate() stub!\n");
    // TODO!
}

void Gfx::CParticle::FrameParticle(float rTime)
{
    GetLogger()->Trace("CParticle::FrameParticle() stub!\n");
    // TODO!
}

void Gfx::CParticle::DrawParticle(int sheet)
{
    GetLogger()->Trace("CParticle::DrawParticle() stub!\n");
    // TODO!
}

bool Gfx::CParticle::WriteWheelTrace(const char *filename, int width, int height, Math::Vector dl, Math::Vector ur)
{
    GetLogger()->Trace("CParticle::WriteWheelTrace() stub!\n");
    // TODO!
    return true;
}

void Gfx::CParticle::DeleteRank(int rank)
{
    GetLogger()->Trace("CParticle::DeleteRank() stub!\n");
    // TODO!
}

bool Gfx::CParticle::CheckChannel(int &channel)
{
    GetLogger()->Trace("CParticle::CheckChannel() stub!\n");
    // TODO!
    return true;
}

void Gfx::CParticle::DrawParticleTriangle(int i)
{
    GetLogger()->Trace("CParticle::DrawParticleTriangle() stub!\n");
    // TODO!
}

void Gfx::CParticle::DrawParticleNorm(int i)
{
    GetLogger()->Trace("CParticle::DrawParticleNorm() stub!\n");
    // TODO!
}

void Gfx::CParticle::DrawParticleFlat(int i)
{
    GetLogger()->Trace("CParticle::DrawParticleFlat() stub!\n");
    // TODO!
}

void Gfx::CParticle::DrawParticleFog(int i)
{
    GetLogger()->Trace("CParticle::DrawParticleFog() stub!\n");
    // TODO!
}

void Gfx::CParticle::DrawParticleRay(int i)
{
    GetLogger()->Trace("CParticle::DrawParticleRay() stub!\n");
    // TODO!
}

void Gfx::CParticle::DrawParticleSphere(int i)
{
    GetLogger()->Trace("CParticle::DrawParticleSphere() stub!\n");
    // TODO!
}

void Gfx::CParticle::DrawParticleCylinder(int i)
{
    GetLogger()->Trace("CParticle::DrawParticleCylinder() stub!\n");
    // TODO!
}

void Gfx::CParticle::DrawParticleWheel(int i)
{
    GetLogger()->Trace("CParticle::DrawParticleWheel() stub!\n");
    // TODO!
}

CObject* Gfx::CParticle::SearchObjectGun(Math::Vector old, Math::Vector pos, Gfx::ParticleType type, CObject *father)
{
    GetLogger()->Trace("CParticle::SearchObjectGun() stub!\n");
    // TODO!
    return nullptr;
}

CObject* Gfx::CParticle::SearchObjectRay(Math::Vector pos, Math::Vector goal, Gfx::ParticleType type, CObject *father)
{
    GetLogger()->Trace("CParticle::SearchObjectRay() stub!\n");
    // TODO!
    return nullptr;
}

void Gfx::CParticle::Play(Sound sound, Math::Vector pos, float amplitude)
{
    GetLogger()->Trace("CParticle::Play() stub!\n");
    // TODO!
}

bool Gfx::CParticle::TrackMove(int i, Math::Vector pos, float progress)
{
    GetLogger()->Trace("CParticle::TrackMove() stub!\n");
    // TODO!
    return true;
}

void Gfx::CParticle::TrackDraw(int i, Gfx::ParticleType type)
{
    GetLogger()->Trace("CParticle::TrackDraw() stub!\n");
    // TODO!
}
