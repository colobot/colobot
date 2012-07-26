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

// light.cpp

#include "graphics/common/light.h"

#include "common/iman.h"
#include "graphics/common/device.h"
#include "math/geometry.h"

#include <cmath>


void Gfx::LightProgression::Init(float value)
{
    starting = value;
    ending   = value;
    current  = value;
    progress = 0.0f;
    speed    = 100.0f;
}

void Gfx::LightProgression::Update(float rTime)
{
    if (speed < 100.0f)
    {
        if (progress < 1.0f)
        {
            progress += speed * rTime;
            if (progress > 1.0f)
                progress = 1.0f;
        }

        current = starting + progress * (ending - starting);
    }
    else
    {
        current = ending;
    }
}

void Gfx::LightProgression::SetTarget(float value)
{
    starting = current;
    ending   = value;
    progress = 0.0f;
}


Gfx::DynamicLight::DynamicLight()
{
    used = enabled = false;
}



Gfx::CLightManager::CLightManager(CInstanceManager* iMan, Gfx::CEngine* engine)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_LIGHT, this);

    m_device = NULL;
    m_engine = engine;

    m_time = 0.0f;
}

Gfx::CLightManager::~CLightManager()
{
    m_iMan->DeleteInstance(CLASS_LIGHT, this);

    m_iMan = NULL;
    m_device = NULL;
    m_engine = NULL;
}

void Gfx::CLightManager::SetDevice(Gfx::CDevice* device)
{
    m_device = device;

    m_dynLights = std::vector<Gfx::DynamicLight>(m_device->GetMaxLightCount(), Gfx::DynamicLight());
}

void Gfx::CLightManager::FlushLights()
{
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        m_dynLights[i].used = false;
        m_device->SetLightEnabled(i, false);
    }
}

/** Returns the index of light created or -1 if all lights are used. */
int Gfx::CLightManager::CreateLight()
{
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (m_dynLights[i].used) continue;

        m_dynLights[i] = Gfx::DynamicLight();

        m_dynLights[i].used    = true;
        m_dynLights[i].enabled = true;

        m_dynLights[i].includeType = Gfx::ENG_OBJTYPE_NULL;
        m_dynLights[i].excludeType = Gfx::ENG_OBJTYPE_NULL;

        m_dynLights[i].light.type      = Gfx::LIGHT_DIRECTIONAL;
        m_dynLights[i].light.diffuse   = Gfx::Color(0.5f, 0.5f, 0.5f);
        m_dynLights[i].light.position  = Math::Vector(-100.0f,  100.0f, -100.0f);
        m_dynLights[i].light.direction = Math::Vector( 1.0f, -1.0f,  1.0f);

        m_dynLights[i].intensity.Init(1.0f);  // maximum
        m_dynLights[i].colorRed.Init(0.5f);
        m_dynLights[i].colorGreen.Init(0.5f);
        m_dynLights[i].colorBlue.Init(0.5f);  // gray

        return i;
    }

    return -1;
}

bool Gfx::CLightManager::DeleteLight(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].used = false;
    m_device->SetLightEnabled(lightRank, false);

    return true;
}

// Specifies a light.

bool Gfx::CLightManager::SetLight(int lightRank, const Gfx::Light &light)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].light = light;

    m_dynLights[lightRank].colorRed.Init(m_dynLights[lightRank].light.diffuse.r);
    m_dynLights[lightRank].colorGreen.Init(m_dynLights[lightRank].light.diffuse.g);
    m_dynLights[lightRank].colorBlue.Init(m_dynLights[lightRank].light.diffuse.b);

    return true;
}

bool Gfx::CLightManager::GetLight(int lightRank, Gfx::Light &light)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    light = m_dynLights[lightRank].light;
    return true;
}

bool Gfx::CLightManager::SetLightEnabled(int lightRank, bool enabled)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].enabled = enabled;
    return true;
}

bool Gfx::CLightManager::SetLightIncludeType(int lightRank, Gfx::EngineObjectType type)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].includeType = type;
    return true;
}

bool Gfx::CLightManager::SetLightExcludeType(int lightRank, Gfx::EngineObjectType type)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].excludeType = type;
    return true;
}

bool Gfx::CLightManager::SetLightPos(int lightRank, const Math::Vector &pos)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].light.position = pos;
    return true;
}

Math::Vector Gfx::CLightManager::GetLightPos(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return Math::Vector(0.0f, 0.0f, 0.0f);

    return m_dynLights[lightRank].light.position;
}

bool Gfx::CLightManager::SetLightDir(int lightRank, const Math::Vector &dir)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].light.direction = dir;
    return true;
}

Math::Vector Gfx::CLightManager::GetLightDir(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return Math::Vector(0.0f, 0.0f, 0.0f);

    return m_dynLights[lightRank].light.direction;
}

bool Gfx::CLightManager::SetLightIntensitySpeed(int lightRank, float speed)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].intensity.speed = speed;
    return true;
}

bool Gfx::CLightManager::SetLightIntensity(int lightRank, float value)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].intensity.SetTarget(value);
    return true;
}

float Gfx::CLightManager::GetLightIntensity(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return 0.0f;

    return m_dynLights[lightRank].intensity.current;
}


bool Gfx::CLightManager::SetLightColorSpeed(int lightRank, float speed)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].colorRed.speed   = speed;
    m_dynLights[lightRank].colorGreen.speed = speed;
    m_dynLights[lightRank].colorBlue.speed  = speed;
    return true;
}

bool Gfx::CLightManager::SetLightColor(int lightRank, const Gfx::Color &color)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return false;

    m_dynLights[lightRank].colorRed.SetTarget(color.r);
    m_dynLights[lightRank].colorGreen.SetTarget(color.g);
    m_dynLights[lightRank].colorBlue.SetTarget(color.b);
    return true;
}

Gfx::Color Gfx::CLightManager::GetLightColor(int lightRank)
{
    if ( (lightRank < 0) || (lightRank >= static_cast<int>( m_dynLights.size() )) )
        return Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f);

    Gfx::Color color;
    color.r = m_dynLights[lightRank].colorRed.current;
    color.g = m_dynLights[lightRank].colorGreen.current;
    color.b = m_dynLights[lightRank].colorBlue.current;
    return color;
}

void Gfx::CLightManager::AdaptLightColor(const Gfx::Color &color, float factor)
{
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;

        Gfx::Color value;
        value.r = m_dynLights[i].colorRed.current;
        value.g = m_dynLights[i].colorGreen.current;
        value.b = m_dynLights[i].colorBlue.current;

        value.r += color.r * factor;
        value.g += color.g * factor;
        value.b += color.b * factor;

        m_dynLights[i].colorRed.Init(value.r);
        m_dynLights[i].colorGreen.Init(value.g);
        m_dynLights[i].colorBlue.Init(value.b);
    }

    UpdateLights();
}

void Gfx::CLightManager::UpdateProgression(float rTime)
{
    if (m_engine->GetPause())
        return;

    m_time += rTime;

    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;

        m_dynLights[i].intensity.Update(rTime);
        m_dynLights[i].colorRed.Update(rTime);
        m_dynLights[i].colorGreen.Update(rTime);
        m_dynLights[i].colorBlue.Update(rTime);

        if (m_dynLights[i].includeType == Gfx::ENG_OBJTYPE_QUARTZ)
        {
            m_dynLights[i].light.direction.x = sinf(1.0f * (m_time + i*Math::PI*0.5f));
            m_dynLights[i].light.direction.z = cosf(1.1f * (m_time + i*Math::PI*0.5f));
            m_dynLights[i].light.direction.y = -1.0f + 0.5f * cosf((m_time + i*Math::PI*0.5f)*2.7f);
        }

        if (m_dynLights[i].includeType == Gfx::ENG_OBJTYPE_METAL)
        {
            Math::Vector dir = m_engine->GetEyePt() - m_engine->GetLookatPt();
            float angle = Math::RotateAngle(dir.x, dir.z);
            angle += Math::PI * 0.5f * i;
            m_dynLights[i].light.direction.x = sinf(2.0f * angle);
            m_dynLights[i].light.direction.z = cosf(2.0f * angle);
        }
    }
}


void Gfx::CLightManager::UpdateLights()
{
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;

        bool enabled = m_dynLights[i].enabled;
        if (m_dynLights[i].intensity.current == 0.0f)
            enabled = false;

        if (enabled)
        {
            float value = m_dynLights[i].colorRed.current * m_dynLights[i].intensity.current;
            m_dynLights[i].light.diffuse.r = value;

            value = m_dynLights[i].colorGreen.current * m_dynLights[i].intensity.current;
            m_dynLights[i].light.diffuse.g = value;

            value = m_dynLights[i].colorBlue.current * m_dynLights[i].intensity.current;
            m_dynLights[i].light.diffuse.b = value;

            m_device->SetLight(i, m_dynLights[i].light);
            m_device->SetLightEnabled(i, enabled);
        }
        else
        {
            m_dynLights[i].light.diffuse.r = 0.0f;
            m_dynLights[i].light.diffuse.g = 0.0f;
            m_dynLights[i].light.diffuse.b = 0.0f;

            m_device->SetLightEnabled(i, enabled);
        }
    }
}

void Gfx::CLightManager::UpdateLightsEnableState(Gfx::EngineObjectType type)
{
    for (int i = 0; i < static_cast<int>( m_dynLights.size() ); i++)
    {
        if (! m_dynLights[i].used)
            continue;
        if (! m_dynLights[i].enabled)
            continue;
        if (m_dynLights[i].intensity.current == 0.0f)
            continue;

        if (m_dynLights[i].includeType != Gfx::ENG_OBJTYPE_NULL)
        {
            bool enabled = (m_dynLights[i].includeType == type);
            m_device->SetLightEnabled(i, enabled);
        }

        if (m_dynLights[i].excludeType != Gfx::ENG_OBJTYPE_NULL)
        {
            bool enabled = (m_dynLights[i].excludeType != type);
            m_device->SetLightEnabled(i, enabled);
        }
    }
}
