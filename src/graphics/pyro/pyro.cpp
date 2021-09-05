/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "graphics/pyro/pyro.h"

#include "app/app.h"

#include "common/logger.h"

#include "graphics/engine/lightman.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/motion/motionhuman.h"

#include "sound/sound.h"


// Graphics module namespace
namespace Gfx
{


CPyro::CPyro(CObject *obj)
{
    m_object      = obj;
    m_engine      = CEngine::GetInstancePointer();
    m_main        = CRobotMain::GetInstancePointer();
    m_terrain     = m_main->GetTerrain();
    m_camera      = m_main->GetCamera();
    m_particle    = m_engine->GetParticle();
    m_lightMan    = m_engine->GetLightManager();
    m_sound       = CApplication::GetInstancePointer()->GetSound();
}

CPyro::~CPyro()
{
}

void CPyro::DeleteObject()
{
    if ( m_lightRank != -1 )
    {
        m_lightMan->DeleteLight(m_lightRank);
        m_lightRank = -1;
    }
}
bool CPyro::Create()
{
    ObjectType oType = m_object->GetType();
    int objRank = m_object->GetObjectRank(0);
    if (objRank == -1) return false;

    Math::Vector min, max;
    m_engine->GetObjectBBox(objRank, min, max);
    Math::Vector pos = m_object->GetPosition();

    for (const auto& crashSphere : m_object->GetAllCrashSpheres())
    {
        m_crashSpheres.push_back(crashSphere.sphere);
    }

    // Calculates the size of the effect.
    if ( oType == OBJECT_ANT    ||
         oType == OBJECT_BEE    ||
         oType == OBJECT_WORM   ||
         oType == OBJECT_SPIDER )
    {
        m_size = 40.0f;
    }
    else
    {
        m_size = Math::Distance(min, max)*2.0f;
        if ( m_size <  4.0f )  m_size =  4.0f;
        if ( m_size > 80.0f )  m_size = 80.0f;
    }
    if ( oType == OBJECT_TNT  ||
         oType == OBJECT_BOMB )
    {
        m_size *= 2.0f;
    }

    m_pos = pos+(min+max)/2.0f;
    m_progress = 0.0f;
    m_speed = 1.0f/20.0f;    m_time = 0.0f;
    m_lastParticle = 0.0f;
    m_lastParticleSmoke = 0.0f;
    m_lightRank = -1;

    if ( oType == OBJECT_TEEN28 ||
         oType == OBJECT_TEEN31 )
    {
        m_pos.y = pos.y+1.0f;
    }

    // Seeking the position of the battery.

    CObject* power = nullptr;
    if (m_object->Implements(ObjectInterfaceType::Powered))
        power = dynamic_cast<CPoweredObject&>(*m_object).GetPower();

    if (power == nullptr)
    {
        m_power = false;
    }
    else
    {
        m_power = true;
        pos = power->GetPosition();
        pos.y += 1.0f;
        Math::Matrix* mat = m_object->GetWorldMatrix(0);
        m_posPower = Math::Transform(*mat, pos);
    }

    if ( oType == OBJECT_POWER   ||
         oType == OBJECT_ATOMIC  ||
         oType == OBJECT_URANIUM ||
         oType == OBJECT_TNT     ||
         oType == OBJECT_BOMB    )
    {
        m_power = true;
        m_posPower = m_pos;
        m_posPower.y += 1.0f;
        m_pos = m_posPower;
    }
    if ( oType == OBJECT_STATION )
    {
        m_power = true;
        Math::Matrix* mat = m_object->GetWorldMatrix(0);
        m_posPower = Math::Transform(*mat, Math::Vector(-15.0f, 7.0f, 0.0f));
        m_pos = m_posPower;
    }
    if ( oType == OBJECT_ENERGY )
    {
        m_power = true;
        Math::Matrix* mat = m_object->GetWorldMatrix(0);
        m_posPower = Math::Transform(*mat, Math::Vector(-7.0f, 6.0f, 0.0f));
        m_pos = m_posPower;
    }
    if ( oType == OBJECT_NUCLEAR )
    {
        m_power = true;
        m_posPower = m_pos;
    }
    if ( oType == OBJECT_PARA )
    {
        m_power = true;
        m_posPower = m_pos;
    }

    AfterCreate();

    return true;
}

void CPyro::AfterCreate()
{
}

bool CPyro::EventProcess(const Event &event)
{
    if (event.type != EVENT_FRAME) return true;
    if (m_engine->GetPause()) return true;

    m_time += event.rTime;
    m_progress += event.rTime*m_speed;

    if (m_soundChannel != -1 && m_object != nullptr)
    {
        Math::Vector pos = m_object->GetPosition();
        m_sound->Position(m_soundChannel, pos);

        if (m_lightRank != -1)
        {
            pos.y += m_lightHeight;
            m_lightMan->SetLightPos(m_lightRank, pos);
        }
    }

    UpdateEffect();

    if (m_lightRank != -1)
        LightOperFrame();

    return true;
}

void CPyro::UpdateEffect()
{
}

Error CPyro::IsEnded()
{
    // End of the pyrotechnic effect?
    if ( m_progress < 1.0f )  return ERR_CONTINUE;

    return ERR_STOP;
}

void CPyro::AfterEnd()
{
}

void CPyro::CutObjectLink(CObject* obj)
{
    if (m_object == obj)
        m_object = nullptr;
}

void CPyro::CreateLight(Math::Vector pos, float height)
{
    if (!m_engine->GetLightMode()) return;

    m_lightHeight = height;

    Gfx::Light light;
    light.type        = LIGHT_SPOT;
    light.ambient     = Gfx::Color(0.0f, 0.0f, 0.0f);
    light.position    = Math::Vector(pos.x, pos.y+height, pos.z);
    light.direction   = Math::Vector(0.0f, -1.0f, 0.0f);  // against the bottom
    light.spotIntensity = 1.0f;
    light.attenuation0 = 1.0f;
    light.attenuation1 = 0.0f;
    light.attenuation2 = 0.0f;
    light.spotAngle = Math::PI/4.0f;

    m_lightRank = m_lightMan->CreateLight();

    m_lightMan->SetLight(m_lightRank, light);
    m_lightMan->SetLightIntensity(m_lightRank, 0.0f);

    // Only illuminates the objects on the ground.
    m_lightMan->SetLightIncludeType(m_lightRank, ENG_OBJTYPE_TERRAIN);
}

void CPyro::DeleteObject(bool primary, bool secondary)
{
    if (m_object == nullptr) return;

    ObjectType type = m_object->GetType();
    if ( secondary              &&
         type != OBJECT_FACTORY &&
         type != OBJECT_NUCLEAR &&
         type != OBJECT_ENERGY )
    {
        if (m_object->Implements(ObjectInterfaceType::Powered))
        {
            CPoweredObject& poweredObject = dynamic_cast<CPoweredObject&>(*m_object);
            CObject* sub = poweredObject.GetPower();
            if (sub != nullptr)
            {
                CObjectManager::GetInstancePointer()->DeleteObject(sub);
                poweredObject.SetPower(nullptr);
            }
        }

        if (m_object->Implements(ObjectInterfaceType::Carrier))
        {
            CCarrierObject& carrierObject = dynamic_cast<CCarrierObject&>(*m_object);
            CObject* sub = carrierObject.GetCargo();
            if (sub != nullptr)
            {
                CObjectManager::GetInstancePointer()->DeleteObject(sub);
                carrierObject.SetCargo(nullptr);
            }
        }
    }

    if (primary)
    {
        if (m_object->Implements(ObjectInterfaceType::Transportable))
        {
            // TODO: this should be handled in the object's destructor
            CObject* transporter = dynamic_cast<CTransportableObject&>(*m_object).GetTransporter();
            if (transporter != nullptr)
            {
                if (m_object->Implements(ObjectInterfaceType::Powered))
                {
                    CObject* sub = dynamic_cast<CPoweredObject&>(*m_object).GetPower();
                    if (sub != nullptr)  // is there a battery?
                        sub->SetScaleY(1.0f - m_progress);  // complete flattening
                }
            }

            if (transporter->Implements(ObjectInterfaceType::Carrier))
            {
                CCarrierObject& carrier = dynamic_cast<CCarrierObject&>(*transporter);
                if (carrier.GetCargo() == m_object)
                    carrier.SetCargo(nullptr);
            }
        }

        CObjectManager::GetInstancePointer()->DeleteObject(m_object);
        m_object = nullptr;
    }
}

void CPyro::CreateTriangle(CObject* obj, ObjectType oType, int part, float maxHParticleSpeed, float maxVParticleSpeed, float minParticleMass, float maxParticleMass, float overridePercent)
{
    int objRank = obj->GetObjectRank(part);
    if (objRank == -1) return;


    int total = m_engine->GetObjectTotalTriangles(objRank);

    float percent = 0.10f;
    if (total < 50) percent = 0.25f;
    if (total < 20) percent = 0.50f;

    if ( overridePercent >= 0 )
    {
        percent = overridePercent;
    }

    if (oType == OBJECT_POWER    ||
        oType == OBJECT_ATOMIC   ||
        oType == OBJECT_URANIUM  ||
        oType == OBJECT_TNT      ||
        oType == OBJECT_BOMB     ||
        oType == OBJECT_TEEN28)
    {
        percent = 0.75f;
    }
    else if (oType == OBJECT_MOBILEtg)
    {
        percent = 0.50f;
    }

    std::vector<EngineTriangle> buffer;
    total = m_engine->GetPartialTriangles(objRank, percent, 100, buffer);

    for (int i = 0; i < total; i++)
    {
        Math::Vector p1, p2, p3;

        p1.x = buffer[i].triangle[0].coord.x;
        p1.y = buffer[i].triangle[0].coord.y;
        p1.z = buffer[i].triangle[0].coord.z;
        p2.x = buffer[i].triangle[1].coord.x;
        p2.y = buffer[i].triangle[1].coord.y;
        p2.z = buffer[i].triangle[1].coord.z;
        p3.x = buffer[i].triangle[2].coord.x;
        p3.y = buffer[i].triangle[2].coord.y;
        p3.z = buffer[i].triangle[2].coord.z;

        float h;

        h = Math::Distance(p1, p2);
        if ( h > 5.0f )
        {
            p2.x = p1.x+((p2.x-p1.x)*5.0f/h);
            p2.y = p1.y+((p2.y-p1.y)*5.0f/h);
            p2.z = p1.z+((p2.z-p1.z)*5.0f/h);
        }

        h = Math::Distance(p2, p3);
        if ( h > 5.0f )
        {
            p3.x = p2.x+((p3.x-p2.x)*5.0f/h);
            p3.y = p2.y+((p3.y-p2.y)*5.0f/h);
            p3.z = p2.z+((p3.z-p2.z)*5.0f/h);
        }

        h = Math::Distance(p3, p1);
        if ( h > 5.0f )
        {
            p1.x = p3.x+((p1.x-p3.x)*5.0f/h);
            p1.y = p3.y+((p1.y-p3.y)*5.0f/h);
            p1.z = p3.z+((p1.z-p3.z)*5.0f/h);
        }

        buffer[i].triangle[0].coord.x = p1.x;
        buffer[i].triangle[0].coord.y = p1.y;
        buffer[i].triangle[0].coord.z = p1.z;
        buffer[i].triangle[1].coord.x = p2.x;
        buffer[i].triangle[1].coord.y = p2.y;
        buffer[i].triangle[1].coord.z = p2.z;
        buffer[i].triangle[2].coord.x = p3.x;
        buffer[i].triangle[2].coord.y = p3.y;
        buffer[i].triangle[2].coord.z = p3.z;

        Math::Vector offset;
        offset.x = (buffer[i].triangle[0].coord.x+buffer[i].triangle[1].coord.x+buffer[i].triangle[2].coord.x)/3.0f;
        offset.y = (buffer[i].triangle[0].coord.y+buffer[i].triangle[1].coord.y+buffer[i].triangle[2].coord.y)/3.0f;
        offset.z = (buffer[i].triangle[0].coord.z+buffer[i].triangle[1].coord.z+buffer[i].triangle[2].coord.z)/3.0f;

        buffer[i].triangle[0].coord.x -= offset.x;
        buffer[i].triangle[1].coord.x -= offset.x;
        buffer[i].triangle[2].coord.x -= offset.x;

        buffer[i].triangle[0].coord.y -= offset.y;
        buffer[i].triangle[1].coord.y -= offset.y;
        buffer[i].triangle[2].coord.y -= offset.y;

        buffer[i].triangle[0].coord.z -= offset.z;
        buffer[i].triangle[1].coord.z -= offset.z;
        buffer[i].triangle[2].coord.z -= offset.z;

        Math::Vector speed;
        float mass;

        Math::Matrix* mat = obj->GetWorldMatrix(part);
        Math::Vector pos = Math::Transform(*mat, offset);
        speed.x = (Math::Rand()-0.5f)*2.0f*maxHParticleSpeed;
        speed.z = (Math::Rand()-0.5f)*2.0f*maxHParticleSpeed;
        speed.y = Math::Rand()*maxVParticleSpeed;
        mass = Math::Rand()*(maxParticleMass-minParticleMass) + minParticleMass;
        if ( oType == OBJECT_STONE   )  speed *= 0.5f;
        if ( oType == OBJECT_URANIUM )  speed *= 0.4f;
        float duration = Math::Rand()*3.0f+3.0f;
        m_particle->CreateFrag(pos, speed, &buffer[i], PARTIFRAG,
                               duration, mass, 0.5f);
    }
}


void CPyro::LightOperFlush()
{
    m_lightOper.clear();
}

void CPyro::LightOperAdd(float progress, float intensity, float r, float g, float b)
{
    PyroLightOper lightOper;

    lightOper.progress  = progress;
    lightOper.intensity = intensity;
    lightOper.color.r   = r;
    lightOper.color.g   = g;
    lightOper.color.b   = b;

    m_lightOper.push_back(lightOper);
}

void CPyro::LightOperFrame()
{
    for (std::size_t i = 1; i < m_lightOper.size(); i++)
    {
        if ( m_progress < m_lightOper[i].progress )
        {
            float progress = (m_progress-m_lightOper[i-1].progress) / (m_lightOper[i].progress-m_lightOper[i-1].progress);

            float intensity = m_lightOper[i-1].intensity + (m_lightOper[i].intensity-m_lightOper[i-1].intensity)*progress;
            Gfx::Color color;
            color.r = m_lightOper[i-1].color.r + (m_lightOper[i].color.r-m_lightOper[i-1].color.r)*progress;
            color.g = m_lightOper[i-1].color.g + (m_lightOper[i].color.g-m_lightOper[i-1].color.g)*progress;
            color.b = m_lightOper[i-1].color.b + (m_lightOper[i].color.b-m_lightOper[i-1].color.b)*progress;

            m_lightMan->SetLightIntensity(m_lightRank, intensity);
            m_lightMan->SetLightColor(m_lightRank, color);
            break;
        }
    }
}

} // namespace Gfx
